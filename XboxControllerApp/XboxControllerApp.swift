import SwiftUI
import SystemExtensions
import GameController
import Combine // ESSENCIAL para corrigir o erro do ObservableObject

@main
struct XboxControllerApp: App {
    // O gerenciador de estado global do app
    @StateObject private var driverManager = DriverManager()
    
    var body: some Scene {
        WindowGroup {
            // Chama a ContentView definida mais abaixo neste mesmo arquivo
            ContentView()
                .environmentObject(driverManager)
                .frame(minWidth: 800, minHeight: 700)
        }
        .commands {
            CommandGroup(replacing: .appInfo) {
                Button("About Xbox Controller Driver") {
                    NSApplication.shared.orderFrontStandardAboutPanel(
                        options: [
                            NSApplication.AboutPanelOptionKey.credits: NSAttributedString(
                                string: NSLocalizedString("Xbox 360 Wireless Receiver Driver\nDriverKit Implementation", comment: "Credits"),
                                attributes: [NSAttributedString.Key.font: NSFont.systemFont(ofSize: 11)]
                            ),
                            NSApplication.AboutPanelOptionKey.applicationVersion: "1.0.0",
                        ]
                    )
                }
            }
        }
    }
}

// MARK: - Driver Manager (Lógica)

// final class ajuda o compilador a otimizar e evitar erros de herança
final class DriverManager: NSObject, ObservableObject {
    @Published var isDriverInstalled = false
    @Published var isDriverActive = false
    @Published var statusMessage = String(localized: "Aguardando ação do usuário...")
    @Published var connectedControllers: [ControllerInfo] = []
    
    private var gcControllersMap: [UUID: GCController] = [:]
    private let extensionIdentifier = "com.jonasw8.XboxControllerApp.driver"
    
    override init() {
        super.init()
        setupControllerObservers()
        //Codigo para teste de interface
        /*let debugCtrl = ControllerInfo(
                name: "Controle de Teste",
                type: .xbox360,
                isConnected: true,
                batteryLevel: 85,
                vendorID: "000", productID: "000", serialNumber: nil
            )
            self.connectedControllers.append(debugCtrl)*/
    }
    
    // MARK: - Game Controller Logic
    
    private func setupControllerObservers() {
        NotificationCenter.default.addObserver(
            self, selector: #selector(controllerDidConnect),
            name: .GCControllerDidConnect, object: nil
        )
        
        NotificationCenter.default.addObserver(
            self, selector: #selector(controllerDidDisconnect),
            name: .GCControllerDidDisconnect, object: nil
        )
        
        for controller in GCController.controllers() {
            registerController(controller)
        }
    }
    
    @objc private func controllerDidConnect(_ notification: Notification) {
        guard let controller = notification.object as? GCController else { return }
        registerController(controller)
    }
    
    @objc private func controllerDidDisconnect(_ notification: Notification) {
        guard let controller = notification.object as? GCController else { return }
        unregisterController(controller)
    }
    
    private func registerController(_ controller: GCController) {
        let newInfo = ControllerInfo(
            name: controller.vendorName ?? String(localized: "Xbox Controller"),
            type: identifyControllerType(controller),
            isConnected: true,
            batteryLevel: Int((controller.battery?.batteryLevel ?? 0) * 100),
            vendorID: "045E",
            productID: "0719",
            serialNumber: nil
        )
        
        gcControllersMap[newInfo.id] = controller
        
        DispatchQueue.main.async {
            self.connectedControllers.append(newInfo)
            self.statusMessage = String(localized: "Controle conectado: \(newInfo.name)")
        }
        
        if let gamepad = controller.extendedGamepad {
            gamepad.valueChangedHandler = { [weak self] (gamepad: GCExtendedGamepad, element: GCControllerElement) in
                self?.updateControllerState(uuid: newInfo.id, gamepad: gamepad)
            }
        }
    }
    
    private func unregisterController(_ controller: GCController) {
        if let (uuid, _) = gcControllersMap.first(where: { $0.value == controller }) {
            gcControllersMap.removeValue(forKey: uuid)
            DispatchQueue.main.async {
                self.connectedControllers.removeAll { $0.id == uuid }
                self.statusMessage = String(localized: "Controle desconectado")
            }
        }
    }
    
    private func updateControllerState(uuid: UUID, gamepad: GCExtendedGamepad) {
        DispatchQueue.main.async {
            guard let index = self.connectedControllers.firstIndex(where: { $0.id == uuid }) else { return }
            
            var info = self.connectedControllers[index]
            
            // Atualiza os estados usando as structs definidas no Models.swift
            info.buttonState.a = gamepad.buttonA.isPressed
            info.buttonState.b = gamepad.buttonB.isPressed
            info.buttonState.x = gamepad.buttonX.isPressed
            info.buttonState.y = gamepad.buttonY.isPressed
            info.buttonState.lb = gamepad.leftShoulder.isPressed
            info.buttonState.rb = gamepad.rightShoulder.isPressed
            info.buttonState.ls = gamepad.leftThumbstickButton?.isPressed ?? false
            info.buttonState.rs = gamepad.rightThumbstickButton?.isPressed ?? false
            info.buttonState.start = gamepad.buttonMenu.isPressed
            info.buttonState.back = gamepad.buttonOptions?.isPressed ?? false
            info.buttonState.guide = gamepad.buttonHome?.isPressed ?? false
            info.buttonState.dpadUp = gamepad.dpad.up.isPressed
            info.buttonState.dpadDown = gamepad.dpad.down.isPressed
            info.buttonState.dpadLeft = gamepad.dpad.left.isPressed
            info.buttonState.dpadRight = gamepad.dpad.right.isPressed
            
            info.analogState.leftStickX = Double(gamepad.leftThumbstick.xAxis.value)
            info.analogState.leftStickY = Double(gamepad.leftThumbstick.yAxis.value)
            info.analogState.rightStickX = Double(gamepad.rightThumbstick.xAxis.value)
            info.analogState.rightStickY = Double(gamepad.rightThumbstick.yAxis.value)
            info.analogState.leftTrigger = Double(gamepad.leftTrigger.value)
            info.analogState.rightTrigger = Double(gamepad.rightTrigger.value)
            
            self.connectedControllers[index] = info
        }
    }
    
    private func identifyControllerType(_ controller: GCController) -> ControllerInfo.ControllerType {
        let name = controller.vendorName?.lowercased() ?? ""
        if name.contains("360") { return .xbox360 }
        if name.contains("one") { return .xboxOne }
        if name.contains("series") { return .xboxSeriesX }
        return .xbox360
    }
    
    func triggerRumble(uuid: UUID, left: Float, right: Float) {
        print("Driver App: Comando de vibração L:\(left) R:\(right) enviado para \(uuid)")
    }
    
    // MARK: - Driver Installation Logic
    
    func installDriver() {
        statusMessage = String(localized: "Solicitando instalação do driver...")
        let request = OSSystemExtensionRequest.activationRequest(
            forExtensionWithIdentifier: extensionIdentifier,
            queue: .main
        )
        request.delegate = self
        OSSystemExtensionManager.shared.submitRequest(request)
    }
    
    func uninstallDriver() {
        statusMessage = String(localized: "Solicitando remoção do driver...")
        let request = OSSystemExtensionRequest.deactivationRequest(
            forExtensionWithIdentifier: extensionIdentifier,
            queue: .main
        )
        request.delegate = self
        OSSystemExtensionManager.shared.submitRequest(request)
    }
}

// MARK: - System Extension Delegate

extension DriverManager: OSSystemExtensionRequestDelegate {
    func request(_ request: OSSystemExtensionRequest,
                 actionForReplacingExtension existing: OSSystemExtensionProperties,
                 withExtension ext: OSSystemExtensionProperties) -> OSSystemExtensionRequest.ReplacementAction {
        statusMessage = String(localized: "Atualizando driver existente...")
        return .replace
    }
    
    func requestNeedsUserApproval(_ request: OSSystemExtensionRequest) {
        DispatchQueue.main.async {
            self.statusMessage = String(localized: "⚠️ Aprovação necessária! Vá em Ajustes do Sistema > Privacidade.")
        }
    }
    
    func request(_ request: OSSystemExtensionRequest,
                 didFinishWithResult result: OSSystemExtensionRequest.Result) {
        DispatchQueue.main.async {
            switch result {
            case .completed:
                self.isDriverInstalled = true
                self.isDriverActive = true
                self.statusMessage = String(localized: "✅ Driver instalado e ativo!")
            case .willCompleteAfterReboot:
                self.statusMessage = String(localized: "⚠️ Reinicie o Mac para finalizar.")
            @unknown default:
                self.statusMessage = String(localized: "Resultado desconhecido.")
            }
        }
    }
    
    func request(_ request: OSSystemExtensionRequest,
                 didFailWithError error: Error) {
        DispatchQueue.main.async {
            self.statusMessage = String(localized: "❌ Erro: \(error.localizedDescription)")
        }
    }
}

// MARK: - Main View (Dashboard)

struct ContentView: View {
    @EnvironmentObject var driverManager: DriverManager
    
    var body: some View {
        VStack(spacing: 20) {
            HeaderView()
            
            StatusSection(
                isInstalled: driverManager.isDriverInstalled,
                isActive: driverManager.isDriverActive,
                statusMessage: driverManager.statusMessage
            )
            
            DriverControlSection(
                isInstalled: driverManager.isDriverInstalled,
                onInstall: { driverManager.installDriver() },
                onUninstall: { driverManager.uninstallDriver() }
            )
            
            Divider()
            
            ControllersSection(controllers: driverManager.connectedControllers)
            
            // Área dinâmica: Se houver controle, mostra o Visualizador. Senão, mostra aviso.
            if let selectedController = driverManager.connectedControllers.first {
                ScrollView {
                    // Este componente vem do arquivo ControllerVisualizer.swift
                    ControllerVisualizer(controller: selectedController)
                }
            } else {
                Spacer()
                VStack(spacing: 10) {
                    Image(systemName: "gamecontroller.fill")
                        .font(.largeTitle)
                        .foregroundColor(.gray)
                    Text("Conecte o Receiver USB e ligue o controle")
                        .foregroundColor(.secondary)
                }
                Spacer()
            }
            
            FooterView()
        }
        .padding()
    }
}

// MARK: - Componentes Visuais do Dashboard

struct HeaderView: View {
    var body: some View {
        HStack {
            Image(systemName: "gamecontroller.fill")
                .font(.largeTitle)
                .foregroundColor(.blue)
            VStack(alignment: .leading) {
                Text("Xbox Wireless Driver").font(.title2).bold()
                Text("Gerenciador de Extensão").font(.caption).foregroundColor(.secondary)
            }
            Spacer()
        }
    }
}

struct StatusSection: View {
    let isInstalled: Bool
    let isActive: Bool
    let statusMessage: String
    
    var body: some View {
        HStack {
            Circle()
                .fill(isActive ? Color.green : (isInstalled ? Color.orange : Color.red))
                .frame(width: 10, height: 10)
            Text(statusMessage)
                .font(.body)
            Spacer()
        }
        .padding(10)
        .background(Color.gray.opacity(0.1))
        .cornerRadius(8)
    }
}

struct DriverControlSection: View {
    let isInstalled: Bool
    let onInstall: () -> Void
    let onUninstall: () -> Void
    
    var body: some View {
        HStack {
            if !isInstalled {
                Button(action: onInstall) {
                    HStack {
                        Image(systemName: "arrow.down.circle")
                        Text("Instalar Driver")
                    }
                    .frame(maxWidth: .infinity)
                    .padding(8)
                }
                .background(Color.blue)
                .foregroundColor(.white)
                .cornerRadius(8)
            } else {
                Button(action: onUninstall) {
                    HStack {
                        Image(systemName: "trash")
                        Text("Desinstalar")
                    }
                    .frame(maxWidth: .infinity)
                    .padding(8)
                }
                .background(Color.red.opacity(0.1))
                .foregroundColor(.red)
                .cornerRadius(8)
            }
        }
    }
}

struct ControllersSection: View {
    let controllers: [ControllerInfo]
    
    var body: some View {
        VStack(alignment: .leading) {
            Text("Controles Detectados").font(.headline)
            if controllers.isEmpty {
                Text("Nenhum controle detectado.").foregroundColor(.gray).padding(.vertical, 5)
            } else {
                ForEach(controllers) { ctrl in
                    HStack {
                        Image(systemName: "gamecontroller")
                        VStack(alignment: .leading) {
                            Text(ctrl.name)
                            Text(ctrl.type.localizedName)
                                .font(.caption)
                                .foregroundColor(.secondary)
                        }
                        Spacer()
                        if let batt = ctrl.batteryLevel {
                            Text("\(batt)%")
                        }
                    }
                    .padding(5)
                    .background(Color.gray.opacity(0.1))
                    .cornerRadius(5)
                }
            }
        }
    }
}

struct FooterView: View {
    var body: some View {
        Text("v1.0.0 - DriverKit Ad-Hoc Build")
            .font(.caption2)
            .foregroundColor(.secondary)
            .padding(.top, 10)
    }
}
