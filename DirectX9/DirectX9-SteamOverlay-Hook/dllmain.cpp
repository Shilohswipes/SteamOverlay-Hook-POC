#include "includes.h"

Present oPresent = nullptr;
Reset oReset = nullptr;
WNDPROC oWndProc;
HWND oWnd = nullptr;

/* Stolen from https://github.com/rdbo/ImGui-DirectX-9-Kiero-Hook */
BOOL CALLBACK EnumWindowsCallback(HWND hWnd, LPARAM lParam) 
{
	DWORD wndProcId;
	GetWindowThreadProcessId(hWnd, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE;

	oWnd = hWnd;
	return FALSE;
}

HWND GetProcessWindow() 
{
	oWnd = nullptr;
	EnumWindows(EnumWindowsCallback, NULL);
	return oWnd;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool bImGuiInit = false;
HRESULT STDMETHODCALLTYPE hkPresent(IDirect3DDevice9* pDevice, const RECT* src, const RECT* dest, HWND hWnd, const RGNDATA* dirtyRegion) 
{
	if (!bImGuiInit) 
	{
		hWnd = GetProcessWindow();
		if (hWnd != nullptr) 
		{
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
			ImGui_ImplWin32_Init(hWnd);
			ImGui_ImplDX9_Init(pDevice);
			bImGuiInit = true;
		}
	}
	else 
	{
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("ImGui Window");
		ImGui::End();

		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	return oPresent(pDevice, src, dest, hWnd, dirtyRegion);
}

HRESULT STDMETHODCALLTYPE hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* params) 
{
	ImGui_ImplDX9_InvalidateDeviceObjects();
	ImGui_ImplDX9_CreateDeviceObjects();
	return oReset(pDevice, params);
}

void __stdcall HookThread() {
	if (SteamOverlay::GetSteamModule() == NULL) 
	{
		MessageBoxA(GetForegroundWindow(), "\"GameOverlayRenderer.dll\" is not loaded!", "SteamOverlay Error", MB_OK | MB_ICONERROR);
		exit(EXIT_FAILURE);
	}

	uintptr_t pPresentAddr = SteamOverlay::FindPattern("GameOverlayRenderer.dll", "FF 15 ? ? ? ? 8B F0 85 FF") + 2;
	uintptr_t pResetAddr = SteamOverlay::FindPattern("GameOverlayRenderer.dll", "C7 45 ? ? ? ? ? FF 15 ? ? ? ? 8B D8") + 9;
	oPresent = **reinterpret_cast<decltype(&oPresent)*>(pPresentAddr);
	oReset = **reinterpret_cast<decltype(&oReset)*>(pResetAddr);
	**reinterpret_cast<void***>(pPresentAddr) = reinterpret_cast<void*>(&hkPresent);
	**reinterpret_cast<void***>(pResetAddr) = reinterpret_cast<void*>(&hkReset);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) 
{
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(HookThread), hModule, 0, 0);
		break;
	default:
		break;
	}
}
