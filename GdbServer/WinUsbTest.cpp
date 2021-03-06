// WinUsbTest.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <iostream>
#include "stm32DbgCommands.h"
#include "TargetInterface/WinUsbTargetInterface.h"

extern "C" void gdb_main(void);

WinUsbTargetInterfaceFabric targetFabric;

extern "C" size_t usbSendAndReceive(uint8_t* outBuf, size_t outBufSize, uint8_t* inBuf, size_t inBufSize) {
    size_t count = 0;
    do {
        count = targetFabric.getInterfaceCount();
        if (count == 0){
            targetFabric.updateInterfaces();
        }
    } while (0 == count);
    auto t = targetFabric.getInterface(count - 1);
    size_t txSize = t->writePacket(outBuf, outBufSize);
    if (txSize == 0) return 0;
    size_t rxSize = t->readPacket(inBuf, inBufSize);
    return rxSize;
}

#include "Target/TargetSTM32F1.h"
int main()
{   
    gdb_main();
    return  0;
}
