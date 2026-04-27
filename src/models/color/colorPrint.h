#ifndef COLOR_PRINT_H
#define COLOR_PRINT_H

#include <wchar.h>
#include <windows.h>  

void enableANSI();

void printRed(const wchar_t* message);
void printGreen(const wchar_t* message);
void printBlue(const wchar_t* message);
void printYellow(const wchar_t* message);
void printBold(const wchar_t* message);

void printInfo(const wchar_t* message);
void printSuccess(const wchar_t* message);
void printError(const wchar_t* message);

void PrintW(const wchar_t* text);
void SetColor(WORD color);
void printPanel();

#endif 
