/**********************************************************************************
 * GraphicsApp.cpp
 *
 *
 *
 * LI WENHUI
 * 2025/09/09
 **********************************************************************************/

#include "Pch.h"
#include "GraphicsApp.h"
#include "Camera.h"
#include <dxgi1_6.h>
#include "GraphicsUtils.h"
#include "CommonTypes.h"
#include "BufferUtils.h"
#include "Timer.h"
#include "Player.h"
#include "InputController.h"
#include "Ground.h"

GraphicsApp::GraphicsApp() {
    DirectX::XMStoreFloat4x4(
        &m_projection,
        DirectX::XMMatrixIdentity()
    );
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


    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01;
    rid.usUsage = 0x02;
    rid.dwFlags = 0;
    rid.hwndTarget = hwnd;

    if (RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)) == FALSE) {
        MessageBoxW(hwnd, L"Failed to register raw input device.", L"Error", MB_OK);
        return false;
    }
    


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


    
    // depth
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
    
    hr = m_device->CreateTexture2D(                    // 创建深度/模板纹理资源
        &depthBufferDesc, nullptr, m_depthStencilBuffer.GetAddressOf()
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
        m_depthStencilBuffer.Get(), &dsvDesc, m_depthStencilView.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create depth stencil view.", L"Error", MB_OK);
        return false;
    }




    // viewport
    D3D11_VIEWPORT vp = {};          // viewport 把裁剪后的 NDC 坐标映射到渲染目标上的矩形区域
    vp.Width = clientWidth;         
    vp.Height = clientHeight;        
    vp.MinDepth = 0.0f;              // 深度下限（通常 0）
    vp.MaxDepth = 1.0f;              // 深度上限（通常 1）
    m_context->RSSetViewports(1, &vp);  // 绑定到光栅化阶段



    // RTT
    D3D11_TEXTURE2D_DESC cdesc = {};                
    cdesc.Width = m_canvasWidth;             
    cdesc.Height = m_canvasHeight;           
    cdesc.MipLevels = 1;                            // 只有 1 个 mip（不做 mip 采样就够）
    cdesc.ArraySize = 1;                            // 非数组
    cdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      // 8bit RGBA 颜色
    cdesc.SampleDesc.Count = 1;                     // 无 MSAA（如需 MSAA，DSV/RTV 采样数必须一致）
    cdesc.Usage = D3D11_USAGE_DEFAULT;              // GPU 读写
    cdesc.BindFlags = D3D11_BIND_RENDER_TARGET // 既能作为“渲染目标”（RTV）
        | D3D11_BIND_SHADER_RESOURCE;// 又能作为“着色器资源”（SRV）被采样

    hr = m_device->CreateTexture2D(&cdesc, nullptr, m_canvasTex.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create canvas texture.", L"Error", MB_OK);
        return false;
    }

    hr = m_device->CreateRenderTargetView(m_canvasTex.Get(), nullptr, m_canvasRTV.GetAddressOf());//从该纹理创建渲染目标视图（RTV）
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create m_canvasRTV.", L"Error", MB_OK);
        return false;
    }
    hr = m_device->CreateShaderResourceView(m_canvasTex.Get(), nullptr, m_canvasSRV.GetAddressOf());//从该纹理创建着色器资源视图（SRV）
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create m_canvasSRV.", L"Error", MB_OK);
        return false;
    }

    // RTT depth
    D3D11_TEXTURE2D_DESC depthBufferDesc_RTT = {};               
    depthBufferDesc_RTT.Width = m_canvasWidth;
    depthBufferDesc_RTT.Height = m_canvasHeight;
    depthBufferDesc_RTT.MipLevels = 1;
    depthBufferDesc_RTT.ArraySize = 1;
    depthBufferDesc_RTT.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc_RTT.SampleDesc.Count = 1;
    depthBufferDesc_RTT.SampleDesc.Quality = 0;
    depthBufferDesc_RTT.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc_RTT.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc_RTT.CPUAccessFlags = 0;
    depthBufferDesc_RTT.MiscFlags = 0;
    
    hr = m_device->CreateTexture2D(                    
        &depthBufferDesc_RTT, nullptr, m_canvasDepth.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create depth stencil buffer RTT.", L"Error", MB_OK);
        return false;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc_RTT = {};             // DSV（深度/模板视图）描述
    dsvDesc_RTT.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;          // 视图格式需与资源兼容（或资源用 typeless、视图用 typed）
    dsvDesc_RTT.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;   // 非 MSAA 的 2D 视图（若 MSAA 用 TEXTURE2DMS）
    dsvDesc_RTT.Texture2D.MipSlice = 0;                          // 绑定 mip 级别 0
    hr = m_device->CreateDepthStencilView(             // 从纹理创建 DSV 视图
        m_canvasDepth.Get(), &dsvDesc_RTT, m_canvasDSV.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create depth stencil view RTT.", L"Error", MB_OK);
        return false;
    }





    Microsoft::WRL::ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;
    hr = D3DCompileFromFile(
        L"shaders/shader.hlsl",    // HLSL 源文件路径
        nullptr, nullptr,          // 宏定义、包含处理器
        "VSMain", "vs_5_0",        // 函数入口 = VSMain，目标 profile = vs_5_0
        D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, // 严格检查 + Debug 信息
        0,
        &vsBlob,                   // 输出：编译好的字节码
        &errorBlob                 // 输出：错误信息
    );

    if (FAILED(hr)) {
        return false;
    }
    hr = D3DCompileFromFile(
        L"shaders/shader.hlsl",
        nullptr,
        nullptr,
        "PSMain", "ps_5_0",// 函数入口 = PSMain，目标 profile = ps_5_0
        D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
        0,
        &psBlob,
        &errorBlob
    );

    if (FAILED(hr)) {
        return false;
    }


    hr = m_device->CreateVertexShader(// 创建顶点着色器对象
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        m_vertexShader.GetAddressOf()
    );
    if (FAILED(hr)) {
        return false;
    }
    hr = m_device->CreatePixelShader(// 创建像素着色器对象
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        m_pixelShader.GetAddressOf()
    );
    if (FAILED(hr)) {
        return false;
    }

    // inputlayout
    D3D11_INPUT_ELEMENT_DESC layout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    hr = m_device->CreateInputLayout(
        layout,
        ARRAYSIZE(layout),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        m_inputLayout.GetAddressOf()// 输出：IA 阶段的 InputLayout 对象
    );
    if (FAILED(hr)) return false;



    Microsoft::WRL::ComPtr<ID3DBlob> playerVSBlob, playerPSBlob, playerErrorBlob;
    hr = D3DCompileFromFile(
        L"shaders/player.hlsl",    // HLSL 源文件路径
        nullptr, nullptr,          // 宏定义、包含处理器
        "VSMain", "vs_5_0",        // 函数入口 = VSMain，目标 profile = vs_5_0
        D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG, // 严格检查 + Debug 信息
        0,
        &playerVSBlob,                   // 输出：编译好的字节码
        &playerErrorBlob                 // 输出：错误信息
    );

    if (FAILED(hr)) {
        return false;
    }
    hr = D3DCompileFromFile(
        L"shaders/player.hlsl",
        nullptr,
        nullptr,
        "PSMain", "ps_5_0",// 函数入口 = PSMain，目标 profile = ps_5_0
        D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
        0,
        &playerPSBlob,
        &playerErrorBlob
    );
    if (FAILED(hr)) {
        return false;
    }
    hr = m_device->CreateVertexShader(// 创建顶点着色器对象
        playerVSBlob->GetBufferPointer(),
        playerVSBlob->GetBufferSize(),
        nullptr,
        m_playerVS.GetAddressOf()
    );
    if (FAILED(hr)) {
        return false;
    }
    hr = m_device->CreatePixelShader(// 创建像素着色器对象
        playerPSBlob->GetBufferPointer(),
        playerPSBlob->GetBufferSize(),
        nullptr,
        m_playerPS.GetAddressOf()
    );
    if (FAILED(hr)) {
        return false;
    }
    // player inputlayout
    D3D11_INPUT_ELEMENT_DESC playerLayout[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "TANGENT",  0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "BONEIDS",  0, DXGI_FORMAT_R32G32B32A32_SINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
      { "WEIGHTS",  0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    hr = m_device->CreateInputLayout(
        playerLayout,
        ARRAYSIZE(playerLayout),
        playerVSBlob->GetBufferPointer(),
        playerVSBlob->GetBufferSize(),
        m_playerInputLayout.GetAddressOf()// 输出：IA 阶段的 InputLayout 对象
    );
    if (FAILED(hr)) return false;





    D3D11_SAMPLER_DESC sampDesc = {};
    sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;//双线性插值，更柔和但会糊
    sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;//平铺重复时用 WRAP
    sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;     
    sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;    

    sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER; //普通颜色采样不做比较采样（比较采样用于PCF阴影、深度贴图）
    sampDesc.MinLOD = 0;
    sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
    hr = m_device->CreateSamplerState(&sampDesc, m_samplerState.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create sampler state.", L"Error", MB_OK);
        return false;
    }



    // blend
    D3D11_BLEND_DESC blendDesc = {};//混合（Blending）状态 -> 颜色叠加方式。
    // Normal（常规透明）
    blendDesc.RenderTarget[0].BlendEnable = TRUE;
    blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
    blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
    blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
    blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
    blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    m_device->CreateBlendState(&blendDesc, m_blendStateNormal.GetAddressOf());
    // Additive（加法发光）
    D3D11_BLEND_DESC blendDescAdd = blendDesc;
    blendDescAdd.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
    blendDescAdd.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDescAdd.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    m_device->CreateBlendState(&blendDescAdd, m_blendStateAdditive.GetAddressOf());
    // Multiply（正片叠底）
    D3D11_BLEND_DESC blendDescMul = blendDesc;
    blendDescMul.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_COLOR;
    blendDescMul.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
    blendDescMul.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    m_device->CreateBlendState(&blendDescMul, m_blendStateMultiply.GetAddressOf());
    // Screen（滤色）
    D3D11_BLEND_DESC blendDescScreen = blendDesc;
    blendDescScreen.RenderTarget[0].SrcBlend = D3D11_BLEND_INV_DEST_COLOR;
    blendDescScreen.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
    blendDescScreen.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
    m_device->CreateBlendState(&blendDescScreen, m_blendStateScreen.GetAddressOf());




    // default 地面用
    D3D11_DEPTH_STENCIL_DESC defaultDepthStencilDesc = {};
    defaultDepthStencilDesc.DepthEnable = TRUE; //开启深度测试                        
    defaultDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL; //允许写入深度缓冲  
    defaultDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;              
    defaultDepthStencilDesc.StencilEnable = FALSE;
    defaultDepthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    defaultDepthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    hr = m_device->CreateDepthStencilState(&defaultDepthStencilDesc, m_depthStateDefault.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create default depth stencil state.", L"Error", MB_OK);
        return false;
    }
    // 半透明
    D3D11_DEPTH_STENCIL_DESC transparentDepthStencilDesc = {};//半透明物体用的深度/模板状态
    transparentDepthStencilDesc.DepthEnable = TRUE;//开启深度测试
    transparentDepthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;//不写入深度
    transparentDepthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
    transparentDepthStencilDesc.StencilEnable = FALSE;//不使用模板测试
    transparentDepthStencilDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    transparentDepthStencilDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    hr = m_device->CreateDepthStencilState(&transparentDepthStencilDesc, m_depthStencilStateTransparent.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create transparent depth stencil state.", L"Error", MB_OK);
        return false;
    }
    // UI用
    D3D11_DEPTH_STENCIL_DESC depthDisabledDesc = {};
    depthDisabledDesc.DepthEnable = FALSE; //关闭深度测试
    depthDisabledDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;//不写入深度
    depthDisabledDesc.DepthFunc = D3D11_COMPARISON_LESS;
    depthDisabledDesc.StencilEnable = FALSE;
    depthDisabledDesc.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
    depthDisabledDesc.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
    hr = m_device->CreateDepthStencilState(&depthDisabledDesc, m_depthStateDisabled.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create disabled depth stencil state.", L"Error", MB_OK);
        return false;
    }




    // present 
    Microsoft::WRL::ComPtr<ID3DBlob> presentVSBlob, presentPSBlob, presentErrorBlob;
    hr = D3DCompileFromFile(
        L"shaders/present.hlsl",
        nullptr, nullptr,
        "VSQuad", "vs_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
        0,
        &presentVSBlob,
        &presentErrorBlob
    );
    if (FAILED(hr)) {
        return false;
    }
    hr = D3DCompileFromFile(
        L"shaders/present.hlsl",
        nullptr, nullptr,
        "PSPresent", "ps_5_0",
        D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_DEBUG,
        0,
        &presentPSBlob,
        &presentErrorBlob
    );
    if (FAILED(hr)) {
        return false;
    }

    hr = m_device->CreateVertexShader(presentVSBlob->GetBufferPointer(), presentVSBlob->GetBufferSize(), nullptr, m_presentVS.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create present VS.", L"Error", MB_OK);
        return false;
    }
    hr = m_device->CreatePixelShader(presentPSBlob->GetBufferPointer(), presentPSBlob->GetBufferSize(), nullptr, m_presentPS.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create present PS.", L"Error", MB_OK);
        return false;
    }

    PresentVertex quadVerts[4] = {
       { { -1.f,-1.f,0.f }, {1,1,1,1}, {0.f,1.f} },
       { {  1.f,-1.f,0.f }, {1,1,1,1}, {1.f,1.f} },
       { {  1.f, 1.f,0.f }, {1,1,1,1}, {1.f,0.f} },
       { { -1.f, 1.f,0.f }, {1,1,1,1}, {0.f,0.f} },
    };
    const UINT quadIdx[6] = { 0,1,2, 0,2,3 };
    m_presentVB = BufferUtils::CreateStaticVertexBuffer(m_device.Get(), quadVerts, sizeof(quadVerts));
    if (!m_presentVB) { MessageBoxW(m_hwnd, L"Create present VB failed.", L"Error", MB_OK); return false; }
    m_presentIB = BufferUtils::CreateStaticIndexBuffer(m_device.Get(), quadIdx, sizeof(quadIdx));
    if (!m_presentIB) { MessageBoxW(m_hwnd, L"Create present IB failed.", L"Error", MB_OK); return false; }

    
    
    D3D11_INPUT_ELEMENT_DESC presentLayout[] = {
       { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,  D3D11_INPUT_PER_VERTEX_DATA, 0 },
       { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
       { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,       0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = m_device->CreateInputLayout(
        presentLayout,
        ARRAYSIZE(presentLayout),
        presentVSBlob->GetBufferPointer(),
        presentVSBlob->GetBufferSize(),
        m_presentInputLayout.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create present input layout.", L"Error", MB_OK);
        return false;
    }


    // 逆时针默认被视作正面, 顺时针默认被视作背面，三角形三个顶点在屏幕上的环绕顺序判断朝向
    //顺时针顺序定义的，必须手动关闭剔除
    D3D11_RASTERIZER_DESC rsDesc = {};//光栅化状态描述结构体
    rsDesc.FillMode = D3D11_FILL_SOLID;//填充模式 实心填充
    rsDesc.CullMode = D3D11_CULL_NONE; //剔除模式 D3D11_CULL_NONE → 不剔除，正反两面都画, D3D11_CULL_BACK → 默认，丢掉背面三角形, D3D11_CULL_FRONT → 丢掉正面三角形  
    rsDesc.DepthClipEnable = TRUE;//深度裁剪：开启后，超出深度范围 [0,1] 的像素会被裁掉
    hr = m_device->CreateRasterizerState(&rsDesc, m_rsNoCull.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create rasterizer state.", L"Error", MB_OK);
        return false;
    }


    m_constantBuffer = BufferUtils::CreateConstantBuffer(m_device.Get(), sizeof(PerFrameCB));
    if (!m_constantBuffer) {
        MessageBoxW(m_hwnd, L"Failed to create PerFrame constant buffer.", L"Error", MB_OK);
        return false;
    }

    m_camera = std::make_unique<Camera>();

    m_timer = std::make_unique<Timer>();

    m_inputController = std::make_unique<InputController>();

    m_ground = std::make_unique<Ground>();
    if (!m_ground->Initialize(m_device.Get(), 50, 50.0f)) { 
        MessageBoxW(m_hwnd, L"Failed to create ground.", L"Error", MB_OK); 
        return false;
    }

    m_player = std::make_unique<Player>();
    std::string modelSrc = "assets/model";
    m_player->Load(m_device.Get(), m_context.Get(), modelSrc);


    return true;
}

void GraphicsApp::OnResize(UINT width, UINT height) {
    if (!m_swapChain || !m_device || !m_context || width == 0 || height == 0) {
        return;
    }

    m_context->OMSetRenderTargets(0, nullptr, nullptr);
    m_rtv.Reset();
    m_depthStencilView.Reset();
    m_depthStencilBuffer.Reset();
    m_canvasRTV.Reset();
    m_canvasSRV.Reset();
    m_canvasDSV.Reset();
    m_canvasTex.Reset();
    m_canvasDepth.Reset();

    HRESULT hr;

    hr = m_swapChain->ResizeBuffers(0, width, height, DXGI_FORMAT_UNKNOWN, 0);
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to resize swap chain buffers.", L"Error", MB_OK);
        return;
    }

    Microsoft::WRL::ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), &backBuffer);
    if (FAILED(hr)) { return; }
    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, m_rtv.GetAddressOf());
    if (FAILED(hr)) { return; }


    m_canvasWidth = width;
    m_canvasHeight = height;
    UpdateProjectionMatrix();


    D3D11_TEXTURE2D_DESC depthBufferDesc = {};               // 深度/模板纹理的描述结构体，零初始化
    depthBufferDesc.Width = width; // 纹理宽度：与窗口客户区宽一致
    depthBufferDesc.Height = height;// 纹理高度：与窗口客户区高一致
    depthBufferDesc.MipLevels = 1;                           // 仅 1 个 mip 级别（深度贴图通常不需要多级）
    depthBufferDesc.ArraySize = 1;                           // 非数组纹理
    depthBufferDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;  // 24 位深度 + 8 位模板（常用且够用）,可选替代：D32_FLOAT（高精度、无模板）、D16_UNORM（轻量）
    depthBufferDesc.SampleDesc.Count = 1;                    // 采样数=1（无 MSAA；若颜色缓冲是 MSAA，这里必须匹配）
    depthBufferDesc.SampleDesc.Quality = 0;                  // 采样质量=0（与 Count=1 搭配）
    depthBufferDesc.Usage = D3D11_USAGE_DEFAULT;             // 默认用法：GPU 读写
    depthBufferDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;    // 绑定为深度/模板目标（DSV）
    depthBufferDesc.CPUAccessFlags = 0;                      // CPU 不访问
    depthBufferDesc.MiscFlags = 0;

    hr = m_device->CreateTexture2D(                    // 创建深度/模板纹理资源
        &depthBufferDesc, nullptr, m_depthStencilBuffer.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create depth stencil buffer.", L"Error", MB_OK);
        return;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc = {};             // DSV（深度/模板视图）描述
    dsvDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;          // 视图格式需与资源兼容（或资源用 typeless、视图用 typed）
    dsvDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;   // 非 MSAA 的 2D 视图（若 MSAA 用 TEXTURE2DMS）
    dsvDesc.Texture2D.MipSlice = 0;                          // 绑定 mip 级别 0
    hr = m_device->CreateDepthStencilView(             // 从纹理创建 DSV 视图
        m_depthStencilBuffer.Get(), &dsvDesc, m_depthStencilView.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create depth stencil view.", L"Error", MB_OK);
        return;
    }


    D3D11_TEXTURE2D_DESC cdesc = {};
    cdesc.Width = width;
    cdesc.Height = height;
    cdesc.MipLevels = 1;                            
    cdesc.ArraySize = 1;                            
    cdesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;      
    cdesc.SampleDesc.Count = 1;                     
    cdesc.Usage = D3D11_USAGE_DEFAULT;             
    cdesc.BindFlags = D3D11_BIND_RENDER_TARGET 
        | D3D11_BIND_SHADER_RESOURCE;

    hr = m_device->CreateTexture2D(&cdesc, nullptr, m_canvasTex.GetAddressOf());
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create canvas texture.", L"Error", MB_OK);
        return;
    }

    hr = m_device->CreateRenderTargetView(m_canvasTex.Get(), nullptr, m_canvasRTV.GetAddressOf());//从该纹理创建渲染目标视图（RTV）
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create m_canvasRTV.", L"Error", MB_OK);
        return;
    }
    hr = m_device->CreateShaderResourceView(m_canvasTex.Get(), nullptr, m_canvasSRV.GetAddressOf());//从该纹理创建着色器资源视图（SRV）
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create m_canvasSRV.", L"Error", MB_OK);
        return;
    }



    D3D11_TEXTURE2D_DESC depthBufferDesc_RTT = {};
    depthBufferDesc_RTT.Width = m_canvasWidth;
    depthBufferDesc_RTT.Height = m_canvasHeight;
    depthBufferDesc_RTT.MipLevels = 1;
    depthBufferDesc_RTT.ArraySize = 1;
    depthBufferDesc_RTT.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
    depthBufferDesc_RTT.SampleDesc.Count = 1;
    depthBufferDesc_RTT.SampleDesc.Quality = 0;
    depthBufferDesc_RTT.Usage = D3D11_USAGE_DEFAULT;
    depthBufferDesc_RTT.BindFlags = D3D11_BIND_DEPTH_STENCIL;
    depthBufferDesc_RTT.CPUAccessFlags = 0;
    depthBufferDesc_RTT.MiscFlags = 0;

    hr = m_device->CreateTexture2D(
        &depthBufferDesc_RTT, nullptr, m_canvasDepth.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create depth stencil buffer RTT.", L"Error", MB_OK);
        return;
    }

    D3D11_DEPTH_STENCIL_VIEW_DESC dsvDesc_RTT = {};             
    dsvDesc_RTT.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;          
    dsvDesc_RTT.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;   
    dsvDesc_RTT.Texture2D.MipSlice = 0;                          
    hr = m_device->CreateDepthStencilView(            
        m_canvasDepth.Get(), &dsvDesc_RTT, m_canvasDSV.GetAddressOf()
    );
    if (FAILED(hr)) {
        MessageBoxW(m_hwnd, L"Failed to create depth stencil view RTT.", L"Error", MB_OK);
        return;
    }

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<FLOAT>(width);
    vp.Height = static_cast<FLOAT>(height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &vp);
}

void GraphicsApp::UpdateProjectionMatrix() {
    if (m_canvasHeight == 0) m_canvasHeight = 1;

    m_aspectRatio = static_cast<float>(m_canvasWidth) / static_cast<float>(m_canvasHeight);

    DirectX::XMMATRIX projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(
        m_fieldOfView,
        m_aspectRatio,
        m_nearPlane,
        m_farPlane
    );

    DirectX::XMStoreFloat4x4(&m_projection, projectionMatrix);
}

void GraphicsApp::Run()
{
    m_timer->Reset();
    MSG msg = {};
    while (true)
    {
        if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else
        {
            m_timer->Tick();
            float deltaTime = m_timer->GetDeltaTime();

            Update(deltaTime);
            Render();
        }
    }
}

void GraphicsApp::Update(float deltaTime) {
    
    ProcessInputAndUpdateWorld(deltaTime);
    
}

void GraphicsApp::Render() {
    if (!m_context) return;

    D3D11_VIEWPORT canvasVP = {};
    canvasVP.TopLeftX = 0.0f;
    canvasVP.TopLeftY = 0.0f;
    canvasVP.Width = static_cast<FLOAT>(m_canvasWidth);
    canvasVP.Height = static_cast<FLOAT>(m_canvasHeight);
    canvasVP.MinDepth = 0.0f;
    canvasVP.MaxDepth = 1.0f;
    m_context->RSSetViewports(1, &canvasVP);

    const float canvasClearColor[4] = { 0.337f, 0.322f, 0.322f, 1.0f };
    m_context->ClearRenderTargetView(m_canvasRTV.Get(), canvasClearColor);
    m_context->ClearDepthStencilView(m_canvasDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);

    UpdateFrameConstantBuffer();

    ID3D11RenderTargetView* rtCanvas[] = { m_canvasRTV.Get() };
    m_context->OMSetRenderTargets(1, rtCanvas, m_canvasDSV.Get());

    m_context->IASetInputLayout(m_inputLayout.Get());
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);//三角形
    m_context->OMSetDepthStencilState(m_depthStateDefault.Get(), 0);
    m_context->OMSetBlendState(nullptr, nullptr, 0xffffffff);//不透明物体，关闭混合

    ID3D11SamplerState* sams0[] = { m_samplerState.Get() };
    m_context->PSSetSamplers(0, 1, sams0);

    if (m_ground) {
        m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
        m_ground->Render(m_context.Get(), m_camera->GetViewMatrix(), DirectX::XMLoadFloat4x4(&m_projection), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));
    }

    m_context->IASetInputLayout(m_playerInputLayout.Get());
    m_context->VSSetShader(m_playerVS.Get(), nullptr, 0);
    m_context->PSSetShader(m_playerPS.Get(), nullptr, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_player->Render(m_context.Get(), m_camera->GetViewMatrix(), XMLoadFloat4x4(&m_projection), DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f));



    RECT rect;
    GetClientRect(m_hwnd, &rect);
    D3D11_VIEWPORT finalVP = {};
    finalVP.Width = static_cast<float>(rect.right - rect.left);
    finalVP.Height = static_cast<float>(rect.bottom - rect.top);
    finalVP.MaxDepth = 1.0f;

    m_context->RSSetViewports(1, &finalVP);
    ID3D11RenderTargetView* rtFinal[] = { m_rtv.Get() };
    m_context->OMSetRenderTargets(1, rtFinal, nullptr);

    DrawFullscreenTexture(m_canvasSRV.Get());

    m_swapChain->Present(1, 0);
}

void GraphicsApp::DrawFullscreenTexture(ID3D11ShaderResourceView* srv)
{
    if (!srv) return;
    m_context->RSSetState(m_rsNoCull.Get());// 关闭背面剔除
    m_context->IASetInputLayout(m_presentInputLayout.Get());
    m_context->VSSetShader(m_presentVS.Get(), nullptr, 0);
    m_context->PSSetShader(m_presentPS.Get(), nullptr, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    m_context->OMSetBlendState(nullptr, nullptr, 0xffffffff);
    m_context->OMSetDepthStencilState(m_depthStateDisabled.Get(), 0);

    ID3D11ShaderResourceView* srvs[] = { srv };
    m_context->PSSetShaderResources(0, 1, srvs);
    ID3D11SamplerState* sams[] = { m_samplerState.Get() };
    m_context->PSSetSamplers(0, 1, sams);

    const UINT stride = sizeof(PresentVertex);
    const UINT offset = 0;
    ID3D11Buffer* vbs[] = { m_presentVB.Get() };
    m_context->IASetVertexBuffers(0, 1, vbs, &stride, &offset);
    m_context->IASetIndexBuffer(m_presentIB.Get(), DXGI_FORMAT_R32_UINT, 0);

    m_context->DrawIndexed(6, 0, 0);

    ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
    m_context->PSSetShaderResources(0, 1, nullSRV);
}


void GraphicsApp::UpdateFrameConstantBuffer()
{
    PerFrameCB frameData = {};

    DirectX::XMVECTOR lightDirVec = DirectX::XMLoadFloat3(&m_lightDirection);
    DirectX::XMStoreFloat3(&frameData.lightDirWS, DirectX::XMVector3Normalize(lightDirVec));

    frameData.ambientColor = DirectX::XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
    frameData.lightColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

    D3D11_MAPPED_SUBRESOURCE mappedResource;
    m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    memcpy(mappedResource.pData, &frameData, sizeof(PerFrameCB));
    m_context->Unmap(m_constantBuffer.Get(), 0);

    m_context->PSSetConstantBuffers(1, 1, m_constantBuffer.GetAddressOf());
}

void GraphicsApp::OnRawMouseMove(long dx, long dy)
{
    if (m_inputController) {
        m_inputController->OnRawMouseMove(dx, dy);
    }
}


void GraphicsApp::ProcessInputAndUpdateWorld(float deltaTime)
{
    POINT mouseDelta = m_inputController->GetMouseDelta();

    const float mouseSens = 0.0015f;
    float dYaw = mouseDelta.x * mouseSens;
    float dPitch = mouseDelta.y * mouseSens;

    const float Epsilon = 1e-6f;// エプシロン

    if (m_camera)
    {
        m_camera->Rotate(dYaw, dPitch);
    }

    if (m_player && m_inputController && m_camera) {

        bool w = m_inputController->IsKeyPressed('W');
        bool a = m_inputController->IsKeyPressed('A');
        bool s = m_inputController->IsKeyPressed('S');
        bool d = m_inputController->IsKeyPressed('D');

        bool wasDiagonalLastFrame = (m_w_pressed_lastFrame && (m_a_pressed_lastFrame || m_d_pressed_lastFrame)) ||
            (m_s_pressed_lastFrame && (m_a_pressed_lastFrame || m_d_pressed_lastFrame));

        bool isDiagonalThisFrame = (w && (a || d)) || (s && (a || d));

        if (wasDiagonalLastFrame && !isDiagonalThisFrame && m_stopGracePeriodTimer <= 0.0f) {
            m_stopGracePeriodTimer = 0.1f; 
        }

        DirectX::XMVECTOR finalMoveDirection;
        bool isMoving = false;

        if (m_stopGracePeriodTimer > 0.0f)
        {
            m_stopGracePeriodTimer -= deltaTime;

            finalMoveDirection = m_lastMoveDirection;

        }
        else
        {
            float cameraYaw = m_camera->GetYaw();// camera方向
            DirectX::XMMATRIX cameraRotation = DirectX::XMMatrixRotationY(cameraYaw);// 根据方向创建旋转矩阵
            DirectX::XMVECTOR forwardVec = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(0, 0, 1, 0), cameraRotation);// camera正前方的单位方向向量
            DirectX::XMVECTOR rightVec = DirectX::XMVector3TransformNormal(DirectX::XMVectorSet(1, 0, 0, 0), cameraRotation);// camera正右方的单位方向向量

            DirectX::XMVECTOR currentMoveDirection = DirectX::XMVectorZero();// 零向量 (0.0f, 0.0f, 0.0f, 0.0f)
            if (w) currentMoveDirection = DirectX::XMVectorAdd(currentMoveDirection, forwardVec);
            if (s) currentMoveDirection = DirectX::XMVectorSubtract(currentMoveDirection, forwardVec);
            if (a) currentMoveDirection = DirectX::XMVectorSubtract(currentMoveDirection, rightVec);
            if (d) currentMoveDirection = DirectX::XMVectorAdd(currentMoveDirection, rightVec);

            finalMoveDirection = currentMoveDirection;

            isMoving = (DirectX::XMVectorGetX(DirectX::XMVector3LengthSq(finalMoveDirection)) > Epsilon);
            if (isMoving) {
                m_lastMoveDirection = finalMoveDirection;
            }
        }

        DirectX::XMFLOAT3 targetVelocity = { 0.0f, 0.0f, 0.0f };
        if (isMoving) {
            float currentYaw = m_player->GetRotation().y;
            float targetYaw = atan2f(DirectX::XMVectorGetX(finalMoveDirection), DirectX::XMVectorGetZ(finalMoveDirection));// 根据相机朝向和玩家按键计算目标角度
            float angleDiff = targetYaw - currentYaw;
            while (angleDiff > DirectX::XM_PI) { angleDiff -= DirectX::XM_2PI; }
            while (angleDiff < -DirectX::XM_PI) { angleDiff += DirectX::XM_2PI; }
            const float turnSpeed = 15.0f;
            float newYaw = currentYaw + angleDiff * turnSpeed * deltaTime;
            m_player->SetRotationY(newYaw);

            DirectX::XMVECTOR moveDirNormalized = DirectX::XMVector3Normalize(finalMoveDirection);// vector正規化, 防止斜方向移动快
            const float playerSpeed = 5.0f;
            DirectX::XMVECTOR velocity = DirectX::XMVectorScale(moveDirNormalized, playerSpeed);
            DirectX::XMStoreFloat3(&targetVelocity, velocity);

            m_player->PlayAnimation("Run");
        }
        m_player->SetTargetVelocity(targetVelocity);
        m_player->Update(deltaTime);

        m_w_pressed_lastFrame = w;
        m_a_pressed_lastFrame = a;
        m_s_pressed_lastFrame = s;
        m_d_pressed_lastFrame = d;
    }

    if (m_camera && m_player)
    {
        m_camera->Update(m_player->GetPosition());
    }

    m_inputController->EndFrame();
}

