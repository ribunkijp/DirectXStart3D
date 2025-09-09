/**********************************************************************************
 * GraphicsApp.cpp
 *
 *
 *
 * LI WENHUI
 * 2025/09/09
 **********************************************************************************/

#include "Pch.h"
#include " GraphicsApp.h"
#include "Camera.h"
#include <dxgi1_6.h>

GraphicsApp::GraphicsApp() {
    
}

GraphicsApp::~GraphicsApp() {

    if (m_context) {
        m_context->ClearState();
        m_context->Flush();
    }
    if (m_swapChain) {
        m_swapChain->SetFullscreenState(FALSE, nullptr);
    }
#ifdef _DEBUG
    if (m_device) {
        Microsoft::WRL::ComPtr<ID3D11Debug> debug;
        if (SUCCEEDED(m_device.As(&debug))) {
            debug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
        }
    }
#endif
}

bool GraphicsApp::Initialize(HWND hwnd) {
    m_hwnd = hwnd;

    m_camera = std::make_unique<Camera>();


    RECT rect;
    GetClientRect(hwnd, &rect);
    float clientWidth = static_cast<float>(rect.right - rect.left);
    float clientHeight = static_cast<float>(rect.bottom - rect.top);

    m_canvasWidth = static_cast<UINT>(std::round(m_logicalWidth / m_SCALE_FACTOR));
    m_canvasHeight = static_cast<UINT>(std::round(m_logicalHeight / m_SCALE_FACTOR));
    if (m_canvasHeight == 0) m_canvasHeight = 1;

    m_aspectRatio = static_cast<float>(m_canvasWidth) / static_cast<float>(m_canvasHeight);
  
    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        m_fieldOfView, 
        m_aspectRatio,
        m_nearPlane,   
        m_farPlane     
    );

    DirectX::XMStoreFloat4x4(&m_projection, projectionMatrix);




    DXGI_SWAP_CHAIN_DESC scd = {};//交换链（Swap Chain） 的描述结构体
    scd.BufferCount = 1; // 后台缓冲区的数量
    scd.BufferDesc.Width = static_cast<unsigned>(clientWidth); //后台缓冲区的分辨率
    scd.BufferDesc.Height = static_cast<unsigned>(clientHeight);
    scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; //缓冲区的像素格式 R8G8B8A8_UNORM = 32位颜色，每个通道 8 bit，范围 0–255，颜色值存储为 [0.0, 1.0] 的归一化浮点数
    scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; //指定缓冲区的用途 RENDER_TARGET_OUTPUT 表示这个缓冲区用来作为渲染目标
    scd.OutputWindow = hwnd;//指定渲染的目标窗口句柄
    scd.SampleDesc.Count = 1;//多重采样（MSAA，抗锯齿）的采样数, 1 = 不使用 MSAA 每像素只采样一次,如果设置成 4，就是 4x MSAA，但需要额外的性能开销
    scd.Windowed = TRUE;//是否窗口模式


    UINT createDeviceFlags = 0;// 设备标志与调试层
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG; //Debug 构建时启用 D3D 调试层
#endif

    D3D_FEATURE_LEVEL featureLevels[] = { //期望的特性级别
        D3D_FEATURE_LEVEL_11_1,//API：优先尝试 11.1，不行就降级到 11.0
        D3D_FEATURE_LEVEL_11_0,
    };
    UINT numFeatureLevels = ARRAYSIZE(featureLevels);
    D3D_FEATURE_LEVEL featureLevel;

    Microsoft::WRL::ComPtr<IDXGIAdapter1> bestAdapter = GraphicsUtils::SelectBestAdapter(); // 选择最佳显卡
    if (bestAdapter == nullptr) {
        OutputDebugStringW(L"Could not find a high-performance GPU, using default adapter.\n");
    }
    IDXGIAdapter1* pAdapter = bestAdapter.Get();
    D3D_DRIVER_TYPE driverType = pAdapter ? D3D_DRIVER_TYPE_UNKNOWN
        : D3D_DRIVER_TYPE_HARDWARE;

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        pAdapter,//具体适配器或 nullptr
        driverType,// UNKNOWN(配合pAdapter) 或 HARDWARE(系统自己挑默认硬件适配器)
        nullptr,// 软件光栅 DLL，通常 nullptr
        createDeviceFlags,//调试层等标志                           
        featureLevels, numFeatureLevels,
        D3D11_SDK_VERSION,
        &scd,//DXGI_SWAP_CHAIN_DESC scd
        &m_swapChain, // 输出：交换链             
        &m_device, // 输出：设备               
        &featureLevel, //输出：协商到的特性级别
        &m_context); //输出：上下文
    if (FAILED(hr)) return false;


    // 后台缓冲区
    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(backBuffer.GetAddressOf()));//从交换链取第 0 个缓冲（通常就是当前后台缓冲
    if (FAILED(hr)) return false;
    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.GetAddressOf());//把一块 Texture2D（交换链的后台缓冲区）作为“可写的渲染目标” 暴露给输出合并阶段（OM），以便后续用 OMSetRenderTargets 绑定并往里写像素
    if (FAILED(hr)) {
        return false;
    }


    return true;
}