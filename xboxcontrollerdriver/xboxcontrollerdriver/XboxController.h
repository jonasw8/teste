#ifndef XboxController_h
#define XboxController_h

#include <DriverKit/IOService.h>
#include <DriverKit/IOUserClient.h>
#include <DriverKit/IOBufferMemoryDescriptor.h>
#include <DriverKit/OSCollections.h>
#include <HIDDriverKit/IOUserHIDDevice.h>
#include <HIDDriverKit/IOHIDUsageTables.h>
#include <USBDriverKit/IOUSBHostDevice.h>
#include <USBDriverKit/IOUSBHostInterface.h>
#include <USBDriverKit/IOUSBHostPipe.h>

#define XBOX360_INPUT_ENDPOINT  0x81
#define XBOX360_OUTPUT_ENDPOINT 0x02
#define XBOX360_MAX_PACKET_SIZE 32

class XboxController : public IOUserHIDDevice {
    
    struct IVars {
        IOUSBHostDevice* usbDevice;
        IOUSBHostInterface* usbInterface;
        IOUSBHostPipe* inputPipe;
        IOUSBHostPipe* outputPipe;
        
        IOBufferMemoryDescriptor* hidReportBuffer;
        
        uint8_t  inputEndpointAddr;
        uint8_t  outputEndpointAddr;
        uint16_t maxPacketSize;
        uint32_t consecutiveErrors;
        
        struct {
            uint8_t data[32];
        } currentState;
    };
    
    IVars ivars;

public:
    virtual bool init();
    virtual void free();
    
    // Removido 'override' para evitar erros de assinatura estrita
    virtual kern_return_t Start(IOService* provider);
    virtual kern_return_t Stop(IOService* provider);
    
    virtual OSDictionary* newDeviceDescription();
    
    virtual kern_return_t handleReport(uint64_t timestamp,
                                     IOMemoryDescriptor* report,
                                     uint32_t reportType,
                                     IOOptionBits options);
    
    virtual kern_return_t newReportDescriptor(IOMemoryDescriptor** descriptor);

    void ProcessData(IOBufferMemoryDescriptor *dataBuffer);
};

#endif /* XboxController_h */
