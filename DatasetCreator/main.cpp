#pragma once
/*
	THIS FILE IS A PART OF GTA V SCRIPT HOOK SDK
				http://dev-c.com
			(C) Alexander Blade 2015
*/

#include "main.h"
#include "script.h"


//insert start

#include <dxgi.h>
#include <d3d11.h>
#include <d3d11_4.h>
//#include <d3d11_3.h>
#include <wrl.h>
#include <ShlObj.h>
#include <system_error>
#include <string>
#include <filesystem>
#include <wincodec.h>
#include <wingdi.h>
#include <cstdio>
#include <cassert>
#include <chrono>
#include <d3d11shader.h>
#include <queue>
#include <d3dcompiler.h>
#include <vector>
#include <memory>
//#include "scenario.h"
#include "keyboard.h"
#include <mutex>
#include <natives.h>
#include <gdiplus.h>
#include <cstdint>


#pragma comment (lib,"Gdiplus.lib")

using Microsoft::WRL::ComPtr;
using namespace std::experimental::filesystem;
using namespace std::string_literals;
using std::chrono::milliseconds;
using std::chrono::time_point;
using std::chrono::system_clock;
using std::vector;
typedef void(*draw_indexed_hook_t)(ID3D11DeviceContext*, UINT, UINT, INT);
// ScriptHookV Functions
void presentCallback(void* chain);

static time_point<system_clock> last_capture_color;
static time_point<system_clock> last_capture_depth;
static const char* logFilePath = "GTANativePlugin.log";
//--------
//offsets
//--------
const size_t drawIndexedOffset = 12;
const size_t clearDepthStencilViewOffset = 53;
//const size_t max_frames = 1000;
//-------------------------
//interesting D3D resources
//-------------------------
static ComPtr<ID3D11DepthStencilView> lastDsv;
static ComPtr<ID3D11RenderTargetView> lastRtv;
static ComPtr<ID3D11Buffer> lastConstants;

static bool saveNextFrame = false;
static bool hooked = false;

// added in order to be able to tell, if currently recording or not 
static bool recording = false;
static bool load_new = false;
static bool reset = false;
bool mapshot = false;
std::mutex mut;

static int n_frame = 0;


CLSID bmpClsid, pngClsid, tiffClsid;
ULONG_PTR gdiplusToken;

//-------------------------
//global control variables
//-------------------------
static int draw_indexed_count = 0;

//static std::unique_ptr<DatasetAnnotator> annotator=std::make_unique<DatasetAnnotator>(parameters_file,false);

// FIXME make a read-function for the annotator in order not to require updating it every time
// the read-function will be called everytime , a new record is started
// FIXME add also a reset function, if required, that resets some states, here's a need of some more information
// FIXME the constructor should only initialize some required ressources, that will last for the entire time, it lives
//static std::unique_ptr<DatasetAnnotator> annotator = std::make_unique<DatasetAnnotator>(parameters_file, false);


static void(_stdcall ID3D11DeviceContext::* origDrawInstanced)(UINT, UINT, INT) = nullptr;
static ComPtr<ID3D11DeviceContext> ctx;

int __stdcall DllMain(HMODULE hinstance, DWORD reason, LPVOID lpReserved)
{
	auto f = fopen(logFilePath, "a");
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		keyboardHandlerRegister(OnKeyboardMessage);
		scriptRegister(hinstance, ScriptMain);
		break;
	case DLL_PROCESS_DETACH:
		scriptUnregister(hinstance);
		keyboardHandlerUnregister(OnKeyboardMessage);
		break;
	}
	fclose(f);
	return TRUE;
}

void toggleMapshot(const bool flag) {
	std::lock_guard<std::mutex> lock(mut);
	mapshot = flag;
}


inline int StringToWString_(std::wstring &ws, const std::string &s)
{
	std::wstring wsTmp(s.begin(), s.end());
	ws = wsTmp;
	return 0;
}



float bytesToFloat(byte b0, byte b1, byte b2, byte b3)
{
	byte byte_array[] = { b3, b2, b1, b0 };
	float result;
	std::copy(reinterpret_cast<const char*>(&byte_array[0]),
		reinterpret_cast<const char*>(&byte_array[4]),
		reinterpret_cast<char*>(&result));
	return result;
}

float byteToFloat(byte b0)
{
	byte byte_array[] = { b0 };
	float result;
	std::copy(reinterpret_cast<const char*>(&byte_array[0]),
		reinterpret_cast<const char*>(&byte_array[1]),
		reinterpret_cast<char*>(&result));
	return result;
}

int GetEncoderClsid_(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	Gdiplus::ImageCodecInfo* pImageCodecInfo = NULL;

	Gdiplus::GetImageEncodersSize(&num, &size);
	if (size == 0)
		return -1;  // Failure

	pImageCodecInfo = (Gdiplus::ImageCodecInfo*)(malloc(size));
	if (pImageCodecInfo == NULL)
		return -1;  // Failure

	GetImageEncoders(num, size, pImageCodecInfo);

	for (UINT j = 0; j < num; ++j)
	{
		if (wcscmp(pImageCodecInfo[j].MimeType, format) == 0)
		{
			*pClsid = pImageCodecInfo[j].Clsid;
			free(pImageCodecInfo);
			return j;  // Success
		}
	}


	free(pImageCodecInfo);
	return -1;  // Failure
}







