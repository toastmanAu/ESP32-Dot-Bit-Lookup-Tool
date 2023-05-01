//#include <arduino.h>  //Uncomment for Platform.IO
#include <vector>
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include "qrcode.h"
#include "ESP32_SPI_9341.h"
#include <time.h>
#include <TimeLib.h>
#include <ESP32Time.h>
#include <ESP_Mail_Client.h>
#include <EEPROM.h>

//Pin Definitions
#define SD_SCK 18
#define SD_MISO 19
#define SD_MOSI 23
#define SD_CS 5
#define LIGHT_ADC 34

//Colour Definitions
#define LG_BG 0x9F55
#define DG_BG 0x1304
#define TFT_GREY 0x2146
#define EMAIL_BG 0xDE54

//String Constant Def
#define WTAPI "http://worldtimeapi.org/api/timezone/"
#define DOTBIT_ADD_SERVER "https://indexer-v1.did.id/v1/account/records"
#define ntpServer "pool.ntp.org"

//Variables
String WIFI_SSID = "NONE";
String WIFI_PASSWORD = "NONE";
int led_pin[3] = { 17, 4, 16 };   //Onboard RGB Not used yet...
int spk_pin = 26;                 //Internal Speaker Pin on Suntron 2.8" Esp32-HMI 
int CUR_PAGE = 0;                 //0-Main, 1-Search Keyboard, 2-, 3-, 4-, 5-, 6-, 7-, 8-, 9-,10-,11-
int LAST_PAGE = 0;    
int CUR_SETTING = 0;
bool CAPS_LOCK = false;
bool SHIFT = false;
char TEXTBOX[51];
int TEXT_COUNT = 0;
String TEXT_BOX = "";
String MY_DOT_BIT = "";
String CURRENT_DOT_BIT = "";
String CUR_REGION = "";
String CUR_CITY = "";
String HOST_NAME = "";
bool TwentyFourHour = true;
long CUR_TIME_OFFSET;
int CUR_NUM_RECORDS = 0;
int CUR_ADDRESSES = 0;
int CUR_PROFILES = 0;
int PRO_PAGES = 0;
int ADD_PAGES = 0;
bool HAS_KEY[66];
long LAST_TOUCH = millis();
long NOTE_TIMER;
long SHOT_CLOCK;
bool SAVED_NOTE = false;
int REC_PAGE_A = 0;
int REC_PAGE_B = 0;
SPIClass SD_SPI;
const char* DB_PROFILE[23] = { "twitter", "facebook", "reddit", "linkedin", "github", "telegram", "description", "avatar", "instagram", "weibo", "discord", "email", "website", "youtube", "bilibili", "tiktok", "jike", "nextid", "dribbble", "behance", "mirror", "medium", "nostr" };
const char* DB_ADDRESS[38] = { "btc", "eth", "ckb", "bch", "ltc", "doge", "xrp", "dot", "fil", "trx", "eos", "iota", "xmr", "bsc", "heco", "xem", "etc", "dash", "zec", "zil", "flow", "iost", "sc", "near", "ksm", "atom", "xtz", "bsv", "sol", "vet", "xlm", "ada", "polygon", "terra", "avalanche", "dfinity", "stacks", "celo" };
const char* DB_DWEB[5] = { "ipfs", "ipns", "resilio", "skynet", "arweave" };
String DB_KEY[66];
String DB_VALUE[66];
int DB_NUM[66];
String CUR_PRO[28];
String CUR_ADD[38];
String CUR_PV[28];
String CUR_AV[38];
int PRO_ON_PAGE = 0;
int ADD_ON_PAGE = 0;
String OUTPUT_LINK = "";
int SAVED_REC_PAGE = 0;
int SAVED_REC_PAGES = 0;
int SAVED_REC_ON_PAGE = 0;
String SAVED_REC[10];
int NUM_SAVED = 0;
String CUR_QR = "";
struct tm curTime;
int timeZone;
String TZSTRING = "";
long gmtOffset = 34200;
long dayLightOffset = 3600;
bool isDLT = false;
long qTIME;
uint16_t QR_BG = 0x0000;
uint16_t QR_MAIN = 0xffff;
uint16_t QR_BOR = 0x04c4;
String CURLINK;
int SAVREC = 0;
bool SAVED_SETTINGS = false;
long eTIMER = 0;
bool showEP = false;
bool showWP = false;
String SMTP_HOST = "NONE";
uint16_t SMTP_PORT = esp_mail_smtp_port_587;
String AUTHOR_EMAIL = "NONE";
String AUTHOR_PASSWORD = "NONE";
String AUTHOR_NAME = ".bit QR Sender";
String RECIPIENT_EMAIL = "NONE";
String EMAIL_SUBJECT = "NONE";
String EMAIL_BODY = "NONE";
String FN_HOLDER = "";
String UN_HOLDER = "";
String QR_HOLDER = "";
bool NOWIFI = false;
long WIFITIMER = 0;

//Global Instances
DynamicJsonDocument doc(200);
DynamicJsonDocument dbResponse(20000);
JsonObject object;
LGFX lcd;
ESP32Time rtc;
WiFiClientSecure secured_client;
QRCode qrcode;
DeserializationError error;
SMTPSession smtp;

//Function Declarations
String getEmailFromDotBit(String DOTBIT_NAME);
void EMAIL_SENDER(String FN_, String UN_, String CQ_);
bool isWCAG(uint16_t COL1, uint16_t COL2);
void OPEN_SAVED_QR();
void LOAD_PASSWORD();
void LOAD_SSID();
void CLEAR_TEXTBOX();
void loadFromEeprom();
void saveToEeprom();
void DISPLAY_CLOCK();
void GET_TIMEZONE(String REGION_, String CITY_);
void sendMailAttach(String QR_FILE_, String SUBJECT_, String BODY_);
void smtpCallback(SMTP_Status status);
void colourPicker(String SETTING_NAME, uint16_t* COLOUR_CODE);
void GrabImage(String FILE_N, String VALUE_IN);
void playTouch(int STYLE);
void PRINT_RECALLED(String FILENAME_);
void UPDATE_SAVED_RECORDS();
void OPEN_SAVED_RECORDS();
void SET_OUTPUT_LINK(String IN_KEY, int IN_REF);
void GEN_QRCODE(String INPUT_TEXT, String INPUT_NAME);
void UPDATE_ADDRESSES(int AP);
void UPDATE_PROFILES(int PP);
void CHECK_RECORD();
void SAVE_RECORD();
void PRINT_RESPONSE();
int GET_DOT_BIT_RECORDS(String dotbit_name);
void UNSHIFT();
void REFRESH_PAGE(int cPAGE);
void KEY_TOUCH(int X_POS, int Y_POS);
void downloadImageAndDisplay(String urlSender);
void touch_calibration();
void listDir(fs::FS& fs, const char* dirname, uint8_t levels);
void sd_test();
void sd_init();

void setup(void) {
  pinMode(led_pin[0], OUTPUT);
  pinMode(led_pin[1], OUTPUT);
  pinMode(led_pin[2], OUTPUT);
  pinMode(spk_pin, OUTPUT);
  Serial.begin(115200);
  lcd.init();
  sd_init();
  sd_test();
  lcd.setRotation(1);
  lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
  EEPROM.begin(500);
  loadFromEeprom();

  char TSSID[50];
  for (int p = 0; p < WIFI_SSID.length(); p++) {
    TSSID[p] = WIFI_SSID.charAt(p);
  }
  char TPASS[50];

  for (int p = 0; p < WIFI_PASSWORD.length(); p++) {
    TPASS[p] = WIFI_PASSWORD.charAt(p);
  }

  WIFITIMER = millis();

  WiFi.begin(TSSID, TPASS);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
    if (millis() - WIFITIMER > 10000) {
      Serial.println("WiFi Not Connected");
      NOWIFI = true;
      WiFi.disconnect();
      break;
    }
  }

  Serial.println(WiFi.localIP());
  delay(500);

  if (!NOWIFI) {
    configTime(gmtOffset, 0, "pool.ntp.org");  // get UTC time via NTP
    struct tm timeinfo;
    if (getLocalTime(&timeinfo)) {
      rtc.setTimeStruct(timeinfo);
    }
    time_t now = time(nullptr);
    while (now < 24 * 3600) {
      Serial.print(".");
      delay(100);
      now = time(nullptr);
    }
  }

  //Serial.println(now);
  touch_calibration();
  lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
  if(NOWIFI){
    lcd.setCursor(15,36);
    lcd.setTextSize(2);
    lcd.setTextColor(TFT_RED);
    lcd.print("No WiFi Connection");
  }
  REFRESH_PAGE(0);
  DISPLAY_CLOCK();
}

void loop(void) {
  int XPOS;
  int YPOS;

  if (NOWIFI && millis() - WIFITIMER > 30000) {
    char TSSID[50];
    for (int p = 0; p < WIFI_SSID.length(); p++) {
      TSSID[p] = WIFI_SSID.charAt(p);
    }
    char TPASS[50];

    for (int p = 0; p < WIFI_PASSWORD.length(); p++) {
      TPASS[p] = WIFI_PASSWORD.charAt(p);
    }
    WIFITIMER = millis();
    WiFi.begin(TSSID, TPASS);
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
      if (millis() - WIFITIMER > 5000) {
        Serial.println("WiFi Not Connected");
        NOWIFI = true;
        WiFi.disconnect();
        WIFITIMER = millis();
        break;
      }
    }
    if (WiFi.status() == WL_CONNECTED) {
      NOWIFI = false;
      configTime(gmtOffset, 0, "pool.ntp.org");  // get UTC time via NTP
      struct tm timeinfo;
      if (getLocalTime(&timeinfo)) {
        rtc.setTimeStruct(timeinfo);
      }
      time_t now = time(nullptr);
      while (now < 24 * 3600) {
        Serial.print(".");
        delay(100);
        now = time(nullptr);
      }
    }
  }

  if (millis() - SHOT_CLOCK > 59999) {
    DISPLAY_CLOCK();
  }

  if (millis() - NOTE_TIMER > 3000 && SAVED_NOTE == true && CUR_PAGE == 2) {
    SAVED_NOTE = false;
    lcd.drawPngFile(SD, "/gui/save.png", 287, 63);
  }
  if (millis() - qTIME > 2000 && SAVED_NOTE == true && CUR_PAGE == 3) {
    SAVED_NOTE = false;
    lcd.drawPngFile(SD, "/gui/qSAVE.png", 290, 30);
  }
  if (lcd.getTouch(&XPOS, &YPOS) && millis() - LAST_TOUCH > 150) {
    KEY_TOUCH(XPOS, YPOS);
    LAST_TOUCH = millis();
  }

  if (CUR_PAGE == 3 && millis() - eTIMER > 3000) {
    lcd.drawPngFile(SD, "/gui/email.png", 260, 30);
  }

  delay(10);
}

//SD Initialisation from example
void sd_init() {
  SD_SPI.begin(SD_SCK, SD_MISO, SD_MOSI);
  if (!SD.begin(SD_CS, SD_SPI, 40000000)) {
    Serial.println("Card Mount Failed");
    lcd.setCursor(10, 10);
    lcd.println("SD Card Failed");
    while (1)
      delay(1000);
  } else {
    Serial.println("Card Mount Successed");
  }

  Serial.println("SD init over.");
}

//SD Test from example
void sd_test() {
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No SD card attached");
    return;
  }

  Serial.print("SD Card Type: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }

  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("SD Card Size: %lluMB\n", cardSize);
  listDir(SD, "/", 0);
}

//Directory Structure Check
void listDir(fs::FS& fs, const char* dirname, uint8_t levels) {
  Serial.printf("Listing directory: %s\n", dirname);
  int TCOUNT = 0;
  File root = fs.open(dirname);
  if (!root) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!root.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File file = root.openNextFile();
  while (file) {
    if (file.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(file.name());
      if (levels) {
        listDir(fs, file.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(file.name());
      Serial.print("  SIZE: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }

  File RECS = fs.open("/records");
  if (!RECS) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!RECS.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File JSON_REC = RECS.openNextFile();
  while (JSON_REC) {
    if (JSON_REC.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(JSON_REC.name());
      if (levels) {
        listDir(fs, JSON_REC.path(), levels - 1);
      }
    } else {
      Serial.print("  FILE: ");
      Serial.print(JSON_REC.name());
      Serial.print("  SIZE: ");
      Serial.println(JSON_REC.size());
      String JCHECK = JSON_REC.name();
      if (JCHECK.indexOf(".json") != -1) {
        TCOUNT++;
      }
    }
    JSON_REC = RECS.openNextFile();
  }
  SAVREC = TCOUNT;
}

//Touch Screen Calibration
void touch_calibration() {
  lcd.fillScreen(LG_BG);

  lcd.setTextColor(TFT_BLACK);
  lcd.setTextSize(2);
  lcd.setCursor(125, 90);
  lcd.println("SCREEN");
  lcd.setCursor(95, 120);
  lcd.println("CALIBRATION");

  std::uint16_t fg = TFT_WHITE;
  std::uint16_t bg = TFT_BLACK;
  if (lcd.isEPD())
    std::swap(fg, bg);
  lcd.calibrateTouch(nullptr, fg, bg, std::max(lcd.width(), lcd.height()) >> 3);
}

//Download and display image for avatar display
void downloadImageAndDisplay(String urlSender) {
  int IMAGE_TYPE = 0;  //0 - not compatible, 1 - bmp, 2 - png, 3 - jpeg
  String IMAGE_HEADER;

  if (urlSender.indexOf(".bmp") != -1) {
    IMAGE_TYPE = 1;
    IMAGE_HEADER = "image/bmp";
  } else if (urlSender.indexOf(".png") != -1) {
    IMAGE_TYPE = 2;
    IMAGE_HEADER = "image/png";
  } else if (urlSender.indexOf(".jpg") != -1) {
    IMAGE_TYPE = 3;
    IMAGE_HEADER = "image/jpg";
  } else if (urlSender.indexOf(".jpeg") != -1) {
    IMAGE_TYPE = 3;
    IMAGE_HEADER = "image/jpeg";
  } else {
    Serial.println("Image Not compatible");
  }

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    http.begin(urlSender);
    http.addHeader("Content-Type", IMAGE_HEADER);
    http.addHeader("Accepts", IMAGE_HEADER);

    // Send HTTP GET request
    int httpResponseCode = http.GET();

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      File IMG_IN = SD.open("/USERAVITAR.png", FILE_WRITE);
      if (!IMG_IN) {
        Serial.println("Failed to open file for writing");
        return;
      }
      if (IMG_IN.print(payload)) {
        Serial.println("File written");
      } else {
        Serial.println("Write failed");
      }
      IMG_IN.close();
    }

    if (IMAGE_TYPE == 1) {
      lcd.drawBmpFile(SD, "/USERAVITAR.png", 143, 63, 174, 174);
    } else if (IMAGE_TYPE == 2) {
      lcd.drawPngFile(SD, "/USERAVITAR.png", 143, 63, 174, 174);
    } else if (IMAGE_TYPE == 3) {
      lcd.drawJpgFile(SD, "/USERAVITAR.png", 143, 63, 174, 174, 0, 0, .2);
    } else {
      Serial.println("Incompatible File Type");
    }
    delay(1000);
    SD.remove("/USERAVITAR.png");
  }
}

//Main Touch Logic
void KEY_TOUCH(int X_POS, int Y_POS) {

  switch (CUR_PAGE) {
    case 0:

      if (Y_POS > 31 && Y_POS < 60) {  //Search
        playTouch(1);
        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
        lcd.drawPngFile(SD, "/gui/dotbitsearch.png", 261, 30);
        LAST_PAGE = 0;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 229 && X_POS < 260 && Y_POS < 31) {  //Saved Records
        playTouch(7);
        LAST_PAGE = 0;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      } else if (X_POS > 259 && X_POS < 290 && Y_POS < 31) {  //Saved QR Codes
        playTouch(7);
        LAST_PAGE = 0;
        CUR_PAGE = 10;
        OPEN_SAVED_QR();
      } else if (X_POS > 289 && Y_POS < 31) {  //Settings Page
        playTouch(8);
        LAST_PAGE = 0;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
      }

      break;

    case 1:

      if (X_POS < 24 && Y_POS > 60 && Y_POS < 96) {  //ESC
        playTouch(3);
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 1;
        CUR_PAGE = 0;
      } else if (X_POS > 259 && X_POS < 290 && Y_POS < 31) {  //Saved QR Codes
        playTouch(7);
        LAST_PAGE = 1;
        CUR_PAGE = 10;
        OPEN_SAVED_QR();
      } else if (X_POS > 289 && Y_POS < 31) {  //Settings Page
        playTouch(8);
        LAST_PAGE = 1;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
      } else if (X_POS > 23 && X_POS < 48 && Y_POS > 60 && Y_POS < 96) {  //1/!
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "1";
          TEXTBOX[TEXT_COUNT] = '1';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "!";
          TEXTBOX[TEXT_COUNT] = '!';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 47 && X_POS < 72 && Y_POS > 60 && Y_POS < 96) {  //2/@
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "2";
          TEXTBOX[TEXT_COUNT] = '2';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "@";
          TEXTBOX[TEXT_COUNT] = '@';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 71 && X_POS < 96 && Y_POS > 60 && Y_POS < 96) {  //3/#
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "3";
          TEXTBOX[TEXT_COUNT] = '3';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "#";
          TEXTBOX[TEXT_COUNT] = '#';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 95 && X_POS < 120 && Y_POS > 60 && Y_POS < 96) {  //4/$
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "4";
          TEXTBOX[TEXT_COUNT] = '4';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "$";
          TEXTBOX[TEXT_COUNT] = '$';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 119 && X_POS < 144 && Y_POS > 60 && Y_POS < 96) {  //5/%
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "5";
          TEXTBOX[TEXT_COUNT] = '5';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "%";
          TEXTBOX[TEXT_COUNT] = '%';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 143 && X_POS < 168 && Y_POS > 60 && Y_POS < 96) {  //6/^
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "6";
          TEXTBOX[TEXT_COUNT] = '6';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "^";
          TEXTBOX[TEXT_COUNT] = '^';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 167 && X_POS < 192 && Y_POS > 60 && Y_POS < 96) {  //7/&
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "7";
          TEXTBOX[TEXT_COUNT] = '7';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "&";
          TEXTBOX[TEXT_COUNT] = '&';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 191 && X_POS < 216 && Y_POS > 60 && Y_POS < 96) {  //8/*
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "8";
          TEXTBOX[TEXT_COUNT] = '8';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "*";
          TEXTBOX[TEXT_COUNT] = '*';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 215 && X_POS < 240 && Y_POS > 60 && Y_POS < 96) {  //9/(
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "9";
          TEXTBOX[TEXT_COUNT] = '9';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "(";
          TEXTBOX[TEXT_COUNT] = '(';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 239 && X_POS < 264 && Y_POS > 60 && Y_POS < 96) {  //0/)
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "0";
          TEXTBOX[TEXT_COUNT] = '0';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += ")";
          TEXTBOX[TEXT_COUNT] = ')';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 263 && Y_POS > 60 && Y_POS < 96) {  //Back Space
        playTouch(2);
        int TB_LEN = TEXT_BOX.length();
        TEXT_BOX = TEXT_BOX.substring(0, TB_LEN - 1);
        TEXTBOX[TEXT_COUNT - 1] = '\0';
        TEXT_COUNT--;
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS < 31 && Y_POS > 96 && Y_POS < 132) {  //Tab/_
        if (!CAPS_LOCK) {
          TEXT_BOX += "\t";
          TEXTBOX[TEXT_COUNT] = '\t';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "_";
          TEXTBOX[TEXT_COUNT] = '_';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 30 && X_POS < 55 && Y_POS > 96 && Y_POS < 132) {  //q/Q
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "q";
          TEXTBOX[TEXT_COUNT] = 'q';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "Q";
          TEXTBOX[TEXT_COUNT] = 'Q';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 54 && X_POS < 79 && Y_POS > 96 && Y_POS < 132) {  //w/W
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "w";
          TEXTBOX[TEXT_COUNT] = 'w';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "W";
          TEXTBOX[TEXT_COUNT] = 'W';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 78 && X_POS < 103 && Y_POS > 96 && Y_POS < 132) {  //e/E
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "e";
          TEXTBOX[TEXT_COUNT] = 'e';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "E";
          TEXTBOX[TEXT_COUNT] = 'E';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 102 && X_POS < 127 && Y_POS > 96 && Y_POS < 132) {  //r/R
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "r";
          TEXTBOX[TEXT_COUNT] = 'r';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "R";
          TEXTBOX[TEXT_COUNT] = 'R';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 126 && X_POS < 151 && Y_POS > 96 && Y_POS < 132) {  //t/T
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "t";
          TEXTBOX[TEXT_COUNT] = 't';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "T";
          TEXTBOX[TEXT_COUNT] = 'T';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 150 && X_POS < 175 && Y_POS > 96 && Y_POS < 132) {  //y/Y
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "y";
          TEXTBOX[TEXT_COUNT] = 'y';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "Y";
          TEXTBOX[TEXT_COUNT] = 'Y';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 174 && X_POS < 199 && Y_POS > 96 && Y_POS < 132) {  //u/U
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "u";
          TEXTBOX[TEXT_COUNT] = 'u';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "U";
          TEXTBOX[TEXT_COUNT] = 'U';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 198 && X_POS < 223 && Y_POS > 96 && Y_POS < 132) {  //i/I
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "i";
          TEXTBOX[TEXT_COUNT] = 'i';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "I";
          TEXTBOX[TEXT_COUNT] = 'I';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 222 && X_POS < 247 && Y_POS > 96 && Y_POS < 132) {  //o/O
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "o";
          TEXTBOX[TEXT_COUNT] = 'o';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "O";
          TEXTBOX[TEXT_COUNT] = 'O';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 246 && X_POS < 271 && Y_POS > 96 && Y_POS < 132) {  //p/P
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "p";
          TEXTBOX[TEXT_COUNT] = 'p';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "P";
          TEXTBOX[TEXT_COUNT] = 'P';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 270 && X_POS < 295 && Y_POS > 96 && Y_POS < 132) {  //[/{
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "[";
          TEXTBOX[TEXT_COUNT] = '[';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "{";
          TEXTBOX[TEXT_COUNT] = '{';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 294 && Y_POS > 96 && Y_POS < 132) {  //]/}
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "]";
          TEXTBOX[TEXT_COUNT] = ']';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "}";
          TEXTBOX[TEXT_COUNT] = '}';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS < 28 && Y_POS > 132 && Y_POS < 168) {  //Caps Lock
        playTouch(3);
        if (!CAPS_LOCK) {
          lcd.drawPngFile(SD, "/gui/keyboard2.png", 0, 30);
          lcd.drawPngFile(SD, "/gui/dotbitsearch.png", 261, 30);
          CAPS_LOCK = true;
        } else {
          lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
          lcd.drawPngFile(SD, "/gui/dotbitsearch.png", 261, 30);
          CAPS_LOCK = false;
        }
        UNSHIFT();
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 27 && X_POS < 52 && Y_POS > 132 && Y_POS < 168) {  //a/A
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "a";
          TEXTBOX[TEXT_COUNT] = 'a';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "A";
          TEXTBOX[TEXT_COUNT] = 'A';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 51 && X_POS < 76 && Y_POS > 132 && Y_POS < 168) {  //s/S
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "s";
          TEXTBOX[TEXT_COUNT] = 's';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "S";
          TEXTBOX[TEXT_COUNT] = 'S';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 75 && X_POS < 100 && Y_POS > 132 && Y_POS < 168) {  //d/D
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "d";
          TEXTBOX[TEXT_COUNT] = 'd';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "D";
          TEXTBOX[TEXT_COUNT] = 'D';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 99 && X_POS < 124 && Y_POS > 132 && Y_POS < 168) {  //f/F
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "f";
          TEXTBOX[TEXT_COUNT] = 'f';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "F";
          TEXTBOX[TEXT_COUNT] = 'F';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 123 && X_POS < 148 && Y_POS > 132 && Y_POS < 168) {  //g/G
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "g";
          TEXTBOX[TEXT_COUNT] = 'g';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "G";
          TEXTBOX[TEXT_COUNT] = 'G';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 147 && X_POS < 172 && Y_POS > 132 && Y_POS < 168) {  //h/H
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "h";
          TEXTBOX[TEXT_COUNT] = 'h';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "H";
          TEXTBOX[TEXT_COUNT] = 'H';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 171 && X_POS < 196 && Y_POS > 132 && Y_POS < 168) {  //j/J
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "j";
          TEXTBOX[TEXT_COUNT] = 'j';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "J";
          TEXTBOX[TEXT_COUNT] = 'J';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 195 && X_POS < 220 && Y_POS > 132 && Y_POS < 168) {  //k/K
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "k";
          TEXTBOX[TEXT_COUNT] = 'k';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "K";
          TEXTBOX[TEXT_COUNT] = 'K';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 219 && X_POS < 244 && Y_POS > 132 && Y_POS < 168) {  //l/L
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "l";
          TEXTBOX[TEXT_COUNT] = 'l';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "L";
          TEXTBOX[TEXT_COUNT] = 'L';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 243 && X_POS < 268 && Y_POS > 132 && Y_POS < 168) {  //;/:
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += ";";
          TEXTBOX[TEXT_COUNT] = ';';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += ":";
          TEXTBOX[TEXT_COUNT] = ':';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 267 && X_POS < 292 && Y_POS > 132 && Y_POS < 168) {  //'/"
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "'";
          TEXTBOX[TEXT_COUNT] = '\'';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "\"";
          TEXTBOX[TEXT_COUNT] = '\"';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 291 && Y_POS > 132 && Y_POS < 168) {  //\/|
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "\\";
          TEXTBOX[TEXT_COUNT] = '\\';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "|";
          TEXTBOX[TEXT_COUNT] = '|';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS < 37 && Y_POS > 168 && Y_POS < 204) {  //Shift
        playTouch(3);
        if (!CAPS_LOCK) {
          lcd.drawPngFile(SD, "/gui/keyboard2.png", 0, 30);
          lcd.drawPngFile(SD, "/gui/dotbitsearch.png", 261, 30);
          REFRESH_PAGE(CUR_PAGE);
          CAPS_LOCK = true;
          SHIFT = true;
        } else {
        }
      } else if (X_POS > 36 && X_POS < 61 && Y_POS > 168 && Y_POS < 204) {  //z/Z
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "z";
          TEXTBOX[TEXT_COUNT] = 'z';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "Z";
          TEXTBOX[TEXT_COUNT] = 'Z';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 60 && X_POS < 85 && Y_POS > 168 && Y_POS < 204) {  //x/X
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "x";
          TEXTBOX[TEXT_COUNT] = 'x';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "X";
          TEXTBOX[TEXT_COUNT] = 'X';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 84 && X_POS < 109 && Y_POS > 168 && Y_POS < 204) {  //c/C
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "c";
          TEXTBOX[TEXT_COUNT] = 'c';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "C";
          TEXTBOX[TEXT_COUNT] = 'C';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 108 && X_POS < 133 && Y_POS > 168 && Y_POS < 204) {  //v/V
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "v";
          TEXTBOX[TEXT_COUNT] = 'v';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "V";
          TEXTBOX[TEXT_COUNT] = 'V';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 132 && X_POS < 157 && Y_POS > 168 && Y_POS < 204) {  //b/B
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "b";
          TEXTBOX[TEXT_COUNT] = 'b';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "B";
          TEXTBOX[TEXT_COUNT] = 'B';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 156 && X_POS < 181 && Y_POS > 168 && Y_POS < 204) {  //n/N
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "n";
          TEXTBOX[TEXT_COUNT] = 'n';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "N";
          TEXTBOX[TEXT_COUNT] = 'N';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 180 && X_POS < 205 && Y_POS > 168 && Y_POS < 204) {  //m/M
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "m";
          TEXTBOX[TEXT_COUNT] = 'm';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "M";
          TEXTBOX[TEXT_COUNT] = 'M';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 204 && X_POS < 229 && Y_POS > 168 && Y_POS < 204) {  //,/<
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += ",";
          TEXTBOX[TEXT_COUNT] = ',';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "<";
          TEXTBOX[TEXT_COUNT] = '<';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 228 && X_POS < 253 && Y_POS > 168 && Y_POS < 204) {  //./>
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += ".";
          TEXTBOX[TEXT_COUNT] = '.';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += ">";
          TEXTBOX[TEXT_COUNT] = '>';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 252 && X_POS < 277 && Y_POS > 168 && Y_POS < 204) {  ////?
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "/";
          TEXTBOX[TEXT_COUNT] = '/';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "?";
          TEXTBOX[TEXT_COUNT] = '?';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 276 && Y_POS > 168 && Y_POS < 204) {  //ENTER
        if (LAST_PAGE == 0) {
          lcd.drawPngFile(SD, "/gui/entered.png", 277, 169);
          dbResponse.clear();
          int rCODE = GET_DOT_BIT_RECORDS((String(TEXTBOX) + ".bit"));
          playTouch(1);
          PRINT_RESPONSE();
          LAST_PAGE = 1;
          CUR_PAGE = 2;
        } else if (LAST_PAGE == 6) {
          lcd.drawPngFile(SD, "/gui/entered.png", 277, 169);
          if (CUR_SETTING == 0) {
            playTouch(6);
            CUR_REGION = String(TEXTBOX);
            if (CUR_CITY && CUR_REGION != "") {
              GET_TIMEZONE(CUR_REGION, CUR_CITY);
            }
            lcd.drawPngFile(SD, "/gui/timeSettings.png", 0, 0);
            lcd.setCursor(13, 102);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(CUR_REGION);
            lcd.setCursor(13, 142);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(CUR_CITY);
            lcd.setCursor(13, 182);
            lcd.setTextColor(TFT_BLACK);
            lcd.setTextSize(2);
            lcd.print(CUR_TIME_OFFSET);
            LAST_PAGE = 1;
            CUR_PAGE = 6;
          } else if (CUR_SETTING == 1) {
            playTouch(6);
            CUR_CITY = String(TEXTBOX);
            if (CUR_CITY && CUR_REGION != "") {
              GET_TIMEZONE(CUR_REGION, CUR_CITY);
            }
            lcd.drawPngFile(SD, "/gui/timeSettings.png", 0, 0);
            lcd.setCursor(13, 102);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(CUR_REGION);
            lcd.setCursor(13, 142);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(CUR_CITY);
            lcd.setCursor(13, 182);
            lcd.setTextColor(TFT_BLACK);
            lcd.setTextSize(2);
            lcd.print(CUR_TIME_OFFSET);
            LAST_PAGE = 1;
            CUR_PAGE = 6;
          }
        } else if (LAST_PAGE == 8) {
          lcd.drawPngFile(SD, "/gui/entered.png", 277, 169);
          WIFI_PASSWORD = String(TEXTBOX);
          playTouch(1);
          LOAD_PASSWORD();
          LAST_PAGE = 1;
          CUR_PAGE = 8;
        } else if (LAST_PAGE == 9) {
          lcd.drawPngFile(SD, "/gui/entered.png", 277, 169);
          if (CUR_SETTING == 0) {
            playTouch(6);
            AUTHOR_EMAIL = String(TEXTBOX);

            lcd.drawPngFile(SD, "/gui/emailSettings.png", 0, 0);
            lcd.setCursor(15, 93);
            lcd.setTextColor(TFT_YELLOW);
            if (AUTHOR_EMAIL.length() > 22) {
              lcd.setTextSize(1);
            } else {

              lcd.setTextSize(2);
            }
            lcd.print(AUTHOR_EMAIL);
            lcd.setCursor(137, 124);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(1);
            lcd.print(SMTP_HOST);
            lcd.setCursor(171, 151);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(HOST_NAME);
            lcd.setCursor(146, 216);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(1);

            if (!showEP) {
              for (int pw = 0; pw < AUTHOR_PASSWORD.length(); pw++) {
                lcd.print("*");
              }
            } else {
              lcd.print(AUTHOR_PASSWORD);
            }
            lcd.setCursor(156, 184);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(SMTP_PORT);
            LAST_PAGE = 1;
            CUR_PAGE = 9;
          } else if (CUR_SETTING == 1) {
            playTouch(6);
            SMTP_HOST = String(TEXTBOX);

            lcd.drawPngFile(SD, "/gui/emailSettings.png", 0, 0);
            lcd.setCursor(15, 93);
            lcd.setTextColor(TFT_YELLOW);
            if (AUTHOR_EMAIL.length() > 22) {
              lcd.setTextSize(1);
            } else {

              lcd.setTextSize(2);
            }
            lcd.print(AUTHOR_EMAIL);
            lcd.setCursor(137, 124);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(1);
            lcd.print(SMTP_HOST);
            lcd.setCursor(171, 151);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(HOST_NAME);
            lcd.setCursor(146, 216);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(1);
            if (!showEP) {
              for (int pw = 0; pw < AUTHOR_PASSWORD.length(); pw++) {
                lcd.print("*");
              }
            } else {
              lcd.print(AUTHOR_PASSWORD);
            }
            lcd.setCursor(156, 184);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(SMTP_PORT);
            LAST_PAGE = 1;
            CUR_PAGE = 9;
          } else if (CUR_SETTING == 2) {
            playTouch(6);
            HOST_NAME = String(TEXTBOX);

            lcd.drawPngFile(SD, "/gui/emailSettings.png", 0, 0);
            lcd.setCursor(15, 93);
            lcd.setTextColor(TFT_YELLOW);
            if (AUTHOR_EMAIL.length() > 22) {
              lcd.setTextSize(1);
            } else {

              lcd.setTextSize(2);
            }
            lcd.print(AUTHOR_EMAIL);
            lcd.setCursor(137, 124);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(1);
            lcd.print(SMTP_HOST);
            lcd.setCursor(171, 151);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(HOST_NAME);
            lcd.setCursor(146, 216);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(1);
            if (!showEP) {
              for (int pw = 0; pw < AUTHOR_PASSWORD.length(); pw++) {
                lcd.print("*");
              }
            } else {
              lcd.print(AUTHOR_PASSWORD);
            }
            lcd.setCursor(156, 184);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(SMTP_PORT);
            LAST_PAGE = 1;
            CUR_PAGE = 9;
          } else if (CUR_SETTING == 3) {
            playTouch(6);
            AUTHOR_PASSWORD = String(TEXTBOX);
            lcd.drawPngFile(SD, "/gui/emailSettings.png", 0, 0);
            lcd.setCursor(15, 93);
            lcd.setTextColor(TFT_YELLOW);
            if (AUTHOR_EMAIL.length() > 22) {
              lcd.setTextSize(1);
            } else {

              lcd.setTextSize(2);
            }
            lcd.print(AUTHOR_EMAIL);
            lcd.setCursor(137, 124);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(1);
            lcd.print(SMTP_HOST);
            lcd.setCursor(171, 151);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(HOST_NAME);
            lcd.setCursor(146, 216);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(1);
            if (!showEP) {
              for (int pw = 0; pw < AUTHOR_PASSWORD.length(); pw++) {
                lcd.print("*");
              }
            } else {
              lcd.print(AUTHOR_PASSWORD);
            }
            lcd.setCursor(156, 184);
            lcd.setTextColor(TFT_YELLOW);
            lcd.setTextSize(2);
            lcd.print(SMTP_PORT);
            LAST_PAGE = 1;
            CUR_PAGE = 9;
          }
        } else if (LAST_PAGE == 11) {
          lcd.drawPngFile(SD, "/gui/entered.png", 277, 169);

          if (CUR_SETTING == 0) {
            String DBCHECK = String(TEXTBOX);

            if (DBCHECK.indexOf(".bit") == DBCHECK.length() - 4) {
              String DBEMAIL = getEmailFromDotBit(DBCHECK);
              if (DBEMAIL == "NONE") {
                playTouch(1);
                LAST_PAGE = 1;
                CUR_PAGE = 11;
                RECIPIENT_EMAIL = DBCHECK + " doesn't have an email registered";
                EMAIL_SENDER(FN_HOLDER, UN_HOLDER, QR_HOLDER);
              } else {
                playTouch(1);
                LAST_PAGE = 1;
                CUR_PAGE = 11;
                RECIPIENT_EMAIL = DBEMAIL;
                EMAIL_SENDER(FN_HOLDER, UN_HOLDER, QR_HOLDER);
              }
            } else {
              RECIPIENT_EMAIL = String(TEXTBOX);
            }
          } else if (CUR_SETTING == 1) {
            AUTHOR_NAME = String(TEXTBOX);
            playTouch(1);
            LAST_PAGE = 1;
            CUR_PAGE = 11;
            EMAIL_SENDER(FN_HOLDER, UN_HOLDER, QR_HOLDER);
          } else if (CUR_SETTING == 2) {
            EMAIL_SUBJECT = String(TEXTBOX);
            playTouch(1);
            LAST_PAGE = 1;
            CUR_PAGE = 11;
            EMAIL_SENDER(FN_HOLDER, UN_HOLDER, QR_HOLDER);
          } else if (CUR_SETTING == 3) {
            EMAIL_BODY = String(TEXTBOX);
            playTouch(1);
            LAST_PAGE = 1;
            CUR_PAGE = 11;
            EMAIL_SENDER(FN_HOLDER, UN_HOLDER, QR_HOLDER);
          }
        }
      } else if (X_POS < 37 && Y_POS > 204) {  //`/~
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "`";
          TEXTBOX[TEXT_COUNT] = '`';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "~";
          TEXTBOX[TEXT_COUNT] = '~';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 36 && X_POS < 61 && Y_POS > 204) {  //Left Arrow
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "";
        } else {
          TEXT_BOX += "";
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 252 && X_POS < 85 && Y_POS > 204) {  //Right Arrow
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "";
        } else {
          TEXT_BOX += "";
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 85 && X_POS < 230 && Y_POS > 204) {  //Space
        playTouch(3);
        if (LAST_PAGE != 6) {
          if (!CAPS_LOCK) {
            TEXT_BOX += " ";
            TEXTBOX[TEXT_COUNT] = ' ';
            TEXT_COUNT++;
          } else {
            TEXT_BOX += " ";
            TEXTBOX[TEXT_COUNT] = ' ';
            TEXT_COUNT++;

            UNSHIFT();
          }
        } else {
          if (!CAPS_LOCK) {
            TEXT_BOX += "_";
            TEXTBOX[TEXT_COUNT] = '_';
            TEXT_COUNT++;
          } else {
            TEXT_BOX += "_";
            TEXTBOX[TEXT_COUNT] = '_';
            TEXT_COUNT++;

            UNSHIFT();
          }
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 230 && X_POS < 253 && Y_POS > 204) {  //-
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += '-';
          TEXTBOX[TEXT_COUNT] = '-';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += '-';
          TEXTBOX[TEXT_COUNT] = '-';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 252 && X_POS < 277 && Y_POS > 204) {  //+
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "+";
          TEXTBOX[TEXT_COUNT] = '+';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "+";
          TEXTBOX[TEXT_COUNT] = '+';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 276 && Y_POS > 204) {  //=
        playTouch(3);
        if (!CAPS_LOCK) {
          TEXT_BOX += "=";
          TEXTBOX[TEXT_COUNT] = '=';
          TEXT_COUNT++;
        } else {
          TEXT_BOX += "=";
          TEXTBOX[TEXT_COUNT] = '=';
          TEXT_COUNT++;
          UNSHIFT();
        }
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 198 && X_POS < 230 && Y_POS < 31) {  //Home
        playTouch(4);
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 1;
        CUR_PAGE = 0;
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 229 && X_POS < 260 && Y_POS < 31) {  //Saved Records
        playTouch(4);
        LAST_PAGE = 1;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      }

      break;

    //Query Response Page
    case 2:

      //Fav_Icon 252,63
      if (X_POS > 198 && X_POS < 230 && Y_POS < 31) {  //Home
        playTouch(4);
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 2;
        CUR_PAGE = 0;
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 229 && X_POS < 260 && Y_POS < 31) {  //Saved Records
        playTouch(4);
        LAST_PAGE = 2;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      } else if (X_POS > 259 && X_POS < 290 && Y_POS < 31) {  //Saved QR Codes
        playTouch(7);
        LAST_PAGE = 2;
        CUR_PAGE = 10;
        OPEN_SAVED_QR();
      } else if (X_POS > 289 && Y_POS < 31) {  //Settings Page
        playTouch(8);
        LAST_PAGE = 2;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
      } else if (X_POS > 248 && X_POS < 284 && Y_POS > 60 && Y_POS < 95) {  //Favourite
        Serial.println(CURRENT_DOT_BIT);
        Serial.println(MY_DOT_BIT);
        if (MY_DOT_BIT == CURRENT_DOT_BIT) {
          lcd.drawPngFile(SD, "/gui/unfav.png", 252, 63);
          MY_DOT_BIT = "";
        } else {
          lcd.drawPngFile(SD, "/gui/fav.png", 252, 63);
          MY_DOT_BIT = CURRENT_DOT_BIT;
        }
        playTouch(3);

      } else if (X_POS > 287 && X_POS < 318 && Y_POS > 63 && Y_POS < 93) {  //Save
        Serial.println(CURRENT_DOT_BIT);
        Serial.println(MY_DOT_BIT);
        SAVE_RECORD();
        playTouch(6);
      } else if (X_POS < 31 && Y_POS > 115 && Y_POS < 155 && PRO_PAGES > 0) {  //profile left
        Serial.println("Previous Profile Page");
        PRO_PAGES--;
        playTouch(7);
        UPDATE_PROFILES(PRO_PAGES);
      } else if (X_POS > 290 && Y_POS > 115 && Y_POS < 155 && (PRO_PAGES + 1 <= CUR_PROFILES / 7)) {  //profile right
        Serial.println("Next Profile Page");
        PRO_PAGES++;
        playTouch(7);
        UPDATE_PROFILES(PRO_PAGES);
      } else if (X_POS > 30 && X_POS < 67 && Y_POS > 105 && Y_POS < 155 && (PRO_ON_PAGE > 0)) {  //profile 1
        playTouch(6);

        GEN_QRCODE(CUR_PV[(PRO_PAGES * 7) + 0], CUR_PRO[(PRO_PAGES * 7) + 0]);

        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 67 && X_POS < 104 && Y_POS > 105 && Y_POS < 155 && (PRO_ON_PAGE > 1)) {  //profile 2
        playTouch(6);
        GEN_QRCODE(CUR_PV[(PRO_PAGES * 7) + 1], CUR_PRO[(PRO_PAGES * 7) + 1]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 104 && X_POS < 141 && Y_POS > 105 && Y_POS < 155 && (PRO_ON_PAGE > 2)) {  //profile 3
        playTouch(6);
        GEN_QRCODE(CUR_PV[(PRO_PAGES * 7) + 2], CUR_PRO[(PRO_PAGES * 7) + 2]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 141 && X_POS < 178 && Y_POS > 105 && Y_POS < 155 && (PRO_ON_PAGE > 3)) {  //profile 4
        playTouch(6);
        GEN_QRCODE(CUR_PV[(PRO_PAGES * 7) + 3], CUR_PRO[(PRO_PAGES * 7) + 3]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 178 && X_POS < 215 && Y_POS > 105 && Y_POS < 155 && (PRO_ON_PAGE > 4)) {  //profile 5
        playTouch(6);
        GEN_QRCODE(CUR_PV[(PRO_PAGES * 7) + 4], CUR_PRO[(PRO_PAGES * 7) + 4]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 215 && X_POS < 252 && Y_POS > 105 && Y_POS < 155 && (PRO_ON_PAGE > 5)) {  //profile 6
        playTouch(6);
        GEN_QRCODE(CUR_PV[(PRO_PAGES * 7) + 5], CUR_PRO[(PRO_PAGES * 7) + 5]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 252 && X_POS < 289 && Y_POS > 105 && Y_POS < 155 && (PRO_ON_PAGE > 6)) {  //profile 7
        playTouch(6);
        GEN_QRCODE(CUR_PV[(PRO_PAGES * 7) + 6], CUR_PRO[(PRO_PAGES * 7) + 6]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS < 31 && Y_POS > 190 && Y_POS < 225 && ADD_PAGES > 0) {  //address left
        Serial.println("Previous Address Page");
        ADD_PAGES--;
        playTouch(7);
        UPDATE_ADDRESSES(ADD_PAGES);
      } else if (X_POS > 290 && Y_POS > 190 && Y_POS < 225 && (ADD_PAGES + 1 <= CUR_ADDRESSES / 7)) {  //address right
        Serial.println("Next Address Page");
        ADD_PAGES++;
        playTouch(7);
        UPDATE_ADDRESSES(ADD_PAGES);
      } else if (X_POS > 30 && X_POS < 67 && Y_POS > 180 && Y_POS < 230 && (ADD_ON_PAGE > 0)) {  //address 1
        playTouch(6);
        GEN_QRCODE(CUR_AV[(ADD_PAGES * 7) + 0], CUR_ADD[(ADD_PAGES * 7) + 0]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 67 && X_POS < 104 && Y_POS > 180 && Y_POS < 230 && (ADD_ON_PAGE > 1)) {  //address 2
        playTouch(6);
        GEN_QRCODE(CUR_AV[(ADD_PAGES * 7) + 1], CUR_ADD[(ADD_PAGES * 7) + 1]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 104 && X_POS < 141 && Y_POS > 180 && Y_POS < 230 && (ADD_ON_PAGE > 2)) {  //address 3
        playTouch(6);
        GEN_QRCODE(CUR_AV[(ADD_PAGES * 7) + 2], CUR_ADD[(ADD_PAGES * 7) + 2]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 141 && X_POS < 178 && Y_POS > 180 && Y_POS < 230 && (ADD_ON_PAGE > 3)) {  //address 4
        playTouch(6);
        GEN_QRCODE(CUR_AV[(ADD_PAGES * 7) + 3], CUR_ADD[(ADD_PAGES * 7) + 3]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 178 && X_POS < 215 && Y_POS > 180 && Y_POS < 230 && (ADD_ON_PAGE > 4)) {  //address 5
        playTouch(6);
        GEN_QRCODE(CUR_AV[(ADD_PAGES * 7) + 4], CUR_ADD[(ADD_PAGES * 7) + 4]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 215 && X_POS < 252 && Y_POS > 180 && Y_POS < 230 && (ADD_ON_PAGE > 5)) {  //address 6
        playTouch(6);
        GEN_QRCODE(CUR_AV[(ADD_PAGES * 7) + 5], CUR_ADD[(ADD_PAGES * 7) + 5]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      } else if (X_POS > 252 && X_POS < 289 && Y_POS > 180 && Y_POS < 230 && (ADD_ON_PAGE > 6)) {  //address 7
        playTouch(6);
        GEN_QRCODE(CUR_AV[(ADD_PAGES * 7) + 6], CUR_ADD[(ADD_PAGES * 7) + 6]);
        LAST_PAGE = 2;
        CUR_PAGE = 3;
      }

      break;

    //Qr Code Output
    case 3:

      if (X_POS < 31 && Y_POS > 30 && Y_POS < 61) {  //Back
        playTouch(4);
        lcd.drawPngFile(SD, "/gui/entered.png", 277, 169);
        PRINT_RESPONSE();
        LAST_PAGE = 3;
        CUR_PAGE = 2;
      } else if (X_POS > 198 && X_POS < 230 && Y_POS < 31) {  //Home
        playTouch(4);
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 3;
        CUR_PAGE = 0;
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 229 && X_POS < 260 && Y_POS < 31) {  //Saved Records
        playTouch(4);
        LAST_PAGE = 3;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      } else if (X_POS > 259 && X_POS < 290 && Y_POS < 31) {  //Saved QR Codes
        playTouch(7);
        LAST_PAGE = 3;
        CUR_PAGE = 10;
        OPEN_SAVED_QR();
      } else if (X_POS > 289 && Y_POS < 31) {  //Settings Page
        playTouch(8);
        LAST_PAGE = 3;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
      } else if (X_POS > 259 && X_POS < 290 && Y_POS > 29 && Y_POS < 60) {  //Email

        String QRFN = "/qrcodes/" + CURRENT_DOT_BIT.substring(0, CURRENT_DOT_BIT.length() - 4) + "-" + CUR_QR;
        EMAIL_SENDER(QRFN, CURRENT_DOT_BIT, CUR_QR);

      } else if (X_POS > 289 && Y_POS > 29 && Y_POS < 60) {  //Save
        String FOUT = CURRENT_DOT_BIT;
        FOUT = FOUT.substring(0, FOUT.length() - 4);
        FOUT += "-";
        FOUT += CUR_QR;
        GrabImage(FOUT, CURLINK);
      }

      break;

    //Saved Records
    case 4:

      if (X_POS > 198 && X_POS < 230 && Y_POS < 31) {  //Home
        playTouch(4);
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 4;
        CUR_PAGE = 0;
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 259 && X_POS < 290 && Y_POS < 31) {  //Saved QR Codes
        playTouch(7);
        LAST_PAGE = 4;
        CUR_PAGE = 10;
        OPEN_SAVED_QR();
      } else if (X_POS > 289 && Y_POS < 31) {  //Settings Page
        playTouch(8);
        LAST_PAGE = 4;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
      } else if (X_POS > 290 && X_POS < 312 && Y_POS > 100 && Y_POS < 140 && SAVED_REC_PAGE > 0) {  //Page Up
        SAVED_REC_PAGE--;
        playTouch(5);
        UPDATE_SAVED_RECORDS();
      } else if (X_POS > 290 && X_POS < 312 && Y_POS > 140 && Y_POS < 180 && SAVED_REC_PAGE + 1 < SAVED_REC_PAGES) {  //Page Down
        SAVED_REC_PAGE++;
        playTouch(5);
        UPDATE_SAVED_RECORDS();
      } else if (X_POS > 20 && X_POS < 290 && Y_POS > 70 && Y_POS < 92 && SAVED_REC_ON_PAGE > 0) {  //Saved Record 1
        PRINT_RECALLED(SAVED_REC[0]);
        playTouch(5);
        LAST_PAGE = 4;
        CUR_PAGE = 2;
      } else if (X_POS > 20 && X_POS < 290 && Y_POS > 92 && Y_POS < 107 && SAVED_REC_ON_PAGE > 1) {  //Saved Record 2
        PRINT_RECALLED(SAVED_REC[1]);
        playTouch(5);
        LAST_PAGE = 4;
        CUR_PAGE = 2;
      } else if (X_POS > 20 && X_POS < 290 && Y_POS > 107 && Y_POS < 122 && SAVED_REC_ON_PAGE > 2) {  //Saved Record 3
        playTouch(5);
        PRINT_RECALLED(SAVED_REC[2]);
        LAST_PAGE = 4;
        CUR_PAGE = 2;
      } else if (X_POS > 20 && X_POS < 290 && Y_POS > 122 && Y_POS < 137 && SAVED_REC_ON_PAGE > 3) {  //Saved Record 4
        playTouch(5);
        PRINT_RECALLED(SAVED_REC[3]);
        LAST_PAGE = 4;
        CUR_PAGE = 2;
      } else if (X_POS > 20 && X_POS < 290 && Y_POS > 137 && Y_POS < 152 && SAVED_REC_ON_PAGE > 4) {  //Saved Record 5
        playTouch(5);
        PRINT_RECALLED(SAVED_REC[4]);
        LAST_PAGE = 4;
        CUR_PAGE = 2;
      } else if (X_POS > 20 && X_POS < 290 && Y_POS > 152 && Y_POS < 167 && SAVED_REC_ON_PAGE > 5) {  //Saved Record 6
        playTouch(5);
        PRINT_RECALLED(SAVED_REC[5]);
        LAST_PAGE = 4;
        CUR_PAGE = 2;
      } else if (X_POS > 20 && X_POS < 290 && Y_POS > 167 && Y_POS < 182 && SAVED_REC_ON_PAGE > 6) {  //Saved Record 7
        playTouch(5);
        PRINT_RECALLED(SAVED_REC[6]);
        LAST_PAGE = 4;
        CUR_PAGE = 2;
      } else if (X_POS > 20 && X_POS < 290 && Y_POS > 182 && Y_POS < 197 && SAVED_REC_ON_PAGE > 7) {  //Saved Record 8
        playTouch(5);
        PRINT_RECALLED(SAVED_REC[7]);
        LAST_PAGE = 4;
        CUR_PAGE = 2;
      } else if (X_POS > 20 && X_POS < 290 && Y_POS > 197 && Y_POS < 212 && SAVED_REC_ON_PAGE > 8) {  //Saved Record 9
        playTouch(5);
        PRINT_RECALLED(SAVED_REC[8]);
        LAST_PAGE = 4;
        CUR_PAGE = 2;
      } else if (X_POS > 20 && X_POS < 290 && Y_POS > 212 && Y_POS < 230 && SAVED_REC_ON_PAGE > 9) {  //Saved Record 10
        playTouch(5);
        PRINT_RECALLED(SAVED_REC[9]);
        LAST_PAGE = 4;
        CUR_PAGE = 2;
      }
      break;

    //Settings Page
    case 5:

      if (X_POS > 198 && X_POS < 230 && Y_POS < 31) {  //Home
        playTouch(4);
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 5;
        CUR_PAGE = 0;
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS > 229 && X_POS < 260 && Y_POS < 31) {  //Saved Records
        playTouch(4);
        LAST_PAGE = 5;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      } else if (X_POS > 259 && X_POS < 290 && Y_POS < 31) {  //Saved QR Codes
        playTouch(7);
        LAST_PAGE = 5;
        CUR_PAGE = 10;
        OPEN_SAVED_QR();
      } else if (X_POS > 18 && X_POS < 300 && Y_POS > 60 && Y_POS < 106) {  //Time Settings
        playTouch(5);
        LAST_PAGE = 5;
        CUR_PAGE = 6;
        lcd.drawPngFile(SD, "/gui/timeSettings.png", 0, 0);
        lcd.setCursor(13, 102);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(2);
        lcd.print(CUR_REGION);
        lcd.setCursor(13, 142);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(2);
        lcd.print(CUR_CITY);
        lcd.setCursor(13, 182);
        lcd.setTextColor(TFT_BLACK);
        lcd.setTextSize(2);
        lcd.print(CUR_TIME_OFFSET);

        if (TwentyFourHour == false) {
          lcd.drawPngFile(SD, "/gui/toggleLeft.png", 38, 218);
        } else {  //24 Hour
          lcd.drawPngFile(SD, "/gui/toggleRight.png", 38, 218);
        }
      } else if (X_POS > 18 && X_POS < 300 && Y_POS > 105 && Y_POS < 151) {  //QR Settings
        playTouch(5);
        LAST_PAGE = 5;
        CUR_PAGE = 7;
        lcd.drawPngFile(SD, "/gui/qrSettings.png", 0, 0);
        lcd.fillRect(50, 95, 210, 45, QR_BG);
        lcd.fillRect(50, 175, 210, 45, QR_MAIN);
      } else if (X_POS > 18 && X_POS < 300 && Y_POS > 150 && Y_POS < 196) {  //WiFi Settings
        playTouch(5);
        LAST_PAGE = 5;
        CUR_PAGE = 8;
        lcd.drawPngFile(SD, "/gui/wifiSettings.png", 0, 0);
        lcd.setCursor(20, 110);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(2);
        lcd.print(WIFI_SSID);
        lcd.setCursor(20, 178);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(2);
        for (int pw = 0; pw < WIFI_PASSWORD.length(); pw++) {
          lcd.print("*");
        }
        //lcd.print(WIFI_PASSWORD);
      } else if (X_POS > 18 && X_POS < 300 && Y_POS > 184) {  //email Settings
        playTouch(5);
        LAST_PAGE = 5;
        CUR_PAGE = 9;
        lcd.drawPngFile(SD, "/gui/emailSettings.png", 0, 0);
        lcd.setCursor(15, 93);
        lcd.setTextColor(TFT_YELLOW);
        if (AUTHOR_EMAIL.length() > 22) {
          lcd.setTextSize(1);
        } else {

          lcd.setTextSize(2);
        }

        lcd.print(AUTHOR_EMAIL);
        lcd.setCursor(137, 124);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(1);
        lcd.print(SMTP_HOST);
        lcd.setCursor(171, 151);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(2);
        lcd.print(HOST_NAME);
        lcd.setCursor(146, 216);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(1);
        for (int pw = 0; pw < AUTHOR_PASSWORD.length(); pw++) {
          lcd.print("*");
        }
        //lcd.print(AUTHOR_PASSWORD);
        lcd.setCursor(156, 184);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(2);
        lcd.print(SMTP_PORT);
      }

      break;

    //Time Settings
    case 6:

      if (X_POS > 198 && X_POS < 230 && Y_POS < 31) {  //Home
        playTouch(4);
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 6;
        CUR_PAGE = 0;
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS < 31 && Y_POS > 30 && Y_POS < 61) {  //Back
        playTouch(5);
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
        LAST_PAGE = 6;
        CUR_PAGE = 5;
      } else if (X_POS > 229 && X_POS < 260 && Y_POS < 31) {  //Saved Records
        playTouch(5);
        LAST_PAGE = 6;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      } else if (X_POS > 259 && X_POS < 290 && Y_POS < 31) {  //Saved QR Codes
        playTouch(7);
        LAST_PAGE = 6;
        CUR_PAGE = 10;
        OPEN_SAVED_QR();
      } else if (X_POS > 289 && Y_POS < 31) {  //Settings Page
        playTouch(8);
        LAST_PAGE = 6;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
      } else if (X_POS > 10 && X_POS < 300 && Y_POS > 60 && Y_POS < 121) {  //Area/Region/Country
        playTouch(1);
        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);

        CUR_SETTING = 0;
        LAST_PAGE = 6;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
      } else if (X_POS > 10 && X_POS < 300 && Y_POS > 120 && Y_POS < 161) {  //City
        playTouch(1);
        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);

        CUR_SETTING = 1;
        LAST_PAGE = 6;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
      } else if (X_POS > 10 && X_POS < 71 && Y_POS > 215 && Y_POS < 241) {  //12 Hour
        TwentyFourHour = false;
        playTouch(3);
        lcd.drawPngFile(SD, "/gui/toggleLeft.png", 38, 218);


      } else if (X_POS > 70 && X_POS < 136 && Y_POS > 215 && Y_POS < 241) {  //24 Hour
        TwentyFourHour = true;
        playTouch(3);
        lcd.drawPngFile(SD, "/gui/toggleRight.png", 38, 218);
      } else if (X_POS > 289 && Y_POS > 29 && Y_POS < 60) {  //Save
        saveToEeprom();
      }

      break;

    //QR Settings
    case 7:
      playTouch(5);
      if (X_POS > 198 && X_POS < 230 && Y_POS < 31) {  //Home
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 7;
        CUR_PAGE = 0;
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS < 31 && Y_POS > 30 && Y_POS < 61) {  //Back
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
        LAST_PAGE = 7;
        CUR_PAGE = 5;
      } else if (X_POS > 229 && X_POS < 260 && Y_POS < 31) {  //Saved Records
        LAST_PAGE = 7;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      } else if (X_POS > 289 && Y_POS < 31) {  //Settings Page
        playTouch(8);
        LAST_PAGE = 7;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
      } else if (X_POS > 50 && X_POS < 265 && Y_POS > 60 && Y_POS < 135) {  //Background Colour
        colourPicker("Background Colour", &QR_BG);
        playTouch(5);
        lcd.drawPngFile(SD, "/gui/qrSettings.png", 0, 0);
        lcd.fillRect(50, 95, 210, 45, QR_BG);
        lcd.fillRect(50, 175, 210, 45, QR_MAIN);
      } else if (X_POS > 50 && X_POS < 265 && Y_POS > 140 && Y_POS < 235) {  //Foreground Colour
        colourPicker("Main Body Colour", &QR_MAIN);
        playTouch(5);
        lcd.drawPngFile(SD, "/gui/qrSettings.png", 0, 0);
        lcd.fillRect(50, 95, 210, 45, QR_BG);
        lcd.fillRect(50, 175, 210, 45, QR_MAIN);
      } else if (X_POS > 289 && Y_POS > 29 && Y_POS < 60) {  //Save
        saveToEeprom();
      }

      break;

    //WiFi Settings
    case 8:

      if (X_POS > 198 && X_POS < 230 && Y_POS < 31) {  //Home
        playTouch(5);
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 8;
        CUR_PAGE = 0;
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS < 31 && Y_POS > 30 && Y_POS < 61) {  //Back
        playTouch(5);
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
        LAST_PAGE = 8;
        CUR_PAGE = 5;
      } else if (X_POS > 259 && X_POS < 290 && Y_POS < 31) {  //Saved QR Codes
        playTouch(7);
        LAST_PAGE = 8;
        CUR_PAGE = 10;
        OPEN_SAVED_QR();
      } else if (X_POS > 289 && Y_POS < 31) {  //Settings Page
        playTouch(8);
        LAST_PAGE = 8;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
      } else if (X_POS > 229 && X_POS < 260 && Y_POS < 31) {  //Saved Records
        playTouch(5);
        LAST_PAGE = 8;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      } else if (X_POS > 289 && Y_POS > 29 && Y_POS < 60) {  //Save
        saveToEeprom();
      } else if (X_POS > 16 && X_POS < 305 && Y_POS > 107 && Y_POS < 145) {  //Set SSID
        LOAD_SSID();
        lcd.drawPngFile(SD, "/gui/wifiSettings.png", 0, 0);
        lcd.setCursor(20, 110);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(2);
        lcd.print(WIFI_SSID);
        lcd.setCursor(20, 178);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(2);
        lcd.print(WIFI_PASSWORD);
      } else if (X_POS > 16 && X_POS < 305 && Y_POS > 175 && Y_POS < 212) {  //Set PASSWORD
        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
        CUR_SETTING = 0;
        LAST_PAGE = 8;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
      } else if (X_POS > 214 && X_POS < 252 && Y_POS > 145 && Y_POS < 170) {  //Hide/Show Password
        if (showWP) {
          lcd.drawPngFile(SD, "/gui/showPW.png", 217, 145);
          lcd.fillRect(16, 175, 288, 37, DG_BG);
          lcd.setCursor(18, 177);
          lcd.setTextColor(TFT_YELLOW);
          lcd.setTextSize(2);
          for (int pw = 0; pw < WIFI_PASSWORD.length(); pw++) {
            lcd.print("*");
          }
          showWP = false;
        } else {
          lcd.drawPngFile(SD, "/gui/hidePW.png", 217, 145);
          lcd.fillRect(16, 175, 288, 37, DG_BG);
          lcd.setCursor(18, 177);
          lcd.setTextColor(TFT_YELLOW);
          lcd.setTextSize(2);
          lcd.print(WIFI_PASSWORD);
          showWP = true;
        }
      }

      break;

    //Email Settings
    case 9:
      playTouch(5);
      if (X_POS > 198 && X_POS < 230 && Y_POS < 31) {  //Home
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 9;
        CUR_PAGE = 0;
        REFRESH_PAGE(CUR_PAGE);
      } else if (X_POS < 31 && Y_POS > 30 && Y_POS < 61) {  //Back
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
        LAST_PAGE = 9;
        CUR_PAGE = 5;
      } else if (X_POS > 259 && X_POS < 290 && Y_POS < 31) {  //Saved QR Codes
        playTouch(7);
        LAST_PAGE = 9;
        CUR_PAGE = 10;
        OPEN_SAVED_QR();
      } else if (X_POS > 289 && Y_POS < 31) {  //Settings Page
        playTouch(8);
        LAST_PAGE = 9;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
      } else if (X_POS > 229 && X_POS < 260 && Y_POS < 31) {  //Saved Records
        LAST_PAGE = 9;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      } else if (X_POS > 10 && X_POS < 310 && Y_POS > 90 && Y_POS < 115) {  //Email

        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);

        CUR_SETTING = 0;
        LAST_PAGE = 9;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
      } else if (X_POS > 130 && X_POS < 310 && Y_POS > 120 && Y_POS < 145) {  //Host

        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
        CUR_SETTING = 1;
        LAST_PAGE = 9;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
      } else if (X_POS > 165 && X_POS < 310 && Y_POS > 148 && Y_POS < 175) {  //Sender

        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
        CUR_SETTING = 2;
        LAST_PAGE = 9;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
      } else if (X_POS > 127 && X_POS < 153 && Y_POS > 180 && Y_POS < 200) {  //Port Down
        
        if (SMTP_PORT == 25) {
          SMTP_PORT = 2525;
        } else if (SMTP_PORT == 465) {
          SMTP_PORT = 25;
        } else if (SMTP_PORT == 587) {
          SMTP_PORT = 465;
        } else if (SMTP_PORT == 2525) {
          SMTP_PORT = 587;
        }
        lcd.fillRect(154, 182, 55, 22, DG_BG);
        lcd.setCursor(156, 184);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(2);
        lcd.print(SMTP_PORT);
      } else if (X_POS > 220 && X_POS < 260 && Y_POS > 180 && Y_POS < 200) {  //Port Up
        if (SMTP_PORT == 25) {
          SMTP_PORT = 465;
        } else if (SMTP_PORT == 465) {
          SMTP_PORT = 587;
        } else if (SMTP_PORT == 587) {
          SMTP_PORT = 2525;
        } else if (SMTP_PORT == 2525) {
          SMTP_PORT = 25;
        }
        lcd.fillRect(154, 182, 55, 22, DG_BG);
        lcd.setCursor(156, 184);
        lcd.setTextColor(TFT_YELLOW);
        lcd.setTextSize(2);
        lcd.print(SMTP_PORT);
      } else if (X_POS > 142 && X_POS < 310 && Y_POS > 210 && Y_POS < 233) {  //Password

        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
        CUR_SETTING = 3;
        LAST_PAGE = 9;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
      } else if (X_POS > 289 && Y_POS > 29 && Y_POS < 60) {  //Save
        saveToEeprom();
      } else if (X_POS > 110 && X_POS < 142 && Y_POS > 210 && Y_POS < 233) {  //Hide/Show Password
        if (showEP) {
          lcd.drawPngFile(SD, "/gui/showPW.png", 113, 207);
          lcd.fillRect(146, 216, 161, 17, DG_BG);
          lcd.setCursor(146, 216);
          lcd.setTextColor(TFT_YELLOW);
          lcd.setTextSize(1);
          for (int pw = 0; pw < AUTHOR_PASSWORD.length(); pw++) {
            lcd.print("*");
          }
          showEP = false;
        } else {
          lcd.drawPngFile(SD, "/gui/hidePW.png", 113, 207);
          lcd.fillRect(146, 216, 161, 17, DG_BG);
          lcd.setCursor(146, 216);
          lcd.setTextColor(TFT_YELLOW);
          lcd.setTextSize(1);
          lcd.print(AUTHOR_PASSWORD);
          showEP = true;
        }
      }

      break;
  }
}

//Refresh display
void REFRESH_PAGE(int cPAGE) {
  if (cPAGE == 0) {
    lcd.setCursor(75, 74);
    if (MY_DOT_BIT.length() < 30) {
      lcd.setTextSize(2);
    } else if (MY_DOT_BIT.length() > 30 && MY_DOT_BIT.length() < 50) {
      lcd.setTextSize(1);
      lcd.setCursor(75, 78);
    } else {
      MY_DOT_BIT = MY_DOT_BIT.substring(0, 40);
      lcd.setTextSize(1);
      lcd.setCursor(75, 78);
    }
    lcd.setTextColor(TFT_BLACK);
    lcd.print(MY_DOT_BIT);
    lcd.setCursor(115, 138);
    if (CURRENT_DOT_BIT.length() < 17) {
      lcd.setTextSize(2);
    } else if (CURRENT_DOT_BIT.length() > 16 && CURRENT_DOT_BIT.length() < 30) {
      lcd.setTextSize(1);
      lcd.setCursor(115, 142);
    } else {
      CURRENT_DOT_BIT = CURRENT_DOT_BIT.substring(0, 30);
      lcd.setTextSize(1);
      lcd.setCursor(115, 142);
    }
    lcd.setTextColor(TFT_BLACK);
    lcd.print(CURRENT_DOT_BIT);
    lcd.setTextSize(2);
    lcd.setCursor(144, 97);
    lcd.print(SAVREC);
    lcd.setTextSize(1);
    lcd.setCursor(175, 122);
    if (TZSTRING == "") {
      TZSTRING = String(CUR_TIME_OFFSET);
      float HO = CUR_TIME_OFFSET / 60;
      HO = HO / 60;
      TZSTRING += ", " + String(HO, 2) + " UTC";
    }
    lcd.print(TZSTRING);

  } else if (cPAGE == 1) {
    lcd.fillRect(18, 33, 243, 23, TFT_WHITE);
    lcd.setCursor(20, 36);
    String TTB = String(TEXTBOX);
    if (TTB.length() < 20) {
      lcd.setTextSize(2);
    } else if (TTB.length() > 19 && TTB.length() < 41) {
      lcd.setTextSize(1);
    } else {
      TTB = TTB.substring(0, 40);
      lcd.setTextSize(1);
    }
    lcd.setTextColor(TFT_BLACK);
    lcd.print(TTB);
  } else if (cPAGE == 2) {
  }
}

//Shift Key Logic
void UNSHIFT() {
  if (SHIFT == true) {
    SHIFT = false;
    CAPS_LOCK = false;
    lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
    lcd.drawPngFile(SD, "/gui/dotbitsearch.png", 261, 30);
  }
}

//Dot Bit Indexer lookup request
int GET_DOT_BIT_RECORDS(String dotbit_name) {
  int RESP;
  
  ADD_PAGES = 0;
  PRO_PAGES = 0;
  object = doc.to<JsonObject>();
  object["account"] = dotbit_name;

  String jsOut = "";

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;

    String serverPath = String(DOTBIT_ADD_SERVER);

    serializeJson(doc, jsOut);
    Serial.println(DOTBIT_ADD_SERVER);

    http.begin(serverPath);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(jsOut);
    RESP = httpResponseCode;
    jsOut = "\0";
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      error = deserializeJson(dbResponse, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }

      const char* errmsg = dbResponse["errmsg"];  // nullptr
      const long err_no = dbResponse["errno"];
      const char* data_account = dbResponse["data"]["account"];  // "phillip.bit"
      Serial.println(errmsg);

      if (err_no == 0) {
        for (JsonObject data_record : dbResponse["data"]["records"].as<JsonArray>()) {

          const char* data_record_key = data_record["key"];      // "address.ckb", "profile.avatar", ...
          const char* data_record_label = data_record["label"];  // "CKB.PW (relay)", nullptr, nullptr, nullptr
          const char* data_record_value = data_record["value"];
          const char* data_record_ttl = data_record["ttl"];  // "300", "300", "300", "300"
        }
      } else if (err_no == 20007) {
        dbResponse["data"]["records"]["key"] = "Account Doesn't Exist";
        dbResponse["data"]["account"] = "Account Doesn't Exist";
      } else {
        Serial.println("Error " + String(err_no));
      }
    }
  }
  return RESP;
}

//Display returned records
void PRINT_RESPONSE() {
  lcd.drawPngFile(SD, "/gui/response.png", 0, 0);
  lcd.setCursor(5, 37);
  const char* data_account = dbResponse["data"]["account"];
  CURRENT_DOT_BIT = String(data_account);
  if (CURRENT_DOT_BIT.length() > 25) {
    lcd.setCursor(5, 43);
    lcd.setTextSize(1);
  } else {
    lcd.setTextSize(2);
  }
  lcd.setTextColor(TFT_WHITE);
  lcd.print(CURRENT_DOT_BIT);
  if (MY_DOT_BIT == CURRENT_DOT_BIT) {
    lcd.drawPngFile(SD, "/gui/fav.png", 252, 63);
  } else {
    lcd.drawPngFile(SD, "/gui/unfav.png", 252, 63);
  }
  Serial.println(CURRENT_DOT_BIT);
  CHECK_RECORD();
}

//Store Dot Bit record as json file on SD card
void SAVE_RECORD() {
  String payload;
  const char* data_account = dbResponse["data"]["account"];
  String FN = String(data_account);
  FN = FN.substring(0, (FN.length() - 4));
  String FILENAME = "/records/" + FN + ".json";
  serializeJson(dbResponse, payload);
  File JSON_OUT = SD.open(FILENAME, FILE_WRITE);
  if (!JSON_OUT) {
    Serial.println("Failed to open file for writing");
    return;
  }
  if (JSON_OUT.print(payload)) {
    Serial.println("File written");
    lcd.drawPngFile(SD, "/gui/saved.png", 287, 63);
  } else {
    Serial.println("Write failed");
    lcd.drawPngFile(SD, "/gui/notsaved.png", 287, 63);
  }
  SAVED_NOTE = true;
  NOTE_TIMER = millis();
  JSON_OUT.close();
}

//Determine Record details
void CHECK_RECORD() {

  for (int r = 0; r < 66; r++) {
    HAS_KEY[r] = false;
  }

  int RCOUNT = 0;
  CUR_NUM_RECORDS = 0;
  CUR_PROFILES = 0;
  CUR_ADDRESSES = 0;
  for (JsonObject data_record : dbResponse["data"]["records"].as<JsonArray>()) {
    int BCOUNT = 0;

    const char* data_record_key = data_record["key"];      // "address.ckb", "profile.avatar", ...
    const char* data_record_label = data_record["label"];  // nullptr, nullptr, nullptr, nullptr, nullptr
    const char* data_record_value = data_record["value"];
    const char* data_record_ttl = data_record["ttl"];  // "300", "300", "300", "300", "300"

    for (int r = 0; r < 23; r++) {
      String CUR_KEY = "profile." + String(DB_PROFILE[r]);
      if (String(data_record_key) == CUR_KEY) {
        HAS_KEY[BCOUNT] = true;
        CUR_PRO[CUR_PROFILES] = DB_PROFILE[r];
        CUR_PV[CUR_PROFILES] = data_record_value;
        CUR_PROFILES++;
        DB_KEY[r] = data_record_key;
        DB_VALUE[r] = data_record_value;
        DB_NUM[RCOUNT] = BCOUNT;

        RCOUNT++;
        Serial.println(String(r) + ") " + String(DB_KEY[r]) + ":" + String(DB_VALUE[r]));
      }
      BCOUNT++;
    }
    for (int r = 0; r < 5; r++) {
      String CUR_KEY = "dweb." + String(DB_DWEB[r]);
      if (String(data_record_key) == CUR_KEY) {
        HAS_KEY[BCOUNT] = true;
        CUR_PRO[CUR_PROFILES] = DB_DWEB[r];
        CUR_PV[CUR_PROFILES] = data_record_value;
        CUR_PROFILES++;
        DB_KEY[r + 23] = data_record_key;
        DB_VALUE[r + 23] = data_record_value;
        DB_NUM[RCOUNT] = BCOUNT;
        RCOUNT++;
        Serial.println(String(r + 23) + ") " + String(DB_KEY[r + 23]) + ":" + String(DB_VALUE[r + 23]));
      }
      BCOUNT++;
    }
    for (int r = 0; r < 38; r++) {
      String CUR_KEY = "address." + String(DB_ADDRESS[r]);
      if (String(data_record_key) == CUR_KEY) {
        HAS_KEY[BCOUNT] = true;
        CUR_ADD[CUR_ADDRESSES] = DB_ADDRESS[r];
        CUR_AV[CUR_ADDRESSES] = data_record_value;
        CUR_ADDRESSES++;
        DB_KEY[r + 28] = data_record_key;
        DB_VALUE[r + 28] = data_record_value;
        DB_NUM[RCOUNT] = BCOUNT;
        RCOUNT++;
        Serial.println(String(r + 28) + ") " + String(DB_KEY[r + 28]) + ":" + String(DB_VALUE[r + 28]));
      }

      BCOUNT++;
    }
    Serial.println(RCOUNT);
    CUR_NUM_RECORDS++;
    Serial.println(BCOUNT);
  }

  if (CUR_PROFILES == 0) {
    Serial.println("No Profile or Dweb Records");
    lcd.setCursor(34, 118);
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_BLACK);
    lcd.print("No Profile or Dweb Records");
  } else if (CUR_PROFILES > 0 && CUR_PROFILES < 8) {
    int X = 34;
    int Y = 118;
    PRO_ON_PAGE = CUR_PROFILES;
    for (int r = 0; r < CUR_PROFILES; r++) {

      String FNO = "/profile/" + CUR_PRO[r] + ".png";
      lcd.drawPngFile(SD, FNO, X, Y);
      X = X + 37;
    }

  } else if (CUR_PROFILES > 7) {
    int X = 34;
    int Y = 118;
    lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 120);
    PRO_PAGES = 0;
    PRO_ON_PAGE = 7;
    for (int r = 0; r < 7; r++) {

      String FNO = "/profile/" + CUR_PRO[r] + ".png";
      lcd.drawPngFile(SD, FNO, X, Y);
      X = X + 37;
    }
  }

  if (CUR_ADDRESSES == 0) {
    Serial.println("No Address Records");
    lcd.setCursor(34, 188);
    lcd.setTextSize(1);
    lcd.setTextColor(TFT_BLACK);
    lcd.print("No Address Records");
  } else if (CUR_ADDRESSES > 0 && CUR_ADDRESSES < 8) {
    int X = 34;
    int Y = 188;
    ADD_ON_PAGE = CUR_ADDRESSES;
    for (int r = 0; r < CUR_ADDRESSES; r++) {
      String FNO = "/address/" + CUR_ADD[r] + ".png";
      lcd.drawPngFile(SD, FNO, X, Y);
      X = X + 37;
    }


  } else if (CUR_ADDRESSES > 7) {
    int X = 34;
    int Y = 188;
    lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 191);
    ADD_ON_PAGE = 7;
    ADD_PAGES = 0;
    for (int r = 0; r < 7; r++) {
      String FNO = "/address/" + CUR_ADD[r] + ".png";
      lcd.drawPngFile(SD, FNO, X, Y);
      X = X + 37;
    }
  }

  Serial.println(CUR_PROFILES);
  Serial.println(CUR_ADDRESSES);
  Serial.println(CUR_NUM_RECORDS);
}

//Page Shifting logic for Profile/Dweb Records
void UPDATE_PROFILES(int PP) {
  if (PP == 0) {
    lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 120);
    lcd.drawPngFile(SD, "/gui/leftDisabled.png", 0, 120);
  } else if ((PP + 1) > CUR_PROFILES / 7) {
    lcd.fillRect(31, 100, 258, 60, LG_BG);
    lcd.drawPngFile(SD, "/gui/rightDisabled.png", 290, 120);
    lcd.drawPngFile(SD, "/gui/leftEnabled.png", 0, 120);

    int X = 34;
    int Y = 118;
    PRO_ON_PAGE = CUR_PROFILES - (PP * 7);
    for (int r = 0; r < PRO_ON_PAGE; r++) {

      String FNO = "/profile/" + CUR_PRO[r + (PP * 7)] + ".png";
      lcd.drawPngFile(SD, FNO, X, Y);
      X = X + 37;
    }
    return;
  } else {
    lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 120);
    lcd.drawPngFile(SD, "/gui/leftEnabled.png", 0, 120);
  }

  lcd.fillRect(31, 100, 258, 6, LG_BG);

  int X = 34;
  int Y = 118;
  PRO_PAGES = 0;
  PRO_ON_PAGE = 7;
  for (int r = 0; r < 7; r++) {

    String FNO = "/profile/" + CUR_PRO[r + (PP * 7)] + ".png";
    lcd.drawPngFile(SD, FNO, X, Y);
    X = X + 37;
  }
}

//Page Shifting logic for Address Records
void UPDATE_ADDRESSES(int AP) {
  if (AP == 0) {
    Serial.println(AP);
    lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 191);
    lcd.drawPngFile(SD, "/gui/leftDisabled.png", 0, 191);
  } else if ((AP + 1) > CUR_ADDRESSES / 7) {
    Serial.println(AP);
    lcd.fillRect(31, 170, 258, 60, LG_BG);
    lcd.drawPngFile(SD, "/gui/rightDisabled.png", 290, 191);
    lcd.drawPngFile(SD, "/gui/leftEnabled.png", 0, 191);

    int X = 34;
    int Y = 188;
    ADD_ON_PAGE = CUR_ADDRESSES - (AP * 7);
    for (int r = 0; r < ADD_ON_PAGE; r++) {

      String FNO = "/address/" + CUR_ADD[r + (AP * 7)] + ".png";
      lcd.drawPngFile(SD, FNO, X, Y);
      X = X + 37;
    }
    return;
  } else {
    Serial.println(AP);
    lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 191);
    lcd.drawPngFile(SD, "/gui/leftEnabled.png", 0, 191);
  }

  lcd.fillRect(31, 170, 258, 60, LG_BG);

  int X = 34;
  int Y = 188;

  ADD_ON_PAGE = 7;
  for (int r = 0; r < ADD_ON_PAGE; r++) {

    String FNO = "/address/" + CUR_ADD[r + (AP * 7)] + ".png";
    lcd.drawPngFile(SD, FNO, X, Y);
    X = X + 37;
  }
}

//Generate QR code and display to screen
void GEN_QRCODE(String INPUT_TEXT, String INPUT_NAME) {
  lcd.drawPngFile(SD, "/gui/qrdisplay.png", 0, 0);
  lcd.setCursor(35, 37);
  CUR_QR = INPUT_NAME;
  if (CURRENT_DOT_BIT.length() > 17) {
    lcd.setCursor(35, 43);
    lcd.setTextSize(1);
  } else {
    lcd.setTextSize(2);
  }
  lcd.setTextColor(TFT_WHITE);
  lcd.print(CURRENT_DOT_BIT);
  bool isAVI = false;
  for (int i = 0; i < 23; i++) {
    if (INPUT_NAME == String(DB_PROFILE[i])) {
      if (i == 7) {
        downloadImageAndDisplay(INPUT_TEXT);
        isAVI = true;
      } else {
        lcd.drawPngFile(SD, "/profile/" + INPUT_NAME + ".png", 3, 65);
        SET_OUTPUT_LINK(INPUT_TEXT, i);
      }
    }
  }

  for (int i = 0; i < 38; i++) {
    if (INPUT_NAME == String(DB_ADDRESS[i])) {
      lcd.drawPngFile(SD, "/address/" + INPUT_NAME + ".png", 3, 65);
      OUTPUT_LINK = INPUT_TEXT;
    }
  }

  for (int i = 0; i < 5; i++) {
    if (INPUT_NAME == String(DB_DWEB[i])) {
      lcd.drawPngFile(SD, "/dweb/" + INPUT_NAME + ".png", 3, 65);
      OUTPUT_LINK = INPUT_TEXT;
    }
  }

  lcd.setCursor(40, 72);

  if (INPUT_NAME.length() < 6) {
    lcd.setTextSize(3);
  } else if (INPUT_NAME.length() > 5 && INPUT_NAME.length() < 11) {
    lcd.setTextSize(2);
  } else {
    lcd.setTextSize(1);
  }

  lcd.setTextColor(TFT_BLACK);
  String uASSET = INPUT_NAME;
  uASSET.toUpperCase();
  lcd.print(uASSET);
  lcd.setCursor(3, 100);

  if (INPUT_TEXT.length() < 67) {
    lcd.setTextSize(2);
    for (int p = 0; p < (INPUT_TEXT.length() / 11) + 1; p++) {
      int c = p * 11;
      lcd.print(INPUT_TEXT.substring(c, c + 11));
      lcd.setCursor(3, 100 + ((p + 1) * 21));
    }
  } else {
    lcd.setTextSize(1);
    for (int p = 0; p < (INPUT_TEXT.length() / 22) + 1; p++) {
      int c = p * 22;
      lcd.print(INPUT_TEXT.substring(c, c + 22));
      lcd.setCursor(3, 100 + ((p + 1) * 11));
    }
  }

  if (isAVI) {
    return;
  }

  uint8_t qrcodeData[qrcode_getBufferSize(3)];
  char IN_TEXT[OUTPUT_LINK.length() + 1];

  for (int i = 0; i < OUTPUT_LINK.length(); i++) {
    IN_TEXT[i] = OUTPUT_LINK.charAt(i);
    if (i == OUTPUT_LINK.length() - 1) {
      IN_TEXT[i + 1] = '\0';
    }
  }

  CURLINK = OUTPUT_LINK;
  Serial.println(OUTPUT_LINK);
  Serial.println(IN_TEXT);
  qrcode_initText(&qrcode, qrcodeData, 3, 0, IN_TEXT);

  // QR Code Starting Point
  int offset_x = 143;
  int offset_y = 63;
  for (uint8_t y = 0; y < qrcode.size; y++) {
    // Each horizontal module
    for (uint8_t x = 0; x < qrcode.size; x++) {
      int newX = offset_x + (x * 6);
      int newY = offset_y + (y * 6);

      if (qrcode_getModule(&qrcode, x, y)) {
        lcd.fillRect(newX, newY, 6, 6, TFT_BLACK);
      } else {
        lcd.fillRect(newX, newY, 6, 6, LG_BG);
      }
    }
  }
}

//Add additional content to Dot Bit Profile record for QR purposes(ie. add "https://t.me/" to the front of a telegram handle to allow redirection to a profile page)
void SET_OUTPUT_LINK(String IN_KEY, int IN_REF) {
  switch (IN_REF) {
    case 0:  //"twitter":
      OUTPUT_LINK = "https://www.twitter.com/" + IN_KEY;
      break;

    case 1:  //"facebook":
      OUTPUT_LINK = "https://www.facebook.com/" + IN_KEY;
      break;

    case 2:  //"reddit":
      OUTPUT_LINK = "https://www.reddit.com/user/" + IN_KEY;
      break;

    case 3:  //"linkedin":
      OUTPUT_LINK = "https://www.linkedin.com/" + IN_KEY;
      break;

    case 4:  //"github":
      OUTPUT_LINK = "https://www.github.com/" + IN_KEY;
      break;

    case 5:  //"telegram"
      OUTPUT_LINK = "https://t.me/" + IN_KEY;
      break;

    case 6:  //"description":
      OUTPUT_LINK = IN_KEY;
      break;

    case 7:  //"avatar":
      OUTPUT_LINK = IN_KEY;
      break;

    case 8:  //"instagram":
      OUTPUT_LINK = "https://www.instagram.com/" + IN_KEY;
      break;

    case 9:  //"weibo":
      OUTPUT_LINK = "https://e.weibo.com/" + IN_KEY;
      break;

    case 10:  //"discord":
      OUTPUT_LINK = "https://discordapp.com/users/" + IN_KEY;
      break;

    case 11:  //"email":
      OUTPUT_LINK = "mailto:" + IN_KEY;
      break;

    case 12:  //"website":
      OUTPUT_LINK = IN_KEY;
      break;

    case 13:  //"youtube":
      OUTPUT_LINK = "https://www.youtube.com/" + IN_KEY;
      break;

    case 14:  //"bilibili":
      OUTPUT_LINK = "https://www.bilibili.com/" + IN_KEY;
      break;

    case 15:  //"tiktok":
      OUTPUT_LINK = "https://www.tiktok.com/@" + IN_KEY;
      break;

    case 16:  //"jike":
      OUTPUT_LINK = "https://www.okjike.com/" + IN_KEY;
      break;

    case 17:  //"nextid":
      OUTPUT_LINK = "https://next.id/" + IN_KEY;
      break;

    case 18:  //"dribbble":
      OUTPUT_LINK = "https://dribbble.com/" + IN_KEY;
      break;

    case 19:  //"behance":
      OUTPUT_LINK = "https://behance.net/" + IN_KEY;
      break;

    case 20:  //"mirror":
      OUTPUT_LINK = "https://dev.mirror.xyz/" + IN_KEY;
      break;

    case 21:  //"medium":
      OUTPUT_LINK = "https://medium.com/" + IN_KEY;
      break;

    case 22:  //"nostr":
      OUTPUT_LINK = "nostr:" + IN_KEY;
      break;
  }
}

//Saved json record recall
void OPEN_SAVED_RECORDS() {
  NUM_SAVED = 0;
  SAVED_REC_PAGE = 0;
  SAVED_REC_ON_PAGE = 0;
  lcd.drawPngFile(SD, "/gui/savedrecords.png", 0, 0);
  File REC_FOLDER = SD.open("/records");

  if (!REC_FOLDER) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!REC_FOLDER.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File JSON_FILE = REC_FOLDER.openNextFile();
  int CX = 25;
  int CY = 80;

  while (JSON_FILE) {
    if (JSON_FILE.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(JSON_FILE.name());
    } else {
      String DBN = JSON_FILE.name();
      if (DBN.indexOf(".json") != -1) {
        NUM_SAVED++;
        if (NUM_SAVED < 11) {
          SAVED_REC[SAVED_REC_ON_PAGE] = "/records/" + DBN;
          SAVED_REC_ON_PAGE++;
          lcd.setCursor(CX, CY);
          lcd.setTextColor(TFT_BLACK);
          lcd.setTextSize(1);
          DBN = DBN.substring(0, DBN.indexOf(".json"));
          DBN = DBN + ".bit";
          lcd.print(DBN);
          CY += 15;
        }
      }

      Serial.print("  FILE: ");
      Serial.print(JSON_FILE.name());
      Serial.print("  SIZE: ");
      Serial.println(JSON_FILE.size());
    }
    JSON_FILE = REC_FOLDER.openNextFile();
  }
  lcd.setCursor(166, 35);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(3);
  lcd.print(NUM_SAVED);
  lcd.fillRoundRect(260, 205, 50, 25, 2, TFT_GREY);
  lcd.setCursor(270, 209);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE);
  lcd.print("Page#");
  lcd.setCursor(288, 220);
  SAVED_REC_PAGES = int(NUM_SAVED / 10) + 1;

  if (SAVED_REC_PAGES < 2) {
    lcd.drawPngFile(SD, "/gui/upDisabled.png", 293, 108);
    lcd.drawPngFile(SD, "/gui/downDisabled.png", 293, 147);

  } else if (SAVED_REC_PAGES > 1 && SAVED_REC_PAGES < 10) {
    lcd.drawPngFile(SD, "/gui/upDisabled.png", 293, 108);
  } else if (SAVED_REC_PAGES > 9 && SAVED_REC_PAGES < 100) {
    lcd.drawPngFile(SD, "/gui/upDisabled.png", 293, 108);
    lcd.setCursor(275, 220);
  } else if (SAVED_REC_PAGES > 99) {
    lcd.drawPngFile(SD, "/gui/upDisabled.png", 293, 108);
    lcd.setCursor(262, 220);
  }
  lcd.print("1/" + String(SAVED_REC_PAGES));
}

//Page Shifting logic for Saved Records
void UPDATE_SAVED_RECORDS() {
  int NUM_COUNT = 0;
  SAVED_REC_ON_PAGE = 0;
  lcd.drawPngFile(SD, "/gui/savedrecords.png", 0, 0);
  File REC_FOLDER = SD.open("/records");

  if (!REC_FOLDER) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!REC_FOLDER.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }

  File JSON_FILE = REC_FOLDER.openNextFile();
  int CX = 25;
  int CY = 80;
  while (JSON_FILE) {
    if (JSON_FILE.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(JSON_FILE.name());
    } else {
      String DBN = JSON_FILE.name();
      if (DBN.indexOf(".json") != -1) {
        NUM_COUNT++;
        if (NUM_COUNT > (SAVED_REC_PAGE * 10) && NUM_COUNT < ((SAVED_REC_PAGE + 1) * 10) + 1) {
          SAVED_REC[SAVED_REC_ON_PAGE] = "/records/" + DBN;
          SAVED_REC_ON_PAGE++;
          lcd.setCursor(CX, CY);
          lcd.setTextColor(TFT_BLACK);
          lcd.setTextSize(1);
          DBN = DBN.substring(0, DBN.indexOf(".json"));
          DBN = DBN + ".bit";
          lcd.print(DBN);
          CY += 15;
        }
      }
    }
    JSON_FILE = REC_FOLDER.openNextFile();
  }
  lcd.setCursor(166, 35);
  lcd.setTextColor(TFT_WHITE);
  lcd.setTextSize(3);
  lcd.print(NUM_SAVED);
  lcd.fillRoundRect(260, 205, 50, 25, 2, TFT_GREY);
  lcd.setCursor(270, 209);
  lcd.setTextSize(1);
  lcd.setTextColor(TFT_WHITE);
  lcd.print("Page#");
  lcd.setCursor(288, 220);

  if (SAVED_REC_PAGE == 0) {
    lcd.drawPngFile(SD, "/gui/upDisabled.png", 293, 108);
    lcd.drawPngFile(SD, "/gui/downEnabled.png", 293, 147);
  } else if (SAVED_REC_PAGES == SAVED_REC_PAGE + 1) {
    lcd.drawPngFile(SD, "/gui/upEnabled.png", 293, 108);
    lcd.drawPngFile(SD, "/gui/downDisabled.png", 293, 147);
  } else {
    lcd.drawPngFile(SD, "/gui/upEnabled.png", 293, 108);
    lcd.drawPngFile(SD, "/gui/downEnabled.png", 293, 147);
  }

  if (SAVED_REC_PAGES > 10) {
    lcd.setCursor(275, 220);
  } else if (SAVED_REC_PAGES > 100) {
    lcd.setCursor(262, 220);
  }

  lcd.print(String(SAVED_REC_PAGE + 1) + "/" + String(SAVED_REC_PAGES));
}

//Load recall json data to screen
void PRINT_RECALLED(String FILENAME_) {
  dbResponse.clear();
  lcd.drawPngFile(SD, "/gui/response.png", 0, 0);
  lcd.setCursor(5, 37);
  File JSON_READ = SD.open(FILENAME_);
  String PAYLOAD;

  while (JSON_READ.available()) {
    char R_BUFF = JSON_READ.read();
    PAYLOAD += R_BUFF;
  }

  Serial.println(PAYLOAD);
  error = deserializeJson(dbResponse, PAYLOAD);

  if (error) {
    Serial.print("deserializeJson() failed: ");
    Serial.println(error.c_str());
    return;
  }

  const char* data_account = dbResponse["data"]["account"];
  CURRENT_DOT_BIT = String(data_account);

  if (CURRENT_DOT_BIT.length() > 25) {
    lcd.setCursor(5, 43);
    lcd.setTextSize(1);
  } else {
    lcd.setTextSize(2);
  }

  lcd.setTextColor(TFT_WHITE);
  lcd.print(CURRENT_DOT_BIT);

  if (MY_DOT_BIT == CURRENT_DOT_BIT) {
    lcd.drawPngFile(SD, "/gui/fav.png", 252, 63);
  } else {
    lcd.drawPngFile(SD, "/gui/unfav.png", 252, 63);
  }

  Serial.println(CURRENT_DOT_BIT);
  CHECK_RECORD();
}

//Touch response audio
void playTouch(int STYLE) {
  switch (STYLE) {
    case 1:
      ledcAttachPin(spk_pin, 1);
      ledcWriteNote(1, NOTE_E, 6);
      delay(30);
      ledcWriteNote(1, NOTE_A, 5);
      delay(20);
      ledcWriteNote(1, NOTE_B, 6);
      delay(80);
      ledcDetachPin(spk_pin);
      break;

    case 2:
      ledcAttachPin(spk_pin, 1);
      ledcWriteNote(1, NOTE_E, 3);
      delay(20);
      ledcWriteNote(1, NOTE_A, 4);
      delay(20);
      ledcDetachPin(spk_pin);
      break;

    case 3:
      ledcAttachPin(spk_pin, 1);
      ledcWriteNote(1, NOTE_E, 5);
      delay(60);
      ledcWriteNote(1, NOTE_D, 3);
      delay(30);
      ledcWriteNote(1, NOTE_C, 5);
      delay(60);
      ledcDetachPin(spk_pin);
      break;

    case 4:
      ledcAttachPin(spk_pin, 1);
      ledcWriteNote(1, NOTE_E, 2);
      delay(20);
      ledcWriteNote(1, NOTE_A, 4);
      delay(20);
      ledcDetachPin(spk_pin);
      break;

    case 5:
      ledcAttachPin(spk_pin, 1);
      ledcWriteNote(1, NOTE_Cs, 5);
      delay(30);
      ledcWriteNote(1, NOTE_A, 4);
      delay(40);
      ledcDetachPin(spk_pin);
      break;

    case 6:
      ledcAttachPin(spk_pin, 1);
      ledcWriteNote(1, NOTE_Bb, 5);
      delay(90);
      ledcWriteNote(1, NOTE_A, 5);
      delay(40);
      ledcDetachPin(spk_pin);
      break;

    case 7:
      ledcAttachPin(spk_pin, 1);
      ledcWriteNote(1, NOTE_C, 5);
      delay(70);
      ledcWriteNote(1, NOTE_Bb, 4);
      delay(100);
      ledcWriteNote(1, NOTE_F, 5);
      delay(150);
      ledcDetachPin(spk_pin);
      break;

    case 8:
      ledcAttachPin(spk_pin, 1);
      ledcWriteNote(1, NOTE_D, 7);
      delay(70);
      ledcWriteNote(1, NOTE_Bb, 5);
      delay(40);
      ledcWriteNote(1, NOTE_F, 6);
      delay(80);
      ledcDetachPin(spk_pin);
      break;
  }
}

//Is no longer a grab :) Creates bitmap image from generated QR data, adds 12 px of whitespace
void GrabImage(String FILE_N, String VALUE_IN) {
  playTouch(3);
  String OPATH = "/qrcodes/" + FILE_N + ".bmp";
  Serial.println(OPATH);
  if (!SD.exists(OPATH)) {
    byte VH, VL;
    int i = 0;
    int j = 0;
    int h = 198;
    int w = 198;
    File outFile = SD.open(OPATH, "w");
    unsigned char bmFlHdr[14] = {
      'B', 'M', 0, 0, 0, 0, 0, 0, 0, 0, 54, 0, 0, 0
    };
    // 54 = std total "old" Windows BMP file header size = 14 + 40

    unsigned char bmInHdr[40] = {
      40, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 16, 0
    };
    // 40 = info header size
    //  1 = num of color planes
    // 16 = bits per pixel
    // all other header info = 0, including RI_RGB (no compr), DPI resolution

    unsigned long fileSize = 2ul * h * w + 54;  // pix data + 54 byte hdr

    bmFlHdr[2] = (unsigned char)(fileSize);       // all ints stored little-endian
    bmFlHdr[3] = (unsigned char)(fileSize >> 8);  // i.e., LSB first
    bmFlHdr[4] = (unsigned char)(fileSize >> 16);
    bmFlHdr[5] = (unsigned char)(fileSize >> 24);

    bmInHdr[4] = (unsigned char)(w);
    bmInHdr[5] = (unsigned char)(w >> 8);
    bmInHdr[6] = (unsigned char)(w >> 16);
    bmInHdr[7] = (unsigned char)(w >> 24);
    bmInHdr[8] = (unsigned char)(h);
    bmInHdr[9] = (unsigned char)(h >> 8);
    bmInHdr[10] = (unsigned char)(h >> 16);
    bmInHdr[11] = (unsigned char)(h >> 24);

    outFile.write(bmFlHdr, sizeof(bmFlHdr));
    outFile.write(bmInHdr, sizeof(bmInHdr));

    byte L_ROW[33];
    byte H_ROW[33];

    uint8_t qrcodeData[qrcode_getBufferSize(3)];
    char IN_TEXT[VALUE_IN.length() + 1];

    for (int i = 0; i < VALUE_IN.length(); i++) {
      IN_TEXT[i] = VALUE_IN.charAt(i);
      if (i == VALUE_IN.length() - 1) {
        IN_TEXT[i + 1] = '\0';
      }
    }

    Serial.println(VALUE_IN);
    Serial.println(IN_TEXT);

    qrcode_initText(&qrcode, qrcodeData, 3, 0, IN_TEXT);

    // QR Code Starting Point
    int offset_x = 0;
    int offset_y = 0;

    for (uint8_t y = qrcode.size + 4; y > 0; y--) {
      if (y == qrcode.size + 4) {
        VH = (QR_MAIN & 0xFF00) >> 8;  // High Byte
        VL = QR_MAIN & 0x00FF;         // Low Byte
        VL = (VH << 7) | ((VL & 0xC0) >> 1) | (VL & 0x1f);
        VH = VH >> 1;

        for (int a = 0; a < 2; a++) {
          for (int b = 0; b < 6; b++) {
            for (int d = 0; d < 33; d++) {
              for (int WI = 0; WI < 6; WI++) {
                outFile.write(VL);
                outFile.write(VH);
              }
            }
          }
          y--;
        }
      }

      // Each horizontal module
      for (uint8_t x = 0; x < qrcode.size + 4; x++) {

        int newX = offset_x + (x * 6);
        int newY = offset_y + (y * 6);
        uint16_t rgb;

        if (x == 0) {
          VH = (QR_MAIN & 0xFF00) >> 8;  // High Byte
          VL = QR_MAIN & 0x00FF;         // Low Byte
          VL = (VH << 7) | ((VL & 0xC0) >> 1) | (VL & 0x1f);
          VH = VH >> 1;

          for (int d = 0; d < 2; d++) {
            for (int WI = 0; WI < 6; WI++) {
              L_ROW[x] = VL;
              H_ROW[x] = VH;
            }
            x++;
          }
        }

        if (qrcode_getModule(&qrcode, x - 2, y - 3)) {
          //lcd.fillRect( newX, newY, 1, 1, TFT_BLACK);
          rgb = QR_BG;
        } else {
          //lcd.fillRect( newX, newY, 1, 1, LG_BG);
          rgb = QR_MAIN;
        }

        VH = (rgb & 0xFF00) >> 8;  // High Byte
        VL = rgb & 0x00FF;         // Low Byte

        //RGB565 to RGB555 conversion... 555 is default for uncompressed BMP
        //this conversion is from ...topic=177361.0 and has not been verified
        VL = (VH << 7) | ((VL & 0xC0) >> 1) | (VL & 0x1f);
        VH = VH >> 1;
        L_ROW[x] = VL;
        H_ROW[x] = VH;
        //Write image data to file, low byte first

        if (x == 31) {
          VH = (QR_MAIN & 0xFF00) >> 8;  // High Byte
          VL = QR_MAIN & 0x00FF;         // Low Byte
          VL = (VH << 7) | ((VL & 0xC0) >> 1) | (VL & 0x1f);
          VH = VH >> 1;

          for (int d = 0; d < 2; d++) {
            for (int WI = 0; WI < 6; WI++) {
              L_ROW[x] = VL;
              H_ROW[x] = VH;
            }
            x++;
          }
        }
      }
      for (int b = 0; b < 6; b++) {
        for (int d = 0; d < 33; d++) {
          for (int WI = 0; WI < 6; WI++) {
            outFile.write(L_ROW[d]);
            outFile.write(H_ROW[d]);
          }
        }
      }
    }
    VH = (QR_BG & 0xFF00) >> 8;  // High Byte
    VL = QR_BG & 0x00FF;         // Low Byte
    VL = (VH << 7) | ((VL & 0xC0) >> 1) | (VL & 0x1f);
    VH = VH >> 1;

    for (int a = 0; a < 2; a++) {
      for (int b = 0; b < 6; b++) {
        for (int d = 0; d < 33; d++) {
          for (int WI = 0; WI < 6; WI++) {
            outFile.write(VL);
            outFile.write(VH);
          }
        }
      }
    }
    //Close the file
    Serial.println(outFile.size());
    outFile.close();

    lcd.drawPngFile(SD, "/gui/qSAVEd.png", 290, 30);
  }
  qTIME = millis();
  SAVED_NOTE = true;
}

//Randomish colour generator (doesn't yeild control until done, independant touch logic)
void colourPicker(String SETTING_NAME, uint16_t* COLOUR_CODE) {
  lcd.drawPngFile(SD, "/gui/colours.png", 0, 0);
  lcd.fillRect(0, 60, 320, 180, TFT_BLACK);
  lcd.setCursor(35, 35);
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_WHITE);
  lcd.print(SETTING_NAME);
  bool PICKED = false;
  bool SAVED = false;
  int XS = 0;
  int XY = 80;
  uint16_t CUR_COL = 0x0000;
  uint16_t COL_ARRAY[32];
  for (int c = 0; c < 32; c++) {
    lcd.fillRect(XS, XY, 40, 40, CUR_COL);
    XS += 40;
    COL_ARRAY[c] = CUR_COL;
    CUR_COL = random(0x0000, 0xffff);
    if (SETTING_NAME == "Main Body Colour") {
      while (!isWCAG(QR_BG, CUR_COL)) {
        CUR_COL = random(0x0000, 0xffff);
      }
    } else if (SETTING_NAME == "Background Colour") {
      while (!isWCAG(QR_MAIN, CUR_COL)) {
        CUR_COL = random(0x0000, 0xffff);
      }
    }
    if (XS == 320) {
      XS = 0;
      XY += 40;
    }
  }

  int XPOS, YPOS;

  while (!SAVED) {
    if (lcd.getTouch(&XPOS, &YPOS) && millis() - LAST_TOUCH > 150) {
      if (XPOS > 235 && XPOS < 290 && YPOS > 29 && YPOS < 60) {  // Randomise
        XS = 0;
        XY = 80;
        CUR_COL = 0x0000;
        COL_ARRAY[32];
        for (int c = 0; c < 32; c++) {
          lcd.fillRect(XS, XY, 40, 40, CUR_COL);
          XS += 40;
          COL_ARRAY[c] = CUR_COL;
          CUR_COL = random(0x0000, 0xffff);
          if (SETTING_NAME == "Main Body Colour") {
            while (!isWCAG(QR_BG, CUR_COL)) {
              CUR_COL = random(0x0000, 0xffff);
            }
          } else if (SETTING_NAME == "Background Colour") {
            while (!isWCAG(QR_MAIN, CUR_COL)) {
              CUR_COL = random(0x0000, 0xffff);
            }
          }
          if (XS == 320) {
            XS = 0;
            XY += 40;
          }
        }
        CUR_COL = 0x0000;
      } else if (XPOS > 289 && YPOS > 29 && YPOS < 60) {  // Save
        if (PICKED) {
          *COLOUR_CODE = CUR_COL;
          SAVED = true;

        } else {
          lcd.drawPngFile(SD, "/gui/NOPICK.png", 200, 30);
        }
      } else if (YPOS > 79) {  //colours
        XS = 0;
        XY = 80;
        for (int TP = 0; TP < 32; TP++) {
          if (XPOS >= XS && XPOS < XS + 40 && YPOS >= XY && YPOS < XY + 40) {
            CUR_COL = COL_ARRAY[TP];
            lcd.fillRect(0, 60, 320, 20, COL_ARRAY[TP]);
            PICKED = true;
            break;
          }
          XS += 40;
          if (XS == 320) {
            XS = 0;
            XY += 40;
          }
        }
      }
      LAST_TOUCH = millis();
    }
  }
}

//SMTP callback function from ESP Mail Client Example
void smtpCallback(SMTP_Status status) {
  /* Print the current status */
  Serial.println(status.info());

  /* Print the sending result */
  if (status.success()) {
    // ESP_MAIL_PRINTF used in the examples is for format printing via debug Serial port
    // that works for all supported Arduino platform SDKs e.g. AVR, SAMD, ESP32 and ESP8266.
    // In ESP8266 and ESP32, you can use Serial.printf directly.

    Serial.println("----------------");
    ESP_MAIL_PRINTF("Message sent success: %d\n", status.completedCount());
    ESP_MAIL_PRINTF("Message sent failed: %d\n", status.failedCount());
    Serial.println("----------------\n");

    for (size_t i = 0; i < smtp.sendingResult.size(); i++) {
      /* Get the result item */
      SMTP_Result result = smtp.sendingResult.getItem(i);

      // In case, ESP32, ESP8266 and SAMD device, the timestamp get from result.timestamp should be valid if
      // your device time was synched with NTP server.
      // Other devices may show invalid timestamp as the device time was not set i.e. it will show Jan 1, 1970.
      // You can call smtp.setSystemTime(xxx) to set device time manually. Where xxx is timestamp (seconds since Jan 1, 1970)
      time_t ts = (time_t)result.timestamp;

      ESP_MAIL_PRINTF("Message No: %d\n", i + 1);
      ESP_MAIL_PRINTF("Status: %s\n", result.completed ? "success" : "failed");
      ESP_MAIL_PRINTF("Date/Time: %s\n", asctime(localtime(&ts)));
      ESP_MAIL_PRINTF("Recipient: %s\n", result.recipients.c_str());
      ESP_MAIL_PRINTF("Subject: %s\n", result.subject.c_str());
    }
    Serial.println("----------------\n");

    // You need to clear sending result as the memory usage will grow up.
    smtp.sendingResult.clear();
  }
}

//Send Mail with attachment
void sendMailAttach(String QR_FILE_, String SUBJECT_, String BODY_) {
  if (CUR_PAGE == 11) {
    lcd.drawPngFile(SD, "/gui/emailed.png", 290, 30);
  } else {
    lcd.drawPngFile(SD, "/gui/emailed.png", 260, 30);
  }
  eTIMER = millis();
  Session_Config config;

  config.server.host_name = SMTP_HOST;
  config.server.port = SMTP_PORT;
  config.login.email = AUTHOR_EMAIL;
  config.login.password = AUTHOR_PASSWORD;

  Serial.println(AUTHOR_EMAIL);
  Serial.println(SMTP_HOST);
  Serial.println(SMTP_PORT);
  Serial.println(AUTHOR_PASSWORD);
  Serial.println(SUBJECT_);
  Serial.println(BODY_);

  /** Assign your host name or you public IPv4 or IPv6 only
     as this is the part of EHLO/HELO command to identify the client system
     to prevent connection rejection.
     If host name or public IP is not available, ignore this or
     use generic host "mydomain.net".

     Assign any text to this option may cause the connection rejection.
  */
  config.login.user_domain = F("mydomain.net");
  config.time.ntp_server = F("pool.ntp.org,time.nist.gov");
  config.time.gmt_offset = 9.5;
  config.time.day_light_offset = 0;

  /* Declare the message class */
  SMTP_Message message;

  /* Set the message headers */
  message.sender.name = AUTHOR_NAME;    // This witll be used with 'MAIL FROM' command and 'From' header field.
  message.sender.email = AUTHOR_EMAIL;  // This witll be used with 'From' header field.
  message.subject = SUBJECT_;
  message.addRecipient(F("QR Reciever"), F(RECIPIENT_EMAIL));  // This will be used with RCPT TO command and 'To' header field.
  message.text.charSet = F("utf-8");
  message.text.content = "Qr Send " + QR_FILE_ + " sent from my .bit lookup tool\n" + BODY_;
  message.text.transfer_encoding = Content_Transfer_Encoding::enc_7bit;
  message.priority = esp_mail_smtp_priority::esp_mail_smtp_priority_normal;
  message.response.reply_to = AUTHOR_EMAIL;
  message.addHeader(F("Message-ID: .Bit QR Thing"));
  SMTP_Attachment att;
  att.descr.filename = (QR_FILE_ + ".bmp");
  att.descr.mime = F("image/bmp");  // json file
  att.descr.description = ("QR-Code :" + QR_FILE_);
  att.file.path = ("/qrcodes/" + QR_FILE_ + ".bmp");
  att.file.storage_type = esp_mail_file_storage_type_sd;
  att.descr.transfer_encoding = Content_Transfer_Encoding::enc_base64;
  message.addAttachment(att);

  /* Connect to the server */
  if (!smtp.connect(&config)) {
    Serial.println("Failed");
    return;
  } else {
    Serial.println("Connect Success");
  }

  if (smtp.isAuthenticated()) {
    Serial.println("Successfully logged in.");
    playTouch(5);
  } else {
    Serial.println("Connected with no Auth.");
    playTouch(2);
  }
  /* Start sending Email and close the session */
  if (!MailClient.sendMail(&smtp, &message)) {
    Serial.println("Error sending Email, " + smtp.errorReason());
  } else {
    playTouch(5);
  }
  // to clear sending result log
  // smtp.sendingResult.clear();

  ESP_MAIL_PRINTF("Free Heap: %d\n", MailClient.getFreeHeap());
}

//Timezone Offset Lookup using WorldTimeApi
void GET_TIMEZONE(String REGION_, String CITY_) {
  DynamicJsonDocument TZJ(1000);
  String jsOut = "";
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = String(WTAPI) + REGION_ + "/" + CITY_;
    Serial.println(serverPath);
    http.begin(serverPath);
    int httpResponseCode = http.GET();
    jsOut = "\0";

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      //Serial.println(payload);
      error = deserializeJson(TZJ, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }

      const char* abbreviation = TZJ["abbreviation"];  // "ACST"
      //const char* client_ip = TZJ["client_ip"];        // "220.244.174.27"
      //const char* datetime = TZJ["datetime"];          // "2023-04-21T19:24:25.777534+09:30"
      //int day_of_week = TZJ["day_of_week"];            // 5
      //int day_of_year = TZJ["day_of_year"];            // 111
      //bool dst = TZJ["dst"];                           // false
      // TZJ["dst_from"] is null
      int dst_offset = TZJ["dst_offset"];  // 0
      // TZJ["dst_until"] is null
      long raw_offset = TZJ["raw_offset"];     // 34200
      const char* timezone = TZJ["timezone"];  // "Australia/Adelaide"
      long unixtime = TZJ["unixtime"];         // 1682070865
      //const char* utc_datetime = TZJ["utc_datetime"];  // "2023-04-21T09:54:25.777534+00:00"
      const char* utc_offset = TZJ["utc_offset"];  // "+09:30"
      //int week_number = TZJ["week_number"];            // 16

      CUR_TIME_OFFSET = raw_offset;
      TZSTRING = String(abbreviation) + " " + String(utc_offset) + " utc";
      Serial.println(raw_offset);
      Serial.println(CUR_TIME_OFFSET);
      rtc.setTime(unixtime);
    }
  }
}

//Display Clock
void DISPLAY_CLOCK() {
  lcd.fillRect(80, 3, 120, 27, LG_BG);
  lcd.setCursor(80, 3);
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_BLACK);

  if (TwentyFourHour) {
    lcd.print(rtc.getTime("%R "));
  } else {
    lcd.print(rtc.getTime("%I:%M "));
  }

  lcd.setTextSize(1);
  lcd.setCursor(150, 6);
  lcd.print(rtc.getTime("%A"));
  lcd.setCursor(80, 20);
  lcd.print(rtc.getTime("%B %d %G"));
  SHOT_CLOCK = millis();
}

//Save Settings to eeprom
void saveToEeprom() {
  int addressTrack = 0;
  String arraySender = "";
  SAVED_SETTINGS = true;
  Serial.println("clearing eeprom");
  for (int i = 0; i < 500; ++i) {
    EEPROM.write(i, 0);
  }

  EEPROM.write(addressTrack, SAVED_SETTINGS);
  addressTrack += sizeof(bool);

  char TDB[50];
  for (int d = 0; d < MY_DOT_BIT.length(); d++) {
    TDB[d] = MY_DOT_BIT.charAt(d);
    if (d == MY_DOT_BIT.length() - 1) {
      TDB[d + 1] = '\0';
    }
  }
  Serial.println(TDB);
  for (int charCount = 0; charCount < 50; charCount++) {
    EEPROM.write(addressTrack + charCount, TDB[charCount]);  // bytes 1
  }
  addressTrack += 50;

  EEPROM.writeLong(addressTrack, CUR_TIME_OFFSET);  //bytes 4
  addressTrack += sizeof(long);

  EEPROM.writeUShort(addressTrack, QR_MAIN);  //bytes 4
  addressTrack += sizeof(uint16_t);

  EEPROM.writeUShort(addressTrack, QR_BG);  //bytes 4
  addressTrack += sizeof(uint16_t);

  char TWS[50];
  for (int d = 0; d < WIFI_SSID.length(); d++) {
    TWS[d] = WIFI_SSID.charAt(d);
    if (d == WIFI_SSID.length() - 1) {
      TWS[d + 1] = '\0';
    }
  }
  Serial.println(TWS);
  for (int charCount = 0; charCount < 50; charCount++) {
    EEPROM.write(addressTrack + charCount, TWS[charCount]);  // bytes 1
  }

  addressTrack += 50;
  char TWP[50];
  for (int d = 0; d < WIFI_PASSWORD.length(); d++) {
    TWP[d] = WIFI_PASSWORD.charAt(d);
    if (d == WIFI_PASSWORD.length() - 1) {
      TWP[d + 1] = '\0';
    }
  }

  for (int charCount = 0; charCount < 50; charCount++) {
    EEPROM.write(addressTrack + charCount, TWP[charCount]);  // bytes 1
  }

  addressTrack += 50;
  char TAE[50];
  for (int d = 0; d < AUTHOR_EMAIL.length(); d++) {
    TAE[d] = AUTHOR_EMAIL.charAt(d);
    if (d == AUTHOR_EMAIL.length() - 1) {
      TAE[d + 1] = '\0';
    }
  }
  for (int charCount = 0; charCount < 50; charCount++) {
    EEPROM.write(addressTrack + charCount, TAE[charCount]);  // bytes 1
  }
  Serial.println(TAE);
  Serial.println(AUTHOR_EMAIL.length());
  addressTrack += 50;
  char TAP[50];
  for (int d = 0; d < AUTHOR_PASSWORD.length(); d++) {
    TAP[d] = AUTHOR_PASSWORD.charAt(d);
    if (d == AUTHOR_PASSWORD.length() - 1) {
      TAP[d + 1] = '\0';
    }
  }
  for (int charCount = 0; charCount < 50; charCount++) {
    EEPROM.write(addressTrack + charCount, TAP[charCount]);  // bytes 1
  }
  Serial.println(TAP);
  Serial.println(AUTHOR_PASSWORD.length());
  addressTrack += 50;
  char TSH[50];
  for (int d = 0; d < SMTP_HOST.length(); d++) {
    TSH[d] = SMTP_HOST.charAt(d);
    if (d == SMTP_HOST.length() - 1) {
      TSH[d + 1] = '\0';
    }
  }
  for (int charCount = 0; charCount < 50; charCount++) {
    EEPROM.write(addressTrack + charCount, TSH[charCount]);  // bytes 1
  }
  Serial.println(TSH);
  Serial.println(SMTP_HOST.length());
  addressTrack += 50;

  EEPROM.writeUShort(addressTrack, SMTP_PORT);  //bytes 4
  addressTrack += sizeof(uint16_t);

  EEPROM.write(addressTrack, TwentyFourHour);  //bytes 4
  addressTrack += sizeof(bool);


  EEPROM.commit();
  Serial.println("SUCCESSFUL " + String(addressTrack) + " bytes written");
}

//Load Settings from eeprom
void loadFromEeprom() {
  int addressTrack = 0;


  SAVED_SETTINGS = EEPROM.read(addressTrack);
  if (!SAVED_SETTINGS) {
    Serial.println("No Data saved");
    return;
  }
  addressTrack += sizeof(bool);
  Serial.println(SAVED_SETTINGS);
  char T_DBNAME[50];
  for (int charCount = 0; charCount < 50; charCount++) {
    T_DBNAME[charCount] = EEPROM.read(addressTrack + charCount);  // bytes 1
    if(!isAlphaNumeric(T_DBNAME[0])){
      SAVED_SETTINGS = false;
      break;
    }
  }
  MY_DOT_BIT = String(T_DBNAME);
  Serial.println(T_DBNAME);
  addressTrack += 50;

  CUR_TIME_OFFSET = EEPROM.readLong(addressTrack);  //bytes 4
  addressTrack += sizeof(long);

  QR_MAIN = EEPROM.readUShort(addressTrack);  //bytes 4
  addressTrack += sizeof(uint16_t);

  QR_BG = EEPROM.readUShort(addressTrack);  //bytes 4
  addressTrack += sizeof(uint16_t);

  Serial.println(CUR_TIME_OFFSET);
  gmtOffset = CUR_TIME_OFFSET;
  char T_WS[50];
  for (int charCount = 0; charCount < 50; charCount++) {
    T_WS[charCount] = EEPROM.read(addressTrack + charCount);  // bytes 1
    Serial.println(T_WS[charCount]);
    if(!isAlphaNumeric(T_WS[0])){
      SAVED_SETTINGS = false;
      Serial.println("Non AlphaNumeric");
      return;
    }
  }
  WIFI_SSID = String(T_WS);
  Serial.println(WIFI_SSID);
  addressTrack += 50;
  char T_WP[50];
  for (int charCount = 0; charCount < 50; charCount++) {
    T_WP[charCount] = EEPROM.read(addressTrack + charCount);  // bytes 1
  }
  WIFI_PASSWORD = String(T_WP);
  Serial.println(WIFI_PASSWORD);
  addressTrack += 50;
  char TAE[50];
  for (int charCount = 0; charCount < 50; charCount++) {
    TAE[charCount] = EEPROM.read(addressTrack + charCount);  // bytes 1
  }
  AUTHOR_EMAIL = String(TAE);
  Serial.println(AUTHOR_EMAIL);
  addressTrack += 50;
  char TAP[50];

  for (int charCount = 0; charCount < 50; charCount++) {
    TAP[charCount] = EEPROM.read(addressTrack + charCount);  // bytes 1
  }
  AUTHOR_PASSWORD = String(TAP);
  addressTrack += 50;
  char TSH[50];
  for (int charCount = 0; charCount < 50; charCount++) {
    TSH[charCount] = EEPROM.read(addressTrack + charCount);  // bytes 1
  }
  SMTP_HOST = String(TSH);
  Serial.println(SMTP_HOST);
  addressTrack += 50;

  SMTP_PORT = EEPROM.readUShort(addressTrack);  //bytes 4
  addressTrack += sizeof(uint16_t);

  TwentyFourHour = EEPROM.read(addressTrack);  //bytes 4
  addressTrack += sizeof(bool);
}

//Clear Textbox for Keyboard entry
void CLEAR_TEXTBOX() {
  for (int t = 0; t < 51; t++) {
    TEXTBOX[t] = '\0';
  }
}

//WiFi Scan for SSID selection
void LOAD_SSID() {
  lcd.drawPngFile(SD, "/gui/ssid.png", 0, 0);
  lcd.setTextColor(TFT_WHITE);
  lcd.setCursor(35, 34);
  lcd.print("SSID");
  bool ISDONE = false;
  int n = WiFi.scanNetworks();
  String SSIDS[n];
  int RSSIS[n];
  int CHANNELS[n];
  bool ISLOCKED[n];
  int WIFI_POS = 0;

  for (int w = 0; w < n; w++) {
    SSIDS[w] = WiFi.SSID(w);
    RSSIS[w] = WiFi.RSSI(w);
    CHANNELS[w] = WiFi.channel(w);
    if (WiFi.encryptionType(w) == WIFI_AUTH_OPEN) {
      ISLOCKED[w] = false;
    } else {
      ISLOCKED[w] = true;
    }
  }

  lcd.fillRect(83, 135, 153, 94, TFT_WHITE);
  lcd.setTextColor(TFT_BLACK);
  lcd.setColor(TFT_BLACK);
  lcd.drawLine(83, 159, 236, 159);
  lcd.drawLine(83, 181, 236, 181);
  lcd.drawLine(83, 203, 236, 203);
  String SHORTSSID;

  if (n < 5) {
    int CX = 85;
    int CY = 145;
    lcd.setTextSize(1);

    for (int o = 0; o < n; o++) {
      if (SSIDS[o].length() > 22) {
        SHORTSSID = SSIDS[o].substring(0, 22);
      } else {
        SHORTSSID = SSIDS[o];
      }
      lcd.setCursor(CX, CY);
      lcd.print(String(o + 1) + ") " + SHORTSSID);
      CY += 22;
    }
  } else {
    int CX = 85;
    int CY = 145;
    lcd.setTextSize(1);

    for (int o = 0; o < 4; o++) {
      if (SSIDS[o].length() > 22) {
        SHORTSSID = SSIDS[o].substring(0, 22);
      } else {
        SHORTSSID = SSIDS[o];
      }
      lcd.setCursor(CX, CY);
      lcd.print(String(o + 1) + ") " + SHORTSSID);
      CY += 22;
    }
    lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 70);
  }
  int XPOS;
  int YPOS;

  while (!ISDONE) {
    if (lcd.getTouch(&XPOS, &YPOS)) {
      if (XPOS > 83 && XPOS < 236 && YPOS > 136 && YPOS < 159 && n > 0 + WIFI_POS) {  //record 1
        playTouch(5);
        WIFI_SSID = SSIDS[WIFI_POS];
        ISDONE = true;
      } else if (XPOS > 83 && XPOS < 236 && YPOS > 159 && YPOS < 181 && n > 1 + WIFI_POS) {  //record 2
        playTouch(5);
        WIFI_SSID = SSIDS[WIFI_POS + 1];
        ISDONE = true;
      } else if (XPOS > 83 && XPOS < 236 && YPOS > 181 && YPOS < 203 && n > 2 + WIFI_POS) {  //record 3
        playTouch(5);
        WIFI_SSID = SSIDS[WIFI_POS + 2];
        ISDONE = true;
      } else if (XPOS > 83 && XPOS < 236 && YPOS > 203 && YPOS < 230 && n > 3 + WIFI_POS) {  //record 4
        playTouch(5);
        WIFI_SSID = SSIDS[WIFI_POS + 3];
        ISDONE = true;
      } else if (XPOS < 35 && YPOS > 70 && YPOS < 102 && WIFI_POS > 3) {  //previous page
        playTouch(6);
        WIFI_POS = WIFI_POS - 4;
        lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 70);
        if (WIFI_POS == 0) {
          lcd.drawPngFile(SD, "/gui/leftDisabled.png", 0, 70);
        }
        lcd.fillRect(83, 135, 153, 94, TFT_WHITE);
        lcd.setTextColor(TFT_BLACK);
        lcd.setColor(TFT_BLACK);
        lcd.drawLine(83, 159, 236, 159);
        lcd.drawLine(83, 181, 236, 181);
        lcd.drawLine(83, 203, 236, 203);
        int CX = 85;
        int CY = 145;
        lcd.setTextSize(1);

        for (int o = 0; o < 4; o++) {
          if (SSIDS[o + WIFI_POS].length() > 22) {
            SHORTSSID = SSIDS[o + WIFI_POS].substring(0, 22);
          } else {
            SHORTSSID = SSIDS[o + WIFI_POS];
          }
          lcd.setCursor(CX, CY);
          lcd.print(String(o + 1 + WIFI_POS) + ") " + SHORTSSID);
          CY += 22;
        }
      }
    } else if (XPOS > 285 && YPOS > 70 && YPOS < 102 && (n - WIFI_POS > 4)) {  //next page
      playTouch(6);
      WIFI_POS = WIFI_POS + 4;
      lcd.drawPngFile(SD, "/gui/leftEnabled.png", 0, 70);
      if (n - WIFI_POS < 5) {
        lcd.drawPngFile(SD, "/gui/rightDisabled.png", 290, 70);
      }
      lcd.fillRect(83, 135, 153, 94, TFT_WHITE);
      lcd.setTextColor(TFT_BLACK);
      lcd.setColor(TFT_BLACK);
      lcd.drawLine(83, 159, 236, 159);
      lcd.drawLine(83, 181, 236, 181);
      lcd.drawLine(83, 203, 236, 203);
      if (n - WIFI_POS < 4) {
        int CX = 85;
        int CY = 145;
        lcd.setTextSize(1);

        for (int o = 0; o < n; o++) {
          if (SSIDS[o + WIFI_POS].length() > 22) {
            SHORTSSID = SSIDS[o + WIFI_POS].substring(0, 22);
          } else {
            SHORTSSID = SSIDS[o + WIFI_POS];
          }
          lcd.setCursor(CX, CY);
          lcd.print(String(o + 1 + WIFI_POS) + ") " + SHORTSSID);
          CY += 22;
        }
      } else {
        int CX = 85;
        int CY = 145;
        lcd.setTextSize(1);

        for (int o = 0; o < 4; o++) {
          if (SSIDS[o + WIFI_POS].length() > 22) {
            SHORTSSID = SSIDS[o + WIFI_POS].substring(0, 22);
          } else {
            SHORTSSID = SSIDS[o + WIFI_POS];
          }
          lcd.setCursor(CX, CY);
          lcd.print(String(o + 1 + WIFI_POS) + ") " + SHORTSSID);
          CY += 22;
        }
      }
    }
  }
}

//Toggle WiFi Password Display
void LOAD_PASSWORD() {

  lcd.drawPngFile(SD, "/gui/wifiSettings.png", 0, 0);
  lcd.setCursor(20, 110);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setTextSize(2);
  lcd.print(WIFI_SSID);
  lcd.setCursor(20, 178);
  lcd.setTextColor(TFT_YELLOW);
  lcd.setTextSize(2);
  showWP = false;
  if (showWP) {
    lcd.print(WIFI_PASSWORD);
  } else {
    for (int CH = 0; CH < WIFI_PASSWORD.length(); CH++) {
      lcd.print("*");
    }
  }
}

//Display saved QR on screen
void OPEN_SAVED_QR() {
  NUM_SAVED = 0;
  lcd.drawPngFile(SD, "/gui/bitodex.png", 0, 0);
  File REC_FOLDER = SD.open("/qrcodes");

  if (!REC_FOLDER) {
    Serial.println("Failed to open directory");
    return;
  }
  if (!REC_FOLDER.isDirectory()) {
    Serial.println("Not a directory");
    return;
  }
  File BITMAP_FILE = REC_FOLDER.openNextFile();
  while (BITMAP_FILE) {

    if (BITMAP_FILE.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(BITMAP_FILE.name());

    } else {

      String DBN = BITMAP_FILE.name();
      if (DBN.indexOf(".bmp") != -1) {
        NUM_SAVED++;
      }
      Serial.print("  FILE: ");
      Serial.print(BITMAP_FILE.name());
      Serial.print("  SIZE: ");
      Serial.println(BITMAP_FILE.size());
    }
    BITMAP_FILE = REC_FOLDER.openNextFile();
  }
  REC_FOLDER.close();
  String QRFILES[NUM_SAVED];
  REC_FOLDER = SD.open("/qrcodes");
  BITMAP_FILE = REC_FOLDER.openNextFile();
  NUM_SAVED = 0;

  while (BITMAP_FILE) {
    if (BITMAP_FILE.isDirectory()) {
      Serial.print("  DIR : ");
      Serial.println(BITMAP_FILE.name());
    } else {
      String DBN = BITMAP_FILE.name();
      if (DBN.indexOf(".bmp") != -1) {
        QRFILES[NUM_SAVED] = DBN;
        NUM_SAVED++;
      }
      Serial.print("  FILE: ");
      Serial.print(BITMAP_FILE.name());
      Serial.print("  SIZE: ");
      Serial.println(BITMAP_FILE.size());
    }
    BITMAP_FILE = REC_FOLDER.openNextFile();
  }

  bool LEAVE = false;
  int QR_RECORD = 0;
  int XPOS;
  int YPOS;
  String RTYPE[3] = { "address", "profile", "dweb" };
  int R_TYPE = 0;
  lcd.drawBmpFile(SD, "/qrcodes/" + QRFILES[QR_RECORD], 88, 94, 143, 143, 0, 0, 0.7222222, 0.7222222, datum_t::top_left);
  String U_NAME = QRFILES[QR_RECORD].substring(0, QRFILES[QR_RECORD].indexOf("-"));
  String U_ASSET = QRFILES[QR_RECORD].substring(QRFILES[QR_RECORD].indexOf("-") + 1, QRFILES[QR_RECORD].indexOf(".bmp"));
  lcd.setCursor(35, 33);
  lcd.setTextSize(2);
  lcd.setTextColor(TFT_WHITE);
  lcd.print(U_NAME + ".bit");

  if (NUM_SAVED > 1) {
    lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 124);
  }
  for (int q = 0; q < 38; q++) {
    if (q < 5) {
      if (U_ASSET == DB_DWEB[q]) {
        R_TYPE = 2;
        lcd.drawPngFile(SD, "/dweb/" + U_ASSET + ".png", 5, 65, 30, 30, 0, 0, 1, 1, datum_t::top_left);
      }
    }
    if (q < 23) {
      if (U_ASSET == DB_PROFILE[q]) {
        R_TYPE = 1;
        lcd.drawPngFile(SD, "/profile/" + U_ASSET + ".png", 5, 65, 30, 30, 0, 0, 1, 1, datum_t::top_left);
      }
    }
    if (q < 38) {
      if (U_ASSET == DB_ADDRESS[q]) {
        R_TYPE = 0;
        lcd.drawPngFile(SD, "/address/" + U_ASSET + ".png", 5, 65, 30, 30, 0, 0, 1, 1, datum_t::top_left);
      }
    }
  }
  long T_B = millis() + 200;
  while (LEAVE == false) {
    if (lcd.getTouch(&XPOS, &YPOS) && millis() - T_B > 200) {
      if (XPOS < 50 && YPOS > 110 && YPOS < 170 && QR_RECORD != 0) {  //PREV REC
        QR_RECORD--;
        lcd.drawBmpFile(SD, "/qrcodes/" + QRFILES[QR_RECORD], 88, 94, 143, 143, 0, 0, 0.7222222, 0.7222222, datum_t::top_left);
        U_NAME = QRFILES[QR_RECORD].substring(0, QRFILES[QR_RECORD].indexOf("-"));
        U_ASSET = QRFILES[QR_RECORD].substring(QRFILES[QR_RECORD].indexOf("-") + 1, QRFILES[QR_RECORD].indexOf(".bmp"));
        lcd.fillRect(30, 30, 260, 30, TFT_BLACK);
        lcd.setCursor(5, 33);
        lcd.setTextSize(2);
        lcd.setTextColor(TFT_WHITE);
        lcd.print(U_NAME + ".bit");
        if (QR_RECORD == 0) {
          lcd.drawPngFile(SD, "/gui/leftDisabled.png", 0, 124);
          lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 124);
        } else {
          lcd.drawPngFile(SD, "/gui/rightEnabled.png", 290, 124);
        }

        switch (R_TYPE) {
          case 0:
            lcd.drawPngFile(SD, "/address/" + U_ASSET + ".png", 5, 65, 30, 30, 0, 0, 1, 1, datum_t::top_left);
            break;

          case 1:
            lcd.drawPngFile(SD, "/profile/" + U_ASSET + ".png", 5, 65, 30, 30, 0, 0, 1, 1, datum_t::top_left);
            break;

          case 2:
            lcd.drawPngFile(SD, "/dweb/" + U_ASSET + ".png", 5, 65, 30, 30, 0, 0, 1, 1, datum_t::top_left);
            break;
        }
      } else if (XPOS > 270 && YPOS > 110 && YPOS < 170 && QR_RECORD < NUM_SAVED - 1) {  //NEXT REC
        QR_RECORD++;
        lcd.drawBmpFile(SD, "/qrcodes/" + QRFILES[QR_RECORD], 88, 94, 143, 143, 0, 0, 0.7222222, 0.7222222, datum_t::top_left);
        U_NAME = QRFILES[QR_RECORD].substring(0, QRFILES[QR_RECORD].indexOf("-"));
        U_ASSET = QRFILES[QR_RECORD].substring(QRFILES[QR_RECORD].indexOf("-") + 1, QRFILES[QR_RECORD].indexOf(".bmp"));
        lcd.fillRect(0, 30, 290, 30, TFT_BLACK);
        lcd.setCursor(5, 33);
        lcd.setTextSize(2);
        lcd.setTextColor(TFT_WHITE);
        lcd.print(U_NAME + ".bit");
        if (QR_RECORD == NUM_SAVED - 1) {
          lcd.drawPngFile(SD, "/gui/leftEnabled.png", 0, 124);
          lcd.drawPngFile(SD, "/gui/rightDisabled.png", 290, 124);
        } else {
          lcd.drawPngFile(SD, "/gui/leftEnabled.png", 0, 124);
        }
        switch (R_TYPE) {
          case 0:
            lcd.drawPngFile(SD, "/address/" + U_ASSET + ".png", 5, 65, 30, 30, 0, 0, 1, 1, datum_t::top_left);
            break;

          case 1:
            lcd.drawPngFile(SD, "/profile/" + U_ASSET + ".png", 5, 65, 30, 30, 0, 0, 1, 1, datum_t::top_left);
            break;

          case 2:
            lcd.drawPngFile(SD, "/dweb/" + U_ASSET + ".png", 5, 65, 30, 30, 0, 0, 1, 1, datum_t::top_left);
            break;
        }
      } else if (XPOS > 198 && XPOS < 230 && YPOS < 31) {  //HOME
        LEAVE = true;
        playTouch(3);
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 10;
        CUR_PAGE = 0;
      } else if (XPOS > 229 && XPOS < 260 && YPOS < 31) {  //Saved Records
        playTouch(7);
        LAST_PAGE = 10;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      } else if (XPOS > 289 && YPOS < 31) {  //Settings Page
        playTouch(8);
        LAST_PAGE = 10;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);

      } else if (XPOS > 290 && YPOS > 30 && YPOS < 61) {  //Email Record
        playTouch(7);
        LAST_PAGE = 10;
        CUR_PAGE = 11;
        EMAIL_SENDER(QRFILES[QR_RECORD], CURRENT_DOT_BIT, CUR_QR);
      }

      T_B = millis();
    }
  }
}

//Contrast Ratio Compliance ChatGPT with the assist
bool isWCAG(uint16_t COL1, uint16_t COL2) {
  bool IS_COMPLIANT = false;

  // Convert RGB565 to sRGB
  // Extract the red, green, and blue components of each color
  uint8_t r1 = (COL1 >> 11) << 3;
  uint8_t g1 = ((COL1 >> 5) & 0x3F) << 2;
  uint8_t b1 = (COL1 & 0x1F) << 3;

  uint8_t r2 = (COL2 >> 11) << 3;
  uint8_t g2 = ((COL2 >> 5) & 0x3F) << 2;
  uint8_t b2 = (COL2 & 0x1F) << 3;

  // Calculate the relative luminance (brightness) of each color using the formula L = 0.2126*R^gamma + 0.7152*G^gamma + 0.0722*B^gamma, where gamma = 2.2
  float r1_lin = pow(r1 / 255.0, 2.2);
  float g1_lin = pow(g1 / 255.0, 2.2);
  float b1_lin = pow(b1 / 255.0, 2.2);
  float luminance1 = 0.2126 * r1_lin + 0.7152 * g1_lin + 0.0722 * b1_lin;

  float r2_lin = pow(r2 / 255.0, 2.2);
  float g2_lin = pow(g2 / 255.0, 2.2);
  float b2_lin = pow(b2 / 255.0, 2.2);
  float luminance2 = 0.2126 * r2_lin + 0.7152 * g2_lin + 0.0722 * b2_lin;

  // Calculate the contrast ratio between the two colors using the formula (L1 + 0.05) / (L2 + 0.05)
  float contrast_ratio;
  if (luminance1 > luminance2) {
    contrast_ratio = (luminance1 + 0.05) / (luminance2 + 0.05);
  } else {
    contrast_ratio = (luminance2 + 0.05) / (luminance1 + 0.05);
  }

  // Check if the contrast ratio is above 4
  if (contrast_ratio > 4.0) {
    IS_COMPLIANT = true;
  } else {
    IS_COMPLIANT = false;
  }

  return IS_COMPLIANT;
}

//Display Email Sender (Non Yeilding, independant touch logic)
void EMAIL_SENDER(String FN_, String UN_, String CQ_) {
  String SUB = UN_ + "'s " + CQ_ + " QR Code";
  String BOD = SUB;
  FN_HOLDER = FN_;
  UN_HOLDER = UN_;
  QR_HOLDER = CQ_;

  lcd.drawPngFile(SD, "/gui/emailHeader.png", 0, 0);
  if (RECIPIENT_EMAIL != "NONE") {
    lcd.fillRect(110, 72, 195, 26, EMAIL_BG);
    lcd.setCursor(112, 74);
    if (RECIPIENT_EMAIL.length() > 15 && RECIPIENT_EMAIL.length() < 30) {
      lcd.setTextSize(1);
      lcd.print(RECIPIENT_EMAIL);
    } else if (RECIPIENT_EMAIL.length() > 29) {
      lcd.setTextSize(1);
      lcd.print(RECIPIENT_EMAIL.substring(0, 29));
    } else {
      lcd.setTextSize(2);
      lcd.print(RECIPIENT_EMAIL);
    }
  }

  if (AUTHOR_NAME != ".bit QR Sender") {
    lcd.fillRect(110, 113, 195, 26, EMAIL_BG);
    lcd.setCursor(112, 115);
    if (AUTHOR_NAME.length() > 15 && AUTHOR_NAME.length() < 30) {
      lcd.setTextSize(1);
      lcd.print(AUTHOR_NAME);
    } else if (AUTHOR_NAME.length() > 29) {
      lcd.setTextSize(1);
      lcd.print(AUTHOR_NAME.substring(0, 29));
    } else {
      lcd.setTextSize(2);
      lcd.print(AUTHOR_NAME);
    }
  }

  if (EMAIL_SUBJECT != "NONE") {
    lcd.fillRect(131, 154, 174, 26, EMAIL_BG);
    lcd.setCursor(133, 156);
    if (EMAIL_SUBJECT.length() > 12 && EMAIL_SUBJECT.length() < 24) {
      lcd.setTextSize(1);
      lcd.print(EMAIL_SUBJECT);
      SUB = EMAIL_SUBJECT;
    } else if (EMAIL_SUBJECT.length() > 23) {
      lcd.setTextSize(1);
      lcd.print(EMAIL_SUBJECT.substring(0, 23));
      SUB = EMAIL_SUBJECT;
    } else {
      lcd.setTextSize(2);
      lcd.print(EMAIL_SUBJECT);
      SUB = EMAIL_SUBJECT;
    }
  }

  if (EMAIL_BODY != "NONE") {
    lcd.fillRect(131, 204, 174, 26, EMAIL_BG);
    lcd.setCursor(133, 206);
    if (EMAIL_BODY.length() > 12 && EMAIL_BODY.length() < 24) {
      lcd.setTextSize(1);
      lcd.print(EMAIL_BODY);
      BOD = EMAIL_BODY;
    } else if (EMAIL_BODY.length() > 23) {
      lcd.setTextSize(1);
      lcd.print(EMAIL_BODY.substring(0, 23));
      BOD = EMAIL_BODY;
    } else {
      lcd.setTextSize(2);
      lcd.print(EMAIL_BODY);
      BOD = EMAIL_BODY;
    }
  }

  bool isDone = false;
  int XPOS = 0;
  int YPOS = 0;
  long LAST_TOUCH = millis();
  while (!isDone) {
    if (lcd.getTouch(&XPOS, &YPOS)) {
      if (XPOS > 109 && XPOS < 306 && YPOS > 71 && YPOS < 99 && millis() - LAST_TOUCH > 200) {  //To
        playTouch(7);
        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
        CUR_SETTING = 0;
        LAST_PAGE = 11;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
        REFRESH_PAGE(CUR_PAGE);
        isDone = true;
        LAST_TOUCH = millis();
      } else if (XPOS > 109 && XPOS < 306 && YPOS > 112 && YPOS < 130 && millis() - LAST_TOUCH > 200) {  //From
        playTouch(7);
        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
        CUR_SETTING = 1;
        LAST_PAGE = 11;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
        REFRESH_PAGE(CUR_PAGE);
        isDone = true;
        LAST_TOUCH = millis();
      } else if (XPOS > 130 && XPOS < 306 && YPOS > 153 && YPOS < 181 && millis() - LAST_TOUCH > 200) {  //Subject
        playTouch(7);
        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
        CUR_SETTING = 2;
        LAST_PAGE = 11;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
        REFRESH_PAGE(CUR_PAGE);
        isDone = true;
        LAST_TOUCH = millis();
      } else if (XPOS > 130 && XPOS < 306 && YPOS > 203 && YPOS < 231 && millis() - LAST_TOUCH > 200) {  //Body
        playTouch(7);
        lcd.drawPngFile(SD, "/gui/keyboard1.png", 0, 30);
        CUR_SETTING = 3;
        LAST_PAGE = 11;
        CUR_PAGE = 1;
        TEXT_BOX = "";
        TEXT_COUNT = 0;
        CLEAR_TEXTBOX();
        REFRESH_PAGE(CUR_PAGE);
        isDone = true;
        LAST_TOUCH = millis();
      } else if (XPOS > 290 && YPOS > 30 && YPOS < 61 && millis() - LAST_TOUCH > 200) {  //Send
        if (RECIPIENT_EMAIL != "NONE") {
          sendMailAttach(FN_, SUB, BOD);
          lcd.drawPngFile(SD, "/gui/emailed.png", 290, 30);
        } else {
          playTouch(2);
          lcd.drawRect(131, 204, 174, 26, TFT_RED);
        }
        LAST_TOUCH = millis();
      } else if (XPOS > 229 && XPOS < 260 && YPOS < 31) {  //Saved Records
        isDone = true;
        playTouch(7);
        LAST_PAGE = 11;
        CUR_PAGE = 4;
        OPEN_SAVED_RECORDS();
      } else if (XPOS > 259 && XPOS < 290 && YPOS < 31) {  //Saved QR Codes
        isDone = true;
        playTouch(7);
        LAST_PAGE = 11;
        CUR_PAGE = 10;
        OPEN_SAVED_QR();
      } else if (XPOS > 289 && YPOS < 31) {  //Settings Page
        isDone = true;
        playTouch(8);
        LAST_PAGE = 11;
        CUR_PAGE = 5;
        lcd.drawPngFile(SD, "/gui/settings.png", 0, 0);
      } else if (XPOS > 198 && XPOS < 230 && YPOS < 31) {  //Home
        isDone = true;
        playTouch(4);
        lcd.drawPngFile(SD, "/gui/main.png", 0, 0);
        LAST_PAGE = 1;
        CUR_PAGE = 0;
        REFRESH_PAGE(CUR_PAGE);
      }
    }
  }
}

//Dot Bit Email lookup (uses "custom_key.email" record from dot bit)
String getEmailFromDotBit(String DOTBIT_NAME) {
  DynamicJsonDocument EMAILJSON(10000);
  object = doc.to<JsonObject>();
  object["account"] = DOTBIT_NAME;
  bool HAS_EMAIL = false;
  String RETURN_EMAIL;
  String jsOut = "";

  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    String serverPath = String(DOTBIT_ADD_SERVER);
    serializeJson(doc, jsOut);
    Serial.println(DOTBIT_ADD_SERVER);
    http.begin(serverPath);
    http.addHeader("Content-Type", "application/json");
    int httpResponseCode = http.POST(jsOut);
    jsOut = "\0";

    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String payload = http.getString();
      Serial.println(payload);
      error = deserializeJson(EMAILJSON, payload);
      if (error) {
        Serial.print("deserializeJson() failed: ");
        Serial.println(error.c_str());
      }

      const char* errmsg = EMAILJSON["errmsg"];  // nullptr
      const long err_no = EMAILJSON["errno"];
      const char* data_account = EMAILJSON["data"]["account"];  // "phillip.bit"
      Serial.println(errmsg);

      if (err_no == 0) {
        for (JsonObject data_record : EMAILJSON["data"]["records"].as<JsonArray>()) {
          const char* data_record_key = data_record["key"];      // "address.ckb", "profile.avatar", ...
          const char* data_record_label = data_record["label"];  // "CKB.PW (relay)", nullptr, nullptr, nullptr
          const char* data_record_value = data_record["value"];
          const char* data_record_ttl = data_record["ttl"];  // "300", "300", "300", "300"
          if (String(data_record_key) == "custom_key.email") {
            HAS_EMAIL = true;
            RETURN_EMAIL = data_record_value;
            if (RETURN_EMAIL.indexOf("mailto:") == 0) {
              RETURN_EMAIL = RETURN_EMAIL.substring(7);
            }
          }
        }
      } else if (err_no == 20007) {
        EMAILJSON["data"]["records"]["key"] = "Account Doesn't Exist";
        EMAILJSON["data"]["account"] = "Account Doesn't Exist";
      } else {
        Serial.println("Error " + String(err_no));
      }
    }
  }
  if (!HAS_EMAIL) {
    RETURN_EMAIL = "NONE";
  }

  return RETURN_EMAIL;
}