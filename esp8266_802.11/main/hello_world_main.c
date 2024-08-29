#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "esp_log.h"

#define WIFI_CHANNEL 1
// #define PACKET_SIZE 36  // 修改为与Linux程序相同的大小
#define PACKET_SIZE 37
#define TAG "RAW_TX"

// 802.11 帧结构
typedef struct {
    uint16_t frame_control;
    uint16_t duration;
    uint8_t addr1[6];
    uint8_t addr2[6];
    uint8_t addr3[6];
    uint16_t seq_ctrl;
    uint8_t payload[5];  // "0xAA"的ASCII表示
} __attribute__((packed)) wifi_ieee80211_packet_t;

static void wifi_init() {
    tcpip_adapter_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(WIFI_CHANNEL, WIFI_SECOND_CHAN_NONE));
}

static void raw_tx_task(void *pvParameter) {
    wifi_ieee80211_packet_t *packet = (wifi_ieee80211_packet_t *)pvParameter;
    int packet_count = 0;

    uint8_t mac[6];
    ESP_ERROR_CHECK(esp_wifi_get_mac(WIFI_IF_STA, mac));
    ESP_LOGI(TAG, "ESP8266 station MAC address: %02x:%02x:%02x:%02x:%02x:%02x",
             mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    // 设置802.11帧头
    packet->frame_control = 0x0008;  // 数据帧
    packet->duration = 0;
    memset(packet->addr1, 0xFF, 6);  // 广播地址
    memcpy(packet->addr2, mac, 6);   // 源地址（ESP8266的MAC）
    memset(packet->addr3, 0xFF, 6);  // 广播地址
    packet->seq_ctrl = 0;

    // 设置payload
    // memcpy(packet->payload, "0xAA", 4);
    memcpy(packet->payload, "0xAAA", 5);

    while (1) {
        ESP_ERROR_CHECK(esp_wifi_80211_tx(WIFI_IF_STA, (uint8_t*)packet, PACKET_SIZE, false));
        packet_count++;
        ESP_LOGI(TAG, "Sent packet %d", packet_count);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main() {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifi_init();

    wifi_ieee80211_packet_t *packet = (wifi_ieee80211_packet_t *)malloc(sizeof(wifi_ieee80211_packet_t));
    if (!packet) {
        ESP_LOGE(TAG, "Cannot allocate packet buffer");
        return;
    }

    xTaskCreate(&raw_tx_task, "raw_tx_task", 2048, packet, 5, NULL);
}