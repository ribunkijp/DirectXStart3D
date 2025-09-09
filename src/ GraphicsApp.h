/**********************************************************************************
 * GraphicsApp.h
 *
 *
 *
 * LI WENHUI
 * 2025/09/09
 **********************************************************************************/

#ifndef GRAPHICSAPP_H
#define GRAPHICSAPP_H

#include <d3d11.h>
#include <DirectXMath.h>


class GraphicsApp {
public:
    GraphicsApp();
    ~GraphicsApp();

    bool Initialize(HWND hwnd);



private:
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerStateTile;// tile
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilStateTransparent;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStateNormal;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStateAdditive;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStateMultiply;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStateScreen;

    HWND m_hwnd = nullptr;

    const float m_logicalWidth = 1856.0f;
    const float m_logicalHeight = 1044.0f;
    const float m_SCALE_FACTOR = 1.0f;

    UINT m_canvasWidth = 0;
    UINT m_canvasHeight = 0;


};










#endif 