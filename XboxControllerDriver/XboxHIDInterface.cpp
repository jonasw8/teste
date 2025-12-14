//
//  XboxHIDInterface.cpp
//  XboxControllerDriver
//

#include "XboxController.h"
#include <HIDDriverKit/IOHIDUsageTables.h>

// Report Descriptor (Mantido igual, simplificado)
static const uint8_t kHIDDesc[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    0x05, 0x09,        // Usage Page (Buttons)
    0x19, 0x01,        // Usage Min (1)
    0x29, 0x10,        // Usage Max (16)
    0x15, 0x00,        // Logical Min (0)
    0x25, 0x01,        // Logical Max (1)
    0x95, 0x10,        // Report Count (16)
    0x75, 0x01,        // Report Size (1)
    0x81, 0x02,        // Input (Data, Var, Abs)
    0xC0               // End Collection
};

struct __attribute__((packed)) HIDReport {
    uint16_t buttons;
    // Simplificado para teste (Adicionar eixos depois)
};

OSData* XboxController::newReportDescriptor() {
    return OSData::withBytes(kHIDDesc, sizeof(kHIDDesc));
}

OSDictionary* XboxController::newDeviceDescription() {
    OSDictionary* dict = OSDictionary::withCapacity(2);
    // Configurações básicas
    return dict;
}

void XboxController::UpdateHIDReport(XboxControllerState* s) {
    if (!hidReportBuffer) return;
    
    HIDReport r = {};
    
    // Mapear botões
    if (s->buttonA) r.buttons |= (1 << 0);
    if (s->buttonB) r.buttons |= (1 << 1);
    if (s->buttonX) r.buttons |= (1 << 2);
    if (s->buttonY) r.buttons |= (1 << 3);
    if (s->dpadUp)  r.buttons |= (1 << 4);
    // ... mapear outros ...
    
    // CORREÇÃO CRÍTICA: Escrever no buffer de memória
    hidReportBuffer->SetLength(sizeof(r));
    hidReportBuffer->WriteBytes(0, &r, sizeof(r));
    
    // CORREÇÃO: Passar o descritor de memória, não o ponteiro bruto
    // Timestamp 0 instrui o sistema a usar o tempo atual
    handleReport(0, hidReportBuffer, sizeof(r), kIOHIDReportTypeInput, 0);
}
