#pragma once
#include <d3d12.h>
#include <map>
#include <string>

ID3D12Resource* LoadTextureFromFile(
    const std::string& texturePath,
    std::map<std::string, ID3D12Resource*>& _resourceTable,
    ID3D12Device* _dev
);
ID3D12Resource* CreateWhiteTexture(ID3D12Device* _dev);
ID3D12Resource* CreateBlackTexture(ID3D12Device* _dev);
ID3D12Resource* CreateGrayGradationTexture(ID3D12Device* _dev);

template <class T> void SafeRelease(T** t)
{
    if (*t)
    {
        (*t)->Release();
        *t = nullptr;
    }
}