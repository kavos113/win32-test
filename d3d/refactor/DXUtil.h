#pragma once
#include <d3d12.h>
#include <string>

ID3D12Resource* LoadTextureFromFile(std::string& texturePath);
ID3D12Resource* CreateWhiteTexture();
ID3D12Resource* CreateBlackTexture();
ID3D12Resource* CreateGrayGradationTexture();