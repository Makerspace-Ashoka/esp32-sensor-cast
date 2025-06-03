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

8. Once you run this script, your pi should be logging data from the sensors into a csv file in the following format: `time(YYYY-MM-DD HH:MM:SS) nodeName temperature humidity`. The logging is done every 15 seconds, this can be modified by changing the argument x in `time.sleep(x)` present on line 21 in `log_sensor.py`.

This finishes a basic implementaion of our logging system where once you power on your ESP32 circuit and run the script on your Raspberry Pi, it starts logging the data from DHT22 sensor into your code.

### Automating the logging script on the Raspberry Pi

Currently the logging starts only if you run the `log_sensor.py` script manually after powering on the pi but we want a more robust system which will automatically start logging as soon as it is powered on. This makes our implementation less susceptible to losing data after power cuts.

We can make this script run automatically by turning it into a system service unit(basically a program that always keeps running on the machine in the background) as follows:

1. Open terminal on your raspberry pi and type the following command:

    `chmod +x path_to_your_log_sensor_file`

    This adds the execute permission to the `log_sensor.py` file, the file can now directly be run from the terminal if the interpreter is present.


2. Create a systemmd service file using the following command in the terminal:

    `sudo nano /etc/systemd/system/log_sensor.service
`

    This opens a text editor window in the terminal. Paste the following contents using `Ctrl+Shift+V`:

    ```
    [Unit]
    Description=ESP32 Sensor Logger
    After=network-online.target
    Wants=network-online.target

    [Service]
    ExecStart=/usr/bin/python3 path_to_your_log_sensor_file
    WorkingDirectory=/home/yourUserName/
    StandardOutput=journal
    StandardError=journal
    Restart=on-failure
    User=yourUserName
    Environment=PYTHONUNBUFFERED=1

    [Install]
    WantedBy=multi-user.target
    ```

    Ensure that you enter the correct path to your log_sensor.py file and replace yourUserName with your user name on the raspberry pi in the above contents.


3. Enable the service file using the following commands in your pi terminal:

    ```sudo systemctl daemon-reexec       # ensures all changes are read
    sudo systemctl daemon-reload
    sudo systemctl enable sensor-logger.service
    sudo systemctl start sensor-logger.service
    ```

4. The file should now be automatically running in the background as soon as your pi starts. It does not require you to manually run the script to log the data anymore.

5. You can check the status of the service and view its logs using the following two commands in the terminal:

    ```
    sudo systemctl status sensor-logger.service # Check Status
    journalctl -u sensor-logger.service -f # View Logs
    ```

6. To confirm that it is working, you can reboot your pi and run:

    ```
    cat path_to_your_log_csv_file
    ```

    You should now be seeing new entries being added every 15 seconds.

### Grafana Server

Serving data to grafana using: 

```
cd /path/to/your/csv
python3 -m http.server 8080
```

### Version 2

Now that we have a basic implementation of the system running. We will look at storing the data on the esp32 itself as well in addition to sending it to the raspberry pi, this is useful in cases where the network might be facing some errors.

The approach is now as follows:

- The ESP32 will read the sensor repeatedly after a fixed time interval.

- If it is connected to a Grafana Server running on the Pi then we will use websocket to immediately share the data with the Grafana Server.

- The data will also be stored on the local memory of the ESP32 using LittleFS.

- When the memory of the esp32 gets about 80% full with the data, we will write all the data to a SD card connected to the esp32 and clear the memory of the esp32. This allows us to permanently store to the SD card, we do not write to SD card directly every time we read to reduce the total number of write operations and chances of memory corruption.
