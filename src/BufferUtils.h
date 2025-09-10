/**********************************************************************************
 * BufferUtils.h
 *
 *
 *
 * LI WENHUI
 * 2025/09/10
 **********************************************************************************/

#include <d3d11.h>
#include <wrl/client.h>

struct Vertex;


namespace BufferUtils
{
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateDynamicVertexBuffer(ID3D11Device* device, size_t byteWidth);
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateDynamicIndexBuffer(ID3D11Device* device, size_t byteWidth);
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateStaticVertexBuffer(ID3D11Device* device, const void* pVertexData, size_t byteWidth);
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateStaticIndexBuffer(ID3D11Device* device, const void* pIndexData, size_t byteWidth);

    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer(ID3D11Device* device, size_t byteWidth);
}