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
  Serial.flush();
  printVers();  // Show source filename, date and time of compilation and version number.

  Serial.println("Initializing Display"); Serial.flush();

  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(SpriteDarkBlueBG); // Clear screen to navy background

#if defined TFT_BL
  ledcAttach(TFT_BL, 5000, 8);  // PWM timer automatically assigned.
  ledcWrite(TFT_BL, 120);       // Turn the display on medium for startup.
#endif

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
  Serial.println(); Serial.println("Done with initWifi\r\n"); Serial.flush();

  spriteBG.fillSprite(SpriteDarkBlueBG);
  spriteBG.drawString("ESPTouch Broadcast Test",
                      spriteBG.width() / 2, spriteBG.height() / 5 - 10);

  spriteBG.drawString("Get Geo Location",
                      spriteBG.width() / 2, spriteBG.height() / 3 + 15);
  spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.

  Serial.println("Getting Geo information by IP address."); Serial.flush();
  getGeoFromIP();
  Serial.println(); Serial.println("Done with getGeoFromIP\r\n"); Serial.flush();

  spriteBG.fillSprite(SpriteDarkBlueBG);
  spriteBG.drawString("ESPTouch Broadcast Test",
                      spriteBG.width() / 2, spriteBG.height() / 5 - 10);

  spriteBG.drawString("Getting Time",
                      spriteBG.width() / 2, spriteBG.height() / 3 + 15);
  spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.

  initTime();
  Serial.println("Done with initTime"); Serial.flush();
  getLocalTime(&timeinfo);  // This is what I was after.  Local time!
  Adjust_Brightness(timeinfo.tm_hour);

  spriteBG.setTextPadding(0);

  Serial.println("Setup is complete."); Serial.flush();
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

  // Adjust brightness based on hour.
  if (iPrev_Hour != iHour && iMin == 0 && iSec == 0)
    Adjust_Brightness(iHour);

  // Here is where you write your repeating procedure.
  doMyCode();

  iPrev_Sec  = iSec;
  iPrev_Min  = iMin;
  iPrev_Hour = iHour;
}

/*******************************************************************************************/
void doMyCode()
/*******************************************************************************************/
{
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

    // Boring
    // spriteBG.setTextColor(TFT_YELLOW, SpriteDarkBlueBG);  // Stable, yellow on blue.
    // Nutz!
    // spriteBG.setTextColor(RGB565(random(80, 256), random(80, 256), random(80, 256)),
    //                       SpriteDarkBlueBG);
    // Reddish
    spriteBG.setTextColor(RGB565(255, 80, 80), SpriteDarkBlueBG);
    spriteBG.drawString(chBuffer,
                        spriteBG.width() / 2, spriteBG.height() / 3 * 2);

    spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.
    if (iPrev_Hour != iHour) HourDance();

    // End of user statements area.

    if (iPrev_Min != iMin) {
      Serial.printf("It's %02i/%02i/%02i %02i:%02i:%02i\r\n",
                    iMonth, iDOM, iYear, iHour, iMin, iSec);
      Serial.flush();
      if (WiFi.status() != WL_CONNECTED) {
        Serial.println("Wi-Fi lost, reconnecting..."); Serial.flush();
        WiFi.beginSmartConfig();
        delay(1000);
        while (!WiFi.smartConfigDone()) {
          delay(1000);
          Serial.print("W"); Serial.flush();
        }
      }
    }
    // End of repeating procedure.
  } else {
    delay(50);  // Same second.
  }
}
