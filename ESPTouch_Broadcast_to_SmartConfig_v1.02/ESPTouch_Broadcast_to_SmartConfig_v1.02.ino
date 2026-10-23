/*
    This is the app to receive SmartConfig from the ESPTouch phone app, configure the
    WiFi, save the credentials and get the correct local time.

    This program was written and fonts sized and placed for a T-DisplayS3 but can be
    changed to any display you like if it is supported by tft_espi.
*/
#define RGB565(r,g,b) ((((r>>3)<<11) | ((g>>2)<<5) | (b>>3)))
#define RGB888(r,g,b) ((r << 16) | (g << 8) | b)

#define STRCMP(a, b) (!strcmp((a), (b)))  // Thank you, CoPilot!  I am weak on macros.
#define doThis(n) for (int _Myi = 0; _Myi < (n); _Myi++)
#define SpriteDarkBlueBG RGB565(0, 0, 166)

#include <TFT_eSPI.h> // Hardware-specific library
#include "Free_Fonts.h"     // Include the header file attached to this sketch
TFT_eSPI tft = TFT_eSPI();  // Invoke custom library with default width and height
TFT_eSprite spriteBG = TFT_eSprite(&tft);    // Background sprite.  No flashy flashy here!

#include <WiFi.h>
String   chSSID;
String   chPassword;
bool     CredsSet;
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include "zones.h"

#include "Preferences.h"
Preferences preferences;
#define RO_MODE true     // Preferences calls Read-only mode
#define RW_MODE false    // Preferences calls Read-write mode
#include <esp_sntp.h>    // Get UTC epoch here.

#include <GeoIP.h>
GeoIP  geoip;            // create GeoIP object 'geoip'
location_t loc;          // data structure to hold geoip results
bool   zoneFound = false;

int    looper    = 100;  // Over 40 means connection problem.
int    iDOM, iMonth, iYear, iHour, iMin, iSec;
int    iPrev_Sec = -1, iPrev_Min = -1, iPrev_Hour = -1;
time_t TS_Epoch = 0, UTC;
struct tm timeinfo;
char   chBuffer[100];    // Work area for sprintf, etc.
String sVer;
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
  spriteBG.drawString("Use ESPTouch in Broadcast mode now.",
                      spriteBG.width() / 2, spriteBG.height() / 3 + 65);
  spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.

  Serial.println("Initializing WiFi");
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
  spriteBG.setTextPadding(0);

  Serial.println("Done with initTime");
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
    spriteBG.setFreeFont(FSB9);   // Select Free Serif 9 point font.
    spriteBG.fillSprite(SpriteDarkBlueBG);
    spriteBG.drawString("ESPTouch Broadcast Test",
                        spriteBG.width() / 2, spriteBG.height() / 5 - 10);

    strftime(chBuffer, sizeof(chBuffer), "%A, %B %d, %G", &timeinfo);
    Serial.printf("It's %02i/%02i/%02i %02i:%02i:%02i\r\n",
                  iMonth, iDOM, iYear, iHour, iMin, iSec);
    spriteBG.setFreeFont(FSB12);
    spriteBG.drawString(chBuffer,
                        spriteBG.width() / 2, spriteBG.height() / 3 + 10);

    strftime(chBuffer, sizeof(chBuffer), "%T", &timeinfo);
    spriteBG.setFreeFont(FSB24);
    spriteBG.drawString(chBuffer,
                        spriteBG.width() / 2, spriteBG.height() / 3 * 2);

    spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.
    if (iPrev_Hour != iHour) HourDance();

    // End of user statements area.

    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("Wi-Fi lost, reconnecting...");
      WiFi.beginSmartConfig();
      while (!WiFi.smartConfigDone()) {
        delay(1000);
        Serial.print("W");
      }
    }
    // End of repeating procedure.
  } else {
    delay(50);  // Same second.
  }
  iPrev_Sec = iSec;
  iPrev_Min = iMin;
  iPrev_Hour = iHour;
}
/*******************************************************************************************/
void HourDance()
/*******************************************************************************************/
{
  //  Serial.printf("It is now %02i:00\r\n", iHour);

  // Do HourDance after updating the display to xx:00:00
  doThis(4) {
    tft.invertDisplay(false); delay(200);
    tft.invertDisplay(true); delay(200);
  }
}
/*******************************************************************************************/
void initTime()
/*******************************************************************************************/
{
  // The following is set in milliseconds.  Not sure why but they did not ask me!
  sntp_set_sync_interval(43217000);  // Get updated time every 12 hours plus a little.
  sntp_set_time_sync_notification_cb(timeSyncCallback);
  sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);

  int zoneCt = sizeof(zones) / sizeof(zones[0]);
  Serial.printf("There are %i zones in the table.\r\n", zoneCt);

  for (int i = 0; i < zoneCt; i++) {
    // Serial.printf("Comparing %s to %s\r\n", zones[i].name, loc.timezone);
    if (STRCMP(zones[i].name, loc.timezone)) {
      Serial.printf("Found a zone match: %s\r\n.", zones[i].name);
      configTzTime(zones[i].zone, "pool.ntp.org");  // Use the env string from the match.
      delay(2000);
      zoneFound = true;
      break;
    }
  }
  if (!zoneFound) {
    Serial.printf("Zone %s not found. Cannot continue.\r\n", loc.timezone);
    while (1);
  }
  Serial.print("Waiting for correct time..."); delay(5000);
  uint32_t startTime = millis();
  while (!TS_Epoch)
  {
    if (millis() - startTime > 60000)
    {
      Serial.println("NTP timeout at 1 minute. Rebooting.");
      ESP.restart();
    }
    delay(100);
  }
  time(&UTC);
  // I will double check by checking the year is reasonable.
  strftime(chBuffer, sizeof(chBuffer), "%Y", localtime(&UTC));
  iYear = atoi(chBuffer);
  int iLooper = 0;
  while (iYear < 2026) {
    Serial.print("T");
    time(&UTC);
    strftime (chBuffer, 100, "%Y", localtime(&UTC));
    iYear = atoi(chBuffer);
    if (iLooper++ > 30) {  // Wait one minute for time hack.
      Serial.println("\r\nCannot get time set. Rebooting.");
      ESP.restart();
    }
    delay(1000);
  }
  Serial.println();
}
/*******************************************************************************************/
void initWiFi()
/*******************************************************************************************/
{
  // First, I will see if there are saved credentials.  I have to do this myself
  //  since, after 6 years, this tiny thing has not been coded into the ESP
  //  SmartConfig code.  I guess they have been busy... FOR SIX YEARS!
  Serial.println("Reading from saved preferences.");
  preferences.begin("WiFiConf", RO_MODE);
  CredsSet = preferences.getBool("CredsSet", false);  // Have they been set already?
  if (CredsSet) {  // If so, use what is there.
    Serial.println("Credentials claim to be set.");
    chSSID = preferences.getString("SSID", "NoWAP");
    chPassword = preferences.getString("Password", "NoPass");
    preferences.end();
    Serial.printf("SSID: %s, PW: %s\r\n", chSSID, chPassword);

    // If we have something reasonable, try it.
    if (chSSID != "NoWAP") {
      Serial.print("Starting WiFi...");
      WiFi.begin(chSSID, chPassword); delay(2000);
      looper = 0;
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000); Serial.print("W");
        if (looper++ > 80) {
          Serial.println();
          break;  // 40 seconds should be enough!
        }
      }
    }
  }
  // looper init'd to 100.  If still over 40, there was some error. Use SmartConfig.
  // Either credentials were not set or they are outdated and no connection is
  //  possible and it ran out of the 40 tries.
  if (looper > 40) {
    Serial.println("Credentials not set or cannot connect to a WAP.  "
                   "Starting SmartConfig.");
    // Set to STA mode and initialize SmartConfig
    WiFi.mode(WIFI_STA); // Station mode
    Serial.println("Beginning Smart Config.");
    WiFi.beginSmartConfig();

    Serial.println("Waiting for SmartConfig...");
    Serial.println("Run ESPTouch (Broadcast mode) on your phone and submit credentials.");
    while (!WiFi.smartConfigDone()) {
      delay(1000); Serial.print("S");
    }
    Serial.print("\nSmartConfig info received. Connected to: ");
    Serial.printf("%s/%s\r\n", WiFi.SSID(), WiFi.psk());
    Serial.print("at IP Address: "); Serial.println(WiFi.localIP());

    preferences.begin("WiFiConf", RW_MODE);
    preferences.putString("SSID", WiFi.SSID());
    preferences.putString("Password", WiFi.psk());
    preferences.putBool("CredsSet", true);
    preferences.end();
    ESP.restart();
  }
}
/*******************************************************************************************/
void getGeoFromIP()
/*******************************************************************************************/
{
  /* The following proposed change was added to: location_t GeoIP::getInfo(bool hasKey)
     //if (!client.connect(host, httpsPort))  // Original code
       if (!client.connect(host, httpsPort, 10000))  // Hoping to shorten the timeout.
     to try to reduce the wait time for a lost connection attempt.
  */
  loc.status = false;
  uint8_t count = 0;

  while (!loc.status && count < 21)     // Try up to 20 times to connect to geoip server.
    // Usually it happens on the first try.
  {
    count++;
    Serial.printf("Attempt %u of 20 to get Geo info.\r\n", count);

    // Use one of the following function calls.
    // If using API key it must be inside double quotation marks.

    // no key, results not shown on serial monitor
    loc = geoip.getGeoFromWiFi(); delay(1000);

    // no key, results not shown on serial monitor
    //loc = geoip.getGeoFromWiFi(false);

    // no key, show results on on serial monitor
    //loc = geoip.getGeoFromWiFi(true);

    // use API key, results not shown on serial monitor
    //loc = geoip.getGeoFromWiFi("Your API Key");

    // use API key, results not shown on serial monitor
    //loc = geoip.getGeoFromWiFi("Your API Key", false);

    // use API key, show results on on serial monitor
    //loc = geoip.getGeoFromWiFi("Your API Key", true);
  }
  if (loc.status)      // Check to see if the data came in from the server.
  {
    // Display information from GeoIP.
    // The library can do this too if true is added to the function call.

    Serial.print("\nLatitude: ");               Serial.println(loc.latitude);   // float
    Serial.print("Longitude: ");                Serial.println(loc.longitude);  // float
    Serial.print("City: ");                     Serial.println(loc.city);       // char[24]
    Serial.print("Region: ");                   Serial.println(loc.region);     // char[24]
    Serial.print("Country: ");                  Serial.println(loc.country);    // char[24]
    Serial.print("Timezone: ");                 Serial.println(loc.timezone);   // char[24]
    // int  (eg. -1000 means -10 hours, 0 minutes)
    Serial.print("UTC Offset: ");               Serial.println(loc.offset);
    Serial.print("Offset Seconds: ");           Serial.println(loc.offsetSeconds); // long
  }
  else
  {
    Serial.println("Data received was not valid.");
  }
}
/*******************************************************************************************/
void timeSyncCallback(struct timeval * tv)
/*******************************************************************************************/
{
  //  struct timeval {  // Instantiated as "*tv"
  //    Number of whole seconds of elapsed time.
  //   time_t      tv_sec;
  //    Number of microseconds of rest of elapsed time minus tv_sec.
  //     Always less than one million.
  //   long int    tv_usec;
  //  Serial.print("\n** > Time Sync Received at ");
  //  Serial.println(ctime(&tv->tv_sec));
  TS_Epoch = tv->tv_sec;
}
/*******************************************************************************************/
void HandleSerialInput()
/*******************************************************************************************/
{
  if (!Serial.available()) return;

  int i, iHTemp;

  char input = Serial.read();
  input = toupper(input);

  // Ignore carriage return and newline entirely
  if (input == '\r' || input == '\n') return;

  Serial.printf("User input %c\r\n", input);

  switch (input) {
    case '?':
      showInputOptionsFull();
      break;

    default:
      Serial.printf("*>> Unknown input '%c'!\r\n", input);
      showInputOptionsFull();
      break;
  }
}
/*******************************************************************************************/
void showInputOptionsFull()
/*******************************************************************************************/
{
  Serial.println("ESPTouch app Broadcast for SmartConfig, v" + sVer);
  Serial.println("Running from:"); Serial.println(__FILE__);
  Serial.print("Compiled on "); Serial.print(__DATE__);
  Serial.print(" at "); Serial.println(__TIME__);
  Serial.println("Enter ? for this list.  Upper or Lower case OK.");
  Serial.println("If you make an unknown entry, this list is printed on the monitor.\r\n");
}
/*******************************************************************************************/
void printVers()
/*******************************************************************************************/
{
  int      lastDot, lastV;
  String   sTemp;

  // Serial.println(__FILENAME__);  // Same as __FILE__
  sTemp = String(__FILE__);
  // Get rid of the trailing .ino tab name. In this case, "\Utilities.ino"
  sTemp = sTemp.substring(0, sTemp.lastIndexOf("\\"));
  Serial.print("Running from: "); Serial.println(sTemp);

  sTemp = String(__FILE__);  // Start again for the version number.
  lastDot = sTemp.lastIndexOf(".");
  if (lastDot > -1) {  // Found a dot.  Thank goodness!
    lastV = sTemp.lastIndexOf("v");  // Find start of version number
    if (lastV > -1) {  // Oh, good, found version number, too
      sVer = sTemp.substring(lastV + 1, lastDot); // Pick up version number
      lastV = sVer.lastIndexOf("\\");
      if (lastV > -1) sVer = sVer.substring(0, lastV);
    } else {
      sVer = "0.00";  // Unknown version.
    }
  } else {
    sVer = "n/a";  // Something badly wrong here!
  }
  Serial.print("Version " + sVer + ", ");
  Serial.printf("Compiled on %s at %s.\r\n", __DATE__, __TIME__);
}
