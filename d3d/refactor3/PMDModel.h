#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ConstantBuffer.h"
#include "GlobalDescriptorHeap1.h"

class PMDModel
{
    struct PMDHeader
    {
        float version;
        char model_name[20];
        char comment[256];
    };

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

    struct TransformMatrix
    {
        DirectX::XMMATRIX world;
    };

public:

#pragma pack(push, 1)
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

    void Read();
    void Render();
    void SetIA() const;

    PMDModel(std::string filepath, const std::shared_ptr<GlobalDescriptorHeap1>& globalHeap)
        : str_model_path_(filepath),
        num_vertices_(0),
        num_indices_(0),
        num_materials_(0),
        globalHeap(globalHeap),
        m_materialHeapId(-1),
        m_matrixHeapId(-1),
        vertex_buffer_view_({}),
        index_buffer_view_({})
    {
    }

private:
    static HRESULT ReadHeader(FILE* fp);
    HRESULT ReadVertices(FILE* fp);
    HRESULT ReadIndices(FILE* fp);
    HRESULT ReadMaterials(FILE* fp);

    HRESULT SetMaterialBuffer();
    HRESULT SetVertexBuffer();
    HRESULT SetIndexBuffer();
    HRESULT SetTransformBuffer();

    std::string str_model_path_;

    std::vector<PMDVertex> vertices_;
    std::vector<unsigned short> indices_;
    std::vector<PMDMaterial> pmd_materials_;

    std::vector<Material> materials_;

    unsigned int num_vertices_;
    unsigned int num_indices_;
    unsigned int num_materials_;

    std::vector<ID3D12Resource*> textures_;
    std::vector<ID3D12Resource*> sph_;
    std::vector<ID3D12Resource*> spa_;
    std::vector<ID3D12Resource*> toon_;

    std::shared_ptr<GlobalDescriptorHeap1> globalHeap;
    GLOBAL_HEAP_ID m_materialHeapId;
    GLOBAL_HEAP_ID m_matrixHeapId;

    const size_t pmd_vertex_size = 38;

    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view_;

    ConstantBuffer<PMDVertex> vertex_buffer_;
    ConstantBuffer<unsigned short> index_buffer_;
    ConstantBuffer<TransformMatrix> matrix_buffer_;

    std::map<std::string, ID3D12Resource*> resourceTable;

    float angle = 0.0f;
};

