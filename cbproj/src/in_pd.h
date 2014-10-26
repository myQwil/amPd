
#pragma once

#include <windows.h>
#include "PdBase.hpp"
#include "winamp.h"

#define NCH 2
#define SAMPLERATE 44100
#define BPS 16
#define ticks 9

using namespace pd;

std::string name(char* fn);
std::string path(char* fn);

PdBase libpd;
Patch patch;

int length;     // total time in milliseconds
int pos;		// current time in milliseconds.
int paused;		// are we paused?
int bufSize;    // size of buffer
volatile int seek;

char* bufr;
//char* artist;

PdReceiver* rec;

volatile int stopped=0;			// the kill switch for the decode thread
HANDLE hand=INVALID_HANDLE_VALUE;	// the handle to the decode thread
