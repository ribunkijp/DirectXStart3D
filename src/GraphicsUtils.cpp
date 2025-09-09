/**********************************************************************************
 * GraphicsUtils.cpp
 *
 *
 *
 * LI WENHUI
 * 2025/09/10
 **********************************************************************************/

#include "Pch.h"
#include "GraphicsUtils.h"
#include <stdio.h> 


namespace GraphicsUtils
{
    /**
     * @brief 遍历系统上所有的GPU适配器，并根据专用显存大小挑选出性能最好的一个。
     * @param
     * @return 指向最佳适配器的 ComPtr 智能指针。如果找不到合适的硬件适配器，则返回 nullptr。
     */
    Microsoft::WRL::ComPtr<IDXGIAdapter1> SelectBestAdapter()
    {
        Microsoft::WRL::ComPtr<IDXGIFactory1> dxgiFactory;
        // 创建 DXGI Factory 接口，这是枚举适配器的起点
        if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&dxgiFactory)))) {
            // 如果创建失败，无法继续，返回空指针
            return nullptr;
        }

        std::vector<Microsoft::WRL::ComPtr<IDXGIAdapter1>> adapters;
        Microsoft::WRL::ComPtr<IDXGIAdapter1> tempAdapter;

        // 遍历所有可用的适配器，并将它们存入一个 vector
        for (UINT i = 0; dxgiFactory->EnumAdapters1(i, &tempAdapter) != DXGI_ERROR_NOT_FOUND; ++i) {
            adapters.push_back(tempAdapter);
        }

        // 如果没有找到任何适配器，返回空指针
        if (adapters.empty()) {
            return nullptr;
        }

        // 根据专用视频内存（Dedicated Video Memory）对适配器进行降序排序
        std::sort(adapters.begin(), adapters.end(), [](const Microsoft::WRL::ComPtr<IDXGIAdapter1>& a, const Microsoft::WRL::ComPtr<IDXGIAdapter1>& b) {
            DXGI_ADAPTER_DESC1 descA = {};
            a->GetDesc1(&descA);

            DXGI_ADAPTER_DESC1 descB = {};
            b->GetDesc1(&descB);

            return descA.DedicatedVideoMemory > descB.DedicatedVideoMemory;
            });

        // 排序后，第一个是显存最大的独立显卡。
        // 过滤掉“Microsoft基本呈现驱动程序”(软件模拟的显卡)
        for (const auto& adapter : adapters) {
            DXGI_ADAPTER_DESC1 desc = {};
            adapter->GetDesc1(&desc);

            // 检查描述字符串中是否不包含 "Microsoft Basic Render Driver"
            if (wcsstr(desc.Description, L"Microsoft Basic Render Driver") == nullptr) {
                // 找到第一个不是软件模拟的、性能最好的硬件适配器
                wchar_t buffer[256];
                swprintf_s(buffer, L"Selected GPU: %s\nDedicated Memory: %llu MB\n", desc.Description, desc.DedicatedVideoMemory / (1024 * 1024));
                //OutputDebugStringW(buffer); // 打印选中的显卡信息

                return adapter;
            }
        }

        // 没找到合适的硬件显卡，返回空指针
        return nullptr;
    }
}