import time
import network
from machine import Pin

print("hello world!")
current_time = time.time()
print(current_time)

#formatted_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(current_time))
#print(formatted_time)

sta_if = network.WLAN(network.STA_IF); sta_if.active(True)
sta_if.scan()                             # Scan for available access points
sta_if.connect("uxiang.cn", "meiyoumima") # Connect to an AP
sta_if.isconnected()                      # Check for successful connection

pin2 = Pin(2, Pin.OUT)

while True:
    pin2.value(sta_if.isconnected())
    time.sleep(1)