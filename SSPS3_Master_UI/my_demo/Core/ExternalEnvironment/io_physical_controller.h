#pragma once
#ifndef IO_PHYSICAL_CONTROLLER_H
#define IO_PHYSICAL_CONTROLLER_H

#include <Arduino.h>
#include "./io_physical_controller_core.h"

class IOPhysicalController
{
private:
    IOPhysicalController() {
        IOPhysicalControllerCore::instance();
    }

public:
};

#endif // !IO_PHYSICAL_CONTROLLER_H