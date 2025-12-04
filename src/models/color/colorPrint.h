#ifndef COLOR_PRINT_H
#define COLOR_PRINT_H

#include <wchar.h>
#include <windows.h>  // <-- مهم جداً لتعريف WORD و دوال Windows API

// to work on cmd 
void enableANSI();

// Basic color printers
void printRed(const wchar_t* message);
void printGreen(const wchar_t* message);
void printBlue(const wchar_t* message);
void printYellow(const wchar_t* message);
void printBold(const wchar_t* message);

// Formatted messages
void printInfo(const wchar_t* message);
void printSuccess(const wchar_t* message);
void printError(const wchar_t* message);

// PRINTING FUNCTIONS WITH COLOR
void PrintW(const wchar_t* text);
void SetColor(WORD color);
void printPanel();

#endif // COLOR_PRINT_H
