#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <map>
#include <string>
#include <vector>

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

public:
    void Read();
    void Render();

    PMDModel(std::string filepath, ID3D12Device* dev);

private:
    HRESULT ReadHeader(FILE* fp);
    HRESULT ReadVertices(FILE* fp);
    HRESULT ReadIndices(FILE* fp);
    HRESULT ReadMaterials(FILE* fp);

    HRESULT SetMaterialBuffer();
    HRESULT SetVertexBuffer();
    HRESULT SetIndexBuffer();

    std::string str_model_path_;

    std::vector<unsigned char> vertices_;
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

    ID3D12Device* m_device;
    ID3D12DescriptorHeap* m_materialDescriptorHeap;

    const size_t pmd_vertex_size = 38;

    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view_;

    std::map<std::string, ID3D12Resource*> resourceTable;
};

