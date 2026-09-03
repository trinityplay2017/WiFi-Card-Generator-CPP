#pragma once
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <string>

class WifiCardWindow
{
public:
    static int Run(HINSTANCE hInstance, int nCmdShow);

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    static void Paint(HWND hwnd, HDC hdc);
    static void UpdateQr(HWND hwnd);
    static void PrintCard(HWND hwnd);
    static std::string GetEditText(HWND hwnd, int id);
    static std::string GetSecurityType(HWND hwnd);
    static void DrawQr(HDC hdc, const RECT& rc, const std::string& payload);
    static void DrawCard(HWND hwnd, HDC hdc, const RECT& rc,
                         const std::string& ssid,
                         const std::string& password,
                         const std::string& security,
                         const std::string& payload);
};
