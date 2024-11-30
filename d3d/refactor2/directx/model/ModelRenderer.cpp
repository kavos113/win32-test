#include "ModelRenderer.h"

#include <d3dcompiler.h>
#include <string>
#include <tchar.h>

HRESULT ModelRenderer::CompileShaders()
{
    ID3D10Blob* errorBlob = nullptr;

    HRESULT hr = D3DCompileFromFile(
        L"BasicVertexShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "BasicVS",
        "vs_5_0",
        D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG,
        0,
        &vsblob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to compile vertex shader\n"));

        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            OutputDebugString(_T("File not found\n"));
            return 0;
        }

        std::string errStr;
        errStr.resize(errorBlob->GetBufferSize());
        std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errStr.begin());

        OutputDebugStringA(errStr.c_str());
        return 1;
    }

    hr = D3DCompileFromFile(
        L"BasicPixelShader.hlsl",
        nullptr,
        D3D_COMPILE_STANDARD_FILE_INCLUDE,
        "BasicPS",
        "ps_5_0",
        D3DCOMPILE_SKIP_OPTIMIZATION | D3DCOMPILE_DEBUG,
        0,
        &psblob,
        &errorBlob
    );
    if (FAILED(hr))
    {
        OutputDebugString(_T("Failed to compile pixel shader\n"));

        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            OutputDebugString(_T("File not found\n"));
            return 0;
        }

        std::string errStr;
        errStr.resize(errorBlob->GetBufferSize());
        std::copy_n((char*)errorBlob->GetBufferPointer(), errorBlob->GetBufferSize(), errStr.begin());

        OutputDebugStringA(errStr.c_str());
        return 1;
    }

    return S_OK;
}
