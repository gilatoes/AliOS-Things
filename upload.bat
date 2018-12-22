for /f "delims=" %%t in ('dir /A:-D /S /B out\*@esp32devkitc.bin') do set binPath=%%t

esptool.py --port COM8 --baud 921600 write_flash 0x1000 .\platform\mcu\esp32\bsp\bootloader.bin 0x8000 .\platform\mcu\esp32\bsp\custom_partitions.bin 0x10000 %binPath%