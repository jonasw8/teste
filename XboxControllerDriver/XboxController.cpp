#include "XboxController.h"
#include <os/log.h>

#define Log(fmt, ...) os_log(OS_LOG_DEFAULT, "XboxController: " fmt, ##__VA_ARGS__)

bool XboxController::init() {
    return IOUserHIDDevice::init();
}

kern_return_t XboxController::Start(IOService* provider) {
    Log("Start");

    if (!IOUserHIDDevice::Start(provider))
        return kIOReturnError;

    usbInterface = OSDynamicCast(IOUSBHostInterface, provider);
    if (!usbInterface)
        return kIOReturnNoDevice;

    usbInterface->retain();
    usbInterface->Open(this, 0, 0);

    SetupUSB();
    SendInitCommand();
    StartReading();

    RegisterService();
    return kIOReturnSuccess;
}

kern_return_t XboxController::Stop(IOService* provider) {
    if (inputPipe) inputPipe->Abort();
    if (usbInterface) usbInterface->Close(this, 0);
    return IOUserHIDDevice::Stop(provider);
}

void XboxController::free() {
    OSSafeReleaseNULL(inputPipe);
    OSSafeReleaseNULL(outputPipe);
    OSSafeReleaseNULL(usbInterface);
    OSSafeReleaseNULL(inputBuffer);
    OSSafeReleaseNULL(hidBuffer);
    OSSafeReleaseNULL(readAction);
    IOUserHIDDevice::free();
}
