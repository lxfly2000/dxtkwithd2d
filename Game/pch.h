//
// pch.h
// Header for standard system include files.
//

#pragma once

#include <WinSDKVer.h>
#define _WIN32_WINNT 0x0601
#include <SDKDDKVer.h>

// Use the C++ standard templated min/max
#define NOMINMAX

// DirectX apps don't need GDI
#define NODRAWTEXT
//#define NOGDI//老子要用这东西！
#define NOBITMAP

// Include <mcx.h> if you need this
#define NOMCX

// Include <winsvc.h> if you need this
#define NOSERVICE

// WinHelp is deprecated
#define NOHELP

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <wrl/client.h>

#include <d3d11_1.h>
#include <dxgi1_2.h>
#include <DirectXMath.h>
#include <DirectXColors.h>

#include <algorithm>
#include <exception>
#include <memory>
#include <stdexcept>

//DXTK头文件
#include "CommonStates.h"
#include "DDSTextureLoader.h"
#include "DirectXHelpers.h"
#include "Effects.h"
#include "GamePad.h"
#include "GeometricPrimitive.h"
#include "GraphicsMemory.h"
#include "Keyboard.h"
#include "Model.h"
#include "Mouse.h"
#include "PostProcess.h"
#include "PrimitiveBatch.h"
#include "ScreenGrab.h"
#include "SimpleMath.h"
#include "SpriteBatch.h"
#include "SpriteFont.h"
#include "VertexTypes.h"
#include "WICTextureLoader.h"

namespace DX
{
    inline void ThrowIfFailed(HRESULT hr,LPCTSTR f)
    {
        if (FAILED(hr))
        {
            TCHAR msg[256],buf[128];
            wsprintf(msg, TEXT("%s\n%#x\n"), f, hr);
            FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, hr, GetUserDefaultLangID(), buf, ARRAYSIZE(buf), NULL);
            lstrcat(msg, buf);
            MessageBox(NULL, msg, NULL, MB_ICONERROR);
            // Set a breakpoint on this line to catch DirectX API errors
            throw std::exception();
        }
    }
}

#define DXThrowIfFailed(hr) DX::ThrowIfFailed(hr,__FUNCTIONW__)
