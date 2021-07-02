@echo off

SET RAMN_PORT=COM3

SET STM32PROG_PATH="C:\Program Files\STMicroelectronics\STM32Cube\STM32CubeProgrammer\bin\STM32_Programmer_CLI.exe"
SET ECU_FIRMWARE_PATH=..\..\firmware\ECUA.hex


python ECUA_goToDFU.py %RAMN_PORT%

timeout /t 2 > NUL

%STM32PROG_PATH% -c port=usb1 pid=0xdf11 vid=0x0483

%STM32PROG_PATH% -c port=usb1 pid=0xdf11 vid=0x0483 -d %ECU_FIRMWARE_PATH% --verify --start 0x08000000 

timeout /t 2 > NUL
