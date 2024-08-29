# 802.11 Raw Frame Communication Project

This project demonstrates low-level 802.11 frame communication between Linux systems and ESP8266 devices. It consists of two main components: Linux 802.11 send/receive programs and ESP8266 firmware for raw frame transmission and reception.

## Linux 802.11 Send/Receive

### Prerequisites
- A Linux system with a wireless network interface that supports monitor mode
- Root access to the system

### Setup
1. Clone this repository:
   ```
   git clone https://github.com/tianrking/802.11-raw-frame.git
   cd 802.11-raw-frame-project
   ```

2. Compile the sender and receiver programs:
   ```
   gcc -o sender linux_802.11_sender.c
   gcc -o receiver linux_802.11_receiver.c
   ```

### Usage
1. Set your wireless interface to monitor mode:
   ```
   sudo ifconfig <interface> down
   sudo iwconfig <interface> mode monitor
   sudo ifconfig <interface> up
   ```
   Replace `<interface>` with your wireless interface name (e.g., wlan0).

2. Run the receiver:
   ```
   sudo ./receiver <interface>
   ```

3. Run the sender:
   ```
   sudo ./sender <interface> "Your message here"
   ```

### Advanced Filtering with grep
You can use grep to filter the output of the receiver for specific patterns. Here are some useful examples:

1. Filter for packets containing a specific string:
   ```
   sudo ./receiver wlan0 | grep "Hello World"
   ```

2. Display packets with a specific length:
   ```
   sudo ./receiver wlan0 | grep -A 7 "Received packet (64 bytes):"
   ```

3. Show only the payload of received packets:
   ```
   sudo ./receiver wlan0 | grep "Payload (ASCII):" -A 1
   ```

4. Filter for packets from a specific MAC address:
   ```
   sudo ./receiver wlan0 | grep -E "SA:([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}"
   ```

5. Display packets containing hexadecimal pattern:
   ```
   sudo ./receiver wlan0 | grep -E "([0-9A-Fa-f]{2} ){16}"
   ```

6. Combine multiple filters:
   ```
   sudo ./receiver wlan0 | grep -E "Received packet \(64 bytes\)|Payload \(ASCII\):"
   ```

## ESP8266 Development

### Prerequisites
- ESP8266 RTOS SDK
- Xtensa LX106 toolchain

### Setup ESP8266 RTOS SDK
1. Download the ESP8266 RTOS SDK:
   ```
   git clone https://github.com/espressif/ESP8266_RTOS_SDK.git
   ```

2. Set the `IDF_PATH` environment variable:
   ```
   export IDF_PATH=~/path/to/ESP8266_RTOS_SDK
   ```
   Add this line to your `~/.bashrc` or `~/.profile` for persistence.

### Toolchain Setup
1. Download the Xtensa LX106 toolchain:
   - For 64-bit Linux:
     ```
     https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
     ```
   - For 32-bit Linux:
     ```
     https://dl.espressif.com/dl/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-i686.tar.gz
     ```

2. Extract the toolchain:
   ```
   mkdir -p ~/esp
   cd ~/esp
   tar -xzf ~/Downloads/xtensa-lx106-elf-gcc8_4_0-esp-2020r3-linux-amd64.tar.gz
   ```

3. Add the toolchain to your PATH:
   ```
   export PATH="$PATH:$HOME/esp/xtensa-lx106-elf/bin"
   ```
   Add this line to your `~/.bashrc` or `~/.profile` for persistence.

### Building and Flashing
1. Navigate to the ESP8266 project directory:
   ```
   cd esp8266_project
   ```

2. Build the project:
   ```
   make
   ```

3. Flash the firmware:
   ```
   make flash
   ```

## Contributing
Contributions to this project are welcome. Please submit pull requests or open issues for any improvements or bug fixes.

## License
