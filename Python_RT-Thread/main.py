import utime
import network
from machine import Pin
import ntptime
import urequests
import ure

def extract_title(html):
    # 正则表达式模式，匹配<title>标签及其内容
    pattern = r"<title>(.*?)</title>"
    
    # 使用re.search查找匹配的内容
    match = ure.search(pattern, html)
    
    if match:
        # 返回匹配的内容（去除标签）
        return match.group(1)
    else:
        # 如果没有找到匹配，返回None
        return None

def stream_html(url, chunk_size=1024):
    response = urequests.get(url, stream=True)
    html_content = ""
    while True:
        chunk = response.raw.read(chunk_size)
        if not chunk:
            break
        html_content += chunk
        # 检查是否已经包含<title>标签
        if "<title>" in html_content and "</title>" in html_content:
            break
    response.close()
    del response  # 释放HTTP响应对象
    return html_content

# 获取当前时间戳并格式化输出
def get_formatted_time():
    now = utime.localtime()
    return "{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}".format(*now)

print(f"[{get_formatted_time()}] Hello World, I'm play.")

led = Pin(2, Pin.OUT)
led.value(False)

# 连接到WiFi
sta_if = network.WLAN(network.STA_IF)
led.value(sta_if.isconnected())

if not sta_if.isconnected():
    sta_if.connect("uxiang.cn", "meiyoumima")  # Connect to an AP

while not sta_if.isconnected():
    led.value(not led.value())
    utime.sleep(1)

# 通过NTP同步时间
#ntptime.settime()

# 访问网页
# url = "https://coffeedrunk.cn"
# html = stream_html(url, 512)
# title = extract_title(html)
# print(title)

print(f"[{get_formatted_time()}] Goodbye.\n\t{sta_if.ifconfig()}")
