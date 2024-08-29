#!/bin/bash

# Check if the script is running with root privileges
if [[ $EUID -ne 0 ]]; then
   echo "Please run this script with root privileges!" 
   exit 1
fi

# Define the network interface name
INTERFACE="wlo1"

# Check the current WiFi mode
if iwconfig $INTERFACE | grep -q "Mode:Monitor"; then
    echo "Current WiFi mode is Monitor mode, switching to Managed mode..."
    
    # Restart NetworkManager service
    systemctl restart NetworkManager
    
    # Shut down the network interface
    ifconfig $INTERFACE down
    
    # Switch WiFi mode to Managed mode  
    iwconfig $INTERFACE mode managed
    
    # Enable the network interface
    ifconfig $INTERFACE up
    systemctl restart NetworkManager
    
    echo "Switched to Managed mode!"
else
    echo "Current WiFi mode is Managed mode, switching to Monitor mode..."
    
    # Stop NetworkManager service
    systemctl stop NetworkManager
    
    # Shut down the network interface 
    ifconfig $INTERFACE down
    
    # Switch WiFi mode to Monitor mode
    iwconfig $INTERFACE mode monitor
    
    # Enable the network interface
    ifconfig $INTERFACE up
    
    echo "Switched to Monitor mode!"  
fi
