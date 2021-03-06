#pragma once

#include "TargetInterface.h"
#include <Windows.h>
#include <winusb.h>
#include <string>
#include <list>
#include <memory>

class WinUsbTargetInterface : public TargetInterface {
    static constexpr const uint8_t outEndpointId = 0x01;
    static constexpr const uint8_t inEndpointId = 0x81;
    static constexpr const size_t maxWinUsbPacketSize = 64;
public:
    const size_t getMaxPacketSize() { return maxWinUsbPacketSize; };
    WinUsbTargetInterface(class WinUsbTargetInterfaceFabric& fabric, std::wstring deviceName);
    ~WinUsbTargetInterface() final;
    size_t writePacket(uint8_t* out, size_t size) final;
    size_t readPacket(uint8_t* in, size_t size) final;
    size_t readSerial(uint8_t* in, size_t size) final;
private:
    class WinUsbTargetInterfaceFabric& _fabric;
    HANDLE _WinUsbDeviceHandle = INVALID_HANDLE_VALUE;
    WINUSB_INTERFACE_HANDLE _WinUsbIfaceHandle{};
};


class WinUsbTargetInterfaceFabric {
public:
    size_t getInterfaceCount();
    TargetInterface* getInterface(size_t index);
    void updateInterfaces();
    ~WinUsbTargetInterfaceFabric();
private:
    std::list<std::wstring> getWinUsbDeviceNameList();
    std::list<std::shared_ptr<WinUsbTargetInterface>> _targetList;
};

