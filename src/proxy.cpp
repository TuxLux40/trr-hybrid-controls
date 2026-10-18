// TRR Hybrid Controls: xinput1_3.dll proxy for Tomb Raider IV-VI Remastered (tomb456.exe).
//
// The host polls XInputGetState and keeps the control scheme in bit 1 of one byte (1 = Modern, 0 = Tank). Every poll
// this proxy looks at the pad and sets that bit: stick -> Modern, d-pad -> Tank (TRRHybridControls.ini can change either). The
// game's own two code paths do the rest. Active on the exe build
// below, so an unknown build is left alone.
//
// The XInput forwarding pattern is adapted from re4og-modern-controls (MIT).
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdarg.h>
#include <stdio.h>

struct XGamepad {
    WORD wButtons;
    BYTE bLeftTrigger, bRightTrigger;
    SHORT sThumbLX, sThumbLY, sThumbRX, sThumbRY;
};
struct XState {
    DWORD dwPacketNumber;
    XGamepad Gamepad;
};
enum : WORD { XB_DPAD_ALL = 0x000F };
static const int kTriggerThreshold = 30;
static const int kStickDeadzone = 7849;  // XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE

// tomb456.exe, Steam build of 2026-04-01 (found by diffing memory while toggling Control Type in the options menu).
static const uintptr_t kHostBase = 0x140000000;
static const DWORD kHostStamp = 0x68c12feb;
static const DWORD kHostImageSize = 0x0e9e4000;
static volatile char* const kOptions = (volatile char*)0x140585f64;  // bit 1: 1 = Modern, 0 = Tank
static const char kModernBit = 2;

// ---- log -------------------------------------------------------------------------------------------------------
static FILE* g_log;
static void Log(const char* fmt, ...) {
    if (!g_log) return;
    SYSTEMTIME t;
    GetLocalTime(&t);
    fprintf(g_log, "[%02d:%02d:%02d.%03d] ", t.wHour, t.wMinute, t.wSecond, t.wMilliseconds);
    va_list ap;
    va_start(ap, fmt);
    vfprintf(g_log, fmt, ap);
    va_end(ap);
    fputc('\n', g_log);
    fflush(g_log);
}

static void OpenLog(HINSTANCE inst) {
    char path[MAX_PATH];
    GetModuleFileNameA(inst, path, MAX_PATH);
    char* slash = strrchr(path, '\\');
    if (slash) slash[1] = 0;
    strcat(path, "TRRHybridControls.log");
    g_log = fopen(path, "w");
}

// ---- the mod ---------------------------------------------------------------------------------------------------
static bool g_active;  // host build verified
static int g_aimKeepsStick = 1;  // stable: aiming stays Modern; 0 = Tank aiming with the d-pad (beta option)
static int g_stickModern = 1, g_dpadModern = 0;  // movement scheme of the left stick / the d-pad (1 = Modern, 0 = Tank)

static void LoadConfig(HINSTANCE inst) {
    char path[MAX_PATH];
    GetModuleFileNameA(inst, path, MAX_PATH);
    char* slash = strrchr(path, '\\');
    if (slash) slash[1] = 0;
    strcat(path, "TRRHybridControls.ini");
    if (GetFileAttributesA(path) == INVALID_FILE_ATTRIBUTES) {
        WritePrivateProfileStringA("Movement", "Stick", "Modern", path);
        WritePrivateProfileStringA("Movement", "DPad", "Tank", path);
        WritePrivateProfileStringA("Movement", "AimKeepsStick", "1", path);
    }
    char v[16];
    GetPrivateProfileStringA("Movement", "Stick", "Modern", v, sizeof v, path);
    g_stickModern = lstrcmpiA(v, "Tank") != 0;
    GetPrivateProfileStringA("Movement", "DPad", "Tank", v, sizeof v, path);
    g_dpadModern = lstrcmpiA(v, "Modern") == 0;
    g_aimKeepsStick = GetPrivateProfileIntA("Movement", "AimKeepsStick", 1, path);
    Log("config: stick = %s, d-pad = %s", g_stickModern ? "Modern" : "Tank", g_dpadModern ? "Modern" : "Tank");
}

static void VerifyHost() {
    const BYTE* base = (const BYTE*)GetModuleHandleA(nullptr);
    if ((uintptr_t)base != kHostBase) {
        Log("inactive: exe is at %p, expected %p (not tomb456.exe?)", base, (void*)kHostBase);
        return;
    }
    const IMAGE_NT_HEADERS* nt = (const IMAGE_NT_HEADERS*)(base + ((const IMAGE_DOS_HEADER*)base)->e_lfanew);
    Log("exe timestamp %08lX, image size %08lX", nt->FileHeader.TimeDateStamp, nt->OptionalHeader.SizeOfImage);
    if (nt->FileHeader.TimeDateStamp != kHostStamp || nt->OptionalHeader.SizeOfImage != kHostImageSize) {
        Log("inactive: unknown exe build (expected timestamp %08lX, size %08lX)", kHostStamp, kHostImageSize);
        return;
    }
    g_active = true;
    Log("active: options byte at %p = %02X (%s)", kOptions, *kOptions & 0xFF, (*kOptions & kModernBit) ? "Modern" : "Tank");
}

// The bit also picks one of two button tables (15 rows of {action, input A, input B}, 180 bytes each). Rows 0-3 are
// the movement actions (Tank binds them to the d-pad); rows 4-14 are every other button. Those shared rows are kept
// identical in both tables and follow the menu's choice, so flipping the bit changes movement only.
static unsigned char* const kTables = (unsigned char*)0x140585f98;
static volatile char* const kMenuMirror = (volatile char*)0x1401a8b78;  // copy of the options byte the menu writes, not us
static const int kRowBytes = 12, kTableBytes = 15 * 12, kFirstShared = 4, kSharedBytes = (15 - 4) * 12;
static unsigned char* Shared(int table) { return kTables + table * kTableBytes + kFirstShared * kRowBytes; }

static void SyncButtonTables(int menu) {
    static bool init;
    static unsigned char pristine[2][kSharedBytes];  // each scheme's own button layout, as the player set it
    static unsigned char shadow[kSharedBytes];       // what both tables held after our last sync
    static int lastMenu = -1;
    if (!init) {
        memcpy(pristine[0], Shared(0), kSharedBytes);
        memcpy(pristine[1], Shared(1), kSharedBytes);
        init = true;
    }
    const bool d0 = memcmp(Shared(0), shadow, kSharedBytes) != 0, d1 = memcmp(Shared(1), shadow, kSharedBytes) != 0;
    if (menu == lastMenu && !d0 && !d1) return;
    if (menu == lastMenu && d0 && d1) {  // both changed at once: the game reloaded its settings
        memcpy(pristine[0], Shared(0), kSharedBytes);
        memcpy(pristine[1], Shared(1), kSharedBytes);
        Log("button tables reloaded by the game");
    } else if (menu == lastMenu) {  // one changed: the player edited a binding in Change Controls
        memcpy(pristine[menu], Shared(d0 ? 0 : 1), kSharedBytes);
        Log("button binding edited");
    }
    memcpy(shadow, pristine[menu], kSharedBytes);
    memcpy(Shared(0), shadow, kSharedBytes);
    memcpy(Shared(1), shadow, kSharedBytes);
    if (menu != lastMenu) Log("button layout follows the menu: %s", menu ? "Modern" : "Tank");
    lastMenu = menu;
}

// Called with every pad state the game asks for.
static void Apply(const XGamepad& g) {
    static int logged = 0;
#ifdef TRRHC_FOLLOW_MENU  // beta: purists keep the Tank buttons when the menu says Tank
    SyncButtonTables((*kMenuMirror & kModernBit) ? 1 : 0);
#else                     // stable: Modern buttons always, only the d-pad moves like Tank
    SyncButtonTables(1);
#endif

    const int cur = (*kOptions & kModernBit) ? 1 : 0;
    const int sx = g.sThumbLX < 0 ? -g.sThumbLX : g.sThumbLX, sy = g.sThumbLY < 0 ? -g.sThumbLY : g.sThumbLY;
    const bool stick = sx > kStickDeadzone || sy > kStickDeadzone, dpad = (g.wButtons & XB_DPAD_ALL) != 0;
    // Optional (AimKeepsStick): holding the left trigger keeps the game in the stick's scheme. Modern draws weapons while the
    // trigger is held, Tank toggles them with each press, and the game asks the bit for that everywhere (weapons, firing).
    const bool aiming = g_aimKeepsStick && g.bLeftTrigger > kTriggerThreshold;
    // Idle falls back to the stick's scheme, so everything that starts from standing (aiming, drawing weapons) begins in the
    // same state instead of depending on which input was used last.
    int want = g_stickModern;
    if (!stick && !aiming && dpad) want = g_dpadModern;
    if (cur != want) {
        if (want) __atomic_fetch_or(kOptions, kModernBit, __ATOMIC_SEQ_CST);
        else __atomic_fetch_and(kOptions, (char)~kModernBit, __ATOMIC_SEQ_CST);
    }
    // Input log (every change of stick / d-pad / trigger / scheme), to see what the game was told and when.
    static int prev = -1;
    const int sig = (stick ? 1 : 0) | (dpad ? 2 : 0) | (aiming ? 4 : 0) | (want << 3);
    if (sig != prev && logged < 4000) {
        prev = sig;
        ++logged;
        Log("stick %d dpad %d lt %3d%s -> %s%s", stick, dpad, g.bLeftTrigger, aiming ? " (aim)" : "", want ? "Modern" : "Tank",
            cur != want ? " [switched]" : "");
    }
}

// ---- XInput forwarding ---------------------------------------------------------------------------------------------
typedef DWORD(WINAPI* PFN_GetState)(DWORD, XState*);
typedef DWORD(WINAPI* PFN_SetState)(DWORD, void*);
typedef DWORD(WINAPI* PFN_GetCaps)(DWORD, DWORD, void*);
typedef void(WINAPI* PFN_Enable)(BOOL);
typedef DWORD(WINAPI* PFN_GetBattery)(DWORD, BYTE, void*);
typedef DWORD(WINAPI* PFN_GetKeystroke)(DWORD, DWORD, void*);
typedef DWORD(WINAPI* PFN_WaitGuide)(DWORD, DWORD, void*);
typedef DWORD(WINAPI* PFN_DwordOnly)(DWORD);

static struct {
    HMODULE mod;
    PFN_GetState getState, getStateEx;
    PFN_SetState setState;
    PFN_GetCaps getCaps;
    PFN_Enable enable;
    PFN_GetBattery getBattery;
    PFN_GetKeystroke getKeystroke;
    PFN_WaitGuide waitGuide;
    PFN_DwordOnly cancelGuide, powerOff;
} R;
static INIT_ONCE g_once = INIT_ONCE_STATIC_INIT;

static BOOL CALLBACK LoadReal(PINIT_ONCE, PVOID, PVOID*) {
    const char* candidates[] = {"XInput1_4.dll", "XInput9_1_0.dll"};
    char sys[MAX_PATH];
    GetSystemDirectoryA(sys, MAX_PATH);
    for (const char* name : candidates) {
        char path[MAX_PATH];
        wsprintfA(path, "%s\\%s", sys, name);
        if ((R.mod = LoadLibraryA(path))) {
            Log("forwarding to %s", path);
            break;
        }
    }
    if (!R.mod) {
        Log("no system XInput found");
        return TRUE;
    }
    R.getState = (PFN_GetState)GetProcAddress(R.mod, "XInputGetState");
    R.getStateEx = (PFN_GetState)GetProcAddress(R.mod, (LPCSTR)100);
    R.setState = (PFN_SetState)GetProcAddress(R.mod, "XInputSetState");
    R.getCaps = (PFN_GetCaps)GetProcAddress(R.mod, "XInputGetCapabilities");
    R.enable = (PFN_Enable)GetProcAddress(R.mod, "XInputEnable");
    R.getBattery = (PFN_GetBattery)GetProcAddress(R.mod, "XInputGetBatteryInformation");
    R.getKeystroke = (PFN_GetKeystroke)GetProcAddress(R.mod, "XInputGetKeystroke");
    R.waitGuide = (PFN_WaitGuide)GetProcAddress(R.mod, (LPCSTR)101);
    R.cancelGuide = (PFN_DwordOnly)GetProcAddress(R.mod, (LPCSTR)102);
    R.powerOff = (PFN_DwordOnly)GetProcAddress(R.mod, (LPCSTR)103);
    return TRUE;
}
static void EnsureReal() { InitOnceExecuteOnce(&g_once, LoadReal, nullptr, nullptr); }

static DWORD GetStateCommon(PFN_GetState fn, DWORD user, XState* st) {
    EnsureReal();
    DWORD rc = fn ? fn(user, st) : ERROR_DEVICE_NOT_CONNECTED;
    static LONG calls = 0;
    const LONG n = InterlockedIncrement(&calls);
    if (n == 1 || n == 100 || n % 3000 == 0) Log("GetState call #%ld user %lu rc %lu", n, user, rc);
    if (g_active && st && rc == ERROR_SUCCESS) Apply(st->Gamepad);
    return rc;
}

extern "C" {
DWORD WINAPI Proxy_XInputGetState(DWORD user, XState* st) { EnsureReal(); return GetStateCommon(R.getState, user, st); }
DWORD WINAPI Proxy_XInputGetStateEx(DWORD user, XState* st) {
    EnsureReal();
    return GetStateCommon(R.getStateEx ? R.getStateEx : R.getState, user, st);
}
DWORD WINAPI Proxy_XInputSetState(DWORD user, void* vib) {
    EnsureReal();
    return R.setState ? R.setState(user, vib) : ERROR_DEVICE_NOT_CONNECTED;
}
DWORD WINAPI Proxy_XInputGetCapabilities(DWORD user, DWORD flags, void* caps) {
    EnsureReal();
    return R.getCaps ? R.getCaps(user, flags, caps) : ERROR_DEVICE_NOT_CONNECTED;
}
void WINAPI Proxy_XInputEnable(BOOL enable) {
    EnsureReal();
    if (R.enable) R.enable(enable);
}
DWORD WINAPI Proxy_XInputGetDSoundAudioDeviceGuids(DWORD, GUID* render, GUID* capture) {
    if (render) ZeroMemory(render, sizeof(GUID));
    if (capture) ZeroMemory(capture, sizeof(GUID));
    return ERROR_SUCCESS;
}
DWORD WINAPI Proxy_XInputGetBatteryInformation(DWORD user, BYTE type, void* info) {
    EnsureReal();
    return R.getBattery ? R.getBattery(user, type, info) : ERROR_DEVICE_NOT_CONNECTED;
}
DWORD WINAPI Proxy_XInputGetKeystroke(DWORD user, DWORD reserved, void* ks) {
    EnsureReal();
    return R.getKeystroke ? R.getKeystroke(user, reserved, ks) : ERROR_EMPTY;
}
DWORD WINAPI Proxy_XInputWaitForGuideButton(DWORD user, DWORD flags, void* listen) {
    EnsureReal();
    return R.waitGuide ? R.waitGuide(user, flags, listen) : ERROR_DEVICE_NOT_CONNECTED;
}
DWORD WINAPI Proxy_XInputCancelGuideButtonWait(DWORD user) {
    EnsureReal();
    return R.cancelGuide ? R.cancelGuide(user) : ERROR_DEVICE_NOT_CONNECTED;
}
DWORD WINAPI Proxy_XInputPowerOffController(DWORD user) {
    EnsureReal();
    return R.powerOff ? R.powerOff(user) : ERROR_DEVICE_NOT_CONNECTED;
}
}  // extern "C"

BOOL WINAPI DllMain(HINSTANCE inst, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(inst);
        OpenLog(inst);
        LoadConfig(inst);
        Log("TRR Hybrid Controls %s", TRRHC_VERSION_STR);
        VerifyHost();
    }
    return TRUE;
}
