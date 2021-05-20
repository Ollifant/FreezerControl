/*
 * Display - Uno
 * VCC     - 3.3V
 * GND     - GND
 * SDI     - D11
 * SCLK    - D13
 * CS      - D10
 * DC      - D9
 * Res     - D8
 * Busy    - D7
 */
#define GxEPD2_DISPLAY_CLASS GxEPD2_3C

// select the display driver class (only one) for your  panel
// 3-color e-papers
#define GxEPD2_DRIVER_CLASS GxEPD2_213c     // GDEW0213Z16 104x212, UC8151 (IL0373)

// SS is usually used for CS. define here for easy change
// Note: SS is on 10 on UNO, NANO
#ifndef EPD_CS
#define EPD_CS SS
#endif

// somehow there should be an easier way to do this
#define GxEPD2_BW_IS_GxEPD2_BW true
#define GxEPD2_3C_IS_GxEPD2_3C true
#define GxEPD2_7C_IS_GxEPD2_7C true
#define GxEPD2_1248_IS_GxEPD2_1248 true
#define IS_GxEPD(c, x) (c##x)
#define IS_GxEPD2_BW(x) IS_GxEPD(GxEPD2_BW_IS_, x)
#define IS_GxEPD2_3C(x) IS_GxEPD(GxEPD2_3C_IS_, x)
#define IS_GxEPD2_7C(x) IS_GxEPD(GxEPD2_7C_IS_, x)
#define IS_GxEPD2_1248(x) IS_GxEPD(GxEPD2_1248_IS_, x)

#include "GxEPD2_selection_check.h"

#define MAX_DISPLAY_BUFFER_SIZE 800 

#define MAX_HEIGHT(EPD) (EPD::HEIGHT <= (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8) ? EPD::HEIGHT : (MAX_DISPLAY_BUFFER_SIZE / 2) / (EPD::WIDTH / 8))

// adapt the constructor parameters to your wiring
GxEPD2_DISPLAY_CLASS<GxEPD2_DRIVER_CLASS, MAX_HEIGHT(GxEPD2_DRIVER_CLASS)> display(GxEPD2_DRIVER_CLASS(/*CS=*/ EPD_CS, /*DC=*/ 9, /*RST=*/ 8, /*BUSY=*/ 7));
