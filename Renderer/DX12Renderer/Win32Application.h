//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers.
#endif
#include <windows.h>
#include <wrl.h>
#include <string>
#include <functional>
class Win32Application
{
public:


    Win32Application(std::function<void(void)> = dummy_render_func);
    
    ~Win32Application() { }

    void SetRenderFunc(std::function<void(void)> p_func);

    void RenderWindow();

protected:
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
    HWND m_hwnd;
private:
    std::function<void(void)> m_render_func;
    static void dummy_render_func() {};

};
