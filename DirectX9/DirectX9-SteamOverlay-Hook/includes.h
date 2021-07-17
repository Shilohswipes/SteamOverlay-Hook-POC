#pragma once
#include <Windows.h>
#include <cstdint>
#include <Psapi.h>
#include <cstdio>
#include <iostream>
#include <d3d9.h>
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx9.h"
#include "ImGui/imgui_impl_win32.h"
#include "SteamOverlay.hpp"
typedef HRESULT(STDMETHODCALLTYPE* Present) (IDirect3DDevice9*, const RECT*, const RECT*, HWND, const RGNDATA*);
typedef LRESULT(CALLBACK* WNDPROC)(HWND, UINT, WPARAM, LPARAM);