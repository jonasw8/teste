//
//  XboxController.h
//  XboxControllerDriverKit
//
//  Versão corrigida para Xbox 360 Wireless apenas
//

#ifndef XboxController_h
#define XboxController_h

#include <DriverKit/DriverKit.h>
#include <USBDriverKit/USBDriverKit.h>
#include <HIDDriverKit/HIDDriverKit.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>

// Estrutura de estado do controle Xbox 360
struct XboxControllerState {
    bool buttonA, buttonB, buttonX, buttonY;
    bool buttonLB, buttonRB;
    bool buttonBack, buttonStart, buttonGuide;
    bool buttonLS, buttonRS;
    bool dpadUp, dpadDown, dpadLeft, dpadRight;
    int16_t leftStickX, leftStickY;
    int16_t rightStickX, rightStickY;
    uint8_t leftTrigger, rightTrigger;
};

class XboxController : public IOUserHIDDevice
{
public:
    virtual bool init() override;
    virtual kern_return_t Start(IOService* provider) override;
    virtual kern_return_t Stop(IOService* provider) override;
    virtual void free() override;
    
    // HID Device Methods
    virtual OSDictionary* newDeviceDescription() override;
    virtual OSData* newReportDescriptor() override;
    
    // USB Communication
    kern_return_t SetupUSBCommunication();
    kern_return_t FindEndpoints();
    kern_return_t StartReading();
    kern_return_t SendInitCommand();
    
    // Callbacks
    void HandleReadComplete(IOReturn status, uint32_t bytesTransferred);
    
    // Input Processing
    void ProcessInput(const uint8_t* data, size_t length);
    void UpdateHIDReport(XboxControllerState* state);
    
private:
    IOUSBHostDevice* usbDevice;
    IOUSBHostInterface* usbInterface;
    IOUSBHostPipe* inputPipe;
    IOUSBHostPipe* outputPipe;
    IOBufferMemoryDescriptor* inputBuffer;
    IOBufferMemoryDescriptor* reportBuffer;
    
    uint8_t inputEndpointAddr;
    uint8_t outputEndpointAddr;
    uint16_t maxPacketSize;
    
    XboxControllerState currentState;
};

#endif /* XboxController_h */
