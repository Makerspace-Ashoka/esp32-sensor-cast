import requests
import time

espNodes = {
    "esp1": "http://esp1.local/sensor"
}

while True:
    for node, url in esp_nodes.items():
        try:
            response = requests.get(url, timeout=5)
            if response.status_code == 200:
                data = response.json()
                with open(pathToCsv, "a") as f:
                    f.write(f"{time.strftime('%Y-%m-%d %H:%M:%S')},{node},{data['temperature']},{data['humidity']}\n")
                print("Logged", node, data)
            else:
                print("ESP32 error:", response.text)
        except Exception as e:
            print(f"Connection error with {node}:", e)
    time.sleep(15)
