#include "XboxInputProcessor.h"
#include <string.h> // Para memset

// Função auxiliar para converter eixo (-32768 a 32767) -> (0 a 255)
uint8_t ConvertAxis(int16_t value) {
    // Adiciona o offset para tornar positivo e divide por 256 (shift right 8)
    // -32768 + 32768 = 0
    // 0 + 32768 = 32768 (>>8 = 128)
    // 32767 + 32768 = 65535 (>>8 = 255)
    int32_t temp = value;
    temp += 32768;
    return (uint8_t)(temp >> 8);
}

void ProcessXboxInput(const uint8_t* xboxData, size_t length, XboxHIDReport* report) {
    if (xboxData == NULL || report == NULL || length < 14) {
        return;
    }

    // Limpa o relatório antes de preencher
    memset(report, 0, sizeof(XboxHIDReport));

    // Mapeamento baseado no protocolo USB padrão do Xbox 360 (Wired/Wireless receiver)
    // Byte 0: Message Type (0x00)
    // Byte 1: Length (0x14)
    // Byte 2: Buttons (D-Pad, Start, Back, Stick Clicks)
    // Byte 3: Buttons (A, B, X, Y, LB, RB, Guide)
    // Byte 4: Left Trigger (0-255)
    // Byte 5: Right Trigger (0-255)
    // Byte 6-7: Left Stick X
    // Byte 8-9: Left Stick Y
    // Byte 10-11: Right Stick X
    // Byte 12-13: Right Stick Y

    // --- Botões ---
    uint16_t buttons = 0;
    
    // Byte 2
    if (xboxData[2] & 0x01) buttons |= XBOX_BTN_DPAD_UP;
    if (xboxData[2] & 0x02) buttons |= XBOX_BTN_DPAD_DOWN;
    if (xboxData[2] & 0x04) buttons |= XBOX_BTN_DPAD_LEFT;
    if (xboxData[2] & 0x08) buttons |= XBOX_BTN_DPAD_RIGHT;
    if (xboxData[2] & 0x10) buttons |= XBOX_BTN_START;
    if (xboxData[2] & 0x20) buttons |= XBOX_BTN_BACK;
    if (xboxData[2] & 0x40) buttons |= XBOX_BTN_LS;
    if (xboxData[2] & 0x80) buttons |= XBOX_BTN_RS;

    // Byte 3
    if (xboxData[3] & 0x01) buttons |= XBOX_BTN_LB;
    if (xboxData[3] & 0x02) buttons |= XBOX_BTN_RB;
    if (xboxData[3] & 0x04) buttons |= XBOX_BTN_GUIDE; // Às vezes não mapeado padrão
    if (xboxData[3] & 0x10) buttons |= XBOX_BTN_A;
    if (xboxData[3] & 0x20) buttons |= XBOX_BTN_B;
    if (xboxData[3] & 0x40) buttons |= XBOX_BTN_X;
    if (xboxData[3] & 0x80) buttons |= XBOX_BTN_Y;

    report->buttons = buttons;

    // --- Gatilhos (Triggers) ---
    // Já vêm como 0-255 no protocolo USB
    report->leftTrigger = xboxData[4];
    report->rightTrigger = xboxData[5];

    // --- Analógicos (Joysticks) ---
    // Vêm como signed 16-bit little endian
    int16_t lx = (int16_t)(xboxData[6] | (xboxData[7] << 8));
    int16_t ly = (int16_t)(xboxData[8] | (xboxData[9] << 8));
    int16_t rx = (int16_t)(xboxData[10] | (xboxData[11] << 8));
    int16_t ry = (int16_t)(xboxData[12] | (xboxData[13] << 8));

    // O eixo Y do Xbox é invertido em relação a alguns padrões HID, mas vamos manter raw
    // Inverte o Y se necessário: ly = ~ly;
    
    report->leftX = ConvertAxis(lx);
    report->leftY = ConvertAxis(ly); // DriverKit HID geralmente espera 0=Top, 255=Bottom
    report->rightX = ConvertAxis(rx);
    report->rightY = ConvertAxis(ry);
}
