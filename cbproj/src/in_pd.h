
#pragma once

#include <windows.h>
#include "PdBase.hpp"
#include "winamp.h"

#define WM_WA_MPEG_EOF WM_USER+2
#define NCH 2
#define SAMPLERATE 44100
#define BPS 16
#define ticks 9

using namespace pd;

PdBase libpd;
Patch p;

char lastfn[MAX_PATH];	// currently playing file

int file_length;// file length, in bytes
int pos;		// time in milliseconds.
int paused;		// are we paused?
int bufSize;    // size of buffer

short* inBuf; // interleaved input audio buffer
short* outBuf;
short* inBufb;
short* outBufb;

volatile int killDecodeThread=0;			// the kill switch for the decode thread
HANDLE thread_handle=INVALID_HANDLE_VALUE;	// the handle to the decode thread
