# BluePillUsbDebugPrototyping
GDB debugging and firmware upload of STM32F103C8T6 via USB.
- Bootloader represented itself as WinUSB device. No Windows drivers needed. 
- Simple test GDB server for windows
- Simple demo application.

https://youtu.be/sllNL5L_Tb0
https://drive.google.com/file/d/17XqRzrK-bwF6581R2oCWMhu9Jscyt4ob/view?usp=sharing

This is proof of concept only project. It is really works. You can bulid and run GdbServer, STM32 bootloader and simple firmware. But this is not a real debugging system. This is PROTOTYPING of onchip debugging of arm core. Flash size ~8K (16K reserved) Ram size 4K This project allows debug stm32f103c8t6 via USB without SWD, but with GDB. Bootloader supports upload firmware, breakpoints, stepping.




