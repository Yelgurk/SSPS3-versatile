#pragma once
#ifndef I_ARDUINO_SPI_H
#define I_ARDUINO_SPI_H

#define DEFAULT_SPI_FRQ 20000000
#define MSBFIRST        1
#define SPI_MODE0       0
#define FSPI            0
#define HSPI            1

#ifdef DEV_SSPS3_RUN_ON_PLC

#include "./i_arduino_io.h"
#include <SPI.h>

typedef SPISettings     SPI_CONF;
typedef SPIClass        SPI_CH;

#else

#include "./i_arduino_io.h"

class IArduinoSPIConf
{
public:
    IArduinoSPIConf(uint32_t clock, uint8_t bitOrder, uint8_t dataMode)
    {}
};

class IArduinoSPI
{
public:
    IArduinoSPI(int spi_port) {}

    void begin(int sck = -1, int miso = -1, int mosi = -1, int ss = -1) {}

    void beginTransaction(IArduinoSPIConf settings) {}

    uint8_t transfer(uint8_t data) {}

    void endTransaction() {}

    void end() {}
};

typedef IArduinoSPIConf SPI_CONF;
typedef IArduinoSPI     SPI_CH;
SPI_CH SPI;

#endif // !DEV_SSPS3_RUN_ON_PLC

#endif // !I_ARDUINO_SPI_H
