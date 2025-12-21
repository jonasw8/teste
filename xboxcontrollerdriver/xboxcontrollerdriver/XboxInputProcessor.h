#ifndef XboxInputProcessor_h
#define XboxInputProcessor_h

#include <stdint.h>
#include <stddef.h>

// Estrutura do relatório HID que será enviado ao sistema
struct XboxHIDReport {
    uint16_t buttons;      // 16 botões
    uint8_t leftX;         // Analógico esquerdo X (0-255)
    uint8_t leftY;         // Analógico esquerdo Y (0-255)
    uint8_t rightX;        // Analógico direito X (0-255)
    uint8_t rightY;        // Analógico direito Y (0-255)
    uint8_t leftTrigger;   // Gatilho esquerdo (0-255)
    uint8_t rightTrigger;  // Gatilho direito (0-255)
} __attribute__((packed));

// Mapeamento de botões Xbox 360
enum XboxButtons {
    XBOX_BTN_DPAD_UP    = 0x0001,
    XBOX_BTN_DPAD_DOWN  = 0x0002,
    XBOX_BTN_DPAD_LEFT  = 0x0004,
    XBOX_BTN_DPAD_RIGHT = 0x0008,
    XBOX_BTN_START      = 0x0010,
    XBOX_BTN_BACK       = 0x0020,
    XBOX_BTN_LS         = 0x0040,  // Left Stick Click
    XBOX_BTN_RS         = 0x0080,  // Right Stick Click
    XBOX_BTN_LB         = 0x0100,  // Left Bumper
    XBOX_BTN_RB         = 0x0200,  // Right Bumper
    XBOX_BTN_GUIDE      = 0x0400,  // Xbox Guide Button
    XBOX_BTN_A          = 0x1000,
    XBOX_BTN_B          = 0x2000,
    XBOX_BTN_X          = 0x4000,
    XBOX_BTN_Y          = 0x8000
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Processa dados brutos do Xbox 360 Controller e converte para relatório HID
 * 
 * @param xboxData Ponteiro para os dados brutos do Xbox (mínimo 20 bytes)
 * @param length Tamanho dos dados recebidos
 * @param report Ponteiro para a estrutura que receberá o relatório HID processado
 */
void ProcessXboxInput(const uint8_t* xboxData, size_t length, XboxHIDReport* report);

/**
 * Converte valor de eixo analógico de int16_t (-32768 a 32767) para uint8_t (0 a 255)
 * 
 * @param value Valor do eixo em formato int16_t
 * @return Valor convertido em formato uint8_t
 */
uint8_t ConvertAxis(int16_t value);

#ifdef __cplusplus
}
#endif

#endif /* XboxInputProcessor_h */
