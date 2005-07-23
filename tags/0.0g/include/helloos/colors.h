/*
 * -= HelloOS Educational Project =-
 * -===============================-
 *
 *  $Id$
 *
 *  Цветовые константы
 *
 *  Предназначены для использования в вызовах puts,
 *  например: puts("Hello "FLGREEN"World!").
 *
 */


#ifndef __COLORS_H
#define __COLORS_H


// Константы цветов
#define BLACK     0x0
#define BLUE      0x1
#define GREEN     0x2
#define CYAN      0x3
#define RED       0x4
#define MAGENTA   0x5
#define YELLOW    0x6
#define LGRAY     0x7
#define GRAY      0x8
#define LBLUE     0x9
#define LGREEN    0xa
#define LCYAN     0xb
#define LRED      0xc
#define LMAGENTA  0xd
#define LYELLOW   0xe
#define WHITE     0xf


#define C_BLACK     "\010"
#define C_BLUE      "\011"
#define C_GREEN     "\012"
#define C_CYAN      "\013"
#define C_RED       "\014"
#define C_MAGENTA   "\015"
#define C_YELLOW    "\016"
#define C_LGRAY     "\017"
#define C_GRAY      "\020"
#define C_LBLUE     "\021"
#define C_LGREEN    "\022"
#define C_LCYAN     "\023"
#define C_LRED      "\024"
#define C_LMAGENTA  "\025"
#define C_LYELLOW   "\026"
#define C_WHITE     "\027"


#define FG  "\001"
#define BG  "\002"
#define RST "\003"

#define FBLACK    FG C_BLACK
#define FBLUE     FG C_BLUE
#define FGREEN    FG C_GREEN
#define FCYAN     FG C_CYAN
#define FRED      FG C_RED
#define FMAGENTA  FG C_MAGENTA
#define FYELLOW   FG C_YELLOW
#define FLGRAY    FG C_LGRAY
#define FGRAY     FG C_GRAY
#define FLBLUE    FG C_LBLUE
#define FLGREEN   FG C_LGREEN
#define FLCYAN    FG C_LCYAN
#define FLRED     FG C_LRED
#define FLMAGENTA FG C_LMAGENTA
#define FLYELLOW  FG C_LYELLOW
#define FWHITE    FG C_WHITE

#define BBLACK    BG C_BLACK
#define BBLUE     BG C_BLUE
#define BGREEN    BG C_GREEN
#define BCYAN     BG C_CYAN
#define BRED      BG C_RED
#define BMAGENTA  BG C_MAGENTA
#define BYELLOW   BG C_YELLOW
#define BLGRAY    BG C_LGRAY
#define BGRAY     BG C_GRAY
#define BLBLUE    BG C_LBLUE
#define BLGREEN   BG C_LGREEN
#define BLCYAN    BG C_LCYAN
#define BLRED     BG C_LRED
#define BLMAGENTA BG C_LMAGENTA
#define BLYELLOW  BG C_LYELLOW
#define BWHITE    BG C_WHITE


#endif // __COLORS_H
