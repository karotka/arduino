#ifndef CONFIGURE_H
#define CONFIGURE_H

const int NUMDSTEPS=20;
short dimtable[NUMDSTEPS][2] = {
   {600, 100},
   {620, 90},
   {640, 80},
   {660, 70},
   {680, 65},
   {700, 60},
   {720, 55},
   {740, 50},
   {760, 45},
   {780, 40},
   {800, 35},
   {820, 30},
   {840, 24},
   {860, 22},
   {880, 20},
   {900, 18},
   {960, 15},
   {980, 10},
   {990, 5},
   {1024, 2}
};

// define some color codes
#define white 0xFFFF
#define green 0x07E0
#define red 0xF800
#define cyan 0x07FF
#define magenta 0xF81F
#define yellow 0xFFE0
#define blue 0x001F

char daysOfTheWeek[7][12] = {
    "Nedele", "Pondeli", "Utery", "Streda", "Ctvrtek", "Patek", "Sobota"
};

enum {
    SHOW_TIME = 0,
    SHOW_DATE
};

// end of the header file
#endif
