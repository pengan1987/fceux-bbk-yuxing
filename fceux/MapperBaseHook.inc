#ifndef _USE_TASEDITOR
bool __cdecl isTaseditorRecording(void) { return false; }
void __cdecl recordInputByTaseditor(void) {}
void __cdecl handleEmuCmdByTaseditor(int) {}
int __cdecl getInputType(class MovieData&) { return 0; }
bool __cdecl exitTASEditor(void) { return 0; }
bool __cdecl enterTASEditor(void) { return 0; }
bool __cdecl loadProject(char const*) { return 0; }
//#include "hq2x.h"
//#include "win/video.h"
//void hq3x_32(unsigned char* pIn, unsigned char* pOut, int Xres, int Yres, int BpL) {}
//void __cdecl hq3x_Kill(void) { hq2x_Kill(); }
//int __cdecl hq3x_InitLUTs(void) { return hq2x_InitLUTs(); }
void __cdecl enableGeneralKeyboardInput(void) {}
void __cdecl disableGeneralKeyboardInput(void) {}
void __cdecl updateTASEditor(void) {}
bool __cdecl askToSaveProject(void) { return false; }
bool mustRewindNow = false;
bool mustEngageTaseditor = false;
bool taseditorEnableAcceleratorKeys = false;
char** recentProjectsArray;
/*
extern INT_PTR CALLBACK VideoConCallB(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
typedef INT(CALLBACK* tVideoConCallB)(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
tVideoConCallB sysVideoConCallB = (tVideoConCallB)VideoConCallB;
INT CALLBACK  myVideoConCallB(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	extern int winspecial;
	if (uMsg == WM_COMMAND && (wParam & 0xFFFF) == ID_CANCEL) {
		winspecial = SendDlgItemMessage(hwndDlg, IDC_VIDEOCONFIG_SCALER_WIN, CB_GETCURSEL, 0, (LPARAM)(LPSTR)0);
		if (winspecial == 4)
			SendDlgItemMessage(hwndDlg, IDC_VIDEOCONFIG_SCALER_WIN, CB_SETCURSEL, 1, (LPARAM)(LPSTR)0);
		winspecial = SendDlgItemMessage(hwndDlg, IDC_VIDEOCONFIG_SCALER_FS, CB_GETCURSEL, 0, (LPARAM)(LPSTR)0);
		if (winspecial == 4)
			SendDlgItemMessage(hwndDlg, IDC_VIDEOCONFIG_SCALER_FS, CB_SETCURSEL, 1, (LPARAM)(LPSTR)0);
	}
	return sysVideoConCallB(hwndDlg, uMsg, wParam, lParam);
}
static bool bVideoConCallB = Mhook_SetHook((PVOID*)&sysVideoConCallB, myVideoConCallB);
*/
#include "taseditor/taseditor_config.h"
TASEDITOR_CONFIG::TASEDITOR_CONFIG() {}
TASEDITOR_CONFIG taseditorConfig;
class GREENZONE {};
GREENZONE greenzone;
#include "history.h"
HISTORY history;
#include "taseditor/taseditor_window.h"
void TASEDITOR_WINDOW::changeBookmarksListHeight(int) {}
void TASEDITOR_WINDOW::updatePatternsMenu() {}
void TASEDITOR_WINDOW::updateCaption() {}
void TASEDITOR_WINDOW::updateTooltips() {}
TASEDITOR_WINDOW::TASEDITOR_WINDOW(void) {}
TASEDITOR_WINDOW taseditorWindow;
#include "taseditor_project.h"
TASEDITOR_PROJECT::TASEDITOR_PROJECT(void) {}
void TASEDITOR_PROJECT::setProjectChanged() {}
TASEDITOR_PROJECT project;
HISTORY::HISTORY(void) {}
class POPUP_DISPLAY {};
POPUP_DISPLAY popupDisplay;
class SPLICER {};
SPLICER splicer;
//class PLAYBACK {
//public:
//	void handleMiddleButtonClick(void) {}
//};
#include "playback.h"
PLAYBACK playback;
PLAYBACK::PLAYBACK(void) {}
void PLAYBACK::handleMiddleButtonClick(void) {}
class RECORDER {};
RECORDER recorder;
class SELECTION {};
SELECTION selection;
class PIANO_ROLL {};
PIANO_ROLL pianoRoll;
class MARKERS_MANAGER {};
MARKERS_MANAGER markersManager;
class BRANCHES {};
BRANCHES branches;
class BOOKMARKS {};
BOOKMARKS bookmarks;
class EDITOR {};
EDITOR editor;

#ifdef _USE_LUA
class SNAPSHOT;
#include "taseditor_lua.h"
void TASEDITOR_LUA::enableRunFunction(char const*) { }
void TASEDITOR_LUA::disableRunFunction(void) { }
bool TASEDITOR_LUA::engaged(void) { return 0; }
bool TASEDITOR_LUA::markedframe(int) { return 0; }
int TASEDITOR_LUA::getmarker(int) { return 0; }
int TASEDITOR_LUA::setmarker(int) { return 0; }
void TASEDITOR_LUA::removemarker(int) { }
char const* TASEDITOR_LUA::getnote(int) { return 0; }
void TASEDITOR_LUA::setnote(int, char const*) { }
int TASEDITOR_LUA::getcurrentbranch(void) { return 0; }
char const* TASEDITOR_LUA::getrecordermode(void) { return 0; }
int TASEDITOR_LUA::getsuperimpose(void) { return 0; }
int TASEDITOR_LUA::getlostplayback(void) { return 0; }
int TASEDITOR_LUA::getplaybacktarget(void) { return 0; }
void TASEDITOR_LUA::setplayback(int) { }
void TASEDITOR_LUA::stopseeking(void) { }
void TASEDITOR_LUA::getselection(class std::vector<int, class std::allocator<int> >&) { }
void TASEDITOR_LUA::setselection(class std::vector<int, class std::allocator<int> >&) { }
int TASEDITOR_LUA::getinput(int, int) { return 0; }
void TASEDITOR_LUA::submitinputchange(int, int, int) { }
void TASEDITOR_LUA::submitinsertframes(int, int) { }
void TASEDITOR_LUA::submitdeleteframes(int, int) { }
int TASEDITOR_LUA::applyinputchanges(char const*) { return 0; }
void TASEDITOR_LUA::clearinputchanges(void) { }
TASEDITOR_LUA taseditor_lua;
TASEDITOR_LUA::TASEDITOR_LUA(void) {}
#endif
#endif

#ifndef _USE_LUA
#include "fceulua.h"

int __cdecl FCEU_LoadLuaCode(char const*, char const*) { return 0; }
void __cdecl FCEU_LuaStop(void){}
int __cdecl FCEU_LuaFrameskip(void) { return 0; }
struct HWND__* LuaConsoleHWnd;
int __cdecl FCEU_LuaRunning(void) { return 0; }
int __stdcall DlgLuaScriptDialog(struct HWND__*, unsigned int, unsigned int, long) { return 0; }
void __cdecl UpdateLuaConsole(char const*) {}
void __cdecl CallRegisteredLuaFunctions(enum LuaCallID) {}
void __cdecl FCEU_LuaFrameBoundary(void) {}
int __cdecl FCEU_LuaRerecordCountSkip(void) { return 0; }
void __cdecl CallRegisteredLuaMemHook(unsigned int, int, unsigned int, enum LuaMemHookType) {}
void __cdecl FCEU_LuaReadZapper(unsigned int const*, unsigned int*) {}
void __cdecl FCEU_ReloadLuaCode(void) {}
uint8 FCEU_LuaReadJoypad(int which, uint8 joyl) { return joyl; }
void __cdecl FCEU_LuaUpdatePalette(void) {}
void __cdecl CallRegisteredLuaSaveFunctions(int, struct LuaSaveData&) {}
void __cdecl CallRegisteredLuaLoadFunctions(int, struct LuaSaveData const&) {}
void __cdecl FCEU_LuaGui(unsigned char*) {}
void __thiscall LuaSaveData::ClearRecords(void){}
void __thiscall LuaSaveData::ExportRecords(void*)const {}
void __thiscall LuaSaveData::ImportRecords(void*) {}
#endif
