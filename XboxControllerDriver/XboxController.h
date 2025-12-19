#pragma once

#include <DriverKit/DriverKit.h>
#include <USBDriverKit/USBDriverKit.h>
#include <HIDDriverKit/HIDDriverKit.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>

struct XboxControllerState {
    bool buttonA, buttonB, buttonX, buttonY;
    bool dpadUp, dpadDown, dpadLeft, dpadRight;
    int16_t leftStickX, leftStickY;
};

class XboxController : public IOUserHIDDevice {
    OSDeclareDefaultStructors(XboxController)

public:
    // Lifecycle
    bool init() override;
    kern_return_t Start(IOService* provider) override;
    kern_return_t Stop(IOService* provider) override;
    void free() override;

    // HID
    OSDictionary* newDeviceDescription() override;
    OSData* newReportDescriptor() override;
    void UpdateHIDReport(const XboxControllerState& state);

    // USB
    kern_return_t SetupUSB();
    kern_return_t StartReading();
    kern_return_t SendInitCommand();

    // Callback
    kern_return_t HandleReadComplete(
        OSAction* action,
        IOReturn status,
        uint32_t bytesTransferred
    );

private:
    IOUSBHostInterface* usbInterface = nullptr;
    IOUSBHostPipe* inputPipe = nullptr;
    IOUSBHostPipe* outputPipe = nullptr;

    IOBufferMemoryDescriptor* inputBuffer = nullptr;
    IOBufferMemoryDescriptor* hidBuffer = nullptr;

    OSAction* readAction = nullptr;
};
