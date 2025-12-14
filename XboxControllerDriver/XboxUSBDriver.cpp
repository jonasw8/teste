//
//  XboxUSBDriver.cpp
//  XboxControllerDriver
//

#include "XboxController.h"
#include <USBDriverKit/USBStandard.h> // Essencial para constantes USB

// Definição de constantes USB se faltarem
#ifndef kUSBEndpointDesc
#define kUSBEndpointDesc 0x05
#endif

struct Xbox360_Report {
    uint8_t msgType;
    uint8_t packetSize;
    uint8_t buttons1;
    uint8_t buttons2;
    uint8_t leftTrigger;
    uint8_t rightTrigger;
    int16_t leftStickX;
    int16_t leftStickY;
    int16_t rightStickX;
    int16_t rightStickY;
    uint8_t padding[6];
};

bool XboxController::init() {
    if (!IOUserHIDDevice::init()) return false;
    
    usbDevice = nullptr;
    usbInterface = nullptr;
    inputPipe = nullptr;
    outputPipe = nullptr;
    inputBuffer = nullptr;
    hidReportBuffer = nullptr;
    readAction = nullptr;
    
    return true;
}

kern_return_t XboxController::Start(IOService* provider) {
    kern_return_t ret = IOUserHIDDevice::Start(provider);
    if (ret != kIOReturnSuccess) {
        return ret;
    }

    // 1. Obter interface USB
    usbInterface = OSDynamicCast(IOUSBHostInterface, provider);
    if (!usbInterface) {
        return kIOReturnNoDevice;
    }
    usbInterface->retain();
    
    // 2. Abrir Interface
    ret = usbInterface->Open(this, 0, 0);
    if (ret != kIOReturnSuccess) {
        return ret;
    }
    
    // 3. Configurar e Iniciar Leitura
    ret = SetupUSBCommunication();
    if (ret == kIOReturnSuccess) {
        SendInitCommand(); // Acende a luz do controle
        StartReading();    // Começa o loop de input
    }
    
    return ret;
}

kern_return_t XboxController::Stop(IOService* provider) {
    if (readAction) {
        readAction->Cancel();
        readAction->release();
        readAction = nullptr;
    }
    
    if (inputPipe) {
        inputPipe->Abort();
        inputPipe->release();
        inputPipe = nullptr;
    }
    
    if (usbInterface) {
        usbInterface->Close(this, 0);
        usbInterface->release();
        usbInterface = nullptr;
    }
    
    return IOUserHIDDevice::Stop(provider);
}

void XboxController::free() {
    if (inputBuffer) inputBuffer->release();
    if (hidReportBuffer) hidReportBuffer->release();
    IOUserHIDDevice::free();
}

kern_return_t XboxController::SetupUSBCommunication() {
    // Preparar Buffer de Leitura (32 bytes é padrão para Xbox 360)
    IOBufferMemoryDescriptor::Create(kIOMemoryDirectionIn, 32, 0, &inputBuffer);
    
    // Preparar Buffer de Relatório HID (para handleReport)
    IOBufferMemoryDescriptor::Create(kIOMemoryDirectionOut, sizeof(uint8_t) * 32, 0, &hidReportBuffer);
    
    return FindEndpoints();
}

kern_return_t XboxController::FindEndpoints() {
    // Iterar descritores para achar endpoints IN e OUT
    const StandardUSB::ConfigurationDescriptor* confDesc = nullptr;
    const StandardUSB::InterfaceDescriptor* ifaceDesc = nullptr;
    const StandardUSB::EndpointDescriptor* epDesc = nullptr;
    
    // Simplificação: Assume endpoints fixos do Xbox 360 Receiver
    // Geralmente 0x81 (Input) e 0x02 (Output)
    inputEndpointAddr = 0x81;
    outputEndpointAddr = 0x02;
    maxPacketSize = 32;

    // Criar Pipes
    struct StandardUSB::EndpointDescriptor inputDesc = {};
    inputDesc.bEndpointAddress = inputEndpointAddr;
    inputDesc.wMaxPacketSize = maxPacketSize;
    inputDesc.bmAttributes = 0x03; // Interrupt
    
    usbInterface->CreateIOUSBHostPipe(&inputDesc, &inputPipe);
    
    // Criar Action para o Callback (Substitui ponteiro de função antigo)
    OSAction::Create(this, 
                     XboxController_HandleReadComplete_ID, 
                     IOUSBHostPipe_CompleteAsyncIO_ID, 
                     0, 
                     &readAction);
    
    return kIOReturnSuccess;
}

kern_return_t XboxController::StartReading() {
    if (!inputPipe || !inputBuffer || !readAction) return kIOReturnError;
    
    // Inicia leitura assíncrona
    return inputPipe->AsyncIO(inputBuffer, 32, readAction, 0);
}

kern_return_t XboxController::SendInitCommand() {
    // "Magic Packet" para inicializar (LED girando -> Player 1)
    uint8_t initPacket[] = { 0x01, 0x03, 0x00 }; // Comando simples de LED
    
    IOBufferMemoryDescriptor* cmdBuffer = nullptr;
    IOBufferMemoryDescriptor::Create(kIOMemoryDirectionOut, sizeof(initPacket), 0, &cmdBuffer);
    
    if (cmdBuffer) {
        cmdBuffer->WriteBytes(0, initPacket, sizeof(initPacket));
        // Envio síncrono para simplificar
        // usbInterface->DeviceRequest(...) seria o ideal, mas vamos pular por agora
        cmdBuffer->release();
    }
    return kIOReturnSuccess;
}

// Callback (Chamado pelo OSAction)
void XboxController::HandleReadComplete(OSAction* action, IOReturn status, uint32_t bytesTransferred) {
    if (status == kIOReturnSuccess && bytesTransferred > 0) {
        uint8_t buffer[32];
        inputBuffer->ReadBytes(0, buffer, bytesTransferred);
        ProcessInput(buffer, bytesTransferred);
    }
    
    // Reiniciar leitura (Loop)
    if (status != kIOReturnAborted) {
        StartReading();
    }
}

// Stub necessário para o compilador gerar a tabela de despacho
// Isso substitui o .iig gerado
kern_return_t IMPL(XboxController, HandleReadComplete) {
    return kIOReturnSuccess; 
}

void XboxController::ProcessInput(const uint8_t* data, size_t length) {
    // Validar pacote de input (0x00 = Status change, 0x08 = Data)
    // O pacote de dados do controle começa com 0x00 0x14 (20 bytes)
    
    if (length < 14) return; // Muito pequeno
    
    // Mapeamento simples (Exemplo)
    XboxControllerState s = {};
    const Xbox360_Report* r = (const Xbox360_Report*)data;
    
    // Se não for pacote de dados, ignora
    if (r->packetSize != 0x14) return;

    s.buttonA = (r->buttons2 & 0x10);
    s.buttonB = (r->buttons2 & 0x20);
    s.buttonX = (r->buttons2 & 0x40);
    s.buttonY = (r->buttons2 & 0x80);
    
    s.dpadUp    = (r->buttons1 & 0x01);
    s.dpadDown  = (r->buttons1 & 0x02);
    s.dpadLeft  = (r->buttons1 & 0x04);
    s.dpadRight = (r->buttons1 & 0x08);
    
    s.leftStickX = r->leftStickX;
    s.leftStickY = r->leftStickY;
    
    UpdateHIDReport(&s);
}
