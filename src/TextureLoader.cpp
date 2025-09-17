
/**********************************************************************************
 * TextureLoader.cpp
 *
 *
 *
 * LI WENHUI
 * 2025/09/17
 **********************************************************************************/

#include "pch.h"
#include "TextureLoader.h"

namespace TextureLoader
{
    HRESULT LoadTextureAndCreateSRV(ID3D11Device* device, const wchar_t* filename, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv, float* outWidth, float* outHeight) {
        if (!device || !filename) return E_INVALIDARG;
        srv.Reset();

        DirectX::TexMetadata metadata;
        DirectX::ScratchImage scratchImage;
        HRESULT hr = S_OK;

        // 判断是否是 DDS 文件
        std::wstring path(filename);
        std::transform(path.begin(), path.end(), path.begin(), ::towlower);
        bool isDDS = (path.size() >= 4 && path.substr(path.size() - 4) == L".dds");

        if (isDDS) {
            hr = DirectX::LoadFromDDSFile(filename, DirectX::DDS_FLAGS_NONE, &metadata, scratchImage);
        }
        else {
            hr = DirectX::LoadFromWICFile(filename, DirectX::WIC_FLAGS_NONE, &metadata, scratchImage);
        }
        if (FAILED(hr)) return hr;

        if (outWidth)  *outWidth = static_cast<float>(metadata.width);
        if (outHeight) *outHeight = static_cast<float>(metadata.height);

        HRESULT hrCreate = DirectX::CreateShaderResourceView(
            device,
            scratchImage.GetImages(),
            scratchImage.GetImageCount(),
            metadata,
            srv.ReleaseAndGetAddressOf()
        );
        return hrCreate;
    }
}