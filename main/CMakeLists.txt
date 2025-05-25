idf_component_register(
    SRCS         "main.c" "lcd.c"
    INCLUDE_DIRS "."              # katalog z nagłówkami własnymi
    REQUIRES                     # ↓ wszystkie potrzebne zależności
        espressif__ds18b20
        espressif__onewire_bus
        esp_driver_i2c
        esp_driver_gpio
)