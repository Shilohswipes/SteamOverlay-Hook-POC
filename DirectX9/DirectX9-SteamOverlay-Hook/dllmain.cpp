#include "includes.h"

Present oPresent = NULL;
WNDPROC oWndProc;
HWND hWnd = NULL;

/*Stolen from https://github.com/rdbo/ImGui-DirectX-9-Kiero-Hook*/
BOOL CALLBACK EnumWindowsCallback(HWND handle, LPARAM lParam) {
	DWORD wndProcId;
	GetWindowThreadProcessId(handle, &wndProcId);

	if (GetCurrentProcessId() != wndProcId)
		return TRUE;

	hWnd = handle;
	return FALSE;
}

HWND GetProcessWindow() {
	hWnd = NULL;
	EnumWindows(EnumWindowsCallback, NULL);
	return hWnd;
}

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool ImGuiInit = false;
HRESULT STDMETHODCALLTYPE hkPresent(IDirect3DDevice9* pDevice, const RECT* src, const RECT* dest, HWND hWnd, const RGNDATA* dirtyRegion) {
	if (!ImGuiInit) {
		hWnd = GetProcessWindow(); //gets the process window that we are injecting into
		if (hWnd != NULL) {
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			oWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			ImGui_ImplWin32_Init(hWnd);
			ImGui_ImplDX9_Init(pDevice);
			ImGuiInit = true;
		}
	}
	else {
		// creates frame so we can draw stuff
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//creates a little menu
		ImGui::Begin("ImGui Window");
		ImGui::End();

		//ends frame and renders stuff
		ImGui::EndFrame();
		ImGui::Render();
		ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
	}

	return oPresent(pDevice, src, dest, hWnd, dirtyRegion); // we have to return oPresent or it will not draw anything on the screen!
}

HRESULT STDMETHODCALLTYPE hkReset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* params) {
	ImGui_ImplDX9_InvalidateDeviceObjects();
	ImGui_ImplDX9_CreateDeviceObjects();
	return oReset(pDevice, params);
}

void STDMETHODCALLTYPE HookThread() {
	/*Sees if steam module is loaded or not*/
	if (SteamOverlay::GetSteamModule() == NULL) {
		MessageBoxA(GetForegroundWindow(), "\"GameOverlayRenderer.dll\" is not loaded!", "SteamOverlay Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	uintptr_t pPresentAddr = SteamOverlay::FindPattern("GameOverlayRenderer.dll", "FF 15 ? ? ? ? 8B F0 85 FF") + 2;
	uintptr_t pResetAddr = SteamOverlay::FindPattern("GameOverlayRenderer.dll", "C7 45 ? ? ? ? ? FF 15 ? ? ? ? 8B D8") + 8;
	oPresent = **reinterpret_cast<decltype(&oPresent)*>(pPresentAddr);
	oReset = **reinterpret_cast<decltype(&oReset)*>(pResetAddr);
	**reinterpret_cast<void***>(pPresentAddr) = reinterpret_cast<void*>(&hkPresent);
	**reinterpret_cast<void***>(pResetAddr) = reinterpret_cast<void*>(&hkReset);
}

BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) {
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HookThread, hModule, 0, 0);
		break;
	default:
		break;
	}
}
