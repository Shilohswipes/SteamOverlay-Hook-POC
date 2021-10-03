#include "includes.h"

HWND hWnd = nullptr;
Present oPresent = nullptr;
ID3D11Device* pDevice = nullptr;
ID3D11DeviceContext* pContext = nullptr;
ID3D11RenderTargetView* pRenderTargetView;
WNDPROC oWndProc;

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProc(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	if (true && ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}

bool bImGuiInit = false;
HRESULT hkPresent(IDXGISwapChain* pSwapchain, UINT SyncInterval, UINT Flags) 
{
	if (!bImGuiInit) 
	{
		if (SUCCEEDED(pSwapchain->GetDevice(__uuidof(ID3D11Device), reinterpret_cast<void**>(&pDevice)))) 
		{
			pDevice->GetImmediateContext(&pContext);
			DXGI_SWAP_CHAIN_DESC sd;
			pSwapchain->GetDesc(&sd);
			hWnd = sd.OutputWindow;
			ID3D11Texture2D* pBackBuffer;
			pSwapchain->GetBuffer(NULL, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer));
			pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
			pBackBuffer->Release();
			oWndProc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WndProc)));
			ImGui::CreateContext();
			ImGuiIO& io = ImGui::GetIO();
			io.ConfigFlags = ImGuiConfigFlags_NoMouseCursorChange;
			ImGui_ImplWin32_Init(hWnd);
			ImGui_ImplDX11_Init(pDevice, pContext);
			bImGuiInit = true;
		}
		else 
			return oPresent(pSwapchain, SyncInterval, Flags);
	}
	else
	{
		ImGui_ImplDX11_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		ImGui::Begin("ImGui Window");
		ImGui::End();

		ImGui::Render();
		pContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	}
	return oPresent(pSwapchain, SyncInterval, Flags);
}

void __stdcall HookThread() 
{
	if (SteamOverlay::GetSteamModule() == NULL) {
		MessageBoxA(nullptr, "\"GameOverlayRenderer64.dll\" is not loaded!", "SteamOverlay Error", MB_OK | MB_ICONERROR);
		exit(EXIT_FAILURE);
	}

	uintptr_t pPresentAddr = SteamOverlay::FindPattern("GameOverlayRenderer64.dll", "48 89 6C 24 18 48 89 74 24 20 41 56 48 83 EC 20 41 8B E8");
	SteamOverlay::CreateHook(pPresentAddr, reinterpret_cast<uintptr_t>(hkPresent), reinterpret_cast<long long*>(&oPresent));
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved) 
{ 
	switch (dwReason) {
	case DLL_PROCESS_ATTACH:
		DisableThreadLibraryCalls(hModule);
		CreateThread(nullptr, NULL, reinterpret_cast<LPTHREAD_START_ROUTINE>(HookThread), hModule, NULL, nullptr);
		break;
	default:
		break;
	}
}