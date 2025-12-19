#include "XboxController.h"
#include <HIDDriverKit/IOHIDUsageTables.h>
#include <os/log.h>

#define Log(fmt, ...) os_log(OS_LOG_DEFAULT, "XboxController: " fmt, ##__VA_ARGS__)

bool XboxController::init()
{
    Log("init()");
    if (!IOHIDDevice::init())
        return false;

    // Cria buffer para 16 botões (2 bytes)
    IOBufferMemoryDescriptor::Create(
        kIOMemoryDirectionOut,
        sizeof(uint16_t),
        0,
        &reportBuffer
    );

    return true;
}

kern_return_t XboxController::Start(IOService *provider)
{
    Log("Start()");
    auto ret = IOHIDDevice::Start(provider);

    if (ret != kIOReturnSuccess)
        return ret;

    RegisterService();

    return kIOReturnSuccess;
}

kern_return_t XboxController::Stop(IOService *provider)
{
    Log("Stop()");
    return IOHIDDevice::Stop(provider);
}

void XboxController::free()
{
    Log("free()");
    OSSafeReleaseNULL(reportBuffer);
    IOHIDDevice::free();
}
