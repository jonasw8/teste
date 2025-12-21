#include "XboxController.h"
#include <os/log.h>
#include <DriverKit/IOLib.h>
#include <DriverKit/OSCollections.h>

// Macro simplificada para garantir compatibilidade
#define LogError(fmt, ...) os_log(OS_LOG_DEFAULT, "XboxDriver [ERROR]: " fmt, ##__VA_ARGS__)
#define LogInfo(fmt, ...)  os_log(OS_LOG_DEFAULT, "XboxDriver [INFO]: " fmt, ##__VA_ARGS__)

bool XboxController::init() {
    memset(&ivars, 0, sizeof(ivars));
    ivars.inputEndpointAddr = XBOX360_INPUT_ENDPOINT;
    ivars.outputEndpointAddr = XBOX360_OUTPUT_ENDPOINT;
    ivars.maxPacketSize = XBOX360_MAX_PACKET_SIZE;
    
    if (!super::init()) {
        return false;
    }
    return true;
}

void XboxController::free() {
    super::free();
}

kern_return_t XboxController::Start(IOService* provider) {
    kern_return_t ret = kIOReturnSuccess;
    
    LogInfo("Start chamado");

    ret = super::Start(provider);
    if (ret != kIOReturnSuccess) {
        LogError("super::Start falhou: 0x%x", ret);
        return ret;
    }

    ivars.usbDevice = OSDynamicCast(IOUSBHostDevice, provider);
    if (!ivars.usbDevice) {
        LogError("Provider nao e IOUSBHostDevice");
        return kIOReturnNoDevice;
    }

    ret = ivars.usbDevice->Open(this, 0, 0);
    if (ret != kIOReturnSuccess) {
        LogError("Falha ao abrir dispositivo USB: 0x%x", ret);
        return ret;
    }
    
    return ret;
}

kern_return_t XboxController::Stop(IOService* provider) {
    LogInfo("Stop chamado");
    
    if (ivars.inputPipe) {
        // CORREÇÃO: Argumentos do Abort e release minúsculo
        ivars.inputPipe->Abort(0, kIOReturnAborted, nullptr);
        ivars.inputPipe->release(); 
        ivars.inputPipe = nullptr;
    }
    
    if (ivars.usbDevice) {
        ivars.usbDevice->Close(this, 0);
        ivars.usbDevice = nullptr;
    }
    
    return super::Stop(provider);
}

OSDictionary* XboxController::newDeviceDescription() {
    OSDictionary* dict = OSDictionary::withCapacity(5);
    return dict;
}

kern_return_t XboxController::newReportDescriptor(IOMemoryDescriptor** descriptor) {
    *descriptor = nullptr;
    return kIOReturnSuccess;
}

kern_return_t XboxController::handleReport(uint64_t timestamp,
                                           IOMemoryDescriptor* report,
                                           uint32_t reportType,
                                           IOOptionBits options) {
    return kIOReturnSuccess;
}

void XboxController::ProcessData(IOBufferMemoryDescriptor *dataBuffer) {
    // Implementação pendente
}
