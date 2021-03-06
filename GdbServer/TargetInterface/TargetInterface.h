#pragma once

#include <cstdint>
#include <cstddef>

struct TargetInterface {
    virtual const size_t getMaxPacketSize() = 0;
    virtual size_t writePacket(uint8_t* out, size_t size) = 0;
    virtual size_t readPacket(uint8_t* in, size_t size) = 0;
    virtual size_t readSerial(uint8_t* in, size_t size) = 0;
    virtual ~TargetInterface() = default;
};

