//
//  ControllerVisualizer.swift
//  XboxControllerApp
//
//  Visualização em tempo real do estado do controle
//

import SwiftUI

struct ControllerVisualizer: View {
    // Recebe as informações do modelo definido em Models.swift
    let controller: ControllerInfo
    
    @State private var rumbleLeft: Double = 0.0
    @State private var rumbleRight: Double = 0.0
    
    var body: some View {
        VStack(spacing: 20) {
            Text(controller.name)
                .font(.title2)
                .fontWeight(.bold)
            
            HStack(spacing: 40) {
                // Lado esquerdo
                VStack(spacing: 20) {
                    DPadView(
                        up: controller.buttonState.dpadUp,
                        down: controller.buttonState.dpadDown,
                        left: controller.buttonState.dpadLeft,
                        right: controller.buttonState.dpadRight
                    )
                    
                    AnalogStickView(
                        x: controller.analogState.leftStickX,
                        y: controller.analogState.leftStickY,
                        pressed: controller.buttonState.ls,
                        label: "L"
                    )
                    
                    TriggerView(
                        value: controller.analogState.leftTrigger,
                        label: "LT",
                        isLeft: true
                    )
                }
                
                // Centro
                VStack(spacing: 20) {
                    HStack(spacing: 30) {
                        Image(systemName: "line.3.horizontal")
                            .foregroundColor(controller.buttonState.back ? .blue : .gray)
                        
                        Image(systemName: "xbox.logo")
                            .foregroundColor(controller.buttonState.guide ? .green : .gray)
                            .font(.title)
                        
                        Image(systemName: "play.fill")
                            .foregroundColor(controller.buttonState.start ? .blue : .gray)
                    }
                    
                    HStack(spacing: 80) {
                        BumperView(pressed: controller.buttonState.lb, label: "LB")
                        BumperView(pressed: controller.buttonState.rb, label: "RB")
                    }
                }
                
                // Lado direito
                VStack(spacing: 20) {
                    ABXYButtonsView(
                        a: controller.buttonState.a,
                        b: controller.buttonState.b,
                        x: controller.buttonState.x,
                        y: controller.buttonState.y
                    )
                    
                    AnalogStickView(
                        x: controller.analogState.rightStickX,
                        y: controller.analogState.rightStickY,
                        pressed: controller.buttonState.rs,
                        label: "R"
                    )
                    
                    TriggerView(
                        value: controller.analogState.rightTrigger,
                        label: "RT",
                        isLeft: false
                    )
                }
            }
            .padding()
            .background(Color.gray.opacity(0.1))
            .cornerRadius(15)
            
            VibrationControls(
                leftMotor: $rumbleLeft,
                rightMotor: $rumbleRight,
                onTest: {
                    // Simulação de chamada de vibração
                    print("Vibrar: \(rumbleLeft), \(rumbleRight)")
                }
            )
        }
        .padding()
    }
}

// MARK: - Components (Estes precisam estar aqui para o código funcionar)

struct DPadView: View {
    let up, down, left, right: Bool
    
    var body: some View {
        VStack(spacing: 2) {
            DPadButton(pressed: up, icon: "arrow.up")
            HStack(spacing: 2) {
                DPadButton(pressed: left, icon: "arrow.left")
                Rectangle().fill(Color.clear).frame(width: 30, height: 30)
                DPadButton(pressed: right, icon: "arrow.right")
            }
            DPadButton(pressed: down, icon: "arrow.down")
        }
    }
}

struct DPadButton: View {
    let pressed: Bool
    let icon: String
    
    var body: some View {
        ZStack {
            RoundedRectangle(cornerRadius: 4)
                .fill(pressed ? Color.blue : Color.gray.opacity(0.5))
                .frame(width: 30, height: 30)
            Image(systemName: icon)
                .font(.system(size: 14, weight: .bold))
                .foregroundColor(.white)
        }
    }
}

struct AnalogStickView: View {
    let x, y: Double
    let pressed: Bool
    let label: String
    
    var body: some View {
        VStack(spacing: 5) {
            ZStack {
                Circle().fill(Color.gray.opacity(0.3)).frame(width: 80, height: 80)
                Circle().fill(pressed ? Color.blue : Color.gray.opacity(0.8))
                    .frame(width: 30, height: 30)
                    .offset(x: x * 25, y: -y * 25)
                Text(label).font(.caption).foregroundColor(.white)
            }
            HStack {
                ValueDisplay(label: "X", value: x)
                ValueDisplay(label: "Y", value: y)
            }
        }
    }
}

struct ValueDisplay: View {
    let label: String
    let value: Double
    
    var body: some View {
        VStack(spacing: 0) {
            Text(label).font(.caption2).foregroundColor(.secondary)
            Text(String(format: "%.2f", value))
                .font(.caption)
                .fontWeight(.medium)
                .monospacedDigit() // Requer macOS 12.0+
        }
    }
}

struct ABXYButtonsView: View {
    let a, b, x, y: Bool
    
    var body: some View {
        ZStack {
            ActionButton(pressed: y, label: "Y", color: .yellow).offset(y: -35)
            ActionButton(pressed: a, label: "A", color: .green).offset(y: 35)
            ActionButton(pressed: x, label: "X", color: .blue).offset(x: -35)
            ActionButton(pressed: b, label: "B", color: .red).offset(x: 35)
        }
        .frame(width: 100, height: 100)
    }
}

struct ActionButton: View {
    let pressed: Bool
    let label: String
    let color: Color
    
    var body: some View {
        ZStack {
            Circle().fill(pressed ? color : Color.gray.opacity(0.5))
                .frame(width: 35, height: 35)
            Text(label).bold().foregroundColor(.white)
        }
    }
}

struct TriggerView: View {
    let value: Double
    let label: String
    let isLeft: Bool
    
    var body: some View {
        VStack {
            Text(label).font(.caption).bold()
            ZStack(alignment: .bottom) {
                RoundedRectangle(cornerRadius: 4).fill(Color.gray.opacity(0.3))
                    .frame(width: 20, height: 60)
                RoundedRectangle(cornerRadius: 4).fill(Color.blue)
                    .frame(width: 20, height: 60 * value)
            }
            Text(String(format: "%.0f%%", value * 100))
                .font(.caption2)
                .monospacedDigit()
        }
    }
}

struct BumperView: View {
    let pressed: Bool
    let label: String
    
    var body: some View {
        ZStack {
            RoundedRectangle(cornerRadius: 5)
                .fill(pressed ? Color.blue : Color.gray.opacity(0.5))
                .frame(width: 50, height: 20)
            Text(label).font(.caption2).bold().foregroundColor(.white)
        }
    }
}

struct VibrationControls: View {
    @Binding var leftMotor: Double
    @Binding var rightMotor: Double
    let onTest: () -> Void
    
    var body: some View {
        VStack(spacing: 10) {
            Text("Vibração").font(.headline)
            HStack {
                VStack {
                    Text("L").font(.caption)
                    Slider(value: $leftMotor)
                }
                VStack {
                    Text("R").font(.caption)
                    Slider(value: $rightMotor)
                }
            }
            HStack {
                Button("Testar", action: onTest)
                    .buttonStyle(.borderedProminent)
                
                Button("Parar") {
                    leftMotor = 0
                    rightMotor = 0
                    onTest()
                }
                .buttonStyle(.bordered)
            }
        }
        .padding()
        .background(Color.gray.opacity(0.1))
        .cornerRadius(10)
    }
}
