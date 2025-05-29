# Temperature and Humidity Logging System

### Introduction

---

This documentation provides a detailed guide to implementing a temperature and humidity logging system. Such a system is useful for labs or other storage facilities which may be housing machinery, equipments, or products(foods, medicine, etc.) that need to be stored in a specific temperature range.

We will use ESP32 board and a DHT22 sensor to record the data and a raspberry pi to store the data from all sensors in the proximity and visualise the results.

### Hardware Requirement

---

The following components were used for our implementation:

1. ESP32 dev board
2. Raspberry Pi 4 2gb
3. DHT22 Sensor
4. MicroSD Card 32 gb
5. A home wifi router

### Setup

1. The following guide is useful to connect the DHT22 sensor with your esp32 and understanding how to host a webserver: [ESP32 DHT22 Web Server](https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/ "Random Nerd Tutorial")

2. Flash(using arduino ide or platform IO) the code from `esp32Dht22.ino` to your ESP32 board after setting up the connection with DHT22. Make sure to switch `yourWifiSsid` and `yourWifiPassword` with your local wifi's credentials.

3. Now we will set up our raspberry pi to read the data from the esp32's web server. Start with using this guide to flash an OS to your pi and get it running: [Getting Started With Raspberry Pi](https://www.raspberrypi.com/documentation/computers/getting-started.html)
   
4. Connect your pi to a monitor, keyboard, and mouse or use VNC to access your pi through your local computer without additional components.

5. Connect your raspberry pi to your local wifi. It must have the same credentials as `yourWifiSsid` and `yourWifiPassword` as in Step 2. Your esp32 and pi must connect to the same network for our implementation to work, more about using your pi as a hotspot instead of home wifi in Optional Section.

6. Save the `log_sensor.py` file on your pi and replace `pathToCsv` in line 14 with path to your the csv file where you want to log the data.

7. If you have more than one esp to record data from more than one sensor then populate `espNodes` with the label you want to give to your esp and their mDNS address.

8. Once you run this script, your pi should be logging data from the sensors into a csv file in the following format: `time(YYYY-MM-DD HH:MM:SS) nodeName temperature humidity`