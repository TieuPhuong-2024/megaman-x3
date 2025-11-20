#ifndef __TRACE_H_
#define __TRACE_H_
#include <signal.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <fstream>
#include <iostream>
#include <sstream>

void trace(const wchar_t* format, ...);

//Show and save game logs
#define GAMELOG(s,...) printLog(s , ##__VA_ARGS__)
void printLog(LPCSTR s, ...);

#endif

