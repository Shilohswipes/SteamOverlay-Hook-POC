#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <cstdint>

namespace SteamOverlay {
#define InRange(x, a, b) (x >= a && x <= b) 
#define GetBits(x) (InRange((x & (~0x20)), 'A', 'F') ? ((x & (~0x20)) - 'A' + 0xA): (InRange(x, '0', '9') ? x - '0': 0))
#define GetByte(x) (GetBits(x[0]) << 4 | GetBits(x[1]))

	uintptr_t FindPatternEx(const uintptr_t& pStartAddr, const uintptr_t& pEndAddr, const char* szTPattern) {
		const char* szPattern = szTPattern;
		uintptr_t pFirstMatch = NULL;

		for (uintptr_t pPosition = pStartAddr; pPosition < pEndAddr; pPosition++) {
			if (!*szPattern) return pFirstMatch;
			const uint8_t pCurrentPattern = *reinterpret_cast<const uint8_t*>(szPattern);
			const uint8_t pCurrentMemory = *reinterpret_cast<const uint8_t*>(pPosition);

			if (pCurrentPattern == '\?' || pCurrentMemory == GetByte(szPattern)) {
				if (!pFirstMatch) pFirstMatch = pPosition;
				if (!szPattern[2]) return pFirstMatch;
				szPattern += pCurrentPattern != '\?' ? 3 : 2;
			}
			else {
				szPattern = szTPattern;
				pFirstMatch = NULL;
			}
		}
		return NULL;
	}

	uintptr_t FindPattern(const char* szModule, const char* szPattern) {
		MODULEINFO hModInfo = { nullptr };
		if (!GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(szModule), &hModInfo, sizeof(hModInfo))) return NULL;
		const uintptr_t pStartAddr = (uintptr_t)hModInfo.lpBaseOfDll;
		const uintptr_t pEndAddr = pStartAddr + hModInfo.SizeOfImage;
		return FindPatternEx(pStartAddr, pEndAddr, szPattern);
	}

	uintptr_t GetSteamModule() {
		return (uintptr_t)GetModuleHandleA("GameOverlayRenderer.dll");
	}
}