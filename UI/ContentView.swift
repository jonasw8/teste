//
//  ContentView.swift
//  XboxControllerApp
//
//  Created by jonas on 21/12/25.
//


import SwiftUI

struct ContentView: View {
    // Aqui criamos a instância do gerenciador
    @StateObject var manager = ExtensionManager()
    
    var body: some View {
        VStack(spacing: 20) {
            Image(systemName: "gamecontroller.fill")
                .font(.system(size: 60))
                .foregroundColor(.blue)
            
            Text("Instalador Xbox Driver")
                .font(.title)
                .bold()
            
            HStack(spacing: 20) {
                Button(action: { manager.installDriver() }) {
                    Label("Instalar Driver", systemImage: "arrow.down.circle.fill")
                        .padding()
                }
                .buttonStyle(.borderedProminent)
                
                Button(action: { manager.uninstallDriver() }) {
                    Label("Desinstalar", systemImage: "trash")
                        .padding()
                }
            }
            
            Divider()
            
            Text("Status: \(manager.status)")
                .font(.headline)
                .foregroundColor(.gray)
            
            ScrollView {
                Text(manager.logText)
                    .font(.system(.caption, design: .monospaced))
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding()
            }
            .background(Color.black.opacity(0.1))
            .cornerRadius(8)
        }
        .padding()
    }
}
