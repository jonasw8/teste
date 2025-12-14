//
//  XboxUSBDriver.cpp
//
#include "XboxController.h"
#include <os/log.h>

#define Log(fmt, ...) os_log(OS_LOG_DEFAULT, "XboxUSB: " fmt, ##__VA_ARGS__)

kern_return_t XboxController::SetupUSBCommunication() {
    kern_return_t ret;
    
    // Pega a Interface 0
    ret = usbDevice->CopyInterface(0, &usbInterface);
    if (ret != kIOReturnSuccess) return ret;
    
    // Abre a interface
    ret = usbInterface->Open(this, 0, 0);
    if (ret != kIOReturnSuccess) return ret;
    
    return FindEndpoints();
}

kern_return_t XboxController::FindEndpoints() {
    const IOUSBDescriptorHeader* header = nullptr;
    
    // Usa a constante manual kUSBEndpointDesc (0x05)
    while ((header = usbInterface->FindNextDescriptor(header, kUSBEndpointDesc))) {
        const uint8_t* raw = (const uint8_t*)header;
        
        // Estrutura manual do Endpoint Descriptor
        // Offset 2 = Address, Offset 3 = Attributes, Offset 4 = MaxPacketSize
        uint8_t addr = raw[2];
        uint8_t attr = raw[3];
        uint16_t size = raw[4] | (raw[5] << 8);
        
        // 0x03 = Interrupt Transfer
        if ((attr & 0x03) == 0x03) {
            bool isInput = (addr & 0x80) != 0;
            
            if (isInput && !inputPipe) {
                inputEndpointAddr = addr;
                maxPacketSize = size;
                usbInterface->CopyPipe(addr, &inputPipe);
                Log("Pipe IN encontrado: 0x%x", addr);
            } else if (!isInput && !outputPipe) {
                outputEndpointAddr = addr;
                usbInterface->CopyPipe(addr, &outputPipe);
                Log("Pipe OUT encontrado: 0x%x", addr);
            }
        }
    }
    
    return (inputPipe) ? kIOReturnSuccess : kIOReturnError;
}

// Callback Estático
static void StaticReadCallback(OSObject* target, void* context, IOReturn status, uint32_t transferred) {
    XboxController* driver = OSDynamicCast(XboxController, target);
    if (driver) driver->HandleReadComplete(status, transferred);
}

kern_return_t XboxController::StartReading() {
    if (!inputPipe) return kIOReturnError;
    
    if (!inputBuffer) {
        IOBufferMemoryDescriptor::Create(kIOMemoryDirectionIn, maxPacketSize, 0, &inputBuffer);
    }
    
    IOUSBHostPipe::AsyncIOCallback callback = &StaticReadCallback;
    return inputPipe->AsyncIO(inputBuffer, maxPacketSize, callback, this);
}

void XboxController::HandleReadComplete(IOReturn status, uint32_t transferred) {
    if (status == kIOReturnSuccess && transferred > 0) {
        uint64_t addr;
        inputBuffer->Map(0, 0, 0, 0, &addr, NULL);
        ProcessInput((const uint8_t*)addr, transferred);
    }
    
    if (status != kIOReturnAborted) {
        IOUSBHostPipe::AsyncIOCallback callback = &StaticReadCallback;
        inputPipe->AsyncIO(inputBuffer, maxPacketSize, callback, this);
    }
}

void XboxController::ProcessInput(const uint8_t* data, size_t length) {
    // Protocolo Wireless (Offset 6)
    if (length < 12 || data[0] != 0x00) return;
    
    XboxControllerState s = {};
    uint8_t b1 = data[6];
    uint8_t b2 = data[7];
    
    s.dpadUp = (b1 & 1); s.dpadDown = (b1 & 2);
    s.dpadLeft = (b1 & 4); s.dpadRight = (b1 & 8);
    s.buttonStart = (b1 & 16); s.buttonBack = (b1 & 32);
    s.buttonLS = (b1 & 64); s.buttonRS = (b1 & 128);
    
    s.buttonLB = (b2 & 1); s.buttonRB = (b2 & 2);
    s.buttonGuide = (b2 & 4); s.buttonA = (b2 & 16);
    s.buttonB = (b2 & 32); s.buttonX = (b2 & 64); s.buttonY = (b2 & 128);
    
    s.leftTrigger = data[8]; s.rightTrigger = data[9];
    s.leftStickX = (int16_t)(data[10] | (data[11] << 8));
    s.leftStickY = (int16_t)(data[12] | (data[13] << 8));
    s.rightStickX = (int16_t)(data[14] | (data[15] << 8));
    s.rightStickY = (int16_t)(data[16] | (data[17] << 8));
    
    UpdateHIDState(&s);
}