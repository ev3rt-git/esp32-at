# esp32-at-via-bluetooth

A fork of [espressif/esp32-at](https://github.com/espressif/esp32-at) to support using AT command through Bluetooth SPP (Serial Port Profile).

# Prerequisites

ESP-IDF is installed. [ESP32-IDF v3.1.4](https://github.com/espressif/esp-idf/releases/tag/v3.1.4) is strongly recommended.

# Usage

1. Clone the project
```
git clone --recursive https://github.com/ev3rt-git/esp32-at-via-bluetooth.git
```

2. Connect the ESP32 board to your PC

3. Run `make menuconfig` in the project folder 

In `Serial flasher config`, configure `Default serial port` for downloading  
In `Component config -> AT -> AT Bluetooth SPP settings`, you can change the device name and PIN code

4. Run `make flash monitor` to build the project and run on the ESP32 board

In the output messages, you can see the MAC address for Bluetooth like:
```
BT Addr: 12:34:56:78:ab:cd
```

# References

See [ESP-IDF Get Started](https://docs.espressif.com/projects/esp-idf/en/latest/get-started/index.html) for ESP-IDF installation.  
See [espressif/esp32-at](https://github.com/espressif/esp32-at) for other information about esp32-at.
