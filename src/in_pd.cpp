
#include "in_pd.h"

extern In_Module mod;

extern "C" {
	void bonk_tilde_setup();
	void choice_setup();
	void expr_setup();
	void fiddle_tilde_setup();
	void loop_tilde_setup();
	void lrshift_tilde_setup();
	//void pd_tilde_setup(); // not sure if this builds on windows
	void pique_setup();
	void sigmund_tilde_setup();
	void stdout_setup();

	void rand_setup();
	void randv_setup();
	void grand_setup();
	void graid_setup();
	void counter_setup();
	void demux_setup();
	void muse_setup();
	void ii_setup();
	void vectorPlus_setup();
	void vectorMinus_setup();
	void cup_setup();
	void cupd_setup();
	void setup_0x3c0x7e();
	void setup_0x3e0x7e();

	void makesymbol_setup();
	void filter_tilde_setup();
	void tanh_tilde_setup();
}

void Config(HWND hwndParent) {
    MessageBox(hwndParent,"do it, faggot","Configuration",MB_OK);
}

void About(HWND hwndParent) {
    MessageBox(hwndParent,"pdPlayer by Mike Willmot","About",MB_OK);
}

int InfoBox(const char *file, HWND hwndParent) {

	MessageBox(hwndParent,(char *)outBuf,"Message",MB_OK);
	return INFOBOX_UNCHANGED;
}

void Init() {
    libpd.init(NCH, NCH, SAMPLERATE);

    expr_setup();

	rand_setup();
	randv_setup();
	grand_setup();
	graid_setup();
	muse_setup();

	ii_setup();
	demux_setup();
	counter_setup();
	vectorPlus_setup();
	vectorMinus_setup();
	cup_setup();
	cupd_setup();
	setup_0x3c0x7e();
	setup_0x3e0x7e();

	makesymbol_setup();
	filter_tilde_setup();
	tanh_tilde_setup();

	bufSize = NCH*ticks*PdBase::blockSize()*2;
	inBuf = new short[bufSize];
    outBuf = new short[bufSize];

    libpd.addToSearchPath("C:\\Program Files (x86)\\Common Files\\Pd");
    libpd.computeAudio(true);
}

void Quit() { }

int IsOurFile(const char *fn) { return 0; }

int GetLength() { return 100000; }
int GetOutputTime() { return mod.outMod->GetOutputTime(); }
void SetOutputTime(int time_in_ms) { pos = time_in_ms; }
void SetVolume(int volume) { mod.outMod->SetVolume(volume); }
void SetPan(int pan) { mod.outMod->SetPan(pan); }
void Pause() { paused=1; mod.outMod->Pause(1); }
void UnPause() { paused=0; mod.outMod->Pause(0); }
int IsPaused() { return paused; }

void GetFileInfo(const char *filename, char *title, int *length_in_ms) {
    if (!filename || !*filename) { // currently playing file
		if (length_in_ms) *length_in_ms=GetLength();
		if (title) { // get non-path portion.of filename
			char *p=lastfn+strlen(lastfn);
			while (*p != '\\' && p >= lastfn) p--;
			strcpy(title,++p);
		}
	}
	else // some other file
	{
		if (length_in_ms) // calculate length
		{
			*length_in_ms=-1000; // the default is unknown file length (-1000).
		}
		if (title) // get non path portion of filename
		{
			const char *p=filename+strlen(filename);
			while (*p != '\\' && p >= filename) p--;
			strcpy(title,++p);
		}
	}
}

void DecodeThread() {
	int done=0; // set to TRUE if decoding has finished
	while (!killDecodeThread)
	{
		if (done) // done was set to TRUE during decoding, signaling eof
		{
			mod.outMod->CanWrite();
			if (!mod.outMod->IsPlaying())
			{
				// we're done playing, so tell Winamp and quit the thread.
				PostMessage(mod.hMainWindow,WM_WA_MPEG_EOF,0,0);
				killDecodeThread = true;
			}
			Sleep(10);		// give a little CPU time back to the system.
		}
		else if (mod.outMod->CanWrite() >= (bufSize<<(mod.dsp_isactive()?1:0))) {
			libpd.processShort(ticks, inBuf, outBuf);

			pos = mod.outMod->GetWrittenTime();
			mod.SAAddPCMData((char *)outBuf,NCH,BPS,pos);
			mod.VSAAddPCMData((char *)outBuf,NCH,BPS,pos);

			mod.outMod->Write((char *)outBuf, bufSize);
		}
		else Sleep(20);
		// if we can't write data, wait a little bit. Otherwise, continue
		// through the loop writing more data (without sleeping)
	}
}

DWORD CALLBACK LaunchThread(void* arg) {
	DecodeThread();
	return 0;
}

int Play(const char *fn) {
    int maxlatency;
	DWORD tid; // thread id
	paused=0;

	std::string name = fn;
	std::string path = fn;
	name = name.substr(name.find_last_of("\\") + 1);
	path = path.substr(0,path.find_last_of("\\"));

	p = libpd.openPatch(name,path);
	libpd.sendFloat("vol", 1);
	libpd.sendBang("play");

	maxlatency = mod.outMod->Open(SAMPLERATE,NCH,BPS, -1,-1);

	// maxlatency is the maxium latency between a outMod->Write() call and
	// when you hear those samples. In ms. Used primarily by the visualization
	// system.
	if (maxlatency < 0) { return 1; } // error opening device

	// dividing by 1000 for the first parameter of setinfo makes it
	// display 'H'... for hundred.. i.e. 14H Kbps.
	mod.SetInfo((SAMPLERATE*BPS*NCH)/1000,SAMPLERATE/1000,NCH,1);

	// initialize visualization stuff
	mod.SAVSAInit(maxlatency,SAMPLERATE);
	mod.VSASetInfo(SAMPLERATE,NCH);

	// -666 is a token for current volume
	mod.outMod->SetVolume(-666);

	// launch decode thread
	killDecodeThread=0;
	thread_handle = (HANDLE)
		CreateThread(0,0,LaunchThread,0,0,&tid);

	return 0;
}

void Stop() {
    if (thread_handle != INVALID_HANDLE_VALUE) {
		killDecodeThread=1;
		if (WaitForSingleObject(thread_handle,INFINITE) == WAIT_TIMEOUT) {
			MessageBox(mod.hMainWindow,"error asking thread to die!\n",
				"error killing decode thread",0);
			TerminateThread(thread_handle,0);
		}
		CloseHandle(thread_handle);
		thread_handle = INVALID_HANDLE_VALUE;
	}

	libpd.closePatch(p); // close the patch
	mod.outMod->Close(); // close output system
	mod.SAVSADeInit(); // deinitialize visualization
}

void EQSet(int on, char data[10], int preamp) { }

In_Module mod =
{
	IN_VER,	// defined in IN2.H
	"amPd (Pure Data Plugin)",
	0,	// hMainWindow (filled in by winamp)
	0,  // hDllInstance (filled in by winamp)
	"PD\0Pure Data Files (*.pd)\0",	// this is a double-null limited list. "EXT\0Description\0EXT\0Description\0" etc.
	1,	// is_seekable
	1,	// uses output plug-in system
	Config,
	About,
	Init,
	Quit,
	GetFileInfo,
	InfoBox,
	IsOurFile,
	Play,
	Pause,
	UnPause,
	IsPaused,
	Stop,

	GetLength,
	GetOutputTime,
	SetOutputTime,

	SetVolume,
	SetPan,

	0, 0, 0, 0, 0, 0, 0, 0, 0, // visualization calls filled in by winamp

	0, 0, // dsp calls filled in by winamp

	EQSet,

	NULL,		// setinfo call filled in by winamp

	0, // out_mod filled in by winamp
};

In_Module* winampGetInModule2(void) {
	return &mod;
}
