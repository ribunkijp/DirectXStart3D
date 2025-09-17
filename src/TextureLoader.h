#ifndef TEXTURELOADER_H
#define TEXTURELOADER_H

/**********************************************************************************
 * TextureLoader.h
 *
 *
 *
 * LI WENHUI
 * 2025/09/17
 **********************************************************************************/

#include <d3d11.h>
#include <DirectXTex.h>
#include <wrl/client.h>



namespace TextureLoader
{
    /**
     * @brief 从文件加载纹理并创建 SRV（支持 WIC：png/jpg/bmp/tiff...）
     * @param device   D3D11 设备
     * @param filename 宽字符文件路径
     * @param srv      输出：着色器资源视图（调用方负责释放）
     * @param outWidth  输出：宽度（可选）
     * @param outHeight 输出：高度（可选）
     */
    HRESULT LoadTextureAndCreateSRV(
        ID3D11Device* device,
        const wchar_t* filename,
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>& srv,
        float* outWidth = nullptr,
        float* outHeight = nullptr);
}

#endif