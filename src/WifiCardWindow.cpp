#include "WifiCardWindow.h"
#pragma comment(lib, "comdlg32.lib")
#include "WifiQrGenerator.h"

#include <commdlg.h>
#include <algorithm>
#include <memory>
#include <vector>

#include "qrcodegen.hpp"

namespace
{
constexpr int IDC_SSID = 1001;
constexpr int IDC_PASSWORD = 1002;
constexpr int IDC_PRINT = 1003;
constexpr int IDC_QR = 1004;
constexpr int IDC_SECURITY = 1005;

HWND g_ssid = nullptr;
HWND g_password = nullptr;
HWND g_security = nullptr;
HFONT g_font = nullptr;

std::wstring Utf8ToWide(const std::string& text)
{
    if (text.empty())
        return std::wstring();

    int count = ::MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0);
    if (count <= 0)
        return std::wstring();

    std::wstring result(static_cast<size_t>(count), L'\0');
    ::MultiByteToWideChar(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), count);
    return result;
}

std::string WideToUtf8(const std::wstring& text)
{
    if (text.empty())
        return std::string();

    int count = ::WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), nullptr, 0, nullptr, nullptr);
    if (count <= 0)
        return std::string();

    std::string result(static_cast<size_t>(count), '\0');
    ::WideCharToMultiByte(CP_UTF8, 0, text.data(), static_cast<int>(text.size()), result.data(), count, nullptr, nullptr);
    return result;
}

void DrawTextCentered(HDC hdc, const std::wstring& text, const RECT& rc, UINT format)
{
    ::DrawTextW(hdc, text.c_str(), -1, const_cast<RECT*>(&rc), format | DT_NOPREFIX);
}
}

int WifiCardWindow::Run(HINSTANCE hInstance, int nCmdShow)
{
    const wchar_t CLASS_NAME[] = L"WiFiCardGeneratorWindow";

    WNDCLASSEXW wc{};
    wc.cbSize = sizeof(wc);
    wc.hInstance = hInstance;
    wc.lpfnWndProc = &WifiCardWindow::WndProc;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = ::LoadCursorW(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);

    if (!::RegisterClassExW(&wc))
        return 1;

    HWND hwnd = ::CreateWindowExW(
        0, CLASS_NAME, L"WiFi Card Generator",
        WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
        CW_USEDEFAULT, CW_USEDEFAULT, 760, 620,
        nullptr, nullptr, hInstance, nullptr);

    if (!hwnd)
        return 1;

    ::ShowWindow(hwnd, nCmdShow);
    ::UpdateWindow(hwnd);

    MSG msg{};
    while (::GetMessageW(&msg, nullptr, 0, 0) > 0)
    {
        ::TranslateMessage(&msg);
        ::DispatchMessageW(&msg);
    }

    return static_cast<int>(msg.wParam);
}

LRESULT CALLBACK WifiCardWindow::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        g_font = ::CreateFontW(-18, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
                               DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
                               CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

        HINSTANCE hInstance = reinterpret_cast<LPCREATESTRUCT>(lParam)->hInstance;

        g_ssid = ::CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"My WiFi",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            30, 45, 360, 34, hwnd, reinterpret_cast<HMENU>(IDC_SSID),
            hInstance, nullptr);

        g_security = ::CreateWindowExW(WS_EX_CLIENTEDGE, L"COMBOBOX", L"",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | CBS_DROPDOWNLIST | WS_VSCROLL,
            30, 120, 360, 180, hwnd, reinterpret_cast<HMENU>(IDC_SECURITY),
            hInstance, nullptr);

        ::SendMessageW(g_security, CB_ADDSTRING, 0,
                        reinterpret_cast<LPARAM>(L"WPA / WPA2 / WPA3-Personal"));
        ::SendMessageW(g_security, CB_ADDSTRING, 0,
                        reinterpret_cast<LPARAM>(L"WEP"));
        ::SendMessageW(g_security, CB_ADDSTRING, 0,
                        reinterpret_cast<LPARAM>(L"Open / No password"));
        ::SendMessageW(g_security, CB_SETCURSEL, 0, 0);

        g_password = ::CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"password",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
            30, 195, 360, 34, hwnd, reinterpret_cast<HMENU>(IDC_PASSWORD),
            hInstance, nullptr);

        HWND button = ::CreateWindowExW(0, L"BUTTON", L"Print WiFi Card",
            WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
            30, 250, 180, 40, hwnd, reinterpret_cast<HMENU>(IDC_PRINT),
            hInstance, nullptr);

        ::SendMessageW(g_ssid, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
        ::SendMessageW(g_security, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
        ::SendMessageW(g_password, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
        ::SendMessageW(button, WM_SETFONT, reinterpret_cast<WPARAM>(g_font), TRUE);
        return 0;
    }

    case WM_COMMAND:
        if (HIWORD(wParam) == EN_CHANGE &&
            (LOWORD(wParam) == IDC_SSID || LOWORD(wParam) == IDC_PASSWORD))
        {
            UpdateQr(hwnd);
            return 0;
        }

        if (LOWORD(wParam) == IDC_SECURITY && HIWORD(wParam) == CBN_SELCHANGE)
        {
            const bool openNetwork = (GetSecurityType(hwnd) == "nopass");
            ::EnableWindow(g_password, openNetwork ? FALSE : TRUE);
            UpdateQr(hwnd);
            return 0;
        }

        if (LOWORD(wParam) == IDC_PRINT && HIWORD(wParam) == BN_CLICKED)
        {
            PrintCard(hwnd);
            return 0;
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps{};
        HDC hdc = ::BeginPaint(hwnd, &ps);
        Paint(hwnd, hdc);
        ::EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        if (g_font)
        {
            ::DeleteObject(g_font);
            g_font = nullptr;
        }
        g_ssid = nullptr;
        g_password = nullptr;
        g_security = nullptr;
        ::PostQuitMessage(0);
        return 0;
    }

    return ::DefWindowProcW(hwnd, msg, wParam, lParam);
}

std::string WifiCardWindow::GetEditText(HWND hwnd, int id)
{
    HWND control = ::GetDlgItem(hwnd, id);
    int length = ::GetWindowTextLengthW(control);
    if (length <= 0)
        return std::string();

    std::wstring text(static_cast<size_t>(length), L'\0');
    ::GetWindowTextW(control, text.data(), length + 1);
    return WideToUtf8(text);
}

std::string WifiCardWindow::GetSecurityType(HWND hwnd)
{
    HWND combo = ::GetDlgItem(hwnd, IDC_SECURITY);
    const LRESULT index = ::SendMessageW(combo, CB_GETCURSEL, 0, 0);

    switch (index)
    {
    case 1:
        return "WEP";
    case 2:
        return "nopass";
    case 0:
    default:
        // The Wi-Fi QR format uses T:WPA for WPA/WPA2/WPA3-Personal.
        return "WPA";
    }
}

void WifiCardWindow::UpdateQr(HWND hwnd)
{
    ::InvalidateRect(hwnd, nullptr, FALSE);
}

void WifiCardWindow::Paint(HWND hwnd, HDC hdc)
{
    RECT client{};
    ::GetClientRect(hwnd, &client);

    HBRUSH background = ::CreateSolidBrush(RGB(245, 245, 245));
    ::FillRect(hdc, &client, background);
    ::DeleteObject(background);

    HFONT oldFont = nullptr;
    if (g_font)
        oldFont = reinterpret_cast<HFONT>(::SelectObject(hdc, g_font));

    RECT label{30, 18, 390, 42};
    ::DrawTextW(hdc, L"WiFi network name (SSID)", -1, &label, DT_LEFT | DT_SINGLELINE);

    label.top = 93;
    label.bottom = 117;
    ::DrawTextW(hdc, L"Security type", -1, &label, DT_LEFT | DT_SINGLELINE);

    label.top = 168;
    label.bottom = 192;
    ::DrawTextW(hdc, L"WiFi password", -1, &label, DT_LEFT | DT_SINGLELINE);

    const std::string ssid = GetEditText(hwnd, IDC_SSID);
    const std::string password = GetEditText(hwnd, IDC_PASSWORD);
    const std::string security = GetSecurityType(hwnd);
    const std::string payload = WifiQrGenerator::BuildPayload(ssid, password, security);

    RECT card{430, 25, client.right - 30, client.bottom - 30};
    DrawCard(hwnd, hdc, card, ssid, password, security, payload);

    if (oldFont)
        ::SelectObject(hdc, oldFont);
}

void WifiCardWindow::DrawCard(HWND, HDC hdc, const RECT& rc,
                              const std::string& ssid,
                              const std::string&,
                              const std::string& security,
                              const std::string& payload)
{
    HBRUSH white = ::CreateSolidBrush(RGB(255, 255, 255));
    HPEN border = ::CreatePen(PS_SOLID, 1, RGB(210, 210, 210));
    HGDIOBJ oldBrush = ::SelectObject(hdc, white);
    HGDIOBJ oldPen = ::SelectObject(hdc, border);
    ::RoundRect(hdc, rc.left, rc.top, rc.right, rc.bottom, 18, 18);
    ::SelectObject(hdc, oldBrush);
    ::SelectObject(hdc, oldPen);
    ::DeleteObject(white);
    ::DeleteObject(border);

    RECT title = rc;
    title.top += 18;
    title.bottom = title.top + 30;
    DrawTextCentered(hdc, L"WiFi", title, DT_CENTER | DT_SINGLELINE);

    RECT qr = rc;
    qr.left += 25;
    qr.right -= 25;
    qr.top += 55;
    qr.bottom = qr.top + (std::min)(qr.right - qr.left, rc.bottom - qr.top - 125);
    DrawQr(hdc, qr, payload);

    RECT name = rc;
    name.top = qr.bottom + 8;
    name.bottom = name.top + 25;
    std::wstring wssid = Utf8ToWide(ssid);
    if (wssid.empty())
        wssid = L"Enter WiFi details";
    DrawTextCentered(hdc, wssid, name, DT_CENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

    RECT type = rc;
    type.top = name.bottom + 2;
    type.bottom = type.top + 22;

    std::wstring securityText;
    if (security == "WEP")
        securityText = L"WEP";
    else if (security == "nopass")
        securityText = L"Open network";
    else
        securityText = L"WPA / WPA2 / WPA3-Personal";

    DrawTextCentered(hdc, securityText, type, DT_CENTER | DT_SINGLELINE);
}

void WifiCardWindow::DrawQr(HDC hdc, const RECT& rc, const std::string& payload)
{
    using namespace qrcodegen;

    const QrCode qr = QrCode::encodeText(
        payload.c_str(),
        QrCode::Ecc::MEDIUM);

    const int size = qr.getSize();
    const int quiet = 4;
    const int modules = size + quiet * 2;

    const int width =
        static_cast<int>(rc.right - rc.left);

    const int height =
        static_cast<int>(rc.bottom - rc.top);

    const int available =
        (std::min)(width, height);

    const int pixel =
        (std::max)(1, available / modules);

    const int qrSize =
        modules * pixel;

    const int startX =
        rc.left + (width - qrSize) / 2;

    const int startY =
        rc.top + (height - qrSize) / 2;

    HBRUSH hWhiteBrush = ::CreateSolidBrush(RGB(255, 255, 255));
    HBRUSH hBlackBrush = ::CreateSolidBrush(RGB(0, 0, 0));

    RECT qrBackground;
    qrBackground.left   = startX;
    qrBackground.top    = startY;
    qrBackground.right  = startX + qrSize;
    qrBackground.bottom = startY + qrSize;

    ::FillRect(hdc, &qrBackground, hWhiteBrush);

    for (int y = 0; y < size; ++y)
    {
        for (int x = 0; x < size; ++x)
        {
            if (!qr.getModule(x, y))
                continue;

            RECT moduleRect;
            moduleRect.left = startX + (x + quiet) * pixel;
            moduleRect.top = startY + (y + quiet) * pixel;
            moduleRect.right = moduleRect.left + pixel;
            moduleRect.bottom = moduleRect.top + pixel;

            ::FillRect(hdc, &moduleRect, hBlackBrush);
        }
    }

    ::DeleteObject(hBlackBrush);
    ::DeleteObject(hWhiteBrush);
}

void WifiCardWindow::PrintCard(HWND hwnd)
{
    PRINTDLGW pd{};
    pd.lStructSize = sizeof(pd);
    pd.hwndOwner = hwnd;
    pd.Flags = PD_RETURNDC;

    if (!::PrintDlgW(&pd))
        return;

    DOCINFOW di{};
    di.cbSize = sizeof(di);
    di.lpszDocName = L"WiFi Card";

    if (::StartDocW(pd.hDC, &di) > 0)
    {
        if (::StartPage(pd.hDC) > 0)
        {
            RECT page{0, 0,
                      ::GetDeviceCaps(pd.hDC, HORZRES),
                      ::GetDeviceCaps(pd.hDC, VERTRES)};

            const std::string ssid = GetEditText(hwnd, IDC_SSID);
            const std::string password = GetEditText(hwnd, IDC_PASSWORD);
            const std::string security = GetSecurityType(hwnd);
            const std::string payload = WifiQrGenerator::BuildPayload(ssid, password, security);

            DrawCard(hwnd, pd.hDC, page, ssid, password, security, payload);
            ::EndPage(pd.hDC);
        }
        ::EndDoc(pd.hDC);
    }

    ::DeleteDC(pd.hDC);
}
