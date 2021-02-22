#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <windows.h>
#include <iostream>
#include <time.h>
#include <map>
#include <mutex>
#include <TlHelp32.h>
#include <dwmapi.h>
#include <vector>
#include <direct.h>
#include <fstream>
#include <tchar.h>
#pragma comment (lib, "dwmapi.lib")

#include "Include/d3dx9.h"
#pragma comment (lib, "Lib/x86/d3dx9.lib")

#include "DXWF/DXWF.h"
#pragma comment (lib, "DXWF/DXWF.lib")

#include "imgui/imgui.h"
#include "imgui/imgui_internal.h"
#include "imgui/imgui_impl_dx9.h"
#include "imgui/imgui_impl_win32.h"

#include "console/console.h"
#include "process_enumerator/process_enumerator.h"
#include "scene/scene.h"

extern bool g_bIsMouseCaptured;