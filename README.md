# ESPTouch Skeleton Clock

A lightweight ESP32-based clock project that uses ESPTouch SmartConfig to join Wi‑Fi, looks up the device's approximate geographic location from the IP address, matches that location to a timezone table, and then syncs the current time over NTP. The sketch is intentionally written as a skeleton so you can reuse the setup flow and then add your own display behavior or features.

This project is centered around the Arduino/ESP32 ecosystem and is designed to work well with an ESP32 display board such as the T‑Display S3, using `TFT_eSPI` for graphics.

## Features

- ESPTouch broadcast SmartConfig onboarding for Wi‑Fi credentials
- Stores Wi‑Fi SSID/password in ESP32 `Preferences`
- Reuses saved credentials automatically on reboot
- Uses IP-based geo lookup to estimate location
- Maps timezone names to TZ strings from a built-in zone table
- Sets up NTP time synchronization with `esp_sntp`
- Displays the date and current time on the TFT screen
- Continues to monitor Wi‑Fi and automatically attempts to reconnect if needed
- Includes a simple display update loop and hour-flash animation for visual feedback

## Project structure

```text
ESPTouch_Skeleton_Clock/
├── README.md
├── ESPTouch_Broadcast_to_SmartConfig_v1.02/
│   ├── ESPTouch_Broadcast_to_SmartConfig_v1.02.ino   # Main sketch
│   ├── Definitions.h                                   # Notes on text alignment constants
│   ├── Free_Fonts.h                                   # Custom font definitions
│   └── zones.h                                        # Timezone name and TZ string mapping
└──
```

## Main sketch behavior

The program flow is straightforward:

1. Boot the ESP32 and initialize the display.
2. Try saved Wi‑Fi credentials.
3. If no valid connection is available, trigger SmartConfig and wait for the ESPTouch app.
4. Save the credentials to non-volatile memory.
5. Use the current Wi‑Fi connection to query a geo IP service.
6. Match the returned timezone string against the built-in region list.
7. Configure NTP time sync using the selected timezone.
8. Display the date and time in the local timezone.

## Hardware

This sketch is written for an ESP32 display board and the code includes TFT display setup examples intended for a T‑Display S3 class board.

Typical requirements:

- ESP32 development board
- TFT display supported by `TFT_eSPI`
- USB cable and programming adapter
- Wi‑Fi network and ESPTouch mobile app

## Software requirements

Use the Arduino IDE or VS Code + Arduino extension with the ESP32 board package installed.

Required libraries (as referenced in the sketch):

- `TFT_eSPI`
- `WiFi`
- `HTTPClient`
- `WiFiClientSecure`
- `Preferences`
- `esp_sntp`
- `GeoIP`

Important: the `GeoIP` library and `TFT_eSPI` configuration must match your display and board. For a different board than the T‑Display S3, update the `TFT_eSPI` user config as needed.

## Wiring and setup

1. Open the Arduino sketch at:
   `ESPTouch_Broadcast_to_SmartConfig_v1.02/ESPTouch_Broadcast_to_SmartConfig_v1.02.ino`
2. Install the ESP32 board package in Arduino.
3. Install the libraries listed above.
4. Configure `TFT_eSPI` for your screen model and pin layout.
5. Select the correct ESP32 board and port.
6. Upload the sketch.

## Running the project

1. Power on the ESP32.
2. The device will attempt to connect using stored Wi‑Fi credentials.
3. If none are available or the connection fails, it will wait for SmartConfig.
4. Open the ESPTouch app on your phone.
5. Select the ESPTouch broadcast mode flow and send the Wi‑Fi credentials.
6. Once connected, the device fetches geo information and sets the timezone.
7. NTP sync begins and the current time is displayed on the screen.

## Serial monitor

The sketch uses the serial port for status output. A good starting point is:

- Baud rate: 115200

The serial output includes Wi‑Fi status, geo lookup results, timezone matching, and time update messages.

## Notes

- This is intended as a starting point rather than a finished end-user product.
- The `zones.h` file includes a large timezone table, so you can adapt it to your own region or add custom logic.
- The code includes display initialization, Wi‑Fi handling, and timekeeping in one sketch to keep the concept easy to understand and extend.
- If time sync fails, the sketch is designed to reboot and retry, which is useful for unattended deployment.

## Example use cases

- Desk clock
- Wall-mounted time display
- Learning project for ESP32 + SmartConfig + NTP + local timezone handling
- Starting point for a larger IoT dashboard

## License

This repository does not currently include a license file.  It is open to all without restriction.  It would be nice to include a link in the code to this skeleton so others might find it and save a lot of their time creating it all over again.

## Related references

- ESPTouch SmartConfig
- ESP32 Wi‑Fi management
- Arduino `Preferences` API
- `TFT_eSPI` display library
- NTP and timezone configuration on ESP32

## Summary

This project demonstrates a complete flow for ESP32-based Wi‑Fi configuration and time synchronization: configure the network, get location, determine timezone, and display the local time. It is a practical skeleton for building a custom digital clock or a more advanced device with internet-based time and sensor features.
