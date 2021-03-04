# BluePillUsbDebugPrototyping
GDB debuggin and firmware upload of STM32F103C8T6 via USB.
- Bootloader represented itself as WinUSB device. No Windows drivers needed. 
- Simple test GDB server for windows
- Simple demo application.

This is concept prototyping project. It is really works, but this is not a real debugging system.
Bootloader for bluepill board (stm32f103c8t6) This is PROTOTYPING of onchip debugging of arm core. Flash size ~8K (16K reserved) Ram size 4K This project allows debug stm32f103c8t6 via USB. Bootloader supports upload firmware Bootloader supports breakpoint, stepping (but only on Thread not in Handlers) I know how to debug inside hanlders, but I think there is a good idea to make new project to do it.
