# ESP32 Temperature Controller

Wersja: 1.0

Projekt sterownika temperatury opartego na **ESP32‑WROOM‑32**, który odczytuje dane z 4 czujników **DS18B20**, wyświetla dane na wyświetlaczu **LCD 20×4 (I²C)** i steruje dwoma przekaźnikami **SSR‑40DA**.

Film ilustrujący działanie: https://youtu.be/xhZf3HNrp80

---

## Spis treści
1. [Opis projektu](#opis-projektu)
2. [Funkcjonalności](#funkcjonalno%C5%9Bci)
3. [Wymagania sprzętowe](#wymagania-sprz%C4%99towe)
4. [Schemat połączeń](#schemat-po%C5%82%C4%85cze%C5%84)
5. [Szybki start](#szybki-start)
6. [Licencja](#licencja)
7. [Autorzy](#autorzy)
8. [Bazowane na](#bazowane-na)
---

## Opis projektu
Sterownik mierzy temperatury w czterech punktach, prezentuje je lokalnie na wyświetlaczu LCD oraz załącza dwa wyjścia (SSR) według zdefiniowanych progów. Konfiguracja progów i pinów odbywa się za pomocą sekcji #define w pliku main.c.

## Funkcjonalności
- Odczyt 4 czujników **DS18B20** po magistrali OneWire
- Sterowanie 2 przekaźnikami **SSR‑40DA** (230 V AC, maks 40 A)
- Wyświetlanie aktualnych temperatur i stanów wyjść na **LCD 20×4**
- Konfiguracja za pomocą sekcji #define w main.c
- Kod zgodny z **ESP‑IDF v5** (C/C++)

## Wymagania sprzętowe
|     Element    | Ilość |              Uwagi               |
|----------------|-------|----------------------------------|
| ESP32‑WROOM‑32 | 1     | DevKit V1 lub inny moduł         |
| DS18B20        | 4     | Każdy z rezystorem 4,7 kΩ do VCC |
| LCD 20×4 + I²C | 1     | PCF8574                          |
| SSR‑40DA       | 2     | Sterowanie np. grzałkami         |
| Zasilacz 5 V   | 1     | ≥700 mA                          |

## Schemat połączeń
```
DS18B20 (DQ) ----- GPIO4  <OneWire>
LCD SDA ---------- GPIO21 <I2C>
LCD SCL ---------- GPIO22 <I2C>
SSR1 ------------- GPIO26
SSR2 ------------- GPIO27
```
_Reszta pinów: 3V3, GND, 5V zgodnie z datasheet._

## Szybki start
```bash
# 1. Zainstaluj ESP‑IDF v5.x

# 2. Skonfiguruj projekt
$ idf.py menuconfig   # ustaw PINY i progi domyślne

# 3. Kompilacja i flash, opcjonalnie monitorowanie
$ idf.py build flash monitor
```
Po skompilowaniu urządzenie zacznie wysyłać logi na UART 115200 Bd.

## Licencja
Projekt dystrybuowany na licencji **MIT** – szczegóły w pliku `LICENSE`.

## Autorzy
- **vejetq** //Daniel Łukasik

## Bazowane na
oficjalne komponenty z przykładami Esperessif:
espressif/ds18b20
espressif/onewire_bus
https://github.com/tolacika/esp-lcd-example
