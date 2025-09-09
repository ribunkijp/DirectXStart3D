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


    return true;
}