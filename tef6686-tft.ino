
// Start of ESP32 touchscreen radio for tef6686
// 
// First attempt is to draw the first graphics and removing ssb

// source code from https://github.com/pe0mgb/SI4735-Radio-ESP32-Touchscreen-Arduino
// this code will be converted to tef6686 removing the lib for si4735
// going to use this lib for tef6686 https://github.com/makserge/tef6686_radio
// and the lib will be updated to patch 2.22


#include <TFT_eSPI.h>
#include <SPI.h>
#include "EEPROM.h"

TFT_eSPI tft = TFT_eSPI();

#define FM_BAND_TYPE 0
#define MW_BAND_TYPE 1
#define SW_BAND_TYPE 2
#define LW_BAND_TYPE 3

#define FM          0
#define AM          1


bool FirstLayer = true;
bool SecondLayer = false;
bool ThirdLayer = false;
bool ForthLayer = false;
bool VOLbut = false;
bool AGCgainbut = false;

int   freqDec = 0;

uint8_t currentVOL         =  0;

uint8_t currentAGCgain     =  1;

uint8_t bwIdxFM;
uint8_t bwIdxAM;
uint8_t bandIdx;
uint8_t currentMode = FM;


float Displayfreq      = 0;
float currentFrequency = 0;

String BWtext;

const char *bandwidthFM[] = {"311", "287", "254", "236", "217", "200", "184", "168", "151", "133", "114", " 97", " 84", " 72", " 64", " 56"};
const char *bandwidthAM[]  = {"8.0", "6.0", "4.0", "3.0"};
//const char *Keypathtext[]  = {"1", "2", "3", "4", "5", "6", "7", "8", "9", ".", "0", "Send", "Clear"};
const char *bandModeDesc[] = {"FM ", "AM "};

char buffer[64]; // Useful to handle string
char buffer1[64];

//=======================================================   Buttons First and Third Layer   ==========================

typedef struct // Buttons first layer
{
  const char *ButtonNam;
  
  int    ButtonNum;       // Button location at display from 0 to 11. To move around buttons freely at first layer.
  const char *ButtonNam1;
  int    ButtonNum1;      // Button location at display from 0 to 11. To move around buttons freely at third layer.
  int    XButos;          // Xoffset
  long   YButos;          // Yoffset
} Button;

int ytotoffset = 0;

//  Button table
int Xbutst  =   0;               // X Start location Buttons
int Ybutst  = 160 + ytotoffset;  // Y Start location Buttons

int Xsmtr   =   0;
int Ysmtr   =  85 + ytotoffset;  // S meter

int XVolInd =   0;
int YVolInd = 135 + ytotoffset;  // Volume indicator

int XFreqDispl  =   0;
int YFreqDispl  =   0 + ytotoffset;  // display

int Xbutsiz =  80;  //size of buttons first & third layer
int Ybutsiz =  40;

int Xbut0  = 0 * Xbutsiz ; int Ybut0  = 0 * Ybutsiz; // location calqualation for 12 first layer buttons
int Xbut1  = 1 * Xbutsiz ; int Ybut1  = 0 * Ybutsiz;
int Xbut2  = 2 * Xbutsiz ; int Ybut2  = 0 * Ybutsiz;
int Xbut3  = 0 * Xbutsiz ; int Ybut3  = 1 * Ybutsiz;
int Xbut4  = 1 * Xbutsiz ; int Ybut4  = 1 * Ybutsiz;
int Xbut5  = 2 * Xbutsiz ; int Ybut5  = 1 * Ybutsiz;
int Xbut6  = 0 * Xbutsiz ; int Ybut6  = 2 * Ybutsiz;
int Xbut7  = 1 * Xbutsiz ; int Ybut7  = 2 * Ybutsiz;
int Xbut8  = 2 * Xbutsiz ; int Ybut8  = 2 * Ybutsiz;
int Xbut9  = 0 * Xbutsiz ; int Ybut9  = 3 * Ybutsiz;
int Xbut10 = 1 * Xbutsiz ; int Ybut10 = 3 * Ybutsiz;
int Xbut11 = 2 * Xbutsiz ; int Ybut11 = 3 * Ybutsiz;

#define HAM       0
#define BFO       1
#define FREQ      2
#define AGC       3
#define MUTE      4
#define VOL       5
#define MODE      6
#define BANDW     7
#define STEP      8
#define BROAD     9
#define PRESET   10
#define NEXT     11

#define SEEKUP    0
#define SEEKDN    1
#define STATUS    2
#define RDSbut    3
#define AGCset    4
#define NR4       5
#define NR5       6
#define NR6       7
#define NR7       8  
#define NR8       9  
#define NR9      10  
#define PREV     11   

Button bt[] = {                                                 
  { "   "   ,  0 , "SEEKUP", 0 , Xbut0 , Ybut0  }, //     |----|----|----|
  { "   "   ,  3 , "SEEKDN", 3 , Xbut1 , Ybut1  }, //     |  0 |  1 |  2 |
  { "FREQ"  ,  2 , "STATUS",10 , Xbut2 , Ybut2  }, //     |----|----|----|
  { "AGC"   ,  4 , "RDS"   , 9 , Xbut3 , Ybut3  }, //     |  3 |  4 |  5 |
  { "MUTE"  ,  8 , "AGCset", 2 , Xbut4 , Ybut4  }, //     |----|----|----|
  { "VOL"   ,  7 , ""      , 5 , Xbut5 , Ybut5  }, //     |  6 |  7 |  8 |     
  { "MODE"  ,  9 , ""      , 6 , Xbut6 , Ybut6  }, //     |----|----|----|     
  { "BANDW" ,  5 , ""      , 7 , Xbut7 , Ybut7  }, //     |  9 | 10 | 11 |     
  { "STEP"  ,  6 , ""      , 8 , Xbut8 , Ybut8  }, //     |----|----|----|     
  { "BROAD" ,  1 , ""      , 1 , Xbut9 , Ybut9  },
  { "PRESET", 10 , ""      , 4 , Xbut10, Ybut10 },
  { "NEXT"  , 11 , "PREV"  ,11 , Xbut11, Ybut11 }
};


// You may freely move around the button (blue) position on the display to your flavour by changing the position in ButtonNum and ButtonNum1
// You have to stay in the First or Third Layer
//======================================================= End  Buttons First  and Third Layer   ======================
//======================================================= Broad Band Definitions     ==========================
typedef struct // Broad-Band switch
{
  uint16_t BbandNum; // bandIdx
  uint16_t Xbbandos;          //Xoffset
  uint16_t Xbbandsr;          //X size rectang
  uint16_t Xbbandnr;          //X next rectang
  uint16_t Ybbandos;          //Yoffset
  uint16_t Ybbandsr;          //X size rectang
  uint16_t Ybbandnr;          //Y next rectang
} BBandnumber;

//  Bandnumber table for the broad-bands

uint16_t Xfbband = 0;
uint16_t Yfbband = 45;


BBandnumber bb[] = {
  {  0 , Xfbband, 80 ,   0 , Yfbband , 30 ,   0}, // 0
  {  1 , Xfbband, 80 ,   0 , Yfbband , 30 ,  30}, // 1
  {  2 , Xfbband, 80 ,   0 , Yfbband , 30 ,  60}, // 2
  {  6 , Xfbband, 80 ,   0 , Yfbband , 30 ,  90}, // 3
  {  7 , Xfbband, 80 ,   0 , Yfbband , 30 , 120}, // 4
  {  9 , Xfbband, 80 ,   0 , Yfbband , 30 , 150}, // 5
  { 11 , Xfbband, 80 ,   0 , Yfbband , 30 , 180}, // 6
  { 13 , Xfbband, 80 ,   80 , Yfbband , 30 , 0}, // 7
  { 14 , Xfbband, 80 ,   80 , Yfbband , 30 , 30}, // 8
  { 16 , Xfbband, 80 , 80 , Yfbband , 30 ,   60}, // 9
  { 17 , Xfbband, 80 , 80 , Yfbband , 30 ,  90}, //10
  { 19 , Xfbband, 80 , 80 , Yfbband , 30 ,  120}, //11
  { 21 , Xfbband, 80 , 80 , Yfbband , 30 ,  150}, //12
  { 22 , Xfbband, 80 , 80 , Yfbband , 30 , 180}, //13
  { 24 , Xfbband, 80 , 160 , Yfbband , 30 , 0}, //14
  { 26 , Xfbband, 80 , 160 , Yfbband , 30 , 30}, //15
  { 27 , Xfbband, 80 , 160 , Yfbband , 30 , 60}, //16
  { 29 , Xfbband, 80 , 160 , Yfbband , 30 , 90}, //17

};
//======================================================= End Broad Band Definitions     ======================

//======================================================= Ham Band Definitions     ============================
typedef struct // Ham Band switch
{
  uint16_t BandNum; // bandIdx
  uint16_t HamBandTxt;
  uint16_t Xbandos;          //Xoffset
  uint16_t Xbandsr;          //X size rectang
  uint16_t Xbandnr;          //X next rectang
  uint16_t Ybandos;          //Yoffset
  uint16_t Ybandsr;          //Y size rectang
  uint16_t Ybandnr;          //Y next rectang
} Bandnumber;

//  Bandnumber table for the hambands

  uint16_t Xfband = 10;
  uint16_t Yfband = 30;

Bandnumber bn[] = {
  {  3 , 0 , Xfband, 110 ,   0 , Yfband , 30 ,   0},
  {  4 , 1 , Xfband, 110 ,   0 , Yfband , 30 ,  30},
  {  5 , 2 , Xfband, 110 ,   0 , Yfband , 30 ,  60},
  {  8 , 3 , Xfband, 110 ,   0 , Yfband , 30 ,  90},
  { 10 , 4 , Xfband, 110 ,   0 , Yfband , 30 , 120},
  { 12 , 5 , Xfband, 110 ,   0 , Yfband , 30 , 150},
  { 15 , 6 , Xfband, 110 , 110 , Yfband , 30 ,   0},
  { 18 , 7 , Xfband, 110 , 110 , Yfband , 30 ,  30},
  { 20 , 8 , Xfband, 110 , 110 , Yfband , 30 ,  60},
  { 23 , 9 , Xfband, 110 , 110 , Yfband , 30 ,  90},
  { 25 , 10 , Xfband, 110 , 110 , Yfband , 30 , 120},
  { 28 , 11 , Xfband, 110 , 110 , Yfband , 30 , 150}
};
//======================================================= End Ham Band Definitions     ========================

//======================================================= THE Band Definitions     ============================
typedef struct // Band data
{
  const char *bandName; // Bandname
  uint8_t  bandType;    // Band type (FM, MW or SW)
  uint16_t prefmod;     // Pref. modulation
  uint16_t minimumFreq; // Minimum frequency of the band
  uint16_t maximumFreq; // maximum frequency of the band
  uint16_t currentFreq; // Default frequency or current frequency
  uint16_t currentStep; // Default step (increment and decrement)
  //float BFOf1;            // BFO set for f1
  //float F1b;              // Freq1 in kHz
  //float BFOf2;            // BFO set for f2
  //float F2b;              // Freq2 in kHz
} Band;

//   Band table

Band band[] = {
  {   "FM", FM_BAND_TYPE,  FM,  8750, 10800,  9890,10}, //  FM          0
  {   "LW", LW_BAND_TYPE,  AM,   144,   513,   198, 9}, //  LW          1
  {   "MW", MW_BAND_TYPE,  AM,   522,  1701,  1008, 9}, //  MW          2
  {"BACON", LW_BAND_TYPE,  AM,   280,   470,   284, 1}, // Ham          3
  { "120M", SW_BAND_TYPE,  AM,  2300,  2495,  2400, 5}, //      120M    4
  {  "90M", SW_BAND_TYPE,  AM,  3200,  3400,  3300, 5}, //       90M    5
  {  "75M", SW_BAND_TYPE,  AM,  3900,  4000,  3950, 5}, //       75M    6
  {  "49M", SW_BAND_TYPE,  AM,  5900,  6200,  6000, 5}, //       49M    7
  {  "41M", SW_BAND_TYPE,  AM,  7200,  7450,  7210, 5}, //       41M    8
  {  "31M", SW_BAND_TYPE,  AM,  9400,  9900,  9600, 5}, //       31M    9
  {  "25M", SW_BAND_TYPE,  AM, 11600, 12100, 11700, 5}, //       25M   10
  {  "22M", SW_BAND_TYPE,  AM, 13570, 13870, 13700, 5}, //       22M   11
  {  "19M", SW_BAND_TYPE,  AM, 15100, 15830, 15700, 5}, //       19M   12
  {  "16M", SW_BAND_TYPE,  AM, 17480, 17900, 17600, 5}, //       16M   13
  {  "15M", SW_BAND_TYPE,  AM, 18900, 19020, 18950, 5}, //       15M   14
  {  "13M", SW_BAND_TYPE,  AM, 21450, 21850, 21500, 5}, //       13M   15
  {  "11M", SW_BAND_TYPE,  AM, 25670, 26100, 25800, 5}, //       11M   16
  {   "SW", SW_BAND_TYPE,  AM,  1730, 26999, 15500, 5}  // Whole SW    17
};
//======================================================= End THE Band Definitions     ========================

const int lastButton = (sizeof bt / sizeof(Button)) - 1;

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(0);
  uint16_t calData[5] = { 236, 3535, 440, 3406, 4 };
  tft.setTouch(calData);

  currentMode = AM;
  bandIdx = 3;
  
  DrawFila();

}

void loop() {
  // put your main code here, to run repeatedly:

}

//=======================================================================================
void DrawFila()   {// Draw of first layer
//=======================================================================================
  FirstLayer = true;
  SecondLayer = false;
  tft.fillScreen(TFT_BLACK);
  DrawButFila();
  DrawDispl();
  DrawSmeter();
  DrawVolumeIndicator();
}

//=======================================================================================
void DrawButFila() { // Buttons first layer
//=======================================================================================
  //tft.fillScreen(TFT_BLACK);
  for (int n = 0 ; n <= lastButton; n++) {
    tft.fillRect(bt[bt[n].ButtonNum].XButos + Xbutst, bt[bt[n].ButtonNum].YButos + Ybutst , Xbutsiz , Ybutsiz, TFT_WHITE);
    tft.fillRect((bt[bt[n].ButtonNum].XButos + Xbutst + 3) , (bt[bt[n].ButtonNum].YButos + Ybutst + 3), (Xbutsiz - 6) , (Ybutsiz - 6), TFT_BLUE);
    tft.setTextColor(TFT_YELLOW, TFT_BLUE);
    tft.setTextSize(2);
    tft.setTextDatum(BC_DATUM);
    tft.setTextPadding(0);
    tft.drawString((bt[n].ButtonNam), ( bt[bt[n].ButtonNum].XButos + Xbutst + (Xbutsiz / 2) ), (bt[bt[n].ButtonNum].YButos + Ybutst  + ((Ybutsiz) / 2 + 9)  ));
  }
//  drawBFO();
//  drawAGC();
}

//=======================================================================================
void DrawDispl() {
//=======================================================================================
  tft.fillRect(XFreqDispl, YFreqDispl, 240, 90, TFT_WHITE);
  tft.fillRect(XFreqDispl + 5, YFreqDispl + 5, 230, 80, TFT_BLACK);
  tft.setTextSize(1);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);
  tft.drawString(band[bandIdx].bandName, XFreqDispl + 160, YFreqDispl + 20);
  FreqDispl();
 
  if (band[bandIdx].bandType != FM_BAND_TYPE) {
    tft.setTextSize(1);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.drawString(bandModeDesc[currentMode], XFreqDispl + 80, YFreqDispl + 20);
    tft.setTextPadding(tft.textWidth("2.2kHz"));
    BWtext = bandwidthAM[bwIdxAM];
    tft.drawString(BWtext + "KHz", XFreqDispl + 120, YFreqDispl + 20);
    tft.drawString(String(band[bandIdx].currentStep) + "KHz", XFreqDispl + 200, YFreqDispl + 20);
  }
}

//=======================================================================================
void FreqDispl() {
//=======================================================================================  
  if ((FirstLayer) or (ThirdLayer)) {
    currentFrequency = 9500; //si4735.getFrequency(); 
    tft.fillRect( XFreqDispl + 6, YFreqDispl + 22 , 228, 45, TFT_BLACK); // Black freq. field
//    AGCfreqdisp(); 
    tft.setTextSize(4);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.setTextDatum(BC_DATUM);
    //tft.setTextPadding(0);
    if ((VOLbut) or (AGCgainbut)){
      if (VOLbut) {
        tft.setTextSize(3);
        tft.drawString(String(map(currentVOL, 20, 63, 0, 100)), XFreqDispl + 60, YFreqDispl + 53);
        tft.setTextSize(2);
          tft.drawString( " % Volume", XFreqDispl + 160, YFreqDispl + 53);
      }
      if (AGCgainbut){
        tft.setTextSize(3);
        tft.drawString(String(currentAGCgain), XFreqDispl + 60, YFreqDispl + 53);
        tft.setTextSize(2);
        tft.drawString("Attn-index", XFreqDispl + 160, YFreqDispl + 53);
      }
     
    } else {
      if (band[bandIdx].bandType == MW_BAND_TYPE || band[bandIdx].bandType == LW_BAND_TYPE) {
        Displayfreq =  currentFrequency;
        tft.setTextSize(4);
        tft.drawString(String(Displayfreq, 0), XFreqDispl + 120, YFreqDispl + 61);
        tft.setTextSize(2);
        tft.drawString("KHz", XFreqDispl + 215, YFreqDispl + 61);
      }
      if (band[bandIdx].bandType == FM_BAND_TYPE) {
        Displayfreq =  currentFrequency / 100;
        tft.setTextSize(4);
        tft.drawString(String(Displayfreq, 1), XFreqDispl + 120, YFreqDispl + 54);
        tft.setTextSize(2);
        tft.drawString("MHz", XFreqDispl + 215, YFreqDispl + 54);
      }
      if ((currentMode == AM) and (band[bandIdx].bandType != MW_BAND_TYPE || band[bandIdx].bandType != LW_BAND_TYPE)){
          Displayfreq =  currentFrequency / 1000;
          tft.setTextSize(4);
          tft.drawString(String(Displayfreq, 3), XFreqDispl + 120, YFreqDispl + 61);
          tft.setTextSize(2);
          tft.drawString("MHz", XFreqDispl + 215, YFreqDispl + 61);
      }
    }
  }
}

//=======================================================================================
void DrawSmeter()  {
//=======================================================================================
  String IStr;
  tft.setTextSize(1);
  tft.fillRect(Xsmtr, Ysmtr, 240, 55, TFT_WHITE);
  tft.fillRect(Xsmtr + 5, Ysmtr + 5, 230, 45, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextDatum(BC_DATUM);
  for (int i = 0; i < 10; i++) {
    tft.fillRect(Xsmtr + 15 + (i * 12), Ysmtr + 24, 4, 8, TFT_YELLOW);
    IStr = String(i);
    tft.setCursor((Xsmtr + 14 + (i * 12)), Ysmtr + 13);
    tft.print(i);
  }
  for (int i = 1; i < 7; i++) {
    tft.fillRect((Xsmtr + 123 + (i * 16)), Ysmtr + 24, 4, 8, TFT_RED);
    IStr = String(i * 10);
    tft.setCursor((Xsmtr + 117 + (i * 16)), Ysmtr + 13);
    if ((i == 2) or (i == 4) or (i == 6))  {
      tft.print("+");
      tft.print(i * 10);
    }  
  }
  tft.fillRect(Xsmtr + 15, Ysmtr + 32 , 112, 4, TFT_YELLOW);
  tft.fillRect(Xsmtr + 127, Ysmtr + 32 , 100, 4, TFT_RED);
  // end Smeter
}

//=======================================================================================
void DrawVolumeIndicator()  {
//=======================================================================================
  tft.setTextSize(1);
  tft.fillRect(XVolInd, YVolInd, 240, 30, TFT_WHITE);
  tft.fillRect(XVolInd + 5, YVolInd + 5, 230, 20, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setCursor(XVolInd +  11, YVolInd + 7);
  tft.print("0%");
  tft.setCursor(XVolInd + 116, YVolInd + 7);
  tft.print("50%");
  tft.setCursor(XVolInd + 210, YVolInd + 7);
  tft.print("100%");
}
