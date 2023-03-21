#include <Arduino.h>

//Lora Libraries
#include <SPI.h>
#include <LoRa.h>

//I2C
#include <Wire.h>

//SD Libraries
#include "FS.h"
#include "SD.h"


//RTC Clock
#include "RTClib.h"
RTC_DS3231 rtc;
char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

//SPIClass SDSPI(HSPI);
//SPIClass LoRaSPI(HSPI);
//SPIClass EINKSPI(HSPI);
int sd_status = 0;
int lora_status = 0;

#define EINK_INTERVAL_1 80000
#define EINK_INTERVAL_2 80000
#define EINK_INTERVAL_3 80000

unsigned long lastExecutedMillis_1 = 60000; // vairable to save the last executed time for code block 1 1*EINK_INVERVAL_1
unsigned long lastExecutedMillis_2 = 0; // vairable to save the last executed time for code block 2
unsigned long lastExecutedMillis_3 = 30000; // vairable to save the last executed time for code block 2


//Data Recording
String  folder_name;
String  file_name;
String  data_line;
File myFile;

//Data Initiative

//  data_line = sender_id + String(",") + lora_counter + String(",") + now_local.year() + String(",") + now_local.month() + String(",") + now_local.day() + String(",") + now_local.hour() + String(",") + now_local.minute() + String(",") + now_local.second()
// +  SHT31_temp + String(",") + SHT31_hmd + String(",") + HTU21DF_temp + String(",") + HTU21DF_hmd + String(",") + BME280_temp + String(",") + BME280_hmd + String(",") + BME280_pre + String(",") + BME280_alt + String(",")
//+  BH1750_lux + String(",") + weight + String(",") + sound_average + String(",") + ;

int sender_id = -1;
int lora_counter = 0;
float now_local_year = -1;
float now_local_month = -1;
float now_local_day = -1;
float now_local_hour = -1;
float now_local_minute = -1;
float now_local_second = -1;
float SHT31_temp = -1;
float SHT31_hmd = -1;
float HTU21DF_temp = -1;
float HTU21DF_hmd = -1;
float BME280_temp = -1;
float BME280_hmd = -1;
float BME280_pre = -1;
float BME280_alt = -1;
float BH1750_lux = -1;
float sound_average = -1;
float weight = -1;
float counter_in = -1;
float counter_out = -1;

float SHT31_temp_1 = -1;
float SHT31_hmd_1 = -1;
float HTU21DF_temp_1 = -1;
float HTU21DF_hmd_1 = -1;
float BME280_temp_1 = -1;
float BME280_hmd_1 = -1;
float BME280_pre_1 = -1;
float BME280_alt_1 = -1;
float BH1750_lux_1 = -1;
float sound_average_1 = -1;
float weight_1 = -1;
float counter_in_1 = -1;
float counter_out_1 = -1;


float SHT31_temp_2 = -1;
float SHT31_hmd_2 = -1;
float HTU21DF_temp_2 = -1;
float HTU21DF_hmd_2 = -1;
float BME280_temp_2 = -1;
float BME280_hmd_2 = -1;
float BME280_pre_2 = -1;
float BME280_alt_2 = -1;
float BH1750_lux_2 = -1;
float sound_average_2 = -1;
float weight_2 = -1;
float counter_in_2 = -1;
float counter_out_2 = -1;

int received_counter = 0;
int firebase_counter = 0;

//float data_array[19] = {sender_id, lora_counter, now_local_year, now_local_month, now_local_day, now_local_hour, now_local_minute, now_local_second, SHT31_temp, SHT31_hmd, HTU21DF_temp, HTU21DF_hmd, BME280_temp, BME280_hmd, BME280_pre, BME280_alt, BH1750_lux, weight, sound_average};
float data_array[21] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,-1,-1};
String data_received = "-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,";
String data_previous = "-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,";
//E-ink Definition - Start

// Display Library example for SPI e-paper panels from Dalian Good Display and boards from Waveshare.
// Requires HW SPI and Adafruit_GFX. Caution: the e-paper panels require 3.3V supply AND data lines!
// Display Library based on Demo Example from Good Display: http://www.e-paper-display.com/download_list/downloadcategoryid=34&isMode=false.html
// Author: Jean-Marc Zingg
// Library: https://github.com/ZinggJM/GxEPD2
// Purpose: show uses of GxEPD2_GFX base class for references to a display instance

#define ENABLE_GxEPD2_GFX 1

// uncomment next line to use class GFX of library GFX_Root instead of Adafruit_GFX
//#include <GFX.h>
// Note: if you use this with ENABLE_GxEPD2_GFX 1:
//       uncomment it in GxEPD2_GFX.h too, or add #include <GFX.h> before any #include <GxEPD2_GFX.h>
// !!!!  ============================================================================================ !!!!

#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

//#include "BitmapDisplay.h"
#include "TextDisplay.h"

// select the display class (only one), matching the kind of display panel
#define GxEPD2_DISPLAY_CLASS GxEPD2_BW

/// select the display driver class (only one) for your  panel
#define GxEPD2_DRIVER_CLASS GxEPD2_290_T94_V2 // GDEM029T94  128x296, SSD1680, Waveshare 2.9" V2 variant


// SS is usually used for CS. define here for easy change
#ifndef EPD_CS
#define EPD_CS 25
#endif

// somehow there should be an easier way to do this
#define GxEPD2_BW_IS_GxEPD2_BW true
#define IS_GxEPD(c, x) (c##x)
#define IS_GxEPD2_BW(x) IS_GxEPD(GxEPD2_BW_IS_, x)


#if defined(ESP32)
#define MAX_DISPLAY_BUFFER_SIZE 65536ul // e.g.
#if IS_GxEPD2_BW(GxEPD2_DISPLAY_CLASS)
#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8) ? EPD::HEIGHT : MAX_DISPLAY_BUFFER_SIZE / (EPD::WIDTH / 8))

#endif
// adapt the constructor parameters to your wiring
#if defined(ARDUINO_LOLIN_D32_PRO)
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=5*/ EPD_CS, /*DC=*/ 0, /*RST=*/ 2, /*BUSY=*/ 15));
#elif defined(ARDUINO_ESP32_DEV) // e.g. TTGO T8 ESP32-WROVER
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=5*/ EPD_CS, /*DC=*/ 4, /*RST=*/ 2, /*BUSY=*/ 33));
#else
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=5*/ EPD_CS, /*DC=*/ 4, /*RST=*/ 2, /*BUSY=*/ 33));
#endif
#endif

//BitmapDisplay bitmaps(display);

//E-ink Definition - End

// Wifi Manager
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#define TRIGGER_PIN 0

// wifimanager can run in a blocking mode or a non blocking mode
// Be sure to know how to process loops with no delay() if using non blocking
bool wm_nonblocking = false; // change to true to use non blocking

WiFiManager wm; // global wm instance
WiFiManagerParameter custom_field; // global param ( for non blocking w params )
int wifi_status = 0; //0 not connected, 1 connected

// Firebase
#include <Firebase_ESP_Client.h>
#include "time.h"
// Provide the token generation process info.
#include "addons/TokenHelper.h"
// Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"
// Insert Firebase project API Key
#define API_KEY "AIzaSyCfzSxYQxDzTB-djO_SIyqxn79iczkroS0"
// Insert Authorized Email and Corresponding Password
#define USER_EMAIL "usgaojin@gmail.com"
#define USER_PASSWORD "smarthive"
// Insert RTDB URLefine the RTDB URL
#define DATABASE_URL "https://smarthive-01-default-rtdb.firebaseio.com/"
// Define Firebase objects
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;
// Variable to save USER UID
String uid;
// Database main path (to be updated in setup with the user UID)
String databasePath;
// Database child nodes
String timestamp_Path = "/timestamp";
String sender_id_Path = "/sender_id";
String lora_counter_Path = "/lora_counter";
String now_local_year_Path = "/now_local_year";
String now_local_month_Path = "/now_local_month";
String now_local_day_Path = "/now_local_day";
String now_local_hour_Path = "/now_local_hour";
String now_local_minute_Path = "/now_local_minute";
String now_local_second_Path = "/now_local_second";
String SHT31_temp_Path = "/SHT31_temp";
String SHT31_hmd_Path = "/SHT31_hmd";
String HTU21DF_temp_Path = "/HTU21DF_temp";
String HTU21DF_hmd_Path = "/HTU21DF_hmd";
String BME280_temp_Path = "/BME280_temp";
String BME280_hmd_Path = "/BME280_hmd";
String BME280_pre_Path = "/BME280_pre";
String BME280_alt_Path = "/BME280_alt";
String BH1750_lux_Path = "/BH1750_lux";
String sound_average_Path = "/sound_average";
String weight_Path = "/weight";
String counter_in_Path = "/counter_in";
String counter_out_Path = "/counter_out";

// Parent Node (to be updated in every loop)
String parentPath;
int timestamp;
FirebaseJson json;
const char* ntpServer = "pool.ntp.org";

// Timer variables (send new readings every three minutes)
unsigned long sendDataPrevMillis = 0;
unsigned long timerDelay = 180000;

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
  }
  time(&now);
  return now;
}

// Firebase End


// Function Definition Start

void einkbasicinfo(String loc)
{
  display.setFullWindow();
  display.firstPage();
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(0, 12);

  DateTime now_local = rtc.now();

  display.print(now_local.year(), DEC);
  display.print("-");
  display.print(now_local.month(), DEC);
  display.print("-");
  display.print(now_local.day(), DEC);
  display.print(" ");
  display.print(now_local.hour(), DEC);
  display.print(":");
  display.print(now_local.minute(), DEC);
  display.print(" ");

  display.print("|");
  display.println(loc);


  if (lora_status == 0) {
    display.print("Data Fail");
  } else {
    display.print("Data OK");
  }

  if (sd_status == 0) {
    display.print("|SD Fail");
  } else if (sd_status == 2) {
    display.print("|No SD");
  } else {
    display.print("|SD OK");
  }

  if (wifi_status == 0) {
    display.println("|No Wifi");
  }
  else {
    display.println("|Wifi OK");
  }

  display.print("ID:");
  display.print(lora_counter);
  //display.print("|Recv:");
  //display.print(received_counter);
  display.print("|Cld:");
  display.println(firebase_counter);

}

void einkprintdata(float tmp1, float hmd1, float tmp2, float hmd2, float pre, float h, float lgt, float wgt, float snd, float ct_in, float ct_out)
{
  display.print("TmpUp:");
  display.print(tmp1, 0);
  display.print("C");

  display.print("|HmdUp:");
  display.print(hmd1, 0);
  display.println ("%");

  display.print("TmpDn:");
  display.print(tmp2, 0);
  display.print("C");

  display.print("|HmdDn:");
  display.print(hmd2, 0);
  display.println("%");

  display.print("P:");
  display.print(pre, 0);
  display.print("Pa");
  display.print("|H:");
  display.print(h, 0);
  display.println("m");
  //display.print("|Snd:");
  //display.println(snd, 0);


  display.print("I:");
  display.print(ct_in,0);
  display.print("|O:");
  display.print(ct_out,0);
  display.print("|L:");
  display.print(lgt, 0);
  display.print("|W:");
  display.println(wgt, 0);

}

void csvtoarray(String inputString, float (& init_array)[21] )
{

  int i1 = inputString.indexOf(',');
  int i2 = inputString.indexOf(',', i1 + 1);
  int i3 = inputString.indexOf(',', i2 + 1);
  int i4 = inputString.indexOf(',', i3 + 1);
  int i5 = inputString.indexOf(',', i4 + 1);
  int i6 = inputString.indexOf(',', i5 + 1);
  int i7 = inputString.indexOf(',', i6 + 1);
  int i8 = inputString.indexOf(',', i7 + 1);
  int i9 = inputString.indexOf(',', i8 + 1);
  int i10 = inputString.indexOf(',', i9 + 1);
  int i11 = inputString.indexOf(',', i10 + 1);
  int i12 = inputString.indexOf(',', i11 + 1);
  int i13 = inputString.indexOf(',', i12 + 1);
  int i14 = inputString.indexOf(',', i13 + 1);
  int i15 = inputString.indexOf(',', i14 + 1);
  int i16 = inputString.indexOf(',', i15 + 1);
  int i17 = inputString.indexOf(',', i16 + 1);
  int i18 = inputString.indexOf(',', i17 + 1);
  int i19 = inputString.indexOf(',', i18 + 1);
  int i20 = inputString.indexOf(',', i19 + 1);
  int i21 = inputString.indexOf(',', i20 + 1);

  String v1 = inputString.substring(0, i1);
  String v2 = inputString.substring(i1 + 1, i2);
  String v3 = inputString.substring(i2 + 1, i3);
  String v4 = inputString.substring(i3 + 1, i4);
  String v5 = inputString.substring(i4 + 1, i5);
  String v6 = inputString.substring(i5 + 1, i6);
  String v7 = inputString.substring(i6 + 1, i7);
  String v8 = inputString.substring(i7 + 1, i8);
  String v9 = inputString.substring(i8 + 1, i9);
  String v10 = inputString.substring(i9 + 1, i10);
  String v11 = inputString.substring(i10 + 1, i11);
  String v12 = inputString.substring(i11 + 1, i12);
  String v13 = inputString.substring(i12 + 1, i13);
  String v14 = inputString.substring(i13 + 1, i14);
  String v15 = inputString.substring(i14 + 1, i15);
  String v16 = inputString.substring(i15 + 1, i16);
  String v17 = inputString.substring(i16 + 1, i17);
  String v18 = inputString.substring(i17 + 1, i18);
  String v19 = inputString.substring(i18 + 1, i19);
  String v20 = inputString.substring(i19 + 1, i20);
  String v21 = inputString.substring(i20 + 1, i21);

  init_array[0] = v1.toFloat();
  init_array[1] = v2.toFloat();
  init_array [2] = v3.toFloat();
  init_array [3] = v4.toFloat();
  init_array [4] = v5.toFloat();
  init_array [5] = v6.toFloat();
  init_array [6] = v7.toFloat();
  init_array [7] = v8.toFloat();
  init_array [8] = v9.toFloat();
  init_array [9] = v10.toFloat();
  init_array [10] = v11.toFloat();
  init_array [11] = v12.toFloat();
  init_array [12] = v13.toFloat();
  init_array [13] = v14.toFloat();
  init_array [14] = v15.toFloat();
  init_array [15] = v16.toFloat();
  init_array [16] = v17.toFloat();
  init_array [17] = v18.toFloat();
  init_array [18] = v19.toFloat();
  init_array [19] = v20.toFloat();
  init_array [20] = v21.toFloat();
}


void wifimanager_checkButton() {
  // check for button press
  if ( digitalRead(TRIGGER_PIN) == LOW ) {
    // poor mans debounce/press-hold, code not ideal for production
    delay(50);
    if ( digitalRead(TRIGGER_PIN) == LOW ) {
      Serial.println("Button Pressed");
      //wm.resetSettings();
      display.setCursor(0, 10);
      display.setTextColor(GxEPD_BLACK);
      display.setFont(&FreeMonoBold9pt7b);
      display.setFullWindow();
      display.firstPage();
      display.println("Entering WiFi Setting!");
      display.println("Please connect Hotspot");
      display.println("Name: SmartHive WiFiManager");
      display.println();
      display.println("Timeout 120s Started");
      display.println("Hold 10s to Reset Settings");

      while (display.nextPage());

      // still holding button for 10000 ms, reset settings, code not ideaa for production
      delay(10000); // reset delay hold

      if ( digitalRead(TRIGGER_PIN) == LOW ) {
        Serial.println("Button Held");
        Serial.println("Erasing Config, restarting");
        display.setCursor(0, 10);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setFullWindow();
        display.firstPage();
        display.setTextColor(GxEPD_BLACK);
        display.println("WiFi Reset done.");
        display.println("Restarting!");
        while (display.nextPage());
        wm.resetSettings();
        ESP.restart();
      }

      // start portal w delay
      Serial.println("Starting config portal");
      wm.setConfigPortalTimeout(300);

      if (!wm.startConfigPortal("SmartHive WifiManager")) {
        Serial.println("Failed to connect or hit timeout");
        Serial.println("5min Timeout");
        display.setCursor(0, 10);
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setFullWindow();
        display.firstPage();
        display.firstPage();
        display.println("5min Timeout.Restarting");
        display.println("Wifi Connection Fail");
        display.println("Please Try Again");
        display.println("Restarting SmartHive");
        while (display.nextPage());
        delay(3000);
        ESP.restart();
      }
      else {
        //if you get here you have connected to the WiFi
        Serial.println("Connected to WiFi!");
        display.setCursor(0, 10);
        wifi_status = 1;
        display.setTextColor(GxEPD_BLACK);
        display.setFont(&FreeMonoBold9pt7b);
        display.setFullWindow();
        display.firstPage();
        display.println("WiFi Connected!");
        display.println("Restarting SmartHive");
        while (display.nextPage());
        delay(3000);
        ESP.restart();
      }
    }
  }
}

String getParam(String name) {
  //read parameter from server, for customhmtl input
  String value;
  if (wm.server->hasArg(name)) {
    value = wm.server->arg(name);
  }
  return value;
}

void saveParamCallback() {
  Serial.println("[CALLBACK] saveParamCallback fired");
  Serial.println("PARAM customfieldid = " + getParam("customfieldid"));
}

// Function Definition End



void setup()
{
  Serial.begin(115200);
  Serial.println();
  Serial.println("setup");
  SPI.end(); // release standard SPI pins, e.g. SCK(18), MISO(19), MOSI(23), SS(5)
  SPI.begin(18, 19, 23, 15); // map and init SPI pins SCK(18), MISO(19), MOSI(23), SS(5)

  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    display.println("Couldn't find Clock");
    //while (1);
  }
  else {
    Serial.println("RTC Clock OK");
    display.println("Clock OK");
  }
  //RTC Clock Setup
  //rtc.adjust(DateTime(__DATE__, __TIME__));

  //E-Ink Setup
  //EINKSPI.begin(18, 19, 23, 25);
  delay(100);
  display.init(115200);
  // first update should be full refresh
  helloWorld(display);
  delay(2000);
  display.setFullWindow();
  display.firstPage();
  display.fillScreen(GxEPD_WHITE);
  display.setCursor(0, 10);

  //E-ink Test Codes
  /*
    helloFullScreenPartialMode(display);   // partial refresh mode can be used to full screen,effective if display panel hasFastPartialUpdate
    delay(1000);
    helloArduino(display);
    delay(1000);
    helloEpaper(display);
    delay(1000);
    helloValue(display, 123.9, 1);
    delay(1000);
    showFont(display, "FreeMonoBold9pt7b", &FreeMonoBold9pt7b);
    delay(1000);

    if (display.epd2.hasPartialUpdate)
    {
      Serial.println("hasPartialUpdate");
      showPartialUpdate();
      delay(1000);
    }

    else {Serial.println("noPartialUpdate");}
    BitmapDisplay(display).drawBitmaps();
    bitmaps.drawBitmaps();
    Serial.println("drawBitmaps");
    return;
    drawCornerTest();
    showBox(16, 16, 48, 32, false);
    showBox(16, 56, 48, 32, true);
    display.powerOff();
    deepSleepTest();
  */

  //Lora Setup
  //LoRaSPI.begin(18,19,23,16); //CS0 = 5 CS2 = 14
  /*  Serial Test
    Serial.println(MOSI);
    Serial.println(MISO);
    Serial.println(SCK);
    Serial.println(SS);
  */
  //while (!Serial);
  LoRa.setPins(15, 2, 27); // CS RST DIO0
  Serial.println("LoRa Sender");

  if (!LoRa.begin(868E6)) {
    Serial.println("Starting LoRa failed!");
    display.println("Starting LoRa failed!");
    //while (1);
  }
  if (LoRa.begin(868E6)) {
    Serial.println("Starting LoRa Success!");
    display.println("Data Transfer OK!");
    lora_status = 1;
  }

  //LoRa.onReceive(cbk);
  LoRa.receive();
  Serial.println("Lora Receiver Prepared");

  //SD Setup
  //SDSPI.begin(18, 19, 23, 5); //SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI, SDCARD_CS
  if (!SD.begin(5, SPI)) { //SDCARD_CS, SDSPI
    Serial.println("Card Mount Failed");
    display.println("Card Mount Failed");
    //return;  //Continue Operation
  }
  else {
    sd_status = 1;
    Serial.println("Card Mount Success");
  }

  uint8_t cardType = SD.cardType();


  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    display.println("No SD card attached");
    sd_status = 2;
    //return; //Continue Operation
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
    sd_status = 3;
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
    sd_status = 3;
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
    sd_status = 3;
  } else {
    Serial.println("UNKNOWN");
    sd_status = 3;
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);

  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
  Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
  display.print("SD Card Size: ");
  display.print(cardSize);
  display.println(" MB");

  display.print("Used space: ");
  display.print(SD.usedBytes() / (1024 * 1024));
  display.println(" MB");



  // WifiManager Setup Start
  pinMode(TRIGGER_PIN, INPUT);
  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP
  Serial.println("\n WiFi Manager Starting");

  if (wm_nonblocking) wm.setConfigPortalBlocking(false);

  // add a custom input field
  //int customFieldLength = 40;

  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\"");
  // test custom html input type(checkbox)
  // new (&custom_field) WiFiManagerParameter("customfieldid", "Custom Field Label", "Custom Field Value", customFieldLength,"placeholder=\"Custom Field Placeholder\" type=\"checkbox\""); // custom html type
  // test custom html(radio)
  //const char* custom_radio_str = "<br/><label for='customfieldid'>Custom Field Label</label><input type='radio' name='customfieldid' value='1' checked> One<br><input type='radio' name='customfieldid' value='2'> Two<br><input type='radio' name='customfieldid' value='3'> Three";
  //new (&custom_field) WiFiManagerParameter(custom_radio_str); // custom html input
  //wm.addParameter(&custom_field);
  //wm.setSaveParamsCallback(saveParamCallback);
  // custom menu via array or vector
  // menu tokens, "wifi","wifinoscan","info","param","close","sep","erase","restart","exit" (sep is seperator) (if param is in menu, params will not show up in wifi page!)
  // const char* menu[] = {"wifi","info","param","sep","restart","exit"};
  // wm.setMenu(menu,6);
  std::vector<const char *> menu = {"wifi", "info", "param", "sep", "restart", "exit"};
  wm.setMenu(menu);
  // set dark theme
  //wm.setClass("invert");
  //set static ip
  // wm.setSTAStaticIPConfig(IPAddress(10,0,1,99), IPAddress(10,0,1,1), IPAddress(255,255,255,0)); // set static ip,gw,sn
  // wm.setShowStaticFields(true); // force show static ip fields
  // wm.setShowDnsFields(true);    // force show dns field always


  // wm.setCaptivePortalEnable(false); // disable captive portal redirection
  // wm.setAPClientCheck(true); // avoid timeout if client connected to softap
  // wifi scan settings
  // wm.setRemoveDuplicateAPs(false); // do not remove duplicate ap names (true)
  // wm.setMinimumSignalQuality(20);  // set min RSSI (percentage) to show in scans, null = 8%
  // wm.setShowInfoErase(false);      // do not show erase button on info page
  // wm.setScanDispPerc(true);       // show RSSI as percentage not graph icons
  // wm.setBreakAfterConfig(true);   // always exit configportal even if wifi save fails
  
  wm.setConnectTimeout(10); // how long to try to connect for before continuing
  bool res;
  //No need to configure wifi, run offlinemode
  wm.setConfigPortalTimeout(5); // auto close configportal after n seconds

  // res = wm.autoConnect(); // auto generated AP name from chipid
  res = wm.autoConnect("SmartHive WifiManager"); // anonymous ap
  // res = wm.autoConnect("AutoConnectAP","password"); // password protected ap

  if (!res) {
    Serial.println("Failed to connect wifi or hit timeout, run offline mode");

    // ESP.restart();
  }
  else {
    //if you get here you have connected to the WiFi
    Serial.println("Connected to Wifi!)");
    display.println("WiFi Connected!");
    wifi_status = 1;
  }
  // WifiManager Setup End

  // Firebase Setup Start

  if (wifi_status == 0) {
    Serial.println("Failed to connect wifi, Firebase Skip Setup");
    display.println("NO WiFi");
    display.println("Press Button to Set WiFi!");
  }
  else
  {
    configTime(0, 0, ntpServer); // May not useful when no WIfI, need change to local RTC

    // Assign the api key (required)
    config.api_key = API_KEY;
    // Assign the user sign in credentials
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    // Assign the RTDB URL (required)
    config.database_url = DATABASE_URL;
    Firebase.reconnectWiFi(true);
    fbdo.setResponseSize(4096);

    // Assign the callback function for the long running token generation task */
    config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
    // Assign the maximum retry of token generation
    config.max_token_generation_retry = 5;
    // Initialize the library with the Firebase authen and config
    Firebase.begin(&config, &auth);

    // Getting the user UID might take a few seconds
    Serial.println("Getting User UID");
    while ((auth.token.uid) == "") {
      Serial.print('.');
      delay(1000);
    }
    // Print user UID
    uid = auth.token.uid.c_str();
    Serial.print("User UID: ");
    Serial.println(uid);

    // Update database path
    databasePath = "/UsersData/" + uid + "/readings";
  }


  //Print The Rest
  Serial.println("Setup Done");
  display.println("Setup Done");
  while (display.nextPage());
  delay(3000);
}

void loop()
{

  // WifiManager Loop Start ----------------------------------------------------------------------------------------------------
  if (wm_nonblocking) wm.process(); // avoid delays() in loop when non-blocking and other long running code
  wifimanager_checkButton();
  // WifiManager Loop End

  // Get Local Time  ----------------------------------------------------------------------------------------------------
  DateTime now_local = rtc.now();
  // Lora send packet
  /*
    LoRa.beginPacket();
    LoRa.print("hello ");
    LoRa.print(counter);
    LoRa.endPacket();
    counter++;
  */


  // Lora Receive packet ----------------------------------------------------------------------------------------------------
  // Initialize Data Pack

  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.println("Received packet:");

    // read packet
    while (LoRa.available()) {
      data_previous = data_received;
      data_received = LoRa.readString();
    }

    if (data_received != data_previous) {
      Serial.print("Lora Received: ");
      Serial.println(data_received);
      received_counter++;
      // print RSSI of packet
      //Serial.print("' with RSSI ");
      //Serial.println(LoRa.packetRssi());

      //Data Convert ----------------------------------------------------------------------------------------------------
      //  data_line = sender_id + String(",") + lora_counter + String(",") + now_local.year() + String(",") + now_local.month() + String(",") + now_local.day() + String(",") + now_local.hour() + String(",") + now_local.minute() + String(",") + now_local.second()
      // +  SHT31_temp + String(",") + SHT31_hmd + String(",") + HTU21DF_temp + String(",") + HTU21DF_hmd + String(",") + BME280_temp + String(",") + BME280_hmd + String(",") + BME280_pre + String(",") + BME280_alt + String(",")
      //+  BH1750_lux + String(",") + weight + String(",") + sound_average + String(",") + counter_in + String(",") + counter_out + String(",");;

      csvtoarray(data_received, data_array);
      sender_id = round(data_array[0]);
      lora_counter = round(data_array[1]);
      now_local_year = round(data_array[2]);
      now_local_month = round(data_array[3]);
      now_local_day = round(data_array[4]);
      now_local_hour = round(data_array[5]);
      now_local_minute = round(data_array[6]);
      now_local_second = round(data_array[7]);
      SHT31_temp = data_array[8];
      SHT31_hmd = data_array[9];
      HTU21DF_temp = data_array[10];
      HTU21DF_hmd = data_array[11];
      BME280_temp = data_array[12];
      BME280_hmd = data_array[13];
      BME280_pre = data_array[14];
      BME280_alt = data_array[15];
      BH1750_lux = data_array[16];
      weight = data_array[17];
      sound_average = data_array[18];
      counter_in = data_array[19];
      counter_out = data_array[20];

      if (sender_id == 1) {
        lora_counter = lora_counter;
        SHT31_temp_1 = SHT31_temp;
        SHT31_hmd_1 = SHT31_hmd;
        HTU21DF_temp_1 = HTU21DF_temp;
        HTU21DF_hmd_1 = HTU21DF_hmd;
        BME280_temp_1 = BME280_temp;
        BME280_hmd_1 = BME280_hmd;
        BME280_pre_1 = BME280_pre;
        BME280_alt_1 = BME280_alt;
        BH1750_lux_1 = BH1750_lux;
        weight_1 = weight;
        sound_average_1 = sound_average;
        counter_in_1 = counter_in;
        counter_out_1 = counter_out;        
      }

      else if (sender_id == 2) {
        lora_counter = lora_counter;
        SHT31_temp_2 = SHT31_temp;
        SHT31_hmd_2 = SHT31_hmd;
        HTU21DF_temp_2 = HTU21DF_temp;
        HTU21DF_hmd_2 = HTU21DF_hmd;
        BME280_temp_2 = BME280_temp_1;
        BME280_hmd_2 = BME280_hmd_1;
        BME280_pre_2 = BME280_pre_1;
        BME280_alt_2 = BME280_alt_1;
        BH1750_lux_2 = BH1750_lux_1;
        weight_2 = weight;
        sound_average_2 = sound_average;
        counter_in_2 = counter_in_1;
        counter_out_2 = counter_out_1; 
      }

      // Save data to sd ----------------------------------------------------------------------------------------------------
      //listDir(SD, "/", 0);
      if (!SD.begin(5, SPI)) { //if sd doesn't exist, skip this step
      }
      else {

        folder_name = now_local.year() + String("_") + now_local.month() + String("_") + now_local.day();
        file_name = now_local.year() + String("_") + now_local.month() + String("_") + now_local.day() + String("_") + now_local.hour() + String(".csv") ;

        data_line = sender_id + String(",") + lora_counter + String(",") + now_local_year + String(",") + now_local_month + String(",") + now_local_day + String(",") + now_local_hour + String(",") + now_local_minute + String(",") + now_local_second + String(",")
                    +  SHT31_temp + String(",") + SHT31_hmd + String(",") + HTU21DF_temp + String(",") + HTU21DF_hmd + String(",") + BME280_temp + String(",") + BME280_hmd + String(",") + BME280_pre + String(",") + BME280_alt + String(",")
                    +  BH1750_lux + String(",") + weight + String(",") + sound_average + String(",") + counter_in + String(",") + counter_out + String(",");

        Serial.print("folder_name= ");
        Serial.println (folder_name);
        Serial.print("file_name= ");
        Serial.println (file_name);
        Serial.println ();
        Serial.print("data_line= ");
        Serial.println (data_line);
        Serial.println ();

        // see if the id directory exists, create it if not.
        if ( !SD.exists(String("/") + sender_id) )
        {
          if ( SD.mkdir(String("/") + sender_id)  )
          {
            Serial.println("Sender directory created");
          }
          else {
            Serial.println("Sender directory not created(error)");
          }
        }
        else
        {
          Serial.println("Sender directory exists");
        }

        // see if the directory exists, create it if not.
        if ( !SD.exists(String("/") + sender_id + String("/") + folder_name) )
        {
          if ( SD.mkdir(String("/") + sender_id + String("/") + folder_name)  )
          {
            Serial.println("File directory created");
          }
          else {
            Serial.println("File directory not created(error)");
          }
        }
        else
        {
          Serial.println("File directory exists");
        }

        // see if the file exists, create it if not.
        if ( !SD.exists(String("/") + sender_id + String("/") + folder_name + String("/") + file_name) )
        {
          Serial.println("File doesn't exist");
          myFile = SD.open(String("/") + sender_id + String("/") + folder_name + String("/") + file_name, FILE_WRITE);
          myFile.close();
          Serial.println("File created");
        }
        else
        {
          Serial.println("File exists");
        }

        //Append file
        Serial.print("Appending to file: ");
        Serial.println(file_name);

        myFile = SD.open(String("/") + sender_id + String("/") + folder_name + String("/") + file_name, FILE_APPEND);
        if (!myFile) {
          Serial.println("Failed to open file for appending");
        }
        if (myFile.println(data_line)) {
          Serial.println("Data appended");
        } else {
          Serial.println("Append failed");
        }
        myFile.close();
      }

      // Save Data to Firebase

      // Firebase Start
      if (wifi_status == 0) {
        // No Wifi, Offline Mode
        Serial.println("Firebase offline mode");
      }

      else {
        // Online Mode Send new readings to database
        //Get current timestamp
        timestamp = getTime();
        Serial.print ("Firebase Sending: ");
        Serial.print ("time: ");
        Serial.println (timestamp);

        parentPath = databasePath + "/" + sender_id + "/" + String(timestamp);
        json.set(timestamp_Path, String(timestamp));
        json.set(sender_id_Path, sender_id);
        json.set(lora_counter_Path, lora_counter);
        json.set(now_local_year_Path, now_local_year);
        json.set(now_local_month_Path, now_local_month);
        json.set(now_local_day_Path, now_local_day);
        json.set(now_local_hour_Path, now_local_hour);
        json.set(now_local_minute_Path, now_local_minute);
        json.set(now_local_second_Path, now_local_second);
        json.set(SHT31_temp_Path, SHT31_temp);
        json.set(SHT31_hmd_Path, SHT31_hmd);
        json.set(HTU21DF_temp_Path, HTU21DF_temp);
        json.set(HTU21DF_hmd_Path, HTU21DF_hmd);
        json.set(BME280_temp_Path, BME280_temp);
        json.set(BME280_hmd_Path, BME280_hmd);
        json.set(BME280_pre_Path, BME280_pre);
        json.set(BME280_alt_Path, BME280_alt);
        json.set(BH1750_lux_Path, BH1750_lux);
        json.set(sound_average_Path, sound_average);
        json.set(weight_Path, weight);
        json.set(counter_in_Path, counter_in);
        json.set(counter_out_Path, counter_out);

        Serial.printf("Set json... %s\n", Firebase.RTDB.setJSON(&fbdo, parentPath.c_str(), &json) ? "ok" : fbdo.errorReason().c_str());
        Serial.printf("Firebase Done");
        firebase_counter++;
      }
      // Firebase Loop End

    } // End of if Lora Received Data and different data
  } // End of if Lora Received Data

  // E-ink Display ----------------------------------------------------------------------------------------------------
  // Display every 10 seconds
  unsigned long currentMillis = millis();
  if (currentMillis - lastExecutedMillis_1 >= EINK_INTERVAL_1) {
    lastExecutedMillis_1 = currentMillis; // save the last executed time

    //Screen 1
    Serial.print("displaying page 1");
    einkbasicinfo("1:In"); //Display basic info
    // Then Display Data
    einkprintdata(SHT31_temp_1, SHT31_hmd_1, HTU21DF_temp_1, HTU21DF_hmd_1, BME280_pre_1, BME280_alt_1, BH1750_lux_1, weight_1, sound_average_1,counter_in_1,counter_out_1);
    while (display.nextPage());
  }
  //currentMillis = millis();
  if (currentMillis - lastExecutedMillis_2 >= EINK_INTERVAL_2) {
    lastExecutedMillis_2 = currentMillis; // save the last executed time

    //Screen 2
    //Display basic info
    Serial.print("displaying page 2");
    display.setTextSize(0.5);
    einkbasicinfo("2:In");
    // Then Display Data
    einkprintdata(SHT31_temp_2, SHT31_hmd_2, HTU21DF_temp_2, HTU21DF_hmd_2, BME280_pre_2, BME280_alt_2, BH1750_lux_2, weight_2, sound_average_2,counter_in_2,counter_out_2);
    while (display.nextPage());
  }


  //currentMillis = millis();
  if (currentMillis - lastExecutedMillis_3 >= EINK_INTERVAL_3) {
    lastExecutedMillis_3 = currentMillis; // save the last executed time
    //Screen 3
    //Display basic info
    Serial.print("displaying page 3");
    einkbasicinfo("3:Ext");
    // Then Display Data
    einkprintdata(BME280_temp_1, BME280_hmd_1, 0, 0, BME280_pre_1, BME280_alt_1, BH1750_lux_1, weight_1, sound_average_1,counter_in_1,counter_out_1);
    while (display.nextPage());
  }
}

// End of Void Loop
