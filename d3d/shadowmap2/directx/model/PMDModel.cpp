#include "PMDModel.h"

#include <algorithm>
#include <array>
#include <ranges>
#include <tchar.h>
#include <valarray>

#include "Util.h"
#include "directx/DXUtil.h"
#include "directx/resources/DXCommand.h"
#include "directx/resources/DXDevice.h"
#include "directx/buffer/ConstantBuffer.h"

using DirectX::operator+;
using DirectX::operator-;

namespace 
{
    enum class BoneType
    {
        Rotation,
        RotAndMove,
        IK,
        Undefined,
        IKChild,
        RotationChild,
        IKDestination,
        Invisible
    };
}

void PMDModel::Read()
{
    FILE* fp;
    fopen_s(&fp, str_model_path_.c_str(), "rb");
    if (fp == nullptr)
    {
        OutputDebugString(_T("Failed to open file"));
        return;
    }

    HRESULT hr = ReadHeader(fp);
    if (FAILED(hr)) return;

    hr = ReadVertices(fp);
    if (FAILED(hr)) return;

    hr = ReadIndices(fp);
    if (FAILED(hr)) return;

    hr = ReadMaterials(fp);
    if (FAILED(hr)) return;

    hr = ReadBones(fp);
    if (FAILED(hr)) return;

    hr = ReadIKs(fp);
    if (FAILED(hr)) return;

    int ret = fclose(fp);
    if (ret != 0)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to close file\n"));
        return;
    }

    auto err = fopen_s(&fp, "motion/motion.vmd", "rb");
    if (err != 0)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to open motion file\n"));
        return;
    }

    hr = ReadVMD(fp);
    if (FAILED(hr)) return;

    ret = fclose(fp);
    if (ret != 0)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to close motion file\n"));
        return;
    }

    hr = SetVertexBuffer();
    if (FAILED(hr)) return;

    hr = SetIndexBuffer();
    if (FAILED(hr)) return;

    hr = SetTransformBuffer();
    if (FAILED(hr)) return;

    hr = SetMaterialBuffer();
    if (FAILED(hr)) return;
}

void PMDModel::Render(bool is_shadow) const
{
    DXCommand::GetCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    DXCommand::GetCommandList()->IASetVertexBuffers(0, 1, &vertex_buffer_view_);
    DXCommand::GetCommandList()->IASetIndexBuffer(&index_buffer_view_);

    m_modelManager.SetGraphicsRootDescriptorTable(m_matrixSegment.GetID());

    if (is_shadow)
    {
        DXCommand::GetCommandList()->DrawIndexedInstanced(num_indices_, 1, 0, 0, 0);
    }

    unsigned int indexOffset = 0;

    for (int i = 0; i < materials_.size(); ++i)
    {
        m_modelManager.SetGraphicsRootDescriptorTable(m_materialSegment.GetID(), i * 5);
        DXCommand::GetCommandList()->DrawIndexedInstanced(materials_[i].indices_count, 1, indexOffset, 0, 0);

        indexOffset += materials_[i].indices_count;
    }
}

void PMDModel::UpdateAnimation()
{
    UpdateMotion();
    std::ranges::copy(bone_matrices_, matrix_buffer_.GetMappedBuffer() + 1);
}

HRESULT PMDModel::ReadHeader(FILE* fp)
{
    PMDHeader header = {};
    char signatures[3] = {};
    size_t numRead = fread(signatures, sizeof(signatures), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read file\n"));
        return E_FAIL;
    }
    numRead = fread(&header, sizeof(header), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read pmd file\n"));
        return E_FAIL;
    }

    return S_OK;
}

HRESULT PMDModel::ReadVertices(FILE* fp)
{
    size_t numRead = fread(&num_vertices_, sizeof(num_vertices_), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read vertices\n"));
        return E_FAIL;
    }

    vertices_.resize(num_vertices_ );
    for (unsigned int i = 0; i < num_vertices_; ++i)
    {
        numRead = fread(&vertices_[i], pmd_vertex_size, 1, fp);
        if (numRead != 1)
        {
            OutputDebugString(_T("[PMDModel.cpp] Failed to read vertices\n"));
            return E_FAIL;
        }
    }

    return S_OK;
}

HRESULT PMDModel::ReadIndices(FILE* fp)
{
    size_t numRead = fread(&num_indices_, sizeof(num_indices_), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read indices\n"));
        return E_FAIL;
    }

    indices_.resize(num_indices_);
    numRead = fread(indices_.data(), indices_.size() * sizeof(indices_[0]), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read indices\n"));
        return E_FAIL;
    }

    return S_OK;
}

HRESULT PMDModel::ReadMaterials(FILE* fp)
{
    size_t numRead = fread(&num_materials_, sizeof(num_materials_), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read materials\n"));
        return E_FAIL;
    }

    pmd_materials_.resize(num_materials_);
    numRead = fread(pmd_materials_.data(), pmd_materials_.size() * sizeof(PMDMaterial), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read materials\n"));
        return E_FAIL;
    }

    materials_.resize(num_materials_);
    for (size_t i = 0; i < pmd_materials_.size(); ++i)
    {
        materials_[i].indices_count = pmd_materials_[i].indices_count;
        materials_[i].material.diffuse = pmd_materials_[i].diffuse;
        materials_[i].material.alpha = pmd_materials_[i].alpha;
        materials_[i].material.specular = pmd_materials_[i].specular;
        materials_[i].material.specularity = pmd_materials_[i].specularity;
        materials_[i].material.ambient = pmd_materials_[i].ambient;
        materials_[i].additional.toon_index = pmd_materials_[i].toon_index;
    }

    textures_.resize(num_materials_);
    sph_.resize(num_materials_);
    spa_.resize(num_materials_);
    toon_.resize(num_materials_);

    for (size_t i = 0; i < pmd_materials_.size(); ++i)
    {
        std::string toonFilePath = "toon/";
        char toonFileName[16] = {};

        sprintf_s(toonFileName, 16, "toon%02d.bmp", pmd_materials_[i].toon_index + 1);

        toonFilePath += toonFileName;

        toon_[i] = LoadTextureFromFile(toonFilePath, resourceTable);

        OutputDebugString(GetWideString(pmd_materials_[i].texture_file).c_str());
        OutputDebugString(_T("[PMDModel.cpp] -\n"));
        if (strlen(pmd_materials_[i].texture_file) == 0)
        {
            textures_[i] = nullptr;
        }

        std::string textureFileName = pmd_materials_[i].texture_file;
        std::string sphFileName = "";
        std::string spaFileName = "";

        if (std::ranges::count(textureFileName, '*') > 0)
        {
            auto namepair = SplitPath(textureFileName, '*');
            if (GetExtension(namepair.first) == "sph")
            {
                textureFileName = namepair.second;
                sphFileName = namepair.first;
            }
            else if (GetExtension(namepair.first) == "spa")
            {
                textureFileName = namepair.second;
                spaFileName = namepair.first;
            }
            else
            {
                textureFileName = namepair.first;
                if (GetExtension(namepair.second) == "sph")
                {
                    sphFileName = namepair.second;
                }
                else if (GetExtension(namepair.second) == "spa")
                {
                    spaFileName = namepair.second;
                }
            }
        }
        else
        {
            if (GetExtension(textureFileName) == "sph")
            {
                sphFileName = pmd_materials_[i].texture_file;
                textureFileName = "";
            }
            else if (GetExtension(textureFileName) == "spa")
            {
                spaFileName = pmd_materials_[i].texture_file;
                textureFileName = "";
            }
            else
            {
                sphFileName = "";
                spaFileName = "";
                textureFileName = pmd_materials_[i].texture_file;
            }
        }

        if (textureFileName != "")
        {
            std::string texturePath = GetTexturePathFromModelAndTexPath(
                str_model_path_,
                textureFileName.c_str()
            );
            textures_[i] = LoadTextureFromFile(texturePath, resourceTable);
        }
        else
        {
            textures_[i] = nullptr;
        }

        if (sphFileName != "")
        {
            OutputDebugString(_T("sph file found"));
            OutputDebugString(GetWideString(sphFileName + "\n").c_str());
            std::string sphPath = GetTexturePathFromModelAndTexPath(
                str_model_path_,
                sphFileName.c_str()
            );
            sph_[i] = LoadTextureFromFile(sphPath, resourceTable);
        }
        else
        {
            sph_[i] = nullptr;
        }

        OutputDebugString(GetWideString(spaFileName).c_str());

        if (spaFileName != "")
        {
            OutputDebugString(_T("spa file found"));
            OutputDebugString(GetWideString(spaFileName + "\n").c_str());
            std::string spaPath = GetTexturePathFromModelAndTexPath(
                str_model_path_,
                spaFileName.c_str()
            );
            spa_[i] = LoadTextureFromFile(spaPath, resourceTable);
        }
        else
        {
            spa_[i] = nullptr;
        }
    }

    return S_OK;
}

HRESULT PMDModel::ReadBones(FILE* fp)
{
    unsigned short num_bones = 0;
    size_t numRead = fread(&num_bones, sizeof(num_bones), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read bones\n"));
        return E_FAIL;
    }

    bones_.resize(num_bones);
    numRead = fread(bones_.data(), sizeof(PMDBone), num_bones, fp);
    if (numRead != num_bones)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read bones\n"));
        return E_FAIL;
    }

    std::vector<std::string> bone_names(bones_.size());

    bone_names.resize(bones_.size());
    bone_nodes_.resize(bones_.size());

    for (int i = 0; i < bones_.size(); ++i)
    {
        auto& bone = bones_[i];
        bone_names[i] = bone.bone_name;
        auto& node = bone_nodes_table_[bone.bone_name];
        node.bone_index = i;
        node.start_position = bone.position;

        bone_names[i] = bone.bone_name;
        bone_nodes_[i] = &node;
    }

    for (const auto & bone : bones_)
    {
        if (bone.parent_number >= bones_.size())
        {
            continue;
        }

        auto parent_name = bones_[bone.parent_number].bone_name;
        bone_nodes_table_[parent_name].children.emplace_back(&bone_nodes_table_[bone.bone_name]);
    }

    bone_matrices_.resize(bones_.size());

    std::ranges::fill(bone_matrices_, DirectX::XMMatrixIdentity());

    return S_OK;
}

HRESULT PMDModel::ReadIKs(FILE* fp)
{
    uint16_t num_iks;
    size_t numRead = fread(&num_iks, sizeof(num_iks), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read iks\n"));
        return E_FAIL;
    }

    iks_.resize(num_iks);

    for (auto& ik : iks_)
    {
        numRead = fread(&ik.bone_index, sizeof(ik.bone_index), 1, fp);
        if (numRead != 1)
        {
            OutputDebugString(_T("[PMDModel.cpp] Failed to read iks\n"));
            return E_FAIL;
        }

        numRead = fread(&ik.target_bone_index, sizeof(ik.target_bone_index), 1, fp);
        if (numRead != 1)
        {
            OutputDebugString(_T("[PMDModel.cpp] Failed to read iks\n"));
            return E_FAIL;
        }

        uint8_t chain_length = 0;
        numRead = fread(&chain_length, sizeof(chain_length), 1, fp);
        if (numRead != 1)
        {
            OutputDebugString(_T("[PMDModel.cpp] Failed to read iks\n"));
            return E_FAIL;
        }

        ik.chain_bone_indices.resize(chain_length);

        numRead = fread(&ik.iterations, sizeof(ik.iterations), 1, fp);
        if (numRead != 1)
        {
            OutputDebugString(_T("[PMDModel.cpp] Failed to read iks\n"));
            return E_FAIL;
        }

        numRead = fread(&ik.limit_angle, sizeof(ik.limit_angle), 1, fp);
        if (numRead != 1)
        {
            OutputDebugString(_T("[PMDModel.cpp] Failed to read iks\n"));
            return E_FAIL;
        }

        if (chain_length == 0)
        {
            continue;
        }

        numRead = fread(ik.chain_bone_indices.data(), sizeof(ik.chain_bone_indices[0]), chain_length, fp);
        if (numRead != chain_length)
        {
            OutputDebugString(_T("[PMDModel.cpp] Failed to read iks\n"));
            return E_FAIL;
        }
    }

    return S_OK;
}

HRESULT PMDModel::SetMaterialBuffer()
{
    size_t materialBufferSize = sizeof(MaterialForHlsl);
    materialBufferSize = (materialBufferSize + 0xff) & ~0xff;

    ConstantBuffer<char> materialBuffer;

    materialBuffer.SetResourceWidth(materialBufferSize * num_materials_); // mottainai
    HRESULT hr = materialBuffer.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to create material buffer\n"));
        return hr;
    }

    char* mappedBuffer = materialBuffer.GetMappedBuffer();
    for (auto& m : materials_)
    {
        *reinterpret_cast<MaterialForHlsl*>(mappedBuffer) = m.material;
        mappedBuffer += materialBufferSize;
    }

    materialBuffer.GetBuffer()->SetName(L"material buffer");

    materialBuffer.UmmapBuffer();

    m_materialSegment = m_modelManager.Allocate(num_materials_ * 5);

    D3D12_CONSTANT_BUFFER_VIEW_DESC materialCbvDesc = {};

    materialCbvDesc.BufferLocation = materialBuffer.GetGPUVirtualAddress();
    materialCbvDesc.SizeInBytes = static_cast<UINT>(materialBufferSize); 

    D3D12_SHADER_RESOURCE_VIEW_DESC materialSrvDesc = {};

    materialSrvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    materialSrvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    materialSrvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    materialSrvDesc.Texture2D.MipLevels = 1;

    D3D12_CPU_DESCRIPTOR_HANDLE materialHandle = m_materialSegment.GetCPUHandle();
    auto incSize = m_modelManager.GetIncrementSize();

    ID3D12Resource* whiteTexture = CreateWhiteTexture();
    ID3D12Resource* blackTexture = CreateBlackTexture();
    ID3D12Resource* grayGradationTexture = CreateGrayGradationTexture();

    for (size_t i = 0; i < num_materials_; ++i)
    {
        DXDevice::GetDevice()->CreateConstantBufferView(&materialCbvDesc, materialHandle);

        materialCbvDesc.BufferLocation += materialBufferSize;
        materialHandle.ptr += incSize;

        if (textures_[i] == nullptr)
        {
            materialSrvDesc.Format = whiteTexture->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(whiteTexture, &materialSrvDesc, materialHandle);
        }
        else
        {
            materialSrvDesc.Format = textures_[i]->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(textures_[i], &materialSrvDesc, materialHandle);
        }

        materialHandle.ptr += incSize;

        if (sph_[i] == nullptr)
        {
            materialSrvDesc.Format = whiteTexture->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(whiteTexture, &materialSrvDesc, materialHandle);
        }
        else
        {
            materialSrvDesc.Format = sph_[i]->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(sph_[i], &materialSrvDesc, materialHandle);
        }

        materialHandle.ptr += incSize;

        if (spa_[i] == nullptr)
        {
            materialSrvDesc.Format = blackTexture->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(blackTexture, &materialSrvDesc, materialHandle);
        }
        else
        {
            materialSrvDesc.Format = spa_[i]->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(spa_[i], &materialSrvDesc, materialHandle);
        }

        materialHandle.ptr += incSize;

        if (toon_[i] == nullptr)
        {
            materialSrvDesc.Format = whiteTexture->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(grayGradationTexture, &materialSrvDesc, materialHandle);
        }
        else
        {
            materialSrvDesc.Format = toon_[i]->GetDesc().Format;
            DXDevice::GetDevice()->CreateShaderResourceView(toon_[i], &materialSrvDesc, materialHandle);
        }

        materialHandle.ptr += incSize;
    }

    D3D12_DESCRIPTOR_RANGE* range = new D3D12_DESCRIPTOR_RANGE[2];

    range[0].NumDescriptors = 1;
    range[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    range[0].BaseShaderRegister = 2;
    range[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    range[0].RegisterSpace = 0;

    range[1].NumDescriptors = 4;
    range[1].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    range[1].BaseShaderRegister = 0;
    range[1].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    range[1].RegisterSpace = 0;

    m_modelManager.SetRootParameter(
        m_materialSegment.GetID(),
        D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        D3D12_SHADER_VISIBILITY_ALL,
        range,
        2
    );

    return S_OK;
}

HRESULT PMDModel::SetVertexBuffer()
{
    vertex_buffer_.SetResourceWidth(vertices_.size() * sizeof(PMDVertex));
    HRESULT hr = vertex_buffer_.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to create vertex buffer\n"));
        return hr;
    }

    std::ranges::copy(vertices_, vertex_buffer_.GetMappedBuffer());

    vertex_buffer_.UmmapBuffer();

    vertex_buffer_view_.BufferLocation = vertex_buffer_.GetGPUVirtualAddress();
    vertex_buffer_view_.SizeInBytes = static_cast<UINT>(vertices_.size() * sizeof(PMDVertex));
    vertex_buffer_view_.StrideInBytes = sizeof(PMDVertex);

    return S_OK;
}

HRESULT PMDModel::SetIndexBuffer()
{
    index_buffer_.SetResourceWidth(indices_.size() * sizeof(indices_[0]));
    HRESULT hr = index_buffer_.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to create index buffer\n"));
        return hr;
    }

    std::ranges::copy(indices_, index_buffer_.GetMappedBuffer());

    index_buffer_.UmmapBuffer();

    index_buffer_view_.BufferLocation = index_buffer_.GetGPUVirtualAddress();
    index_buffer_view_.SizeInBytes = static_cast<UINT>(indices_.size() * sizeof(indices_[0]));
    index_buffer_view_.Format = DXGI_FORMAT_R16_UINT;

    return S_OK;
}

HRESULT PMDModel::SetTransformBuffer()
{
    DirectX::XMMATRIX worldMatrix = DirectX::XMMatrixIdentity();

    for (const auto & data : motion_data_)
    {
        auto bone_node_it = bone_nodes_table_.find(data.first);
        if (bone_node_it == bone_nodes_table_.end())
        {
            continue;
        }

        BoneNode node = bone_node_it->second;
        DirectX::XMFLOAT3& pos = node.start_position;

        DirectX::XMMATRIX matrix = DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
            * DirectX::XMMatrixRotationQuaternion(data.second[0].quaternion) // 0番目のキーフレーム
            * DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);
        bone_matrices_[node.bone_index] = matrix;
    }

    RecursiveMatrixMultiply(&bone_nodes_table_["センター"], worldMatrix);

    size_t buffer_size = sizeof(DirectX::XMMATRIX) * (1 + bone_matrices_.size());
    matrix_buffer_.SetResourceWidth((buffer_size + 0xff) & ~0xff);
    HRESULT hr = matrix_buffer_.CreateBuffer();
    if (FAILED(hr))
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to create matrix buffer\n"));
        return hr;
    }

    matrix_buffer_.GetMappedBuffer()[0] = worldMatrix;
    std::ranges::copy(bone_matrices_, matrix_buffer_.GetMappedBuffer() + 1);

    m_matrixSegment = m_modelManager.Allocate(1);

    matrix_buffer_.SetSegment(m_matrixSegment);
    matrix_buffer_.CreateView();

    D3D12_DESCRIPTOR_RANGE* range = new D3D12_DESCRIPTOR_RANGE();

    range->NumDescriptors = 1;
    range->RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    range->BaseShaderRegister = 1;
    range->OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
    range->RegisterSpace = 0;

    m_modelManager.SetRootParameter(
        m_matrixSegment.GetID(),
        D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE,
        D3D12_SHADER_VISIBILITY_VERTEX,
        range,
        1
    );

    return S_OK;
}

void PMDModel::RecursiveMatrixMultiply(const BoneNode* node, const DirectX::XMMATRIX& parent_matrix)
{
    bone_matrices_[node->bone_index] *= parent_matrix;

    for (auto& child : node->children)
    {
        RecursiveMatrixMultiply(child, bone_matrices_[node->bone_index]);
    }
}

HRESULT PMDModel::ReadVMD(FILE* fp)
{
    int ret = fseek(fp, 50, SEEK_SET);
    if (ret != 0)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to seek file\n"));
        return E_FAIL;
    }

    unsigned int num_motion_data = 0;
    size_t numRead = fread(&num_motion_data, sizeof(num_motion_data), 1, fp);
    if (numRead != 1)
    {
        OutputDebugString(_T("[PMDModel.cpp] Failed to read motion data\n"));
        return E_FAIL;
    }

    std::vector<VMDMotion> motions(num_motion_data);

    for (auto& motion : motions)
    {
        fread(motion.bone_name, sizeof(motion.bone_name), 1, fp);
        fread(&motion.frame_number, 
            sizeof(motion.frame_number)
            + sizeof(motion.position)
            + sizeof(motion.quaternion)
            + sizeof(motion.bezier), 1, fp);
    }

    for (size_t i = 0; i < motions.size(); ++i)
    {
        const auto& motion = motions[i];
        motion_data_[motion.bone_name].emplace_back(
            KeyFrame(
                motion.frame_number,
                XMLoadFloat4(&motion.quaternion),
                DirectX::XMFLOAT2(motion.bezier[3] / 127.0f, motion.bezier[7] / 127.0f),
                DirectX::XMFLOAT2(motion.bezier[11] / 127.0f, motion.bezier[15] / 127.0f)
            )
        );

        duration = std::max<unsigned int>(duration, motion.frame_number);

        std::string bone_name = motion.bone_name;
        if (bone_name.find("ひざ") != std::string::npos)
        {
            knee_indices.emplace_back(i);
        }
    }

    for (auto& val : motion_data_ | std::views::values)
    {
        std::ranges::sort(val,
                          [](const KeyFrame& a, const KeyFrame& b)
                          {
                              return a.frame_number <= b.frame_number;
                          }
        );
    }

    return S_OK;
}

void PMDModel::PlayAnimation()
{
    start_time_ = timeGetTime();
}

void PMDModel::UpdateMotion()
{
    DWORD elapsed_time = timeGetTime() - start_time_;
    unsigned int frame_number = static_cast<unsigned int>(elapsed_time / 1000.0f * 30);
    if (frame_number > duration)
    {
        start_time_ = timeGetTime();
        frame_number = 0;
    }
    OutputDebugStringA(std::to_string(frame_number).c_str());
    OutputDebugStringA("\n");

    std::ranges::fill(bone_matrices_, DirectX::XMMatrixIdentity());

    for (const auto & motion_data : motion_data_)
    {
        BoneNode node = bone_nodes_table_[motion_data.first];

        auto motions = motion_data.second;
        auto rit = std::find_if(
            motions.rbegin(), motions.rend(),
            [frame_number](const KeyFrame& keyframe)
            {
                return keyframe.frame_number <= frame_number;
            }
        );

        if (rit == motions.rend())
        {
            continue;
        }

        DirectX::XMMATRIX rotation;
        auto it = rit.base(); // it : rit + 1

        if (it != motions.end())
        {
            float t = static_cast<float>(frame_number - rit->frame_number) / static_cast<float>(it->frame_number - rit->frame_number);
            t = GetYFromXBezier(t, it->p1, it->p2);

            rotation = DirectX::XMMatrixRotationQuaternion(DirectX::XMQuaternionSlerp(rit->quaternion, it->quaternion, t));
        }
        else
        {
            rotation = DirectX::XMMatrixRotationQuaternion(rit->quaternion);
        }

        DirectX::XMFLOAT3& pos = node.start_position;

        DirectX::XMMATRIX matrix = DirectX::XMMatrixTranslation(-pos.x, -pos.y, -pos.z)
            * rotation
            * DirectX::XMMatrixTranslation(pos.x, pos.y, pos.z);

        bone_matrices_[node.bone_index] = matrix;
    }

    RecursiveMatrixMultiply(&bone_nodes_table_["センター"], bone_matrices_[0]);
}

float PMDModel::GetYFromXBezier(float x, const DirectX::XMFLOAT2& p1, const DirectX::XMFLOAT2& p2, uint8_t n)
{
    if (p1.x == p1.y && p2.x == p2.y)
    {
        return x;
    }

    float t = x;
    float k0 = 1 + 3 * p1.x - 3 * p2.x;
    float k1 = 3 * p2.x - 6 * p1.x;
    float k2 = 3 * p1.x;

    constexpr float eps = 0.0005f;

    for (uint8_t i = 0; i < n; i++)
    {
        float ft = k0 * t * t * t + k1 * t * t + k2 * t - x;

        if (std::abs(ft) < eps)
        {
            break;
        }

        float ftd = 3 * k0 * t * t + 2 * k1 * t + k2;
        if (ftd == 0)
        {
            break;
        }

        t -= ft / ftd;
    }

    float s = 1 - t;
    return 3 * s * s * t * p1.y + 3 * s * t * t * p2.y + t * t * t;
}

void PMDModel::IKRotate()
{
    for (const auto & ik : iks_)
    {
        switch (auto count = ik.chain_bone_indices.size(); count)
        {
        case 0:
            break;

        case 1:
            IKLookAt(ik);
            break;

        case 2:
            IKCos(ik);
            break;

        default:
            IKCCD(ik);
        }
    }
}

void PMDModel::IKCCD(const PMDIK& ik) const
{
    BoneNode* target = bone_nodes_[ik.target_bone_index];
    DirectX::XMVECTOR target_pos = DirectX::XMLoadFloat3(&target->start_position);

    DirectX::XMMATRIX parent_matrix = bone_matrices_[bone_nodes_[ik.bone_index]->ik_parent_bone];

    DirectX::XMVECTOR det;
    DirectX::XMMATRIX inv_parent_matrix = DirectX::XMMatrixInverse(&det, parent_matrix);
    DirectX::XMVECTOR target_next_pos = DirectX::XMVector3Transform(target_pos, bone_matrices_[ik.bone_index] * inv_parent_matrix);

    std::vector<DirectX::XMVECTOR> positions;
    DirectX::XMVECTOR end_pos = DirectX::XMLoadFloat3(&bone_nodes_[ik.target_bone_index]->start_position);
    for (uint16_t index : ik.chain_bone_indices)
    {
        positions.emplace_back(DirectX::XMLoadFloat3(&bone_nodes_[index]->start_position));
    }

    std::vector<DirectX::XMMATRIX> matrices(positions.size());
    std::ranges::fill(matrices, DirectX::XMMatrixIdentity());

    float ik_limit = ik.limit_angle * DirectX::XM_PI;

    for (int c = 0; c < ik.iterations; ++c)
    {
        constexpr float eps = 0.0005f;
        if (DirectX::XMVector3Length(DirectX::XMVectorSubtract(end_pos, target_next_pos)).m128_f32[0] <= eps)
        {
            break;
        }

        for (int i = 0; i < positions.size(); ++i)
        {
            const DirectX::XMVECTOR& pos = positions[i];

            DirectX::XMVECTOR vec_to_end = DirectX::XMVectorSubtract(end_pos, pos);
            DirectX::XMVECTOR vec_to_target = DirectX::XMVectorSubtract(target_next_pos, pos);

            vec_to_end = DirectX::XMVector3Normalize(vec_to_end);
            vec_to_target = DirectX::XMVector3Normalize(vec_to_target);

            if (DirectX::XMVector3Length(DirectX::XMVectorSubtract(vec_to_end, vec_to_target)).m128_f32[0] <= eps)
            {
                continue;
            }

            DirectX::XMVECTOR cross = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(vec_to_end, vec_to_target));

            float angle = DirectX::XMVector3AngleBetweenVectors(vec_to_end, vec_to_target).m128_f32[0];
            if (angle > ik_limit)
            {
                angle = ik_limit;
            }

            DirectX::XMMATRIX rot = DirectX::XMMatrixRotationAxis(cross, angle);

            DirectX::XMMATRIX mat = DirectX::XMMatrixTranslationFromVector(-pos)
                * rot
                * DirectX::XMMatrixTranslationFromVector(pos);

            matrices[i] *= mat;

            for (int j = i - 1; j >= 0; --j)
            {
                positions[j] = DirectX::XMVector3Transform(positions[j], mat);
            }

            end_pos = XMVector3Transform(end_pos, mat);

            if (DirectX::XMVector3Length(DirectX::XMVectorSubtract(end_pos, target_next_pos)).m128_f32[0] <= eps)
            {
                break;
            }
        }
    }
}

void PMDModel::IKCos(const PMDIK& ik)
{
    std::vector<DirectX::XMVECTOR> positions;
    std::array<float, 2> edge_lens;

    BoneNode* target = bone_nodes_[ik.target_bone_index];
    DirectX::XMVECTOR target_pos = XMVector3Transform(XMLoadFloat3(&target->start_position), bone_matrices_[ik.bone_index]);

    BoneNode* end_node = bone_nodes_[ik.target_bone_index];
    positions.emplace_back(XMLoadFloat3(&end_node->start_position));

    for (uint16_t bone_index : ik.chain_bone_indices)
    {
        BoneNode* node = bone_nodes_[bone_index];
        positions.emplace_back(XMLoadFloat3(&node->start_position));
    }

    std::ranges::reverse(positions);

    edge_lens[0] = DirectX::XMVector3Length(DirectX::XMVectorSubtract(positions[1], positions[0])).m128_f32[0];
    edge_lens[1] = DirectX::XMVector3Length(DirectX::XMVectorSubtract(positions[2], positions[1])).m128_f32[0];

    positions[0] = XMVector3Transform(positions[0], bone_matrices_[ik.chain_bone_indices[1]]);
    positions[2] = XMVector3Transform(positions[2], bone_matrices_[ik.bone_index]);

    DirectX::XMVECTOR target_vec = DirectX::XMVectorSubtract(positions[2], positions[0]);

    float a = DirectX::XMVector3Length(target_vec).m128_f32[0];
    float b = edge_lens[0];
    float c = edge_lens[1];

    target_vec = DirectX::XMVector3Normalize(target_vec);

    float theta1 = std::acos((a * a + b * b - c * c) / (2 * a * b));
    float theta2 = std::acos((b * b + c * c - a * a) / (2 * b * c));

    DirectX::XMVECTOR axis;
    if (std::ranges::find(knee_indices, ik.chain_bone_indices[0]) == knee_indices.end())
    {
        DirectX::XMVECTOR vm = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(positions[2], positions[0]));
        DirectX::XMVECTOR vt = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target_pos, positions[0]));
        axis = DirectX::XMVector3Cross(vt, vm);
    }
    else
    {
        auto right = DirectX::XMFLOAT3(1, 0, 0);
        axis = XMLoadFloat3(&right);
    }

    DirectX::XMMATRIX mat1 = DirectX::XMMatrixTranslationFromVector(-positions[0]);
    mat1 *= DirectX::XMMatrixRotationAxis(axis, theta1);
    mat1 *= DirectX::XMMatrixTranslationFromVector(positions[0]);

    DirectX::XMMATRIX mat2 = DirectX::XMMatrixTranslationFromVector(-positions[1]);
    mat2 *= DirectX::XMMatrixRotationAxis(axis, theta2 - DirectX::XM_PI);
    mat2 *= DirectX::XMMatrixTranslationFromVector(positions[1]);

    bone_matrices_[ik.chain_bone_indices[1]] = mat1;
    bone_matrices_[ik.chain_bone_indices[0]] = mat2 * bone_matrices_[ik.chain_bone_indices[1]];
    bone_matrices_[ik.target_bone_index] = bone_matrices_[ik.chain_bone_indices[0]];
}

void PMDModel::IKLookAt(const PMDIK& ik)
{
    BoneNode* root = bone_nodes_[ik.chain_bone_indices[0]];
    BoneNode* target = bone_nodes_[ik.target_bone_index];

    DirectX::XMVECTOR root_pos1 = XMLoadFloat3(&root->start_position);
    DirectX::XMVECTOR target_pos1 = XMLoadFloat3(&target->start_position);

    DirectX::XMVECTOR root_pos2 = XMVector3TransformCoord(root_pos1, bone_matrices_[ik.chain_bone_indices[0]]);
    DirectX::XMVECTOR target_pos2 = XMVector3TransformCoord(target_pos1, bone_matrices_[ik.target_bone_index]);

    DirectX::XMVECTOR origin_vec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target_pos1, root_pos1));
    DirectX::XMVECTOR target_vec = DirectX::XMVector3Normalize(DirectX::XMVectorSubtract(target_pos2, root_pos2));

    bone_matrices_[ik.chain_bone_indices[0]] = GetLookAtMatrix(origin_vec, target_vec, DirectX::XMFLOAT3(0, 1, 0), DirectX::XMFLOAT3(1, 0, 0));
}

// z軸の回転
DirectX::XMMATRIX PMDModel::GetLookAtMatrix(
    const DirectX::XMVECTOR& look_at,
    const DirectX::XMFLOAT3& up,
    const DirectX::XMFLOAT3& right
)
{
    DirectX::XMVECTOR z = look_at;
    DirectX::XMVECTOR y = DirectX::XMVector3Normalize(XMLoadFloat3(&up));

    DirectX::XMVECTOR x = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(y, z));
    y = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(z, x));

    if (std::abs(DirectX::XMVector3Dot(y, z).m128_f32[0]) == 1.0f)
    {
        x = DirectX::XMVector3Normalize(XMLoadFloat3(&right));
        y = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(z, x));
        x = DirectX::XMVector3Normalize(DirectX::XMVector3Cross(y, z));
    }

    DirectX::XMMATRIX matrix = DirectX::XMMatrixIdentity();
    matrix.r[0] = x;
    matrix.r[1] = y;
    matrix.r[2] = z;

    return matrix;
}

DirectX::XMMATRIX PMDModel::GetLookAtMatrix(
    const DirectX::XMVECTOR& origin,
    const DirectX::XMVECTOR& target,
    const DirectX::XMFLOAT3& up, 
    const DirectX::XMFLOAT3& right
)
{
    return XMMatrixTranspose(GetLookAtMatrix(origin, up, right) * GetLookAtMatrix(target, up, right));
}
