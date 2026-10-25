/*******************************************************************************************/
void HourDance()
/*******************************************************************************************/
{
  // Serial.printf("It is now %02i:00\r\n", iHour);
  // Serial.printf("Previous hour %d\r\n" iPrev_Hour);

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
  // Only use this if you are in a very time critical situation.  It can cause slow running.
  //  sntp_set_sync_mode(SNTP_SYNC_MODE_SMOOTH);  // Eats the core alive!

  int zoneCt = sizeof(zones) / sizeof(zones[0]);
  Serial.printf("There are %i zones in the table.\r\n", zoneCt); Serial.flush();

  for (int i = 0; i < zoneCt; i++) {
    // Serial.printf("Comparing %s to %s\r\n", zones[i].name, loc.timezone);
    if (STRCMP(zones[i].name, loc.timezone)) {
      Serial.printf("Found a zone match: %s\r\n", zones[i].name);
      Serial.println("Using that zone and getting time.");
      Serial.flush();
      configTzTime(zones[i].zone, "pool.ntp.org");  // Use the env string from the match.
      delay(2000);
      zoneFound = true;
      break;
    }
  }
  if (!zoneFound) {
    Serial.printf("Zone %s not found. Cannot continue.\r\n", loc.timezone);
    Serial.flush();
    spriteBG.fillSprite(SpriteDarkBlueBG);
    spriteBG.drawString("ESPTouch Broadcast Test",
                        spriteBG.width() / 2, spriteBG.height() / 5 - 10);
    spriteBG.drawString("Initializing WiFi",
                        spriteBG.width() / 2, spriteBG.height() / 3 + 15);
    spriteBG.drawString("Zone " + String(loc.timezone) + " not found.",
                        spriteBG.width() / 2, spriteBG.height() / 3 + 65);
    while (1);
  }
  Serial.print("Waiting for correct time..."); Serial.flush(); delay(5000);
  uint32_t startTime = millis();
  while (!TS_Epoch)
  {
    if (millis() - startTime > 60000)
    {
      Serial.println("NTP timeout at 1 minute. Rebooting.");
      delay(5000); Serial.flush();
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
    Serial.print("T"); Serial.flush();
    time(&UTC);
    strftime (chBuffer, 100, "%Y", localtime(&UTC));
    iYear = atoi(chBuffer);
    if (iLooper++ > 30) {  // Wait one minute for time hack.
      Serial.println("\r\nCannot get time set. Rebooting."); Serial.flush();
      delay(5000);
      ESP.restart();
    }
    delay(1000);
  }
  Serial.println(); Serial.flush();
}
/*******************************************************************************************/
void initWiFi()
/*******************************************************************************************/
{
  // First, I will see if there are saved credentials.  I have to do this myself
  //  since, after 6 years, this tiny thing has not been coded into the ESP
  //  SmartConfig code.  I guess they have been busy... FOR SIX YEARS!
  preferences.begin("WiFiConf", RO_MODE);
  CredsSet = preferences.getBool("CredsSet", false);  // Have they been set already?
  if (CredsSet) {  // If so, use what is there.
    Serial.println("Credentials claim to be set. Reading from saved preferences.");
    Serial.flush();
    chSSID = preferences.getString("SSID", "NoWAP");
    chPassword = preferences.getString("Password", "NoPass");
    preferences.end();
    spriteBG.fillSprite(SpriteDarkBlueBG);
    spriteBG.drawString("ESPTouch Broadcast Test",
                        spriteBG.width() / 2, spriteBG.height() / 5 - 10);
    spriteBG.drawString("Initializing WiFi",
                        spriteBG.width() / 2, spriteBG.height() / 3 + 15);
    spriteBG.drawString("Credentials found for " + chSSID,
                        spriteBG.width() / 2, spriteBG.height() / 3 + 65);
    spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.

    Serial.printf("SSID: %s, PW: %s\r\n", chSSID, chPassword);
    Serial.flush();

    // If we have something reasonable, try it.
    if (chSSID != "NoWAP") {
      Serial.print("Starting WiFi..."); Serial.flush();
      WiFi.begin(chSSID, chPassword); delay(2000);
      looper = 0; delay(1000);
      while (WiFi.status() != WL_CONNECTED) {
        delay(1000); Serial.print("W"); Serial.flush();
        if (looper++ > 40) {
          Serial.println(); Serial.flush();
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
                   "Starting SmartConfig."); Serial.flush();
    spriteBG.setFreeFont(FSB9);   // Select Free Serif 9 point font.
    spriteBG.fillSprite(SpriteDarkBlueBG);
    spriteBG.drawString("ESPTouch Broadcast Test",
                        spriteBG.width() / 2, spriteBG.height() / 5 - 10);
    spriteBG.drawString("Initializing WiFi",
                        spriteBG.width() / 2, spriteBG.height() / 3 + 15);
    spriteBG.drawString("Use ESPTouch in Broadcast mode now.",
                        spriteBG.width() / 2, spriteBG.height() / 3 + 65);
    spriteBG.pushSprite(0, 0);  // Show the finished form of the display sprite.

    // Set to STA mode and initialize SmartConfig
    WiFi.mode(WIFI_STA); // Station mode
    Serial.println("Beginning Smart Config."); Serial.flush();
    WiFi.beginSmartConfig();

    Serial.println("Waiting for SmartConfig..."); Serial.flush();
    Serial.println("Run ESPTouch (Broadcast mode) on your phone and submit credentials.");
    Serial.flush();
    while (!WiFi.smartConfigDone()) {
      delay(1000); Serial.print("S"); Serial.flush();
    }
    Serial.print("\nSmartConfig info received. Connected to: ");
    Serial.printf("%s/%s\r\n", WiFi.SSID(), WiFi.psk());
    Serial.print("at IP Address: "); Serial.println(WiFi.localIP());
    Serial.flush();

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
    Serial.printf("Attempt %u of 20 to get Geo info.\r\n", count); Serial.flush();

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
    Serial.flush();
  }
  else
  {
    Serial.println("Data received was not valid."); Serial.flush();
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

  Serial.printf("User input %c\r\n", input); Serial.flush();

  switch (input) {
    case '?':
      showInputOptionsFull();
      break;

    default:
      Serial.printf("*>> Unknown input '%c'!\r\n", input); Serial.flush();
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
  Serial.flush();
}
/*******************************************************************************************/
void printVers()
/*******************************************************************************************/
{
  // Your sketch name should be in the format name_vx.xx
  // This routine loos for the last v and takes the rest as the version number.
  // It also trims off anything afterwards in case this is run from a different tab.3
  // Yeah, like this is run.

  int      lastDot, lastV;
  String   sTemp;

  // Serial.println(__FILENAME__);  // Same as __FILE__
  sTemp = String(__FILE__);
  // Get rid of the trailing .ino tab name. In this case, "\Utilities.ino"
  sTemp = sTemp.substring(0, sTemp.lastIndexOf("\\"));
  Serial.print("Running from: "); Serial.println(sTemp); Serial.flush();

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
  Serial.flush();
}
/*******************************************************************************************/
void Adjust_Brightness(int iHour)
/*******************************************************************************************/
{
#if defined TFT_BL
  // Only try this if there is a BL pin defined.
  Serial.printf("Setting brightness to %d for hour %02d\r\n",
                ihourlyBrilliance[iHour], iHour);
  ledcWrite(TFT_BL, ihourlyBrilliance[iHour]);
#endif
}
