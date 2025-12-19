#include "XboxController.h"

static const uint8_t HIDDesc[] = {
    0x05, 0x01,
    0x09, 0x05,
    0xA1, 0x01,
    0x05, 0x09,
    0x19, 0x01,
    0x29, 0x10,
    0x95, 0x10,
    0x75, 0x01,
    0x81, 0x02,
    0xC0
};

OSData* XboxController::newReportDescriptor() {
    return OSData::withBytes(HIDDesc, sizeof(HIDDesc));
}

OSDictionary* XboxController::newDeviceDescription() {
    auto dict = OSDictionary::withCapacity(4);
    dict->setObject(kIOHIDPrimaryUsagePageKey,
                    OSNumber::withNumber(kHIDPage_GenericDesktop, 32));
    dict->setObject(kIOHIDPrimaryUsageKey,
                    OSNumber::withNumber(kHIDUsage_GD_GamePad, 32));
    return dict;
}

void XboxController::UpdateHIDReport(const XboxControllerState& s) {
    uint16_t buttons = 0;
    if (s.buttonA) buttons |= 1 << 0;
    if (s.buttonB) buttons |= 1 << 1;

    hidBuffer->SetLength(sizeof(buttons));
    hidBuffer->WriteBytes(0, &buttons, sizeof(buttons));

    handleReport(0, hidBuffer, sizeof(buttons),
                 kIOHIDReportTypeInput, 0);
}
