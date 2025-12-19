#include "XboxController.h"
#include <USBDriverKit/USBStandard.h>

struct Xbox360Packet {
    uint8_t header;
    uint8_t size;
    uint8_t buttons1;
    uint8_t buttons2;
    int16_t lx, ly;
};

kern_return_t XboxController::SetupUSB() {
    IOBufferMemoryDescriptor::Create(
        kIOMemoryDirectionIn, 32, 0, &inputBuffer
    );

    IOBufferMemoryDescriptor::Create(
        kIOMemoryDirectionOut, 32, 0, &hidBuffer
    );

    StandardUSB::EndpointDescriptor in = {};
    in.bEndpointAddress = 0x81;
    in.bmAttributes = 0x03;
    in.wMaxPacketSize = 32;

    usbInterface->CreateIOUSBHostPipe(&in, &inputPipe);

    OSAction::Create(
        this,
        XboxController_HandleReadComplete_ID,
        IOUSBHostPipe_CompleteAsyncIO_ID,
        0,
        &readAction
    );

    return kIOReturnSuccess;
}

kern_return_t XboxController::StartReading() {
    return inputPipe->AsyncIO(inputBuffer, 32, readAction, 0);
}

kern_return_t XboxController::SendInitCommand() {
    uint8_t magic[] = { 0x08, 0x00, 0x0F, 0xC0 };

    IOBufferMemoryDescriptor* buf = nullptr;
    IOBufferMemoryDescriptor::Create(
        kIOMemoryDirectionOut, sizeof(magic), 0, &buf
    );

    buf->WriteBytes(0, magic, sizeof(magic));
    outputPipe->Write(buf, 0, sizeof(magic), nullptr, 0);
    buf->release();

    return kIOReturnSuccess;
}

kern_return_t XboxController::HandleReadComplete(
    OSAction*, IOReturn status, uint32_t bytes
) {
    if (status != kIOReturnSuccess)
        return status;

    Xbox360Packet pkt{};
    inputBuffer->ReadBytes(0, &pkt, sizeof(pkt));

    if (pkt.size != 0x14)
        return kIOReturnSuccess;

    XboxControllerState s{};
    s.buttonA = pkt.buttons2 & 0x10;
    s.buttonB = pkt.buttons2 & 0x20;
    s.leftStickX = pkt.lx;
    s.leftStickY = pkt.ly;

    UpdateHIDReport(s);
    StartReading();

    return kIOReturnSuccess;
}
