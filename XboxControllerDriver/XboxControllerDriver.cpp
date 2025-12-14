//
//  XboxControllerDriver.cpp
//  XboxControllerDriver
//
//  Created by jonas on 14/12/25.
//

#include <os/log.h>

#include <DriverKit/IOUserServer.h>
#include <DriverKit/IOLib.h>

#include "XboxControllerDriver.h"

kern_return_t
IMPL(XboxControllerDriver, Start)
{
    kern_return_t ret;
    ret = Start(provider, SUPERDISPATCH);
    os_log(OS_LOG_DEFAULT, "Hello World");
    return ret;
}
