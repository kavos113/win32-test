#pragma once
#include <memory>

#include "Display.h"
#include "DisplayMatrix.h"
#include "GlobalDescriptorHeap.h"
#include "PMDModel.h"
#include "PMDRenderer.h"

class DXProcess
{
public:

    struct SceneMatrix
    {
        DirectX::XMMATRIX world;
        DirectX::XMMATRIX view;
        DirectX::XMMATRIX proj;

        DirectX::XMFLOAT3 eye;
    };

    HRESULT Init();
    void Render();

    void SetHWND(HWND hwnd);

    DXProcess(HWND hwnd, RECT wr)
        : 
        display(hwnd, wr),
        displayMatrix(wr),
        wr(wr),
        hwnd(hwnd),
        model(nullptr)
    {
        
    }

private:
    static void EnableDebug();

    HRESULT OnRender();

    Display display;
    DisplayMatrix displayMatrix;

    std::shared_ptr<GlobalDescriptorHeap> globalHeap;

    RECT wr;
    HWND hwnd;

    std::unique_ptr<PMDModel> model;
    std::unique_ptr<PMDRenderer> renderer; 

    float angle = 0.0f;
};

