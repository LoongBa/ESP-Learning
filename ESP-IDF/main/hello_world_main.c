#include <stdio.h>
#include <time.h>
#include <inttypes.h>
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
// #include "protocol_examples_common.h"
#include "esp_sntp.h"

#define WIFI_SSID "uxiang.cn"
#define WIFI_PASSWORD "meiyoumima"
#define LED_GPIO_PIN GPIO_NUM_2

void wifi_connect(const char *ssid, const char *password)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize the TCP/IP stack
    ESP_ERROR_CHECK(esp_netif_init());

    // Create the default event loop
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Initialize the default WiFi station
    esp_netif_create_default_wifi_sta();

    // Initialize WiFi with default configuration
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // Set WiFi mode to station
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // Configure WiFi connection
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "uxiang.cn",
            .password = "meiyoumima",
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));

    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());

    // Connect to WiFi
    ESP_ERROR_CHECK(esp_wifi_connect());
}

void ntp_sync(void)
{
    // Initialize SNTP
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "pool.ntp.org");
    // 设置时区
    setenv("TZ", "CST-8", 1);
    esp_sntp_init();

    // Wait for time to be set
    int retry = 0;
    const int retry_count = 10;
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET && ++retry < retry_count)
    {
        printf("\033[34mUpdating date time by NTP... (%d/%d)\033[0m\n", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
}

// 颜色名字数组
const char *color_names[] = {"green", "red", "yellow", "blue", "purple", "cyan", "white"};

// 获取颜色代码的函数
const char *getColorCode(const char *color)
{
    for (int i = 0; i < sizeof(color_names) / sizeof(color_names[0]); i++)
    {
        if (strcmp(color_names[i], color) == 0)
        {
            static char color_code_buf[4]; // 分配足够的内存，包括终止符'\0'
            sprintf(color_code_buf, "%03d", 30 + i); // 格式化数字，左补零补全三位
            return color_code_buf;
        }
    }
    return "\033[0m"; // 返回默认颜色的ANSI转义码
}

void print_color_by_name(const char *text, const char *color)
{
    if (color == NULL)
        color = "green"; // 设置默认颜色
    const char *color_code = getColorCode(color);
    printf("\033[%sm%s\033[0m", color_code, text);
}
void print_color(const char *text)
{
    print_color_by_name(text, NULL);
}
void Light()
{
    // 配置GPIO2为输出模式
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;         // 禁用中断
    io_conf.mode = GPIO_MODE_OUTPUT;               // 设置为输出模式
    io_conf.pin_bit_mask = (1ULL << LED_GPIO_PIN); // 设置GPIO2
    io_conf.pull_down_en = 0;                      // 不启用下拉电阻
    io_conf.pull_up_en = 0;                        // 不启用上拉电阻
    gpio_config(&io_conf);

    // 循环点亮和熄灭LED
    for(int i = 0; i < 10; i++)
    {
        gpio_set_level(LED_GPIO_PIN, 1);       // 点亮LED
        vTaskDelay(1000 / portTICK_PERIOD_MS); // 延时1秒
        gpio_set_level(LED_GPIO_PIN, 0);       // 熄灭LED
        vTaskDelay(500 / portTICK_PERIOD_MS); // 延时1秒
    }
}
void app_main(void)
{
    // 亮灯
    Light();

    // 连接到指定的 WiFi 网络
    wifi_connect(WIFI_SSID, WIFI_PASSWORD);
    // 连接到 NPT 更新系统时间
    ntp_sync();

    // 获取当前时间
    time_t now;
    struct tm timeinfo;
    time(&now);

    // 转换为本地时间
    localtime_r(&now, &timeinfo);

    // 格式化时间
    char strftime_buf[64];
    strftime(strftime_buf, sizeof(strftime_buf), "%c", &timeinfo);

    // 显示时间
    print_color_by_name("****************************************************************\n", "purple");
    print_color_by_name("Hello world!\nI'm play.\n", "red");
    printf("当前时间: %s\n", strftime_buf);
    print_color_by_name("****************************************************************\n", "purple");

    /* Print chip information */
    esp_chip_info_t chip_info;
    uint32_t flash_size;
    esp_chip_info(&chip_info);
    printf("This is '%s' chip with %d CPU core(s), %s%s%s%s, ",
           CONFIG_IDF_TARGET,
           chip_info.cores,
           (chip_info.features & CHIP_FEATURE_WIFI_BGN) ? "WiFi/" : "",
           (chip_info.features & CHIP_FEATURE_BT) ? "BT" : "",
           (chip_info.features & CHIP_FEATURE_BLE) ? "BLE" : "",
           (chip_info.features & CHIP_FEATURE_IEEE802154) ? ", 802.15.4 (Zigbee/Thread)" : "");

    unsigned major_rev = chip_info.revision / 100;
    unsigned minor_rev = chip_info.revision % 100;
    printf("Silicon revision v%d.%d, ", major_rev, minor_rev);
    if (esp_flash_get_size(NULL, &flash_size) != ESP_OK)
    {
        printf("Get flash size failed");
        return;
    }

    printf("%" PRIu32 "MB %s flash\n", flash_size / (uint32_t)(1024 * 1024),
           (chip_info.features & CHIP_FEATURE_EMB_FLASH) ? "embedded" : "external");

    printf("Minimum free heap size: %" PRIu32 " bytes\n", esp_get_minimum_free_heap_size());

    // for (int i = 10; i >= 0; i--) {
    //     printf("Restarting in %d seconds...\n", i);
    //     vTaskDelay(1000 / portTICK_PERIOD_MS);
    // }
    // printf("Restarting now.\n");
    fflush(stdout);
    // esp_restart();
}
