#include "includes.h"

HWND hWnd = NULL;
Present oPresent = NULL;
ID3D11Device* pDevice = NULL;
ID3D11DeviceContext* pContext = NULL;
ID3D11RenderTargetView* pRenderTargetView;
WNDPROC oWndProc;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool ImGuiInit = false;
HRESULT hkPresent(IDXGISwapChain* pSwapchain, UINT SyncInterval, UINT Flags) {
	if (!ImGuiInit) {
		if (SUCCEEDED(pSwapchain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice))) {
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapchain->GetDesc(&sd);
			hWnd = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
			pDevice->CreateRenderTargetView(pBackBuffer, NULL, &pRenderTargetView);
			pBackBuffer->Release();
			oWndProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)WndProc);
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
			ImGui_ImplWin32_Init(hWnd);
			ImGui_ImplDX11_Init(pDevice, pContext);
			ImGuiInit = true;
		}
		else return oPresent(pSwapchain, SyncInterval, Flags);
	}
	//creates a frame so we can start drawing
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	//begins a little menu
	ImGui::Begin("ImGui Window");
	ImGui::End();

	//renders the stuff we draw
	ImGui::Render();
	pContext->OMSetRenderTargets(1, &pRenderTargetView, NULL);
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	return oPresent(pSwapchain, SyncInterval, Flags);
}

void STDMETHODCALLTYPE HookThread() {
	//sees if steam module is loaded
	if (SteamOverlay::GetSteamModule() == NULL) {
		MessageBoxA(NULL, "\"GameOverlayRenderer64.dll\" is not loaded!", "SteamOverlay Error", MB_OK | MB_ICONERROR);
		exit(1);
	}

	uintptr_t pPresentAddr = SteamOverlay::FindPattern(SteamOverlay::GetSteamModule(), "48 89 6C 24 18 48 89 74 24 20 41 56 48 83 EC 20 41 8B E8");
	SteamOverlay::CreateHook(pPresentAddr, (uintptr_t)hkPresent, (long long*)&oPresent);
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) { 
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)HookThread, hModule, 0, 0);
		break;
	default:
		break;
	}
}