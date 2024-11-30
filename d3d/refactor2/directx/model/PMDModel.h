#pragma once

#include <d3d12.h>
#include <DirectXMath.h>
#include <map>
#include <string>
#include <vector>
#include <Windows.h>

#include "Model.h"
#include "directx/DXDescriptorHeap.h"

class PMDModel :
    public Model
{
    struct PMDHeader
    {
        float version;
        char model_name[20];
        char comment[256];
    };

#pragma pack(push, 1) // for padding
    struct PMDVertex
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT3 normal;
        DirectX::XMFLOAT2 uv;
        uint16_t bone_num[2];
        uint8_t bone_weight;
        uint8_t edge_flag;
        uint16_t dummy;
    };
#pragma pack(pop)

#pragma pack(1) // for padding
    struct PMDMaterial
    {
        DirectX::XMFLOAT3 diffuse;
        float alpha;
        float specularity;
        DirectX::XMFLOAT3 specular;
        DirectX::XMFLOAT3 ambient;
        unsigned char toon_index;
        unsigned char edge_flag;
        unsigned int indices_count;
        char texture_file[20];
    };
#pragma pack()

    struct MaterialForHlsl
    {
        DirectX::XMFLOAT3 diffuse;
        float alpha;
        DirectX::XMFLOAT3 specular;
        float specularity;
        DirectX::XMFLOAT3 ambient;
    };

    struct AdditionalMaterial
    {
        std::string texture_file;
        int toon_index;
        bool edge_flag;
    };

    struct Material
    {
        unsigned int indices_count;
        MaterialForHlsl material;
        AdditionalMaterial additional;
    };

    struct ModelMatrix
    {
        DirectX::XMMATRIX world;
    };

public:
    PMDModel(const std::string& path)
        : Model(path)
    {

    }

    void Render() override;
    void Read() override;

private:
    HRESULT ReadHeader(FILE* fp);
    HRESULT ReadVertices(FILE* fp);
    HRESULT ReadIndices(FILE* fp);
    HRESULT ReadMaterials(FILE* fp);

    HRESULT SetMaterialBuffer();
    HRESULT SetVertexBuffer();
    HRESULT SetIndexBuffer();
    HRESULT SetMatrixBuffer();

    std::vector<PMDVertex> vertices_;
    std::vector<unsigned short> indices_;
    std::vector<PMDMaterial> pmd_materials_;
    std::vector<Material> materials_;

    std::vector<ID3D12Resource*> textures_;
    std::vector<ID3D12Resource*> sph_;
    std::vector<ID3D12Resource*> spa_;
    std::vector<ID3D12Resource*> toon_;

    DXDescriptorHeap material_heap_;
    DXDescriptorHeap matrix_heap_;

    const size_t pmd_vertex_size_ = 38;

    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view_;

    std::map<std::string, ID3D12Resource*> resourceTable;

    ModelMatrix* matrix_buffer_map_;
};

