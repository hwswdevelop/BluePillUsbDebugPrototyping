#include "WinUsbTargetInterface.h"
#include <SetupAPI.h>
#include <winusb.h>
#include <Usb100.h>


DEFINE_GUID(WinUSB_GUID, 0x88bae032, 0x5a81, 0x49f0, 0xbc, 0x3d, 0xa4, 0xff, 0x13, 0x82, 0x16, 0xd6);

#pragma comment(lib, "SetupAPI.lib")
#pragma comment(lib, "WinUsb.lib")
#pragma comment(lib, "Ws2_32.lib")


WinUsbTargetInterface::WinUsbTargetInterface(class WinUsbTargetInterfaceFabric& fabric, std::wstring deviceName) : _fabric(fabric) {
    const WCHAR* name = deviceName.c_str();
    _WinUsbDeviceHandle = CreateFileW(
        name,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_FLAG_OVERLAPPED,
        NULL);

    if (_WinUsbDeviceHandle == INVALID_HANDLE_VALUE) return;

    BOOL usbDeviceOpened = WinUsb_Initialize(_WinUsbDeviceHandle, &_WinUsbIfaceHandle);
    if (!usbDeviceOpened) {
        CloseHandle(_WinUsbDeviceHandle);
        _WinUsbDeviceHandle = INVALID_HANDLE_VALUE;
    }

}

WinUsbTargetInterface::~WinUsbTargetInterface() {
    if (_WinUsbDeviceHandle != INVALID_HANDLE_VALUE) {
        //_WinUsbIfaceHandle
        WinUsb_Free(_WinUsbIfaceHandle);
        CloseHandle(_WinUsbDeviceHandle);
        _WinUsbDeviceHandle = INVALID_HANDLE_VALUE;
    }
}

size_t WinUsbTargetInterface::writePacket(uint8_t* out, size_t size) {
    ULONG txSize = size;
    BOOL isSent = WinUsb_WritePipe(_WinUsbIfaceHandle, outEndpointId, out, txSize, &txSize, NULL);
    return isSent ? txSize : 0;
}

size_t WinUsbTargetInterface::readPacket(uint8_t* in, size_t size) {
    ULONG rxSize = size;
    BOOL isReceived = WinUsb_ReadPipe(_WinUsbIfaceHandle, inEndpointId, in, rxSize, &rxSize, NULL);
    return isReceived ? rxSize : 0;
}

size_t WinUsbTargetInterface::readSerial(uint8_t* in, size_t size) {
    ULONG rxSize = size;
    BOOL isReceived = WinUsb_ReadPipe(_WinUsbIfaceHandle, 0x82, in, rxSize, &rxSize, NULL);
    return isReceived ? rxSize : 0;
}


size_t WinUsbTargetInterfaceFabric::getInterfaceCount() {
    return _targetList.size();
}

TargetInterface* WinUsbTargetInterfaceFabric::getInterface(size_t index) {
    size_t searchIndex = 0;
    if (index >= _targetList.size()) return nullptr;
    for (auto it = _targetList.begin(); it != _targetList.end(); ++it) {
        if (index == searchIndex) {
            return it->get();
        }
        searchIndex++;
    }
    return nullptr;
}

void WinUsbTargetInterfaceFabric::updateInterfaces() {
    _targetList.clear();
    std::list<std::wstring> devNames = getWinUsbDeviceNameList();
    for (auto devName : devNames) {
        auto winUsbDevice = std::make_shared<WinUsbTargetInterface>(*this, devName);
        _targetList.emplace_back(winUsbDevice);
    }
}

std::list<std::wstring> WinUsbTargetInterfaceFabric::getWinUsbDeviceNameList() {
    static constexpr const size_t MaxStringSize = 2048;
    std::list<std::wstring> result;
    HANDLE devInfoHandle = SetupDiGetClassDevs(&WinUSB_GUID, NULL, NULL, DIGCF_PRESENT);
    if (devInfoHandle == nullptr) return result;
    SP_DEVINFO_DATA deviceInfo;
    SP_INTERFACE_DEVICE_DATA deviceData;
    deviceInfo.cbSize = sizeof(deviceInfo);
    deviceData.cbSize = sizeof(deviceData);
    int usbIfaceIndex = 0;
    while (SetupDiEnumDeviceInfo(devInfoHandle, usbIfaceIndex, &deviceInfo))
    {
        usbIfaceIndex++;
        WCHAR regKeyName[MaxStringSize];
        WCHAR regKeyValue[MaxStringSize];

        HKEY hRegKey = SetupDiOpenDevRegKey(devInfoHandle, &deviceInfo, DICS_FLAG_GLOBAL, 0, DIREG_DEV, KEY_QUERY_VALUE);
        if (hRegKey == INVALID_HANDLE_VALUE) continue;

        for (DWORD regPairIndex = 0; ; regPairIndex++) {
            DWORD nValueNameLen = MaxStringSize;
            DWORD nValueLen = MaxStringSize;
            DWORD dwType = 0;

            LSTATUS keyAccessStatus = RegEnumValueW(hRegKey, regPairIndex,
                regKeyName, &nValueNameLen,
                NULL, &dwType,
                (LPBYTE)regKeyValue, &nValueLen);
            if (keyAccessStatus != 0) break;

            if (wcsncmp(L"SymbolicName", regKeyName, 12) == 0) {
                std::wstring str(regKeyValue);
                result.push_back(str);
            }
        }
        RegCloseKey(hRegKey);
    }
    SetupDiDestroyDeviceInfoList(devInfoHandle);
    return result;
}

WinUsbTargetInterfaceFabric::~WinUsbTargetInterfaceFabric() {
    _targetList.clear();
}
