import utime
import network
from machine import Pin
import ntptime

# 获取当前时间戳
now = utime.localtime()
# 格式化输出
formatted_time = "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}".format(*now)
print(f"[{formatted_time}] Hello World, I'm play.")

led = Pin(2, Pin.OUT)
led.value(False)

# 连接到WiFi
sta_if = network.WLAN(network.STA_IF)
# 检查网络连接状态
led.value(sta_if.isconnected())

if not sta_if.isconnected():
    sta_if.connect("uxiang.cn", "meiyoumima")  # Connect to an AP

while not sta_if.isconnected():
    led.value(not led.value())
    utime.sleep(1)

# 通过NTP同步时间
ntptime.settime()

# 获取当前时间戳
now = utime.localtime()
# 格式化输出
formatted_time = "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}".format(*now)
print(f"[{formatted_time}] Goodbye.\n\t{sta_if.ifconfig()}")