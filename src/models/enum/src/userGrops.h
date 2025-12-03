#ifndef USER_AND_HOST_H
#define USER_AND_HOST_H

#include <windows.h>

// Maximum username/computer name lengths
#define UNLEN 256
#define MAX_COMPUTERNAME_LENGTH 15

void getUserGroups();
void getUsersAndGroups();

#endif // USER_AND_HOST_H
