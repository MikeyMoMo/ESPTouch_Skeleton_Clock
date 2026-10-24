/*
    This is the app to receive SmartConfig from the ESPTouch phone app, configure the
    WiFi, save the credentials and get the correct local time.

    This program was written and fonts sized and placed for a T-DisplayS3 but can be
    changed to any display you like if it is supported by tft_espi.
*/
#include "defines.h"
/*******************************************************************************************/
void setup()
/*******************************************************************************************/
{
  Serial.begin(115200); delay(5000);
  Serial.println("This is the ESPTouch WiFi and time configuration program.");
  printVers();  // Show source filename, date and time of compilation and version number.

  Serial.println("Initializing Display");
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(SpriteDarkBlueBG); // Clear screen to navy background

  ledcAttach(TFT_BL, 5000, 8);  // PWM timer automatically assigned.
  ledcWrite(TFT_BL, 120);       // Turn the display on medium.

  spriteBG.createSprite(tft.width(), tft.height());  // The big kahuna. The big cheese!
  spriteBG.setSwapBytes(false);
  spriteBG.setTextColor(TFT_YELLOW, SpriteDarkBlueBG);
  spriteBG.setFreeFont(FSB9);   // Select Free Serif 9 point font.
  spriteBG.setTextPadding(spriteBG.width());

  // tft.setFreeFont(&FreeSerif9pt7b);  // Can use this series, too.
  spriteBG.setTextDatum(TC_DATUM);
  spriteBG.setTextPadding(spriteBG.width());

  spriteBG.fillSprite(SpriteDarkBlueBG);
  spriteBG.drawString("ESPTouch Broadcast Test",
                      spriteBG.width() / 2, spriteBG.height() / 5 - 10);

  spriteBG.drawString("Initializing WiFi",
                      spriteBG.width() / 2, spriteBG.height() / 3 + 15);
  spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.

  Serial.println("Initializing WiFi"); Serial.flush();
  initWiFi();
  Serial.println(); Serial.println("Done with initWifi\r\n");

  /* Testing */
  //#include <WiFiClientSecure.h>
  //
  //  WiFiClientSecure client;
  //  client.setInsecure();
  //  IPAddress ip;
  //  uint32_t msStart = millis();
  //  bool result = WiFi.hostByName("ipapi.co", ip);
  //  Serial.printf("DNS lookup took %lu ms\r\n",
  //                millis() - msStart);
  //  Serial.println(ip);
  //
  //  msStart = millis();
  //  bool rc = client.connect(ip, 443);
  //  Serial.printf("connect() returned after %lu ms\r\n",
  //                millis() - msStart);
  //  Serial.printf("Result = %s\r\n",
  //                rc ? "SUCCESS" : "FAIL");
  /* End Testing */

  spriteBG.fillSprite(SpriteDarkBlueBG);
  spriteBG.drawString("ESPTouch Broadcast Test",
                      spriteBG.width() / 2, spriteBG.height() / 5 - 10);

  spriteBG.drawString("Get Geo Location",
                      spriteBG.width() / 2, spriteBG.height() / 3 + 15);
  spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.

  Serial.println("Getting Geo information by IP address.");
  getGeoFromIP();
  Serial.println(); Serial.println("Done with getGeoFromIP\r\n");

  spriteBG.fillSprite(SpriteDarkBlueBG);
  spriteBG.drawString("ESPTouch Broadcast Test",
                      spriteBG.width() / 2, spriteBG.height() / 5 - 10);

  spriteBG.drawString("Getting Time",
                      spriteBG.width() / 2, spriteBG.height() / 3 + 15);
  spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.

  Serial.println("Setting that zone and getting time.");
  initTime();
  Serial.println("Done with initTime");

  spriteBG.setTextPadding(0);

  Serial.println("Setup is complete.");
}
/*******************************************************************************************/
void loop()
/*******************************************************************************************/
{
  while (Serial.available() > 0)  // Check if data is available to read
    HandleSerialInput();

  // Just to prove something happened.
  getLocalTime(&timeinfo);  // This is what I was after.  Local time!

  iMonth = timeinfo.tm_mon + 1;
  iDOM   = timeinfo.tm_mday;
  iYear  = timeinfo.tm_year + 1900;

  iHour  = timeinfo.tm_hour;
  iMin   = timeinfo.tm_min;
  iSec   = timeinfo.tm_sec;

  // Here is where you write your repeating procedure.

  if (iPrev_Sec != iSec)  // Wait for the next second to roll around...
  {
    spriteBG.setTextColor(TFT_YELLOW, SpriteDarkBlueBG);
    spriteBG.setFreeFont(FSB9);   // Select Free Serif 9 point font.
    spriteBG.fillSprite(SpriteDarkBlueBG);
    spriteBG.drawString("ESPTouch Broadcast Test",
                        spriteBG.width() / 2, spriteBG.height() / 5 - 10);

    strftime(chBuffer, sizeof(chBuffer), "%A, %B %d, %G", &timeinfo);
    spriteBG.setFreeFont(FSB12);
    spriteBG.drawString(chBuffer,
                        spriteBG.width() / 2, spriteBG.height() / 3 + 10);

    spriteBG.setFreeFont(FSB24);
    strftime(chBuffer, sizeof(chBuffer), "%T", &timeinfo);

    spriteBG.setTextColor(RGB565(random(128, 256), random(128, 256), random(128, 256)),
                          SpriteDarkBlueBG);
    spriteBG.drawString(chBuffer,
                        spriteBG.width() / 2, spriteBG.height() / 3 * 2);

    spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.
    if (iPrev_Hour != iHour) HourDance();

    // End of user statements area.

    if (iPrev_Min != iMin) {
      Serial.printf("It's %02i/%02i/%02i %02i:%02i:%02i\r\n",
                    iMonth, iDOM, iYear, iHour, iMin, iSec);
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi lost, reconnecting...");
        WiFi.beginSmartConfig();
        delay(1000);
        while (!WiFi.smartConfigDone()) {
          delay(1000);
          Serial.print("W");
        }
      }
    }
    // End of repeating procedure.
  } else {
    delay(50);  // Same second.
  }
  iPrev_Sec  = iSec;
  iPrev_Min  = iMin;
  iPrev_Hour = iHour;
}
