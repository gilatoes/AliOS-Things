# AliOS Things with ESP32-DekitC and OPTIGA Trust X

This Github repository works on ESP32-DevKitC with AliOS Things release 2.0.0. Infineon Trust X device is installed as a I2C hardware cryptographic device within AliOS Things OS.

## Getting Started

Refer to AliOS Things https://github.com/alibaba/AliOS-Things
Refer to AliOS Things documentation https://github.com/alibaba/AliOS-Things/wiki

## Overview of Trust X driver and AliOS Things integration

AliOS Things adapts layered and component architecture.

Trust X driver is installed as a I2C Security device within the Security layer.
From the top-most layer, the application example known as HelloTrustX is a sample application that calls the Trust X security service. The security service uses the AOS API to access Rhino RTOS kernel services. Within the kernel, Trust X I2C device is exposed as a I2C VFS (Virtual File System). The VFS service interface allows Trust X I2C slave to be read/write in fashion similar to the filesystem. Lastly, the Trust X PAL layer is integrated into the AliOS Things HAL(Hardware Abstraction Layer) layer which establish communicates with Trust X physical hardware.   

## Hardware configuration and setup

Trust X is connected to ESP32-DevKitC power supply, I2C interface and GPIO for software reset.

|ESP32 | Trust X Signal Name|
|---|:---:|
|GPIO 22(IO22) | SCL |
|GPIO 21(IO21) | SDA |
|GPIO 18(IO18)| Reset |
|3.3V | VCC |
|GND | GND |

Ensures that <AliOS-Things>\platform\mcu\esp32\hal\i2c.c uses the matching I2C configuration
```
static i2c_resource_t g_dev[I2C_NUM_MAX] = {
	{(volatile i2c_dev_t *)0x60013000,22,21},
  {(volatile i2c_dev_t *)0x60027000,19,18}
};
```

## ESP32 Hardware schematic and PCB design
https://easyeda.com/gilatoes/esp32-with-trust-x

## Software setup

AliOS Things SDK supports multiple environment such as Windows and Linux. For this GitHub repository, the focus is only on Windows environment with Visual Studio Code.

Refer to AliOS Things Studio setup:
https://github.com/alibaba/AliOS-Things/wiki/AliOS-Things-Studio

## Additional configuration settings
## Configuration of Intellisense for Visual Studio in c_cpp_properties.json
```
{
    "configurations": [
        {
            "name": "Win32",
            "includePath": [
                "C:\\Users\\<user>\\<path>\\AliOS-Things\\**"
            ],
            "forcedInclude": [],
            "intelliSenseMode": "msvc-x64"
        }
    ],
    "version": 4
}
```


## ESP32 Upload configuration
<code_base>\build\site_scons\upload\esp32.json

Change @PORT@ to esp32 COM#.

```
{
    "cmd": [
        "esptool.py",
        "--chip",
        "esp32",
        "--port",
        "@PORT@",
        "--baud",
        "921600",
        "--before",
        "default_reset",
        "--after",
        "hard_reset",
        "write_flash",
        "-z",
        "--flash_mode",
        "dio",
        "--flash_freq",
        "40m",
        "--flash_size",
        "detect",
        "0x1000",
        "@AOSROOT@/platform/mcu/esp32/bsp/bootloader.bin",
        "0x8000",
        "@AOSROOT@/platform/mcu/esp32/bsp/custom_partitions.bin",
        "0x10000",
        "@AOSROOT@/out/@TARGET@/binary/@TARGET@.bin"
    ]
}
```

## Sample Output:
```
> Executing task in folder AliOS-Things: aos monitor COM8 115200 <

--- Miniterm on COM8  115200,8,N,1 ---
--- Quit: Ctrl+] | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H ---
ets Jun  8 2016 00:22:57

rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
ets Jun  8 2016 00:22:57

rst:0x10 (RTCWDT_RTC_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)
configsip: 0, SPIWP:0xee
clk_drv:0x00,q_drv:0x00,d_drv:0x00,cs0_drv:0x00,hd_drv:0x00,wp_drv:0x00
mode:DIO, clock div:2
load:0x3fff0010,len:4
load:0x3fff0014,len:5568
ho 0 tail 12 room 4
load:0x40078000,len:0
load:0x40078000,len:12484
entry 0x40078f74
␛[0;33mW (73) rtc_clk: Possibly invalid CONFIG_ESP32_XTAL_FREQ setting (40MHz). Detected 40 MHz.␛[0m
␛[0;32mI (39) boot: ESP-IDF v3.0-dev-960-g6cc8099 2nd stage bootloader␛[0m
␛[0;32mI (39) boot: compile time 15:55:51␛[0m
␛[0;32mI (39) boot: Enabling RNG early entropy source...␛[0m
␛[0;32mI (45) boot: SPI Speed      : 40MHz␛[0m
␛[0;32mI (49) boot: SPI Mode       : DIO␛[0m
␛[0;32mI (53) boot: SPI Flash Size : 4MB␛[0m
␛[0;32mI (57) boot: Partition Table:␛[0m
␛[0;32mI (61) boot: ## Label            Usage          Type ST Offset   Length␛[0m
␛[0;32mI (68) boot:  0 nvs              WiFi data        01 02 00009000 00004000␛[0m
␛[0;32mI (75) boot:  1 otadata          OTA data         01 00 0000d000 00002000␛[0m
␛[0;32mI (83) boot:  2 phy_init         RF data          01 01 0000f000 00001000␛[0m
␛[0;32mI (90) boot:  3 factory          factory app      00 00 00010000 00100000␛[0m
␛[0;32mI (98) boot:  4 ota_0            OTA app          00 10 00110000 00100000␛[0m
␛[0;32mI (105) boot:  5 ota_1            OTA app          00 11 00210000 00100000␛[0m
␛[0;32mI (113) boot:  6 parameter1       unknown          40 00 00310000 00001000␛[0m
␛[0;32mI (120) boot:  7 parameter2       unknown          40 00 00311000 00002000␛[0m
␛[0;32mI (128) boot:  8 parameter3       unknown          40 00 00313000 00001000␛[0m
␛[0;32mI (135) boot:  9 parameter4       unknown          40 00 00314000 00001000␛[0m
␛[0;32mI (143) boot: End of partition table␛[0m
␛[0;32mI (147) boot: Defaulting to factory image␛[0m
␛[0;32mI (152) esp_image: segment 0: paddr=0x00010020 vaddr=0x3f400020 size=0x0cb70 ( 52080) map␛[0m
␛[0;32mI (179) esp_image: segment 1: paddr=0x0001cb98 vaddr=0x3ffb0000 size=0x025c4 (  9668) load␛[0m
␛[0;32mI (183) esp_image: segment 2: paddr=0x0001f164 vaddr=0x40080000 size=0x00400 (  1024) load␛[0m
␛[0;32mI (186) esp_image: segment 3: paddr=0x0001f56c vaddr=0x40080400 size=0x00aa4 (  2724) load␛[0m
␛[0;32mI (196) esp_image: segment 4: paddr=0x00020018 vaddr=0x400d0018 size=0x47d98 (294296) map␛[0m
␛[0;32mI (305) esp_image: segment 5: paddr=0x00067db8 vaddr=0x40080ea4 size=0x1335c ( 78684) load␛[0m
␛[0;32mI (338) esp_image: segment 6: paddr=0x0007b11c vaddr=0x400c0000 size=0x00000 (     0) load␛[0m
␛[0;32mI (350) boot: Loaded app from partition at offset 0x10000␛[0m
␛[0;32mI (350) boot: Disabling RNG early entropy source...␛[0m
␛[0;32mI (1079) cpu_start: Pro cpu up.␛[0m
␛[0;32mI (1090) cpu_start: Single core mode␛[0m
␛[0;32mI (1105) cpu_start: Pro cpu start user code␛[0m
␛[0;32mI (1159) cpu_start: Starting scheduler on PRO CPU.␛[0m
wifi_init:346 0
I (1197) wifi: wifi firmware version: 803c265
I (1197) wifi: config NVS flash: enabled
I (1197) wifi: config nano formating: disabled
␛[0;32mI (1197) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE␛[0m
␛[0;32mI (1207) system_api: Base MAC address is not set, read default base MAC address from BLK0 of EFUSE␛[0m
I (1227) wifi: Init dynamic tx buffer num: 32
I (1227) wifi: Init data frame dynamic rx buffer num: 32
I (1227) wifi: Init management frame dynamic rx buffer num: 32
I (1237) wifi: wifi driver task: 3fff1e08, prio:23, stack:4096
I (1237) wifi: Init static rx buffer num: 10
I (1247) wifi: Init dynamic rx buffer num: 32
I (1247) wifi: Init rx ampdu len mblock:7
I (1247) wifi: Init lldesc rx ampdu entry mblock:4
I (1257) wifi: wifi power manager task: 0x3fff71e0 prio: 21 stack: 2560
trace config close!!!
[000070]<V> aos framework init.
****************************************************
Trust X AliOS Things Driver version: Ver 1.50.1153
Compiled time: Feb  6 2019 16:23:13
Task name: main
****************************************************
␛[0;32mI (1287) gpio: GPIO[18]| InputEn: 0| OutputEn: 1| OpenDrain: 0| Pullup: 0| Pulldown: 0| Intr:0 ␛[0m
Trust X UID:
CIM Identifier:         CD
Platform Identifier:    16
Model Identifier:       33
ID of ROM mask:         56 01
Chip type:              00 1C 00 05 00 00
Batch Number:           0A 07 84 D7 00 05
Position X:             00 90
Position Y:             00 94
Firmware Identifier:    80 10 10 70
ESW Build number:       10 48
```

# Documentation
AliOS Things' documentation are mainly hosted at [Github Wiki](https://github.com/alibaba/AliOS-Things/wiki).
