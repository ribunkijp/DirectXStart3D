/**********************************************************************************
 * main.cpp
 *
 *
 *
 * LI WENHUI
 * 2025/09/10
 **********************************************************************************/

#include "Pch.h"
#include "windowsx.h"
#include "GraphicsApp.h"
#include <ShellScalingAPI.h> //dpi


 /**
  * @class CoInitializer
  * @brief 使用 RAII (Resource Acquisition Is Initialization) 模式管理 COM 库生命周期
  */
class CoInitializer {
public:
    CoInitializer() {
        const HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
        if (hr == S_OK || hr == S_FALSE) {
            m_ok = true;
            m_hr = S_OK;
        }
        else if (hr == RPC_E_CHANGED_MODE) {

            m_ok = false;
            m_hr = S_OK;
        }
        else {
            m_ok = false;
            m_hr = hr;
        }
    }
    ~CoInitializer() {
        if (m_ok) CoUninitialize();
    }
    HRESULT GetResult() const { return m_hr; }
private:
    bool m_ok = false;
    HRESULT m_hr = E_FAIL;
};


GraphicsApp* g_pApp = nullptr;

void GetScaledWindowSizeAndPosition(float logicalWidth, float logicalHeight,
    int& outW, int& outH, int& outLeft, int& outTop, DWORD C_WND_STYLE);

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);



int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR pCmdLine,
    _In_ int nCmdShow
) {
    CoInitializer coInit;
    if (FAILED(coInit.GetResult()))
    {
        MessageBoxW(nullptr, L"CoInitializeEx Failed!", L"Error", MB_OK);
        return 0;
    }

    // DPI感知を設定
    SetProcessDpiAwareness(PROCESS_PER_MONITOR_DPI_AWARE);

    (void)hPrevInstance;
    (void)pCmdLine;

    const wchar_t CLASS_NAME[] = L"Window";

    WNDCLASS wc = { };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);


    auto theApp = std::make_unique<GraphicsApp>();


    const DWORD C_WND_STYLE = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;

    int winW = 0, winH = 0, winL = 0, winT = 0;
    GetScaledWindowSizeAndPosition(1856.0f, 1044.0f, winW, winH, winL, winT, C_WND_STYLE);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"3dGAME",
        C_WND_STYLE,
        winL,
        winT,
        winW,
        winH,
        NULL,
        NULL,
        hInstance,
        theApp.get()
    );

    if (hwnd == NULL)
    {
        return 0;
    }

    //ShowWindow(hwnd, nCmdShow);
    ShowWindow(hwnd, SW_SHOWNORMAL);


    RECT rect;
    GetClientRect(hwnd, &rect);
    auto clientWidth = static_cast<float>(rect.right - rect.left);
    auto clientHeight = static_cast<float>(rect.bottom - rect.top);

    if (!theApp->Initialize(hwnd)) {
        MessageBoxW(hwnd, L"App Initialization Failed!", L"Error", MB_OK);
        return 0;
    }

    theApp->Run();

    return 0;

}

LRESULT CALLBACK WindowProc(
    HWND hwnd,
    UINT uMsg,
    WPARAM wParam,
    LPARAM lParam
) {

    if (uMsg == WM_CREATE)
    {
        //从消息中解包出传递的theApp.get()
        CREATESTRUCT* pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
        //把theApp的地址存入全局指针g_pApp
        g_pApp = reinterpret_cast<GraphicsApp*>(pCreate->lpCreateParams);
        return 0;
    }

    switch (uMsg) {
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_SIZE:
        {
            if (g_pApp) {
                UINT width = LOWORD(lParam);
                UINT height = HIWORD(lParam);
                if (width > 0 && height > 0) {
                    g_pApp->OnResize(width, height);
                }
            }
            return 0;
        }
        case WM_INPUT:
        {
            if (g_pApp) {
                UINT dwSize = 0;
                // 第一次调用获取所需缓冲区大小
                GetRawInputData((HRAWINPUT)lParam, RID_INPUT, NULL, &dwSize, sizeof(RAWINPUTHEADER));

                // 分配内存并第二次调用来获取数据
                if (dwSize > 0) {
                    std::vector<BYTE> rawBuffer(dwSize);
                    if (GetRawInputData((HRAWINPUT)lParam, RID_INPUT, rawBuffer.data(), &dwSize, sizeof(RAWINPUTHEADER)) == dwSize)
                    {
                        RAWINPUT* raw = (RAWINPUT*)rawBuffer.data();
                        // 确认是鼠标输入
                        if (raw->header.dwType == RIM_TYPEMOUSE)
                        {
                            // raw->data.mouse.lLastX 和 lLastY 鼠标增量
                            g_pApp->OnRawMouseMove(raw->data.mouse.lLastX, raw->data.mouse.lLastY);


                        }
                    }
                }
            }

        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void GetScaledWindowSizeAndPosition(float logicalWidth, float logicalHeight,
    int& outW, int& outH, int& outLeft, int& outTop, DWORD C_WND_STYLE)
{
    HMONITOR monitor = MonitorFromPoint(POINT{ 0, 0 }, MONITOR_DEFAULTTOPRIMARY);
    UINT dpiX = 96, dpiY = 96;
    GetDpiForMonitor(monitor, MDT_EFFECTIVE_DPI, &dpiX, &dpiY);

    float dpiScale = static_cast<float>(dpiX) / 96.0f;
    int scaledClientW = static_cast<int>(logicalWidth * dpiScale);
    int scaledClientH = static_cast<int>(logicalHeight * dpiScale);

    RECT rect = { 0, 0, scaledClientW, scaledClientH };
    AdjustWindowRectExForDpi(&rect, C_WND_STYLE, FALSE, 0, dpiX);
    outW = rect.right - rect.left;
    outH = rect.bottom - rect.top;

    MONITORINFO mi = { sizeof(mi) };

    int workW = 0, workH = 0;
    if (GetMonitorInfo(monitor, &mi))
    {
        workW = mi.rcWork.right - mi.rcWork.left;
        workH = mi.rcWork.bottom - mi.rcWork.top;
    }
    else
    {
        workW = GetSystemMetricsForDpi(SM_CXSCREEN, dpiX);
        workH = GetSystemMetricsForDpi(SM_CYSCREEN, dpiX);
    }

    float wScale = static_cast<float>(workW) / static_cast<float>(outW);
    float hScale = static_cast<float>(workH) / static_cast<float>(outH);
    float scale = (wScale < hScale) ? wScale : hScale;
    if (scale < 1.0f) {
        outW = static_cast<int>(static_cast<float>(outW) * scale);
        outH = static_cast<int>(static_cast<float>(outH) * scale);
    }

    outLeft = mi.rcWork.left + (workW - outW) / 2;
    outTop = mi.rcWork.top + (workH - outH) / 2;
}