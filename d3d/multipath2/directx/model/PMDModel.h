#pragma once
#include <d3d12.h>
#include <DirectXMath.h>
#include <map>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "../descriptor_heap/DescriptorHeapSegmentManager.h"
#include "directx/buffer/ConstantBuffer.h"


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

#pragma pack(1) // for padding
    struct PMDBone
    {
        char bone_name[20];
        unsigned short parent_number;
        unsigned short next_number;
        unsigned char type;
        unsigned short ik_bone_number;
        DirectX::XMFLOAT3 position;
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

    struct BoneNode
    {
        uint32_t bone_index;
        uint32_t bone_type;
        uint32_t ik_parent_bone;
        DirectX::XMFLOAT3 start_position;
        DirectX::XMFLOAT3 end_position;
        std::vector<BoneNode*> children;
    };

    struct VMDMotion
    {
        char bone_name[15];
        unsigned int frame_number;
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT4 quaternion;
        unsigned char bezier[64];
    };

    struct KeyFrame
    {
        unsigned int frame_number;
        DirectX::XMVECTOR quaternion;
        DirectX::XMFLOAT2 p1, p2;
        KeyFrame(
            unsigned int frame_number,
            const DirectX::XMVECTOR& quaternion,
            const DirectX::XMFLOAT2& p1,
            const DirectX::XMFLOAT2& p2
        )
            : frame_number(frame_number),
            quaternion(quaternion),
            p1(p1),
            p2(p2)
        {
        }
    };

    struct PMDIK
    {
        uint16_t bone_index;
        uint16_t target_bone_index;
        uint16_t iterations;
        float limit_angle;
        std::vector<uint16_t> chain_bone_indices;
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
    void Render() const;
    void SetIA() const;
    void UpdateAnimation();

    void PlayAnimation();

    PMDModel(const std::string& filepath, DescriptorHeapSegmentManager& model_manager)
        : str_model_path_(filepath),
        num_vertices_(0),
        num_indices_(0),
        num_materials_(0),
        m_modelManager(model_manager),
        m_materialSegment(),
        m_matrixSegment(),
        vertex_buffer_view_({}),
        index_buffer_view_({})
    {
    }

private:
    static HRESULT ReadHeader(FILE* fp);
    HRESULT ReadVertices(FILE* fp);
    HRESULT ReadIndices(FILE* fp);
    HRESULT ReadMaterials(FILE* fp);
    HRESULT ReadBones(FILE* fp);
    HRESULT ReadIKs(FILE* fp);

    HRESULT ReadVMD(FILE* fp);

    HRESULT SetMaterialBuffer();
    HRESULT SetVertexBuffer();
    HRESULT SetIndexBuffer();
    HRESULT SetTransformBuffer();

    void RecursiveMatrixMultiply(const BoneNode* node, const DirectX::XMMATRIX& parent_matrix);

    void UpdateMotion();
    static float GetYFromXBezier(float x, const DirectX::XMFLOAT2& p1, const DirectX::XMFLOAT2& p2, uint8_t n = 10);

    void IKRotate();
    void IKCCD(const PMDIK& ik) const;
    void IKCos(const PMDIK& ik);
    void IKLookAt(const PMDIK& ik);

    static DirectX::XMMATRIX GetLookAtMatrix(const DirectX::XMVECTOR& look_at, const DirectX::XMFLOAT3& up, const DirectX::XMFLOAT3& right);
    static DirectX::XMMATRIX GetLookAtMatrix(const DirectX::XMVECTOR& origin, const DirectX::XMVECTOR& target, const DirectX::XMFLOAT3& up, const DirectX::XMFLOAT3& right);

    std::string str_model_path_;

    std::vector<PMDVertex> vertices_;
    std::vector<unsigned short> indices_;
    std::vector<PMDMaterial> pmd_materials_;
    std::vector<PMDBone> bones_;

    std::vector<Material> materials_;

    std::vector<DirectX::XMMATRIX> bone_matrices_;
    std::map<std::string, BoneNode> bone_nodes_table_;

    std::vector<std::string> bone_names;
    std::vector<BoneNode*> bone_nodes_;

    std::unordered_map<std::string, std::vector<KeyFrame>> motion_data_;

    std::vector<PMDIK> iks_;

    std::vector<uint32_t> knee_indices;

    unsigned int num_vertices_;
    unsigned int num_indices_;
    unsigned int num_materials_;

    std::vector<ID3D12Resource*> textures_;
    std::vector<ID3D12Resource*> sph_;
    std::vector<ID3D12Resource*> spa_;
    std::vector<ID3D12Resource*> toon_;

    DescriptorHeapSegmentManager& m_modelManager;

    DescriptorHeapSegment m_materialSegment;
    DescriptorHeapSegment m_matrixSegment;

    constexpr static size_t pmd_vertex_size = 38;

    D3D12_VERTEX_BUFFER_VIEW vertex_buffer_view_;
    D3D12_INDEX_BUFFER_VIEW index_buffer_view_;

    ConstantBuffer<PMDVertex> vertex_buffer_;
    ConstantBuffer<unsigned short> index_buffer_;
    ConstantBuffer<DirectX::XMMATRIX> matrix_buffer_;

    std::map<std::string, ID3D12Resource*> resourceTable;

    float angle = 0.0f;

    DWORD start_time_ = 0;
    unsigned int duration = 0;
};

