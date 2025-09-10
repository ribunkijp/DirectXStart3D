/**********************************************************************************
 * BufferUtils.cpp
 *
 *
 *
 * LI WENHUI
 * 2025/09/10
 **********************************************************************************/

#include "pch.h"
#include "BufferUtils.h"
#include "CommonTypes.h"

static UINT Align16(UINT size) { return (size + 15u) & ~15u; }

namespace BufferUtils
{
    /*
        dynamic
    */
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateDynamicVertexBuffer(ID3D11Device* device, size_t byteWidth) {
        if (!device) return nullptr;
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = static_cast<UINT>(byteWidth); //sizeof(Vertex) * vertices_count
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;
        // 
        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
        //
        HRESULT hr = device->CreateBuffer(&bd, nullptr, vertexBuffer.GetAddressOf());
        if (FAILED(hr)) {
            // 
            return nullptr;
        }
        return vertexBuffer;
    }
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateDynamicIndexBuffer(ID3D11Device* device, size_t byteWidth)
    {
        if (!device) return nullptr;
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = static_cast<UINT>(byteWidth);//indexCount * sizeof(UINT)              
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;
        // 
        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer = nullptr;
        // 
        HRESULT hr = device->CreateBuffer(&bd, nullptr, indexBuffer.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }
        //
        return indexBuffer;
    }


    /*
        static
    */
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateStaticVertexBuffer(ID3D11Device* device, const void* pVertexData, size_t byteWidth) {
        if (!device) return nullptr;
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT; //用于GPU读写，CPU不频繁更新
        bd.ByteWidth = static_cast<UINT>(byteWidth);
        bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        bd.CPUAccessFlags = 0; //CPU不需要访问权限
        bd.MiscFlags = 0;
        bd.StructureByteStride = 0;

        //在创建时提供初始数据
        D3D11_SUBRESOURCE_DATA initialData = {};
        initialData.pSysMem = pVertexData;

        Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer = nullptr;
        HRESULT hr = device->CreateBuffer(&bd, &initialData, vertexBuffer.GetAddressOf());
        if (FAILED(hr)) {
            return nullptr;
        }
        return vertexBuffer;
    }


    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateStaticIndexBuffer(ID3D11Device* device, const void* pIndexData, size_t byteWidth)
    {
        if (!device) return nullptr;
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DEFAULT;
        bd.ByteWidth = static_cast<UINT>(byteWidth);
        bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bd.CPUAccessFlags = 0;

        D3D11_SUBRESOURCE_DATA initialData = {};
        initialData.pSysMem = pIndexData;

        Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer = nullptr;
        HRESULT hr = device->CreateBuffer(&bd, &initialData, indexBuffer.GetAddressOf());
        if (FAILED(hr))
        {
            return nullptr;
        }
        return indexBuffer;
    }



    // ConstantBuffer
    Microsoft::WRL::ComPtr<ID3D11Buffer> CreateConstantBuffer(ID3D11Device* device, size_t byteWidth)
    {
        if (!device) return nullptr;
        D3D11_BUFFER_DESC bd = {};
        bd.Usage = D3D11_USAGE_DYNAMIC;
        bd.ByteWidth = Align16(static_cast<UINT>(byteWidth));
        bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        Microsoft::WRL::ComPtr<ID3D11Buffer> constantBuffer;
        if (FAILED(device->CreateBuffer(&bd, nullptr, constantBuffer.GetAddressOf()))) {
            return nullptr;
        }
        return constantBuffer;
    }
}