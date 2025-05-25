/*
 *  ESP32 temperature controller
 *  author: <Daniel Łukasik>
 */

#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "onewire_bus.h"
#include "ds18b20.h"
#include "lcd.h"
#include "driver/gpio.h"

#define ONEWIRE_PIN   4      // GPIO z magistralą 1-Wire
#define MAX_SENSORS   4      // liczba podłączonych DS18B20
#define GPIO_Q1        25      // pin do pierwszego tranzystora
#define GPIO_Q2        26      // pin do drugiego tranzystora
#define TEMP_THRESHOLD 26.0f   // granica przełączania [°C]

static const char *TAG = "TEMP";
static ds18b20_device_handle_t czujnik[MAX_SENSORS];
static size_t liczba_czujnikow = 0;

/* === GPIO dla tranzystorów =========================================== */
static void tranzystory_init(void)
{
    gpio_config_t cfg = {
        .pin_bit_mask = (1ULL << GPIO_Q1) | (1ULL << GPIO_Q2),
        .mode         = GPIO_MODE_OUTPUT,
        .pull_up_en   = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&cfg);

    /* STAN 0 – oba włączone na start */
    gpio_set_level(GPIO_Q1, 1);
    gpio_set_level(GPIO_Q2, 1);
}

/*–– 1. Inicjalizacja magistrali i wyszukanie czujników ––*/
static void znajdz_czujniki(void)
{
    onewire_bus_handle_t bus = NULL;
    onewire_bus_config_t cfg_bus = {
        .bus_gpio_num = ONEWIRE_PIN,
    };
    onewire_bus_rmt_config_t cfg_rmt = {
        .max_rx_bytes = 10,   // 1 B komendy + 8 B ROM + 1 B CRC
    };
    ESP_ERROR_CHECK(onewire_new_bus_rmt(&cfg_bus, &cfg_rmt, &bus));

    onewire_device_iter_handle_t it;
    ESP_ERROR_CHECK(onewire_new_device_iter(bus, &it));

    onewire_device_t dev;
    while (onewire_device_iter_get_next(it, &dev) == ESP_OK &&
           liczba_czujnikow < MAX_SENSORS) {

        ds18b20_config_t cfg = {};
        if (ds18b20_new_device(&dev, &cfg,
                               &czujnik[liczba_czujnikow]) == ESP_OK) {

            ESP_LOGI(TAG, "DS18B20[%zu] ROM: 0x%016llX",
                     liczba_czujnikow, dev.address);
            liczba_czujnikow++;
        }
    }
    onewire_del_device_iter(it);
    ESP_LOGI(TAG, "Znaleziono %zu czujnik(ów)", liczba_czujnikow);
}

/*–– 2. Jednorazowy odczyt wszystkich temperatur ––*/
static void odczytaj_temperatury(void)
{
    for (size_t i = 0; i < liczba_czujnikow; ++i) {
        float t;
        ESP_ERROR_CHECK(ds18b20_trigger_temperature_conversion(czujnik[i]));
        ESP_ERROR_CHECK(ds18b20_get_temperature(czujnik[i], &t));
        ESP_LOGI(TAG, "T[%zu] = %.2f °C", i, t);
    }
}

/*–– 3. Zadanie FreeRTOS odświeżające co 1 s ––*/
static void task_temperatura(void *arg)
{
  (void)arg;
    char wiersz[21];                      // LCD 20 znaków + NULL

    while (1) {
        size_t powyzej = 0;
        for (size_t i = 0; i < liczba_czujnikow; ++i) {

            /* pojedynczy odczyt czujnika -------------------------------- */
            float t;
            ds18b20_trigger_temperature_conversion(czujnik[i]);
            ds18b20_get_temperature(czujnik[i], &t);
            ESP_LOGI(TAG, "T[%zu] = %.2f °C", i, t);
            if (t >= TEMP_THRESHOLD)
                ++powyzej;


            /* aktualizacja odpowiadającego wiersza LCD ------------------ */
            snprintf(wiersz, sizeof(wiersz), "T%zu: %6.2f C", i, t);
            lcd_set_cursor(0, i);          // jeden czujnik = jeden wiersz
            lcd_write_string(wiersz);
        }

        /* jeżeli czujników < 4 – czyść pozostałe linie */
        for (size_t r = liczba_czujnikow; r < 4; ++r) {
            lcd_set_cursor(0, r);
            lcd_write_string("                    ");  // 20 spacji
        }

        /* ---- logika tranzystorów ---- */
        bool q1, q2;
        if (powyzej == 0) {                 // wszystkie < 26 °C
        q1 = true;  q2 = true;
        } else if (powyzej == liczba_czujnikow) { // wszystkie ≥ 26 °C
            q1 = false; q2 = false;
        } else {                            // częściowo ≥ 26 °C
            q1 = true;  q2 = false;
        }
        gpio_set_level(GPIO_Q1, q1);
        gpio_set_level(GPIO_Q2, q2);

        /* ---- prezentacja stanu na LCD ---- */
        lcd_set_cursor(17, 0);              // prawa strona 1. wiersza
        char stan[3] = { q1 ? '1' : '0', q2 ? '1' : '0', 0 };
        lcd_write_string(stan);

        vTaskDelay(pdMS_TO_TICKS(1000));   // odświeżanie co 1 s
    }
}

/*–– 4. main ––*/
void app_main(void)
{
 znajdz_czujniki();          // wyszukaj DS18B20

    /*  ----------------- LCD: ekran powitalny ----------------- */
    i2c_master_init();
    lcd_init();
    lcd_backlight(true);

    lcd_clear();
    lcd_set_cursor(0, 0);
    lcd_write_string("Inicjalizacja...");
    vTaskDelay(pdMS_TO_TICKS(2000));   // 2 s delay
    lcd_clear();

    tranzystory_init();

    /*  ----------------- Start zadania temperatury ------------- */
    xTaskCreate(task_temperatura,      // odczyt + wyświetlacz
                "temp_task", 2048, NULL, 5, NULL);
}
