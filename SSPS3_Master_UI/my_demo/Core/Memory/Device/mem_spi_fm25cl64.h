#pragma once
#ifndef MEM_I2C_FM25CL64_H
#define MEM_I2C_FM25CL64_H

#ifdef DEV_SSPS3_RUN_ON_PLC

#include "../../MCUsCommunication/Interface/i_arduino_spi.h"
#include "../Interface/i_read_write.h"

#else

#include "./i_arduino_spi.h"
#include "./i_read_write.h"

#endif

    class FM25CL64 : public IReadWrite
    {
    private:
        SPI_CH* _spi;
        int _cs_pin;
        bool _is_own_spi;
        SPI_CONF _spi_conf;

        static const constexpr uint8_t CMD_READ = 0x03;
        static const constexpr uint8_t CMD_WRITE = 0x02;

    public:
        FM25CL64(int mosi, int miso, int sck, int cs, int speed_hz = DEFAULT_SPI_FRQ)
            : _spi(nullptr),
            _cs_pin(cs),
            _is_own_spi(true),
            _spi_conf(speed_hz, MSBFIRST, SPI_MODE0)
        {
            _spi = new SPI_CH(HSPI);
            _spi->begin(sck, miso, mosi, cs);

            pinMode(_cs_pin, OUTPUT);
            digitalWrite(_cs_pin, HIGH);
        }

        FM25CL64(SPI_CH* _spi, int cs, int speed_hz = DEFAULT_SPI_FRQ)
            : _spi(_spi),
            _cs_pin(cs),
            _is_own_spi(false),
            _spi_conf(speed_hz, MSBFIRST, SPI_MODE0)
        {
            pinMode(_cs_pin, OUTPUT);
            digitalWrite(_cs_pin, HIGH);
        }

        ~FM25CL64()
        {
            if (_is_own_spi && _spi)
            {
                _spi->end();
                delete _spi;
                _spi = nullptr;
            }
        }

        virtual void fill(unsigned int addr, const unsigned char& filler, size_t size) override
        {
            _spi->beginTransaction(_spi_conf);
            digitalWrite(_cs_pin, LOW);

            _spi->transfer(CMD_WRITE);
            _spi->transfer((addr >> 8) & 0xFF);
            _spi->transfer(addr & 0xFF);

            for (size_t i = 0; i < size; ++i)
                _spi->transfer(filler);

            digitalWrite(_cs_pin, HIGH);
            _spi->endTransaction();
        }

        virtual bool read(unsigned int addr, unsigned char* data, size_t size, bool check_last_short_crc = false) override
        {
            _spi->beginTransaction(_spi_conf);
            digitalWrite(_cs_pin, LOW);

            _spi->transfer(CMD_READ);
            _spi->transfer((addr >> 8) & 0xFF);
            _spi->transfer(addr & 0xFF);

            for (size_t i = 0; i < size; ++i)
                data[i] = _spi->transfer(0x00);

            digitalWrite(_cs_pin, HIGH);
            _spi->endTransaction();

            return true;
        }

        virtual void write(unsigned int addr, const unsigned char* data, size_t size) override
        {
            _spi->beginTransaction(_spi_conf);
            digitalWrite(_cs_pin, LOW);

            _spi->transfer(CMD_WRITE);
            _spi->transfer((addr >> 8) & 0xFF);
            _spi->transfer(addr & 0xFF);

            for (size_t i = 0; i < size; ++i)
                _spi->transfer(data[i]);

            digitalWrite(_cs_pin, HIGH);
            _spi->endTransaction();
        }

        virtual char byte_read(unsigned int addr) override
        {
            unsigned char b = 0;
            read(addr, &b, 1);
            return b;
        }
    };

#endif // !MEM_I2C_FM25CL64_H
