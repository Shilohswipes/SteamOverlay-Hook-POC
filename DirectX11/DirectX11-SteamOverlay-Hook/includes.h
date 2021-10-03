#pragma once
#include <Windows.h>
#include <d3d11.h>
#include "SteamOverlay.hpp"
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"
using Present = HRESULT(*)(IDXGISwapChain*, UINT, UINT);