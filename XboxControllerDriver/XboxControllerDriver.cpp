//
//  XboxControllerDriver.iig
//  XboxControllerDriver
//
//  Created by jonas on 14/12/25.
//

#ifndef XboxControllerDriver_h
#define XboxControllerDriver_h

#include <Availability.h>
#include <DriverKit/IOService.iig>

class XboxControllerDriver: public IOService
{
public:
    virtual kern_return_t
    Start(IOService * provider) override;
};

#endif /* XboxControllerDriver_h */
