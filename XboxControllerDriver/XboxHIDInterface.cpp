//
//  XboxHIDInterface.cpp
//  XboxControllerDriver
//
//  Implementação da camada HID (Human Interface Device)
//  Responsável por traduzir o estado do controle para algo que o macOS entenda.
//

#include "XboxController.h"
#include <HIDDriverKit/IOHIDUsageTables.h> // Garante acesso às constantes HID

// --------------------------------------------------------------------------
// Report Descriptor
// Define para o macOS que tipo de controle é este (Gamepad com 2 analógicos, botões, etc)
// --------------------------------------------------------------------------
static const uint8_t kHIDDesc[] = {
    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x05,        // Usage (Game Pad)
    0xA1, 0x01,        // Collection (Application)
    
    // --- Botões (16 botões) ---
    0x05, 0x09,        //   Usage Page (Button)
    0x19, 0x01,        //   Usage Minimum (0x01)
    0x29, 0x10,        //   Usage Maximum (0x10) - 16 botões
    0x15, 0x00,        //   Logical Minimum (0)
    0x25, 0x01,        //   Logical Maximum (1)
    0x75, 0x01,        //   Report Size (1 bit)
    0x95, 0x10,        //   Report Count (16)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    
    // --- D-Pad (Hat Switch) ---
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x39,        //   Usage (Hat switch)
    0x15, 0x01,        //   Logical Minimum (1) - 1=Top, 2=TopRight...
    0x25, 0x08,        //   Logical Maximum (8)
    0x35, 0x00,        //   Physical Minimum (0)
    0x46, 0x3B, 0x01,  //   Physical Maximum (315)
    0x65, 0x14,        //   Unit (System: English Rotation)
    0x75, 0x04,        //   Report Size (4 bits)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x42,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,Null State)
    
    // Padding para alinhar o Hat Switch (4 bits restantes do byte)
    0x75, 0x04,        //   Report Size (4 bits)
    0x95, 0x01,        //   Report Count (1)
    0x81, 0x01,        //   Input (Const,Array,Abs,No Wrap,Linear,Preferred State,No Null Position)
    
    // --- Analógicos (4 eixos: X, Y, Rx, Ry) ---
    0x05, 0x01,        //   Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //   Usage (X)
    0x09, 0x31,        //   Usage (Y)
    0x09, 0x33,        //   Usage (Rx)
    0x09, 0x34,        //   Usage (Ry)
    0x15, 0x00,        //   Logical Minimum (0)
    0x26, 0xFF, 0x00,  //   Logical Maximum (255) - 8 bits
    0x75, 0x08,        //   Report Size (8 bits)
    0x95, 0x04,        //   Report Count (4)
    0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
    
    0xC0               // End Collection
};

// Estrutura deve bater com o descriptor (Total: 7 bytes)
struct __attribute__((packed)) HIDReport {
    uint16_t buttons;
    uint8_t hat;      // 4 bits low = hat, 4 bits high = padding
    uint8_t lx;
    uint8_t ly;
    uint8_t rx;
    uint8_t ry;
};

// --------------------------------------------------------------------------
// Implementação dos Métodos Virtuais
// --------------------------------------------------------------------------

OSData* XboxController::newReportDescriptor()
{
    return OSData::withBytes(kHIDDesc, sizeof(kHIDDesc));
}

OSDictionary* XboxController::newDeviceDescription()
{
    OSDictionary* dict = OSDictionary::withCapacity(2);
    if (dict) {
        OSString* name = OSString::withCString("Xbox 360 Wireless Controller");
        if (name) {
            dict->setObject("Product", name);
            name->release();
        }
        
        OSNumber* venID = OSNumber::withNumber(0x045E, 32);
        if (venID) {
            dict->setObject("VendorID", venID);
            venID->release();
        }
    }
    return dict;
}

// --------------------------------------------------------------------------
// Atualização de Estado
// --------------------------------------------------------------------------

// CORREÇÃO: O nome da função agora é UpdateHIDReport para bater com o Header (.h)
void XboxController::UpdateHIDReport(XboxControllerState* s)
{
    HIDReport r = {};
    
    // 1. Mapeamento de Botões
    if (s->buttonA)     r.buttons |= (1 << 0);
    if (s->buttonB)     r.buttons |= (1 << 1);
    if (s->buttonX)     r.buttons |= (1 << 2);
    if (s->buttonY)     r.buttons |= (1 << 3);
    if (s->buttonLB)    r.buttons |= (1 << 4);
    if (s->buttonRB)    r.buttons |= (1 << 5);
    if (s->buttonBack)  r.buttons |= (1 << 6);
    if (s->buttonStart) r.buttons |= (1 << 7);
    if (s->buttonGuide) r.buttons |= (1 << 8);
    if (s->buttonLS)    r.buttons |= (1 << 9);
    if (s->buttonRS)    r.buttons |= (1 << 10);
    
    // 2. Hat Switch (D-Pad)
    uint8_t hatValue = 0; // 0 = Null (Centro)
    
    if (s->dpadUp) {
        if (s->dpadRight) hatValue = 2;      // NE
        else if (s->dpadLeft) hatValue = 8;  // NW
        else hatValue = 1;                   // N
    } else if (s->dpadDown) {
        if (s->dpadRight) hatValue = 4;      // SE
        else if (s->dpadLeft) hatValue = 6;  // SW
        else hatValue = 5;                   // S
    } else if (s->dpadRight) {
        hatValue = 3;                        // E
    } else if (s->dpadLeft) {
        hatValue = 7;                        // W
    }
    
    r.hat = hatValue;
    
    // 3. Analógicos (16-bit signed -> 8-bit unsigned)
    r.lx = (uint8_t)((s->leftStickX >> 8) + 128);
    r.ly = (uint8_t)((~s->leftStickY >> 8) + 128); // Inverte Y
    r.rx = (uint8_t)((s->rightStickX >> 8) + 128);
    r.ry = (uint8_t)((~s->rightStickY >> 8) + 128); // Inverte Y
    
    // 4. Envia para o Sistema
    uint64_t timestamp = 0;
    GetKernelTimestamp(&timestamp);
    
    // CORREÇÃO: Uso explícito do kIOHIDReportTypeInput (0) caso a constante falhe
    handleReport(timestamp, (uint8_t*)&r, sizeof(r), kIOHIDReportTypeInput, 0);
}
