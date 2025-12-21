import Foundation

// MARK: - Modelos de Dados

struct ControllerInfo: Identifiable {
    let id = UUID()
    let name: String
    let type: ControllerType
    let isConnected: Bool
    let batteryLevel: Int?
    let vendorID: String
    let productID: String
    let serialNumber: String?
    
    // Aqui nós inicializamos as structs definidas logo abaixo
    var buttonState = ButtonState()
    var analogState = AnalogState()
    
    // --- Definição das Structs internas (Isso corrige o erro!) ---
    
    struct ButtonState {
        var a = false, b = false, x = false, y = false
        var lb = false, rb = false
        var back = false, start = false, guide = false
        var ls = false, rs = false
        var dpadUp = false, dpadDown = false, dpadLeft = false, dpadRight = false
    }
    
    struct AnalogState {
        var leftStickX: Double = 0.0
        var leftStickY: Double = 0.0
        var rightStickX: Double = 0.0
        var rightStickY: Double = 0.0
        var leftTrigger: Double = 0.0
        var rightTrigger: Double = 0.0
    }
    
    enum ControllerType: String {
        case xbox360 = "Xbox 360"
        case xboxOne = "Xbox One"
        case xboxSeriesX = "Xbox Series X|S"
        case unknown = "Desconhecido"
        
        // Propriedade para tradução
        var localizedName: String {
            switch self {
            case .xbox360: return String(localized: "Xbox 360")
            case .xboxOne: return String(localized: "Xbox One")
            case .xboxSeriesX: return String(localized: "Xbox Series X|S")
            case .unknown: return String(localized: "Desconhecido")
            }
        }
    }
}
