#include "XboxController.h"
#include <HIDDriverKit/IOHIDUsageTables.h>
#include <os/log.h>

#define Log(fmt, ...) os_log(OS_LOG_DEFAULT, "XboxController: " fmt, ##__VA_ARGS__)

// HID descriptor mínimo — 16 botões
static const uint8_t gXboxHIDReport[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (GamePad)
    0xA1, 0x01,        // Collection (Application)
    0x05, 0x09,        // Usage Page (Buttons)
    0x19, 0x01,        // Usage Min (1)
    0x29, 0x10,        // Usage Max (16)
    0x15, 0x00,        // Logical Min (0)
    0x25, 0x01,        // Logical Max (1)
    0x95, 0x10,        // Report Count (16)
    0x75, 0x01,        // Report Size (1)
    0x81, 0x02,        // Input (Data,Var,Abs)
    0xC0               // End Collection
};

bool XboxController::init()
{
    Log("init()");
    if (!IOHIDDevice::init())
        return false;

    // Cria buffer de 2 bytes para o relatório
    kern_return_t ret = IOBufferMemoryDescriptor::Create(
        kIOMemoryDirectionOut,
        sizeof(uint16_t),
        0,
        &reportBuffer
    );

    if (ret != kIOReturnSuccess || !reportBuffer) {
        Log("Falha ao criar reportBuffer");
        return false;
    }

    return true;
}

kern_return_t XboxController::Start(IOService *provider)
{
    Log("Start()");

    auto ret = IOHIDDevice::Start(provider);
    if (ret != kIOReturnSuccess)
        return ret;

    // Registra como HID
    RegisterService();

    Log("Driver HID registrado");
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

OSData* XboxController::newReportDescriptor()
{
    Log("Fornecendo HID Report Descriptor");
    return OSData::withBytes(gXboxHIDReport, sizeof(gXboxHIDReport));
}

OSDictionary* XboxController::newDeviceDescription()
{
    Log("Gerando DeviceDescription");
    auto dict = OSDictionary::withCapacity(4);

    dict->setObject(kIOHIDPrimaryUsagePageKey,
        OSNumber::withNumber(kHIDPage_GenericDesktop, 32));
    
    dict->setObject(kIOHIDPrimaryUsageKey,
        OSNumber::withNumber(kHIDUsage_GD_GamePad, 32));

    return dict;
}

kern_return_t XboxController::UpdateButtons(uint16_t mask)
{
    if (!reportBuffer)
        return kIOReturnError;

    Log("Atualizando HID Report (mask=0x%x)", mask);

    IOMemoryMap *map = nullptr;
    auto ret = reportBuffer->CreateMapping(0, 0, 0, 0, &map);
    if (ret != kIOReturnSuccess)
        return ret;

    void *addr = map->GetAddress();
    memcpy(addr, &mask, sizeof(mask));

    handleReport(
        0,                  // timestamp
        reportBuffer,       // buffer
        sizeof(mask),       // tamanho
        kIOHIDReportTypeInput,
        0                   // opções
    );

    map->release();
    return kIOReturnSuccess;
}
