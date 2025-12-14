//
//  XboxController.cpp
//  XboxControllerDriverKit
//
//  Gerenciamento do ciclo de vida do driver e inicialização do hardware.
//

#include "XboxController.h"
#include <os/log.h>

// Macro para logs facilitados no Console.app
#define Log(fmt, ...) os_log(OS_LOG_DEFAULT, "XboxController: " fmt, ##__VA_ARGS__)

// --------------------------------------------------------------------------
// Ciclo de Vida do Driver (Lifecycle)
// --------------------------------------------------------------------------

bool XboxController::init()
{
    // Inicializa a classe base (IOUserHIDDevice)
    if (!IOUserHIDDevice::init()) {
        return false;
    }
    
    // Inicializa ponteiros como nulos para evitar crashes no free()
    usbDevice = nullptr;
    usbInterface = nullptr;
    inputPipe = nullptr;
    outputPipe = nullptr;
    inputBuffer = nullptr;
    reportBuffer = nullptr;
    
    return true;
}

kern_return_t XboxController::Start(IOService* provider)
{
    kern_return_t ret;
    
    Log("Iniciando Driver...");
    
    // 1. Inicia a superclasse
    if (!IOUserHIDDevice::Start(provider)) {
        Log("Falha ao iniciar superclasse HID");
        return kIOReturnError;
    }
    
    // 2. Obtém referência ao dispositivo USB físico
    usbDevice = OSDynamicCast(IOUSBHostDevice, provider);
    if (!usbDevice) {
        Log("Provider não é um IOUSBHostDevice");
        return kIOReturnNoDevice;
    }
    
    usbDevice->Retain();
    
    // 3. Abre a conexão com o dispositivo USB
    ret = usbDevice->Open(this, 0, 0);
    if (ret != kIOReturnSuccess) {
        Log("Falha ao abrir dispositivo USB: 0x%x", ret);
        return ret;
    }
    
    // 4. Configura Pipes e Interfaces (Implementado em XboxUSBDriver.cpp)
    ret = SetupUSBCommunication();
    if (ret != kIOReturnSuccess) {
        Log("Falha na configuração USB");
        Stop(provider);
        return ret;
    }
    
    // 5. O SEGREDO: Envia o comando para inicializar o Receptor Wireless
    // Sem isso, a luz verde não acende e nenhum dado é recebido.
    ret = SendInitCommand();
    if (ret != kIOReturnSuccess) {
        Log("Aviso: Falha ao enviar Magic Packet. O receptor pode não ligar.");
    } else {
        Log("Hardware inicializado com sucesso (Magic Packet enviado)");
    }
    
    // 6. Começa a ler os dados do controle (Loop de leitura)
    ret = StartReading();
    if (ret != kIOReturnSuccess) {
        Log("Falha ao iniciar leitura do Pipe");
        Stop(provider);
        return ret;
    }
    
    // 7. Registra o serviço no sistema
    // Isso diz ao macOS: "Estou pronto! Sou um Joystick."
    RegisterService();
    
    Log("Driver carregado e operante!");
    return kIOReturnSuccess;
}

kern_return_t XboxController::Stop(IOService* provider)
{
    Log("Parando Driver...");
    
    // Aborta leituras pendentes para evitar crash
    if (inputPipe) {
        inputPipe->Abort();
    }
    
    if (usbDevice) {
        usbDevice->Close(this, 0);
    }
    
    return IOUserHIDDevice::Stop(provider);
}

void XboxController::free()
{
    Log("Liberando memória...");
    
    OSSafeReleaseNULL(usbDevice);
    OSSafeReleaseNULL(usbInterface);
    OSSafeReleaseNULL(inputPipe);
    OSSafeReleaseNULL(outputPipe);
    OSSafeReleaseNULL(inputBuffer);
    OSSafeReleaseNULL(reportBuffer);
    
    IOUserHIDDevice::free();
}

// --------------------------------------------------------------------------
// Inicialização de Hardware (Magic Packet)
// --------------------------------------------------------------------------

kern_return_t XboxController::SendInitCommand()
{
    // Sequência específica para o Receptor Xbox 360 Wireless
    // Diz ao hardware que um driver foi carregado e ele deve ligar os LEDs.
    const uint8_t magic[] = { 
        0x08, 0x00, 0x0F, 0xC0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 
    };
    
    if (!outputPipe) {
        Log("Erro: Pipe de saída não encontrado. Impossível enviar comando de init.");
        return kIOReturnNoDevice;
    }
    
    IOBufferMemoryDescriptor* buffer = nullptr;
    kern_return_t ret = IOBufferMemoryDescriptor::Create(
        kIOMemoryDirectionOut, 
        sizeof(magic), 
        0, 
        &buffer
    );
    
    if (ret != kIOReturnSuccess) return ret;
    
    // Copia os bytes para o buffer
    buffer->GetBytesNoCopy(0, sizeof(magic));
    memcpy(buffer->getBytesNoCopy(0, sizeof(magic)), magic, sizeof(magic));
    
    // Envia via USB
    ret = outputPipe->Write(buffer, 0, sizeof(magic), NULL, 0);
    
    OSSafeReleaseNULL(buffer);
    return ret;
}