#pragma once

#include <DriverKit/DriverKit.h>
#include <HIDDriverKit/IOHIDDevice.h>
#include <DriverKit/IOLib.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>

using namespace std;

class XboxController : public IOHIDDevice
{
    DKDeclareDefaultStructors(XboxController)

public:
    // Lifecycle
    virtual bool init() override;
    virtual kern_return_t Start(IOService *provider) override;
    virtual kern_return_t Stop(IOService *provider) override;
    virtual void free() override;

    // HID reporting
    virtual OSData* newReportDescriptor() override;
    virtual OSDictionary* newDeviceDescription() override;

    kern_return_t UpdateButtons(uint16_t mask);

private:
    IOBufferMemoryDescriptor *reportBuffer = nullptr;
};
