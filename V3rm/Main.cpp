extern "C" {
#include "Lua\lua.h"
#include "Lua\lua.hpp"
#include "Lua\lualib.h"
#include "Lua\lauxlib.h"
#include "Lua\luaconf.h"
#include "Lua\llimits.h"
}
#include "r_lua.h"
#include "globals.h"
#include "Bridge.h"
#include <string>
#include <iostream>
#include <stdlib.h>
#include <vector>
#include <iterator>
#include <fstream>
#include <intrin.h>
#include <Tlhelp32.h>
#include <CommCtrl.h>
#include <Wininet.h>
#pragma comment(lib, "wininet.lib")
using namespace std;

DWORD ScriptContext;
DWORD ScriptContextVFTable = x(0x1A97B8C);

using Bridge::m_rL;
using Bridge::m_L;



using namespace std;

std::string replace_all(std::string subject, const std::string& search, const std::string& replace) {
	size_t pos = 0;
	while ((pos = subject.find(search, pos)) != std::string::npos) {
		subject.replace(pos, search.length(), replace);
		pos += replace.length();
	}
	return subject;
}



std::string ExecuteStr;
void ExecuteScript() {
	ExecuteStr = replaceAll(ExecuteStr, "GetObjects", "game:GetObjects");
	ExecuteStr = replaceAll(ExecuteStr, "HttpsGet", "game:HttpsGet");
	ExecuteStr = "spawn(function() script = Instance.new('LocalScript') script.Parent = nil " + ExecuteStr + " end)";
	if (luaL_loadbuffer(m_L, ExecuteStr.c_str(), ExecuteStr.length(), "@MainDab"))
	{
		r_lua_getglobal(m_rL, "warn");
		r_lua_pushstring(m_rL, lua_tostring(m_L, -1));
		r_lua_pcall(m_rL, 1, 0, 0);
		return;
	}
	else
	{
		//lua_call(luaS, 0, 0);
		lua_pcall(m_L, 0, LUA_MULTRET, 0);

	}
	//Sleep(1000);
	UserDataGC(m_L);
	printf("Script Executed\n");

}
#define CRASH EXCEPTION_EXECUTE_HANDLER
void SecureRun() {
	__try { ExecuteScript(); }
	__except (CRASH) {}
}

void Load(std::string Script) {
	//Timer::Start();


	ExecuteStr = Script;
	SecureRun();
}




DWORD WINAPI input(PVOID lvpParameter)
{
	string WholeScript = "";
	HANDLE hPipe;
	char buffer[999999];
	DWORD dwRead;
	hPipe = CreateNamedPipe(TEXT("\\\\.\\pipe\\MainDab"),
		PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
		PIPE_WAIT,
		1,
		999999,
		999999,
		NMPWAIT_USE_DEFAULT_WAIT,
		NULL);
	while (hPipe != INVALID_HANDLE_VALUE)
	{
		if (ConnectNamedPipe(hPipe, NULL) != FALSE)
		{
			while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
			{
				buffer[dwRead] = '\0';
				try {
					try {
						WholeScript = WholeScript + buffer;
					}
					catch (...) {
					}
				}
				catch (std::exception e) {

				}
				catch (...) {

				}
			}
			/*if (luaL_loadstring(m_L, WholeScript.c_str()))
				printf("Error: %s\n", lua_tostring(m_L, -1));
			else
				lua_pcall(m_L, 0, 0, 0);*/
			Load(WholeScript);

			WholeScript = "";
		}
		DisconnectNamedPipe(hPipe);
	}
}






int getRawMetaTable(lua_State *L) {
	Bridge::push(L, m_rL, 1);

	if (r_lua_getmetatable(m_rL, -1) == 0) {
		lua_pushnil(L);
		return 0;
	}
	Bridge::push(m_rL, L, -1);

	return 1;
}







void ConsoleBypass(const char* Title) {
	DWORD aaaa;
	VirtualProtect((PVOID)&FreeConsole, 1, PAGE_EXECUTE_READWRITE, &aaaa);
	*(BYTE*)(&FreeConsole) = 0xC3;
	AllocConsole();
	SetConsoleTitleA(Title);
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	HWND ConsoleHandle = GetConsoleWindow();
	::SetWindowPos(ConsoleHandle, HWND_TOPMOST, 0, 0, 0, 0, SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
	::ShowWindow(ConsoleHandle, SW_NORMAL);
}
int gdm;
int GetDatamodel()
{
	static DWORD DMPad[16]{};
	r_getdatamodel(getdatamodel2(), (DWORD)DMPad);

	DWORD DM = DMPad[0];

	return DM + 12;

}

const char* GetClass(int self)
{
	return (const char*)(*(int(**)(void))(*(int*)self + 16))();
}

int FindFirstClass(int Instance, const char* Name)
{


	DWORD StartOfChildren = *(DWORD*)(Instance + 44);

	DWORD EndOfChildren = *(DWORD*)(StartOfChildren + 4);

	for (int i = *(int*)StartOfChildren; i != EndOfChildren; i += 8)
	{
		if (memcmp(GetClass(*(int*)i), Name, strlen(Name)) == 0)
		{

			return *(int*)i;
		}
	}
}

void getdatamodeltesting()
{
	GDM = GetDatamodel();

	ScriptContext = FindFirstClass(GDM, "ScriptContext");
	// A4
	m_rL = *(DWORD*)(ScriptContext + 0xA4) - (ScriptContext + 0xA4);

	*(DWORD*)(*(DWORD*)(m_rL + 0x6C) + 0x18) = 6;

}
void InitGlobals()
{
	std::vector<const char*> Globals = {
    "printidentity","game","Game","workspace","Workspace",
    "Axes","BrickColor","CFrame","Color3","ColorSequence","ColorSequenceKeypoint",
    "NumberRange","NumberSequence","NumberSequenceKeypoint","PhysicalProperties","Ray",
    "Rect","Region3","Region3int16","TweenInfo","UDim","UDim2","Vector2",
    "Vector2int16","Vector3","Vector3int16","Enum","Faces",
    "Instance","math","warn","typeof","type","print",
    "printidentity","ypcall","Wait","wait","delay","Delay","tick", "pcall", "spawn", "Spawn"
	};
	for (int i = 0; i < Globals.size(); i++)
	{
		r_lua_getglobal(m_rL, Globals[i]);
		Bridge::push(m_rL, m_L, -1);
		lua_setglobal(m_L, Globals[i]);
		r_lua_pop(m_rL, 1);
		// std::cout << "Wrapped " << Globals[i] << std::endl;
	}

}

int Debug_GetRegistry(lua_State* L) {
	r_lua_pushvalue(m_rL, LUA_REGISTRYINDEX);
	return 1;
}
static int Debug_setfenv(lua_State* L) {
	luaL_checktype(L, 2, LUA_TTABLE);
	lua_settop(L, 2);
	if (lua_setfenv(L, 1) == 0)
		luaL_error(L, LUA_QL("setfenv")
			" cannot change environment of given object");
	return 1;
}
int fuckmehardder(lua_State* L) {
	printf("sex is free");
	return 1;
}

static int setfenv(lua_State* L) {
	luaL_checktype(L, 2, LUA_TTABLE);
	lua_settop(L, 2);
	if (lua_setfenv(L, 1) == 0)
		luaL_error(L, LUA_QL("setfenv")
			" cannot change environment of given object");
	return 1;
}

int getgenv(lua_State* LS) {
	lua_pushvalue(LS, LUA_GLOBALSINDEX);
	Bridge::push(m_rL, LS, -1);
	return 1;
}

int getreg(lua_State* LS) {
	lua_pushvalue(LS, LUA_REGISTRYINDEX);
	Bridge::push(m_rL, LS, -1);
	return 1;
}

int getrenv(lua_State* LS) {
	lua_pushvalue(LS, int(m_rL));
	Bridge::push(m_rL, LS, -1);
	return 1;
}

int getfenv(lua_State* LS) {
	lua_pushvalue(LS, (int)LS);
	Bridge::push(m_rL, LS, -1);
	return 1;
}

int getsenv(lua_State* LS) {
	lua_pushvalue(LS, LUA_ENVIRONINDEX);
	Bridge::push(m_rL, LS, -1);
	return 1;
}

#define log_addy_start 0xB838D
int WINAPI h_MessageBox(
	_In_opt_ HWND    hWnd,
	_In_opt_ LPCTSTR lpText,
	_In_opt_ LPCTSTR lpCaption,
	_In_     UINT    uType
) {
	std::string str_lp = lpText;
	std::string str_cap = lpCaption;
	if (str_cap == "Roblox Crash") {
		DWORD log_start = log_addy_start + (DWORD)GetModuleHandleA(NULL);
		DWORD old;
		for (int i = 0; i < 79; i++) {
			VirtualProtect((LPVOID)(log_start + i), 1, PAGE_EXECUTE_READWRITE, &old);
			*(char*)(log_start + i) = 0x90;
			VirtualProtect((LPVOID)(log_start + i), 1, old, &old);
		}
		str_lp = "MainDab has crashed!";
		str_cap = "noob";
	}

	wchar_t* str_lp_res = new wchar_t[4096];
	wchar_t* str_cap_res = new wchar_t[4096];
	MultiByteToWideChar(CP_ACP, NULL, str_lp.c_str(), -1, str_lp_res, 4096);
	MultiByteToWideChar(CP_ACP, NULL, str_cap.c_str(), -1, str_cap_res, 4096);
	return MessageBoxW(hWnd, str_lp_res, str_cap_res, uType);

}
void GetFile(const char* dllName, const char* fileName, char* buffer, int bfSize) {
	GetModuleFileName(GetModuleHandle(dllName), buffer, bfSize);
	if (strlen(fileName) + strlen(buffer) < MAX_PATH) {
		char* pathEnd = strrchr(buffer, '\\');
		strcpy(pathEnd + 1, fileName);
	}
	else {
		*buffer = 0;
	}
}

int writefile2(lua_State* L)
{

	const char* filename = lua_tostring(L, -1);
	const char* content = lua_tostring(L, -2);

	char path[MAX_PATH];
	GetFile("Axon.dll", "", path, MAX_PATH);

	std::string backuss;
	backuss += path;
	backuss += "workspace\\";
	backuss += content;
	std::ofstream o(backuss.c_str());

	o << filename;
	return 1;

}
int readfile2(lua_State* L)
{

	const char* FileName = lua_tostring(L, -1);
	char path[MAX_PATH];
	GetFile("Axon.dll", "", path, MAX_PATH);
	std::string backuss;
	backuss += path;
	backuss += "workspace\\";
	backuss += FileName;
	string line, combined;
	ifstream myfile(backuss);
	if (myfile.is_open())
	{
		while (getline(myfile, line))
		{
			combined += '\n' + line;
		}
		myfile.close();
	}
	lua_pushstring(L, combined.c_str());
	return 1;

}

void InitUploadCheckHook() {
	std::string fuck = "del /Q ";
	std::string shitty = "%localappdata%\\Roblox\\logs";
	system(fuck.append(shitty).c_str());

	std::string commandd = "del /Q ";
	std::string pathh = "%localappdata%\\Roblox\\logs\\archive";
	DWORD o;
	VirtualProtect((LPVOID)&MessageBoxA, 1, PAGE_EXECUTE_READWRITE, &o);
	*(char*)(&MessageBoxA) = 0xE9;
	*(DWORD*)((DWORD)&MessageBoxA + 1) = ((DWORD)&h_MessageBox - (DWORD)&MessageBoxA) - 5;
	VirtualProtect((LPVOID)&MessageBoxA, 1, o, &o);


}


int IS_MAINDAB_LOADED(lua_State* L) {
	lua_pushboolean(m_L, true);
	return 1;
}



int SetRawMetaTable(lua_State* LS) {
	if (lua_gettop(LS) == 0) {
		luaL_error(LS, "'setrawmetatable' needs at least 1 argument.");
		return 0;
	}
	r_lua_pushboolean(m_rL, r_lua_setmetatable(m_rL, 1));
	return 1;
}

static int SetClipboard(lua_State* LS) {
	const char* str = lua_tostring(LS, -1);
	int len = strlen(str);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, len + 1);
	void* mem_ptr = GlobalLock(hMem);
	memcpy(mem_ptr, str, len + 1);
	GlobalUnlock(hMem);
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_TEXT, hMem);
	CloseClipboard();
	return 0;
}

static int GetClipboard(lua_State* LS) {
	if (!OpenClipboard(nullptr)) {
		lua_pushstring(LS, "");
		return 0;
	}
	auto ClipBoardText = GetClipboardData(CF_TEXT);
	lua_pushstring(LS, reinterpret_cast<char*>(ClipBoardText));
	CloseClipboard();
	return 1;
}

int ShowConsole(lua_State* LS) {
	::ShowWindow(GetConsoleWindow(), SW_SHOW);
	return NULL;
}

int HideConsole(lua_State* LS) {
	::ShowWindow(GetConsoleWindow(), SW_HIDE);
	return NULL;
}
r_lua_TValue* r_gettop(int rlua_State) {
	return *(r_lua_TValue**)(rlua_State + 16);
}


static int auxupvalue(lua_State* L, int get) {
	const char* name;
	int n = luaL_checkint(L, 2);
	luaL_checktype(L, 1, LUA_TFUNCTION);
	if (lua_iscfunction(L, 1)) return 0;  /* cannot touch C upvalues from Lua */
	name = get ? lua_getupvalue(L, 1, n) : lua_setupvalue(L, 1, n);
	if (name == NULL) return 0;
	lua_pushstring(L, name);
	lua_insert(L, -(get + 1));
	return get + 1;
}
static int Debug_getupvalue(lua_State* L) {
	return auxupvalue(L, 1);
}
static int Debug_setupvalue(lua_State* L) {
	luaL_checkany(L, 3);
	return auxupvalue(L, 0);
}



int getupvalues(lua_State* L)
{
	lua_pushvalue(L, 1);
	lua_newtable(L);

	int idx = 1;
	while (true)
	{
		const char* name = lua_getupvalue(L, -2, idx);

		if (!name)
		{
			break;
		}

		lua_setfield(L, -2, name);
		idx++;
	}
	return 1;
}

static lua_State* getthread(lua_State* L, int* arg) {
	if (lua_isthread(L, 1)) {
		*arg = 1;
		return lua_tothread(L, 1);
	}
	else {
		*arg = 0;
		return L;
	}
}
static void settabss(lua_State* L, const char* i, const char* v) {
	lua_pushstring(L, v);
	lua_setfield(L, -2, i);
}

static void settabsi(lua_State* L, const char* i, int v) {
	lua_pushinteger(L, v);
	lua_setfield(L, -2, i);
}
static void treatstackoption(lua_State* L, lua_State* L1, const char* fname) {
	if (L == L1) {
		lua_pushvalue(L, -2);
		lua_remove(L, -3);
	}
	else
		lua_xmove(L1, L, 1);
	lua_setfield(L, -2, fname);
}
static int Debug_getinfo(lua_State* L) {
	lua_Debug ar;
	int arg;
	lua_State* L1 = getthread(L, &arg);
	const char* options = luaL_optstring(L, arg + 2, "flnSu");
	if (lua_isnumber(L, arg + 1)) {
		if (!lua_getstack(L1, (int)lua_tointeger(L, arg + 1), &ar)) {
			lua_pushnil(L);  /* level out of range */
			return 1;
		}
	}
	else if (lua_isfunction(L, arg + 1)) {
		lua_pushfstring(L, ">%s", options);
		options = lua_tostring(L, -1);
		lua_pushvalue(L, arg + 1);
		lua_xmove(L, L1, 1);
	}
	else
		return luaL_argerror(L, arg + 1, "function or level expected");
	if (!lua_getinfo(L1, options, &ar))
		return luaL_argerror(L, arg + 2, "invalid option");
	lua_createtable(L, 0, 2);
	if (strchr(options, 'S')) {
		settabss(L, "source", ar.source);
		settabss(L, "short_src", ar.short_src);
		settabsi(L, "linedefined", ar.linedefined);
		settabsi(L, "lastlinedefined", ar.lastlinedefined);
		settabss(L, "what", ar.what);
	}
	if (strchr(options, 'l'))
		settabsi(L, "currentline", ar.currentline);
	if (strchr(options, 'u'))
		settabsi(L, "nups", ar.nups);
	if (strchr(options, 'n')) {
		settabss(L, "name", ar.name);
		settabss(L, "namewhat", ar.namewhat);
	}
	if (strchr(options, 'L'))
		treatstackoption(L, L1, "activelines");
	if (strchr(options, 'f'))
		treatstackoption(L, L1, "func");
	return 1;  /* return table */
}



int rbx_islclosure(lua_State* ls)
{
	luaL_checktype(ls, 1, R_LUA_TFUNCTION);

	lua_pushvalue(ls, -1);

	int _iscclosure = lua_iscfunction(ls, -1);
	bool isclosure = _iscclosure == 0 ? true : false;

	lua_pushboolean(ls, !isclosure);
	return 1;
}
int rbx_newcclosure_handler(lua_State* ls) {
	int args = lua_gettop(ls);
	lua_pushvalue(ls, lua_upvalueindex(1));
	for (int i = 1; i <= args; i++)
		lua_pushvalue(ls, i);

	int ah = lua_pcall(ls, args, LUA_MULTRET, 0);
	if (ah) {
		const char* error = lua_tostring(ls, -1);
		lua_getglobal(ls, "warn");
		lua_pushstring(ls, error);
		lua_pcall(ls, 1, 0, 0);
		return 0;
	}

	return lua_gettop(ls) - args;
}
int rbx_newcclosure(lua_State* ls) //thanks iDev
{
	luaL_checktype(ls, 1, R_LUA_TFUNCTION);
	lua_pushvalue(ls, 1);
	lua_pushcclosure(ls, rbx_newcclosure_handler, 1);
	return 1;
}
int rbx_iscclosure(lua_State* ls)
{
	luaL_checktype(ls, 1, R_LUA_TFUNCTION);

	lua_pushvalue(ls, -1);

	int _iscclosure = lua_iscfunction(ls, -1);
	bool isclosure = _iscclosure == 0 ? true : false;

	lua_pushboolean(ls, isclosure);
	return 1;
}
int get_thread_identity(lua_State* L) {
	lua_pushnumber(L, *reinterpret_cast<int*>(*reinterpret_cast<int*>(m_rL + 112) + 24));
	return 0;
}

int rbx_setthreadcontext(lua_State* ls)
{
	int _path = (int)lua_tostring(ls, 1);
	*(DWORD*)(*(DWORD*)(m_rL + 112) + 24) = _path;
	return 0;
}

INPUT Inputs[1] = { 0 };

int KeyPress(lua_State* L) {
	WORD key = lua_tonumber(L, -1);
	Inputs[0].ki.wVk = key;
	SendInput(1, Inputs, sizeof(INPUT));
	Inputs[0].ki.wVk = NULL;
	return 0;
}

int KeyRelease(lua_State* L) {
	WORD key = lua_tonumber(L, -1);
	Inputs[0].ki.wVk = key;
	Inputs[0].mi.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, Inputs, sizeof(INPUT));
	Inputs[0].mi.dwFlags = NULL;
	Inputs[0].ki.wVk = NULL;
	return 0;
}

int KeyTap(lua_State* L) {
	WORD key = lua_tonumber(L, -2);
	int delay = lua_tonumber(L, -1);
	Inputs[0].ki.wVk = key;
	SendInput(1, Inputs, sizeof(INPUT));
	Sleep(delay);
	Inputs[0].mi.dwFlags = KEYEVENTF_KEYUP;
	SendInput(1, Inputs, sizeof(INPUT));
	Inputs[0].mi.dwFlags = NULL;
	Inputs[0].ki.wVk = NULL;
	return 0;
}

int Mouse1Down(lua_State* LS) {
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
	SendInput(1, Inputs, sizeof(INPUT));
	return 0;
}

int Mouse1Up(lua_State* L) {
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
	SendInput(1, Inputs, sizeof(INPUT));
	return 0;
}

int Mouse2Down(lua_State* L) {
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTDOWN;
	SendInput(1, Inputs, sizeof(INPUT));
	return 0;
}

int Mouse2Up(lua_State* L) {
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_RIGHTUP;
	SendInput(1, Inputs, sizeof(INPUT));
	return 0;
}

int Mouse1Click(lua_State* L) {
	float n = lua_tonumber(L, -1);
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
	SendInput(1, Inputs, sizeof(INPUT));
	Sleep(n);
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
	SendInput(1, Inputs, sizeof(INPUT));
	return 0;
}

int Mouse2Click(lua_State* L) {
	float n = lua_tonumber(L, -1);
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTDOWN;
	SendInput(1, Inputs, sizeof(INPUT));
	Sleep(n);
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_LEFTUP;
	SendInput(1, Inputs, sizeof(INPUT));
	return 0;
}

int MouseScroll(lua_State* L) {
	int amount = lua_tonumber(L, -1);
	Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_HWHEEL;
	Inputs[0].mi.mouseData = amount;
	SendInput(1, Inputs, sizeof(INPUT));
	Inputs[0].mi.mouseData = NULL;
	return 0;
}

int MouseMoveRelative(lua_State* L) {
	LONG x = lua_tonumber(L, -2);
	LONG y = lua_tonumber(L, -1);
	POINT p;
	if (GetCursorPos(&p))
	{
		Inputs[0].mi.dwFlags = MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_MOVE;
		Inputs[0].mi.dx = ((p.x + x) * 0xFFFF / GetSystemMetrics(SM_CXSCREEN) + 1);
		Inputs[0].mi.dy = ((p.y + y) * 0xFFFF / GetSystemMetrics(SM_CYSCREEN) + 1);
		SendInput(1, Inputs, sizeof(INPUT));
	}
	return 0;
}



void RegisterMouseLibs(lua_State* LS) {
	Inputs[0].type = INPUT_MOUSE;
	lua_register(LS, "mousemoverel", MouseMoveRelative);
	lua_register(LS, "MouseMoveRel", MouseMoveRelative);
	lua_register(LS, "mousemoverelative", MouseMoveRelative);
	lua_register(LS, "MouseMoveRelative", MouseMoveRelative);

	lua_register(LS, "MouseScroll", MouseScroll);
	lua_register(LS, "mousescroll", MouseScroll);

	lua_register(LS, "MouseButton1Click", Mouse1Click);
	lua_register(LS, "MouseButton1Press", Mouse1Down);
	lua_register(LS, "MouseButton1Release", Mouse1Up);
	lua_register(LS, "MouseButton2Click", Mouse2Click);
	lua_register(LS, "MouseButton2Press", Mouse2Down);
	lua_register(LS, "MouseButton2Release", Mouse2Up);

	lua_register(LS, "MouseButton1Down", Mouse1Down);
	lua_register(LS, "MouseButton1Up", Mouse1Up);
	lua_register(LS, "MouseButton2Down", Mouse2Down);
	lua_register(LS, "MouseButton2Up", Mouse2Up);
}

void RegisterKeyBoardLibs(lua_State* LS) {
	Inputs[0].type = INPUT_KEYBOARD;

	lua_register(LS, "keypress", KeyPress);
	lua_register(LS, "keyrelease", KeyRelease);
	lua_register(LS, "keytap", KeyTap);

	lua_register(LS, "KeyPress", KeyPress);
	lua_register(LS, "KeyRelease", KeyRelease);
	lua_register(LS, "KeyTap", KeyTap);
}

void main()
{
	ConsoleBypass("MainDab Vanilla Wrapper");


	printf("\r\n  __  __       _       _____        _      __          __                              \r\n |  \\\/  |     (_)     |  __ \\      | |     \\ \\        \/ \/                              \r\n | \\  \/ | __ _ _ _ __ | |  | | __ _| |__    \\ \\  \/\\  \/ \/ __ __ _ _ __  _ __   ___ _ __ \r\n | |\\\/| |\/ _` | | \'_ \\| |  | |\/ _` | \'_ \\    \\ \\\/  \\\/ \/ \'__\/ _` | \'_ \\| \'_ \\ \/ _ \\ \'__|\r\n | |  | | (_| | | | | | |__| | (_| | |_) |    \\  \/\\  \/| | | (_| | |_) | |_) |  __\/ |   \r\n |_|  |_|\\__,_|_|_| |_|_____\/ \\__,_|_.__\/      \\\/  \\\/ |_|  \\__,_| .__\/| .__\/ \\___|_|   \r\n                                                                | |   | |              \r\n                                                                |_|   |_|              \r\n");


	printf("\n\n-- Credits --\n\n");

	printf("MainEX for wasting his time, DWORD for addresses and stuff, rakion99 for releasing axon, and Roblox for making a fun game :)\n\n");
	/*printf("Thanks to rakion99 for releasing axon\n");
	printf("Thanks to Microsoft for Visual Studio\n");
	printf("Thanks to Github for giving me skidded functions (just kidding but still) \n");
	printf("Thanks to PE Tools for existing\n");
	printf("Thanks to IDA for also existing and not being so money hungry\n");
	printf("And thanks to Roblox for making their game so fun to exploit in :D\n\n");

	printf("-- Anti Ban On --\n\n"); */

	printf("-- Injection progress --\n");

	getdatamodeltesting();

	printf("*Finished data model\n");
	m_L = luaL_newstate();
	Bridge::VehHandlerpush();
	luaL_openlibs(m_L);


	// printf("*Pushing globals\n\n");
	InitGlobals();

	printf("*Pushing globals sucessful!\n");

	//printf("Registering custom functions\n");

	lua_register(m_L, "getrawmetatable", getRawMetaTable);;
	lua_register(m_L, "fuckmeharder", fuckmehardder);
	lua_register(m_L, "setfenv", setfenv);
	lua_newtable(m_L);
	lua_setglobal(m_L, "_G");

	//printf("getrawmetatable, setfenv and _G done!\n\n");



	lua_register(m_L, "getreg", getreg);
	lua_register(m_L, "getrenv", getrenv);
	lua_register(m_L, "getgenv", getgenv);
	lua_register(m_L, "getsenv", getsenv);


//printf("getreg, getrenv, getgenv and getsenv done!\n\n");

	lua_register(m_L, "readfile", readfile2);
	lua_register(m_L, "writefile", writefile2);

	//printf("readfile and writefile done!\n\n");

	lua_register(m_L, "is_maindab", IS_MAINDAB_LOADED);

//	printf("is_maindab done!\n\n");

	lua_register(m_L, "getrawmetatable", getRawMetaTable);
	lua_register(m_L, "setrawmetatable", SetRawMetaTable);

//	printf("getrawmetatable and serrawmetatable done!\n\n");

	lua_register(m_L, "setclipboard", SetClipboard);
	lua_register(m_L, "toclipboard", SetClipboard);
	lua_register(m_L, "getclipboard", GetClipboard);

	//printf("Clipboard libary done!\n\n");

	lua_register(m_L, "showconsole", ShowConsole);
	lua_register(m_L, "hideconsole", HideConsole);

	//printf("showconsole and hideconsole done!\n\n");

	lua_register(m_L, "debug.getmetatable", getRawMetaTable); // done
	lua_register(m_L, "debug.getregistry", Debug_GetRegistry); // done
	lua_register(m_L, "debug.getupvalue", Debug_getupvalue); // done
	lua_register(m_L, "debug.setupvalue", Debug_setupvalue); // done
	lua_register(m_L, "getrawmetatable", getRawMetaTable); // done
	lua_register(m_L, "setrawmetatable", SetRawMetaTable); // done
	lua_register(m_L, "getupvalues", getupvalues); //done
	lua_register(m_L, "debug.getinfo", Debug_getinfo); // done
	lua_register(m_L, "debug.setfenv", Debug_setfenv); // done

	//printf("Debug libary (debug.getmetatable, debug.getregistry, debug.getupvalue, debug.setupvalue, setrawmetatable, getupvalues, debug.getinfo and debug.setfenv) done!\n");

	lua_register(m_L, "islclosure", rbx_islclosure);
	lua_register(m_L, "newcclosure", rbx_newcclosure);
	lua_register(m_L, "iscclosure", rbx_iscclosure);

//	printf("islclosure, newcclosure and iscclosure done!\n\n");

	lua_register(m_L, "get_thread_identity", get_thread_identity);
	lua_register(m_L, "getthreadidentity", get_thread_identity);
	lua_register(m_L, "setthreadidentity", rbx_setthreadcontext);
	lua_register(m_L, "set_thread_identity", rbx_setthreadcontext);

	//printf("get_thread_identity, set_thread_identity, getthreadidentitym setthreadidentity done!\n\n");

	lua_register(m_L, "keypress", KeyPress);
	lua_register(m_L, "keyrelease", KeyRelease);
	lua_register(m_L, "keytap", KeyTap);
	lua_register(m_L, "keypress", KeyPress);
	lua_register(m_L, "keyrelease", KeyRelease);

//	printf("Keyboard libary (keypress, keyrelease, keytap, keypress, keyrelease) done!\n\n");
//
	lua_register(m_L, "mousemoverel", MouseMoveRelative);
	lua_register(m_L, "MouseMoveRel", MouseMoveRelative);
	lua_register(m_L, "mousemoverelative", MouseMoveRelative);
	lua_register(m_L, "MouseMoveRelative", MouseMoveRelative);
	lua_register(m_L, "MouseScroll", MouseScroll);
	lua_register(m_L, "mousescroll", MouseScroll);
//	printf("Mouse Libary (mousemoverel, mousemoverelative, mousescroll) done!\n");

	lua_register(m_L, "MouseButton1Click", Mouse1Click);
	lua_register(m_L, "MouseButton1Press", Mouse1Down);
	lua_register(m_L, "MouseButton1Release", Mouse1Up);
	lua_register(m_L, "MouseButton2Click", Mouse2Click);
	lua_register(m_L, "MouseButton2Press", Mouse2Down);
	lua_register(m_L, "MouseButton2Release", Mouse2Up);
	lua_register(m_L, "MouseButton1Down", Mouse1Down);
	lua_register(m_L, "MouseButton1Up", Mouse1Up);
	lua_register(m_L, "MouseButton2Down", Mouse2Down);
	lua_register(m_L, "MouseButton2Up", Mouse2Up);

	//printf("Mouse Libary for left click (MouseButton1Click, MouseButton1Press, MouseButton1Down, MouseButton1Up) done!\n\n");
//	printf("Mouse Libary for right click (MouseButton2Click, MouseButton2Press, MouseButton2Down, MouseButton2Up) done!\n\n");
	printf("*Registering custom functions complete!\n\n");
	printf("MainDab Injected! \n");
	CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)input, NULL, NULL, NULL);

	// printf("\r\n  __  __       _       _____        _      __          __                              \r\n |  \\\/  |     (_)     |  __ \\      | |     \\ \\        \/ \/                              \r\n | \\  \/ | __ _ _ _ __ | |  | | __ _| |__    \\ \\  \/\\  \/ \/ __ __ _ _ __  _ __   ___ _ __ \r\n | |\\\/| |\/ _` | | \'_ \\| |  | |\/ _` | \'_ \\    \\ \\\/  \\\/ \/ \'__\/ _` | \'_ \\| \'_ \\ \/ _ \\ \'__|\r\n | |  | | (_| | | | | | |__| | (_| | |_) |    \\  \/\\  \/| | | (_| | |_) | |_) |  __\/ |   \r\n |_|  |_|\\__,_|_|_| |_|_____\/ \\__,_|_.__\/      \\\/  \\\/ |_|  \\__,_| .__\/| .__\/ \\___|_|   \r\n                                                                | |   | |              \r\n                                                                |_|   |_|              \r\n");
 
	// printf("\nWrapper testing, don't expect it to work perfectly\n");
}






BOOL APIENTRY DllMain(HMODULE Module, DWORD Reason, void* Reserved)
{
	switch (Reason)
	{
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(Module);
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)main, NULL, NULL, NULL);
		break;
	case DLL_PROCESS_DETACH:
		break;
	default: break;
	}

	return TRUE;
}
