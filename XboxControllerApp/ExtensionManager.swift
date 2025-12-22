//
//  ExtensionManager.swift
//  XboxControllerApp
//
//  Created by jonas on 21/12/25.
//

import Foundation
import SwiftUI
import Combine
import SystemExtensions
import os.log

class ExtensionManager: NSObject, ObservableObject, OSSystemExtensionRequestDelegate {
    @Published var logText: String = "Pronto para instalar.\n"
    @Published var status: String = "Desconhecido"
    
    // O Bundle ID do driver (conforme seu Info.plist)
    let driverID = "com.jonasw8.XboxControllerApp.driver"
    
    func installDriver() {
        log("Iniciando solicitação de ativação para: \(driverID)")
        let request = OSSystemExtensionRequest.activationRequest(
            forExtensionWithIdentifier: driverID,
            queue: .main
        )
        request.delegate = self
        OSSystemExtensionManager.shared.submitRequest(request)
    }
    
    func uninstallDriver() {
        log("Solicitando desinstalação...")
        let request = OSSystemExtensionRequest.deactivationRequest(
            forExtensionWithIdentifier: driverID,
            queue: .main
        )
        request.delegate = self
        OSSystemExtensionManager.shared.submitRequest(request)
    }
    
    // MARK: - Delegate Methods
    
    func request(_ request: OSSystemExtensionRequest, actionForReplacingExtension existing: OSSystemExtensionProperties, withExtension ext: OSSystemExtensionProperties) -> OSSystemExtensionRequest.ReplacementAction {
        log("Substituindo versão antiga do driver pela nova.")
        return .replace
    }
    
    func requestNeedsUserApproval(_ request: OSSystemExtensionRequest) {
        log("⚠️ APROVAÇÃO NECESSÁRIA! Vá em Ajustes do Sistema > Privacidade e Segurança > Permitir.")
        status = "Aguardando Aprovação"
    }
    
    func request(_ request: OSSystemExtensionRequest, didFinishWithResult result: OSSystemExtensionRequest.Result) {
        switch result {
        case .completed:
            log("✅ Sucesso! O driver foi carregado.")
            status = "Ativo"
        case .willCompleteAfterReboot:
            log("⚠️ Sucesso, mas requer reinicialização do Mac.")
            status = "Requer Reboot"
        @unknown default:
            log("Finalizado com resultado desconhecido.")
        }
    }
    
    func request(_ request: OSSystemExtensionRequest, didFailWithError error: Error) {
        log("❌ ERRO: \(error.localizedDescription)")
        status = "Falhou"
    }
    
    private func log(_ message: String) {
        DispatchQueue.main.async {
            print(message)
            self.logText += "\n\(message)"
        }
    }
}
