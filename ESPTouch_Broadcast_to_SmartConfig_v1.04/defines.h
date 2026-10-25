// Either the Boards level or Windows USB is having trouble putting all of the 
//  Serial.prints onto the Serial monitor.  Therefore, there are some Serial.flush()
//  statements here and there.  Without them, stuff gets lost.  Not sure why or where!

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

int ihourlyBrilliance[] = {
  //0   1    2     3    4    5
  40,  20,  20,   20,  20,  20,  //  0- 5
  //6   7    8     9   10   11
  30,  40,  50,   60, 80,  100,  //  6-11
  //12  13   14   15   16   17
  120, 140, 160, 160, 160, 160,  // 12-17
  //18  19   20   21   22   23
  140, 120, 100,  80,  60,  50   // 18-23
};

/* Just to remind myself when I forget!

  The DATUMs:

  TL_DATUM  0 Top left (default)
  TC_DATUM  1 Top centre
  TR_DATUM  2 Top right
  ML_DATUM  3 Middle left
  CL_DATUM  3 Centre left, same as above
  MC_DATUM  4 Middle centre
  CC_DATUM  4 Centre centre, same as above
  MR_DATUM  5 Middle right
  CR_DATUM  5 Centre right, same as above
  BL_DATUM  6 Bottom left
  BC_DATUM  7 Bottom centre
  BR_DATUM  8 Bottom right
  L_BASELINE  9 Left character baseline
  (Line the 'A' character would sit on)
  C_BASELINE  10  Centre character baseline
  R_BASELINE  11  Right character baseline
*/
