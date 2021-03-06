
#pragma once

#include <windows.h>
#include "PdBase.hpp"
#include "winamp.h"
#include <sstream>
#include <fstream>

#define NCH 2 				// number of channels
#define Hz 44100			// sample rate
#define BPS 16 				// bits per sample
#define ticks 9 			// ticks per buffer

using namespace pd;

std::string name(char* fn);
std::string path(char* fn);

PdBase libpd;
Patch patch;

std::string current;

int length;     // total time in milliseconds
int pos;		// trackbar position
int paused;		// are we paused?
int bufSize;    // size of buffer
volatile int seek;

char* bufr;

volatile int stopped=0;			// the kill switch for the decode thread
HANDLE hand=INVALID_HANDLE_VALUE;	// the handle to the decode thread

// equalizer values
int eqOn;
int pre;
int eq[10];
