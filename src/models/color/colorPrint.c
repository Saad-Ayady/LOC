#include <windows.h>
#include <stdio.h>
#include <wchar.h>


#define APP_N "LOC"

void enableANSI() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (hOut == INVALID_HANDLE_VALUE)
        return;

    DWORD dwMode = 0;
    if (!GetConsoleMode(hOut, &dwMode))
        return;

    dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    SetConsoleMode(hOut, dwMode);
}

void printColor(const wchar_t* color, const wchar_t* message) {
    wprintf(L"%ls%ls\033[0m", color, message);
}

void printRed(const wchar_t* message)    { printColor(L"\033[1;31m\n", message); }
void printGreen(const wchar_t* message)  { printColor(L"\033[1;32m\n", message); }
void printBlue(const wchar_t* message)   { printColor(L"\033[1;34m\n", message); }
void printYellow(const wchar_t* message) { printColor(L"\033[1;33m\n", message); }
void printBold(const wchar_t* message)   { printColor(L"\033[1m\n",    message); }

void printInfo(const wchar_t* message) {
    printBlue(L"[INFO] ");
    printBold(message);
    wprintf(L"\n");
}

void printSuccess(const wchar_t* message) {
    printGreen(L"[SUCCESS] ");
    printBold(message);
    wprintf(L"\n");
}

void printError(const wchar_t* message) {
    printRed(L"[ERROR] ");
    printBold(message);
    wprintf(L"\n");
}

void PrintW(const wchar_t* text) {
    DWORD written;
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), text, wcslen(text), &written, NULL);
}

void SetColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void printPanel() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    SetColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    puts("*====================================");
    SetColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    PrintW(L"  ⠀⠀⠀⣠⠶⡒⠒⢬⡲⣮⠂⣆⣀⠀⠀⠀⠀⠀⠀⢀⣤⣴⣦⣤⡀⠀⠀⠀⠀   \n");
    PrintW(L" ⠀⠀⠀⣀⣥⠠⣿⠆⠐⣻⣾⣿⣿⢷⡄⠀⠀⠀⠀⢠⡿⠋⠉⠉⠙⢿⡄⠀⠀⠀  \n");
    PrintW(L" ⠀⠀⢘⡵⢋⠄⡙⠒⣤⣄⣉⠙⣿⣗⠑⡄⠀⠀⠀⠘⡇⠀⠀⠀⠀⠈⡇⠀⠀⠀   \n");
    PrintW(L" ⠀⣴⢿⡜⢡⡞⢀⢼⣿⣿⣿⣿⣿⣿⠟⣂⠀⠀⢀⣀⠱⡀⠀⠀⠀⢰⠁⠀⠀⠀  \n");
    PrintW(L" ⠰⢫⢟⡇⢸⡇⢸⢾⣿⣿⣿⣿⣿⣿⡷⠰⠀⢰⡏⠀⠀⢡⠀⠀⢠⠃⠀⠀⠀⠀\n") ;
    PrintW(L" ⢰⠁⣿⢣⣿⠇⢀⣿⣿⡿⠿⠤⣭⣥⣶⡆⠀⠸⣷⣤⣠⡾⠀⢀⡇⠀⠀⠀⠀⠀\n");
    PrintW(L"  ⡞⣰⣧⠟⡝⢸⢸⣿⣥⠖⣴⡆⣤⣬⠉⠀⠀⠀⠈⠉⠉⠀⠀⢸⣇⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⡿⡟⢸⡇⠸⡄⢹⣿⢸⣿⣇⡏⠟⣰⣄⠀⠀⠀⠀⠀⠀⠀⠀⠉⠉⠁⠀⠀⠀\n");
    PrintW(L" ⠀⠇⣧⠘⡇⠦⣹⣸⣿⡇⡿⡿⣡⣼⣿⣿⣷⣦⣄⡀⠀⠀⣸⣿⣿⠄⠻⢷⣦⠀\n");
    PrintW(L" ⠀⢀⠘⣇⢹⡸⣿⣿⣿⢹⢃⣠⣿⣿⣿⣿⣿⣿⣿⣿⣆⠀⠑⠋⠉⠀⠀⠈⣿⣧\n");
    PrintW(L" ⠀⢸⣿⡌⠘⢷⣿⣿⡏⢀⣾⣿⣿⣿⣿⣿⣿⢻⣿⣿⣿⡆⠀⠀⠀⠀⠀⠀⣿⡿\n");
    PrintW(L" ⠀⠈⣿⣿⣦⡌⢿⠏⣰⣿⣿⣿⣿⣿⣿⡿⡏⣼⣿⣿⣿⡇⣄⠀⠀⠀⢀⣼⣿⠇\n");
    PrintW(L" ⠀⠀⠹⣿⣿⢻⡀⣼⣿⣿⢻⣿⣿⣿⣿⡇⡇⢻⣿⣿⣿⡇⣿⣿⣶⣿⣿⠟⠁⠀\n");
    PrintW(L" ⠀⠀⠀⢻⣿⣦⡓⢿⣿⣿⡆⣿⣿⣿⣿⢃⣶⡸⣿⣿⣿⡇⠀⠉⠉⠁⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠈⣿⣿⣿⡆⠀⠀⠀⣿⣿⣿⡟⣼⡿⠁⢹⣿⣿⣷⠀⠀⠀⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠀⢸⣿⣿⡇⠀⠀⢠⣿⣿⣿⢠⣿⣇⠀⠀⢻⣿⣿⡄⠀⠀⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠀⠘⣿⣿⣧⠀⠀⢸⣿⣿⡿⠘⣿⣿⣆⠀⠀⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠀⠀⣿⣿⣿⡄⠀⢸⣿⣿⡇⢀⣿⣿⡟⠀⠀⢸⣿⣿⡀⠀⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠀⡴⢿⣿⣿⡇⢠⡿⣿⣿⢷⢩⣿⡟⠁⠀⢀⢼⡿⣿⡇⠀⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠀⠁⢿⠸⠿⠇⠈⠃⠛⠃⠚⠈⠁⠀⠀⠀⠈⠸⠜⠛⠀⠀⠀⠀⠀⠀⠀\n");
    SetColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    puts("====================================");
    SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("=          Welcome to %s      =\n", APP_N);
    SetColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    puts("====================================\n");
    SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}