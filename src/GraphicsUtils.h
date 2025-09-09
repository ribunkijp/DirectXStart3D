/**********************************************************************************
 * GraphicsUtils.h
 *
 *
 *
 * LI WENHUI
 * 2025/09/09
 **********************************************************************************/

#ifndef GRAPHICSUTILS_H
#define GRAPHICSUTILS_H

#include <dxgi1_6.h>   // For IDXGIAdapter1
#include <wrl/client.h> // For ComPtr



namespace GraphicsUtils
{
    Microsoft::WRL::ComPtr<IDXGIAdapter1> SelectBestAdapter();
}



#endif