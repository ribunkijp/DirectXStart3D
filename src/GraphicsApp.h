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

class Camera;
class Timer;
class Player;
class InputController;


class GraphicsApp {
public:
    GraphicsApp();
    ~GraphicsApp();

    bool Initialize(HWND hwnd);
    void OnResize(UINT width, UINT height);
    void Run();
    void OnRawMouseMove(long dx, long dy);

private:
    void UpdateProjectionMatrix();
    void Update(float deltaTime);
    void Render();
    void DrawFullscreenTexture(ID3D11ShaderResourceView* srv);
    void UpdateFrameConstantBuffer();
    void ProcessInputAndUpdateWorld(float deltaTime);

private:
    Microsoft::WRL::ComPtr<ID3D11Device> m_device;
    Microsoft::WRL::ComPtr<ID3D11DeviceContext> m_context;
    Microsoft::WRL::ComPtr<IDXGISwapChain> m_swapChain;

    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_rtv;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_inputLayout;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> m_presentInputLayout;
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_pixelShader;
    Microsoft::WRL::ComPtr<ID3D11SamplerState> m_samplerState;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStateDefault;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStencilStateTransparent;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilState> m_depthStateDisabled;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_depthStencilView;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_depthStencilBuffer;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStateNormal;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStateAdditive;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStateMultiply;
    Microsoft::WRL::ComPtr<ID3D11BlendState> m_blendStateScreen;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_constantBuffer;

    HWND m_hwnd = nullptr;

    const float m_logicalWidth = 1856.0f;
    const float m_logicalHeight = 1044.0f;
    const float m_SCALE_FACTOR = 1.0f;

    // 离屏纹理
    UINT m_canvasWidth = 0;
    UINT m_canvasHeight = 0;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_canvasTex;
    Microsoft::WRL::ComPtr<ID3D11RenderTargetView> m_canvasRTV;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> m_canvasSRV;
    Microsoft::WRL::ComPtr<ID3D11DepthStencilView> m_canvasDSV;
    Microsoft::WRL::ComPtr<ID3D11Texture2D> m_canvasDepth;

    // present
    Microsoft::WRL::ComPtr<ID3D11VertexShader> m_presentVS;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> m_presentPS;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_presentVB = nullptr;
    Microsoft::WRL::ComPtr<ID3D11Buffer> m_presentIB = nullptr;
    
    // no cull
    Microsoft::WRL::ComPtr<ID3D11RasterizerState> m_rsNoCull;

    std::unique_ptr<Camera> m_camera;
    DirectX::XMFLOAT4X4 m_projection;
    float m_fieldOfView = DirectX::XM_PI / 4.0f;
    float m_nearPlane = 0.1f;
    float m_farPlane = 1000.0f;
    float m_aspectRatio = 1.0f;

    std::unique_ptr<Timer> m_timer = nullptr;

    std::unique_ptr<Player> m_player = nullptr;

    DirectX::XMFLOAT3 m_lightDirection = { -0.707f, -0.707f, 0.707f };//光来自右上方向(物体坐标-光源坐标)

    std::unique_ptr<InputController> m_inputController;


};










#endif 