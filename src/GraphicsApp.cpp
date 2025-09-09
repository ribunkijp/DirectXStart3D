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



    D3D11_TEXTURE2D_DESC depthBufferDesc = {};               // 深度/模板纹理的描述结构体，零初始化
    depthBufferDesc.Width = static_cast<UINT>(clientWidth); // 纹理宽度：与窗口客户区宽一致
    depthBufferDesc.Height = static_cast<UINT>(clientHeight);// 纹理高度：与窗口客户区高一致
    depthBufferDesc.MipLevels = 1;                           // 仅 1 个 mip 级别（深度贴图通常不需要多级）
    depthBufferDesc.ArraySize = 1;                           // 非数组纹理
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;  // 24 位深度 + 8 位模板（常用且够用）,可选替代：D32_FLOAT（高精度、无模板）、D16_UNORM（轻量）
    depthBufferDesc.SampleDesc.Count = 1;                    // 采样数=1（无 MSAA；若颜色缓冲是 MSAA，这里必须匹配）
    depthBufferDesc.SampleDesc.Quality = 0;                  // 采样质量=0（与 Count=1 搭配）
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;             // 默认用法：GPU 读写
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;    // 绑定为深度/模板目标（DSV）
    depthBufferDesc.CPUAccessFlags = 0;                      // CPU 不访问
    depthBufferDesc.MiscFlags = 0;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> depthStencilBuffer;// 深度/模板（Depth/Stencil）缓冲的 2D 纹理指针// 无其他附加标记
    hr = m_device->CreateTexture2D(                    // 创建深度/模板纹理资源
        &depthBufferDesc, nullptr, &depthStencilBuffer
    );
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create depth stencil buffer.", L"Error", MB_OK);
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};             // DSV（深度/模板视图）描述
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;          // 视图格式需与资源兼容（或资源用 typeless、视图用 typed）
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;   // 非 MSAA 的 2D 视图（若 MSAA 用 TEXTURE2DMS）
    dsvDesc.Texture2D.MipSlice = 0;                          // 绑定 mip 级别 0
    hr = m_device->CreateDepthStencilView(             // 从纹理创建 DSV 视图
        depthStencilBuffer.Get(), &dsvDesc, m_depthStencilView.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create depth stencil view.", L"Error", MB_OK);
        return false;
    }

    m_context->OMSetRenderTargets(  // 将颜色 RTV 与深度/模板 DSV 一起绑定到 OM 阶段
        1, m_rtv.GetAddressOf(), m_depthStencilView.Get()
    );


    return true;
}