#include <fbxsdk.h>
#include <ostream>
#include <print>

int numTabs = 0;

void PrintTabs()
{
    for (int i = 0; i < numTabs; ++i)
    {
        std::print("\t");
    }
}

static FbxString GetAttributeTypeName(FbxNodeAttribute::EType type)
{
    switch (type)
    {
    case FbxNodeAttribute::eUnknown:          return "undefined";
    case FbxNodeAttribute::eNull:             return "null";
    case FbxNodeAttribute::eMarker:           return "marker";
    case FbxNodeAttribute::eSkeleton:         return "skeleton";
    case FbxNodeAttribute::eMesh:             return "mesh";
    case FbxNodeAttribute::eNurbs:            return "nurbs";
    case FbxNodeAttribute::ePatch:            return "patch";
    case FbxNodeAttribute::eCamera:           return "camera";
    case FbxNodeAttribute::eCameraStereo:     return "camera stereo";
    case FbxNodeAttribute::eCameraSwitcher:   return "camera switcher";
    case FbxNodeAttribute::eLight:            return "light";
    case FbxNodeAttribute::eOpticalReference: return "optional reference";
    case FbxNodeAttribute::eOpticalMarker:    return "optical marker";
    case FbxNodeAttribute::eNurbsCurve:       return "nurbs curve";
    case FbxNodeAttribute::eTrimNurbsSurface: return "trim nurbs surface";
    case FbxNodeAttribute::eBoundary:         return "boundary";
    case FbxNodeAttribute::eNurbsSurface:     return "nurbs surface";
    case FbxNodeAttribute::eLODGroup:         return "lod group";
    case FbxNodeAttribute::eSubDiv:           return "subdiv";
    case FbxNodeAttribute::eCachedEffect:     return "cached effect";
    case FbxNodeAttribute::eLine:             return "line";
    case FbxNodeAttribute::eShape:            return "shape";
    }
    return "unknown";
}

void PrintAttribute(FbxNodeAttribute* pAttribute)
{
    if (!pAttribute) return;

    FbxString typeName = GetAttributeTypeName(pAttribute->GetAttributeType());
    FbxString attrName = pAttribute->GetName();
    PrintTabs();

    std::println("<attribute type='{}' name='{}' />", typeName.Buffer(), attrName.Buffer());
}

void PrintNode(FbxNode* pNode)
{
    PrintTabs();

    const char* nodeName = pNode->GetName();
    FbxDouble3 translation = pNode->LclTranslation.Get();
    FbxDouble3 rotation = pNode->LclRotation.Get();
    FbxDouble3 scaling = pNode->LclScaling.Get();

    std::println("<node name='{}' translation='({}, {}, {})' rotation='({}, {}, {})' scaling='({}, {}, {})'>",
        nodeName,
        translation[0], translation[1], translation[2],
        rotation[0], rotation[1], rotation[2],
        scaling[0], scaling[1], scaling[2]
    );
    numTabs++;

    for (int i = 0; i < pNode->GetNodeAttributeCount(); ++i)
    {
        PrintAttribute(pNode->GetNodeAttributeByIndex(i));
    }

    for (int i = 0; i < pNode->GetChildCount(); ++i)
    {
        PrintNode(pNode->GetChild(i));
    }

    numTabs--;
    PrintTabs();
    std::println("</node>");
}

void DisplayIndex(FbxMesh* pMesh)
{
    int polygon_count = pMesh->GetPolygonCount();

    for (int p = 0; p < polygon_count; ++p)
    {

        int polygon_size = pMesh->GetPolygonSize(p);

        std::print("Polygon {}: (", p);

        for (int i = 0; i < polygon_size; ++i)
        {
            int index = pMesh->GetPolygonVertex(p, i);
            std::print("{}, ", index);
        }
        std::println(")");

        for (int i = 0; i < polygon_size; ++i)
        {
            for (int j = 0; j < pMesh->GetElementUVCount(); ++j)
            {
                auto* elementUV = pMesh->GetElementUV(j);

                switch (elementUV->GetMappingMode())
                {
                case FbxGeometryElement::eByControlPoint:
                    std::print("    UV by control point  ");
                    switch (elementUV->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        {
                            std::print("direct  ");
                            FbxVector2 uv = elementUV->GetDirectArray().GetAt(pMesh->GetTextureUVIndex(p, i));
                            std::print("UV {}: ({}, {}), ", pMesh->GetTextureUVIndex(p, i), uv[0], uv[1]);
                        }
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                        {
                            std::print("index to direct  ");
                            int id = elementUV->GetIndexArray().GetAt(pMesh->GetTextureUVIndex(p, i));
                            FbxVector2 uv = elementUV->GetDirectArray().GetAt(id);
                            std::print("UV {}: ({}, {}), ", pMesh->GetTextureUVIndex(p, i), uv[0], uv[1]);
                        }
                        break;
                    case FbxGeometryElement::eIndex:
                        break;
                    }
                    break;
                case FbxGeometryElement::eByPolygonVertex:
                    std::print("    UV by polygon vertex  ");
                    switch (elementUV->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                    case FbxGeometryElement::eIndexToDirect:
                        {
                            std::print("direct  ");
                            FbxVector2 uv = elementUV->GetDirectArray().GetAt(pMesh->GetTextureUVIndex(p, i));
                            std::print("UV {}: ({}, {}), ", pMesh->GetTextureUVIndex(p, i), uv[0], uv[1]);
                        }
                        break;
                    case FbxGeometryElement::eIndex:
                        break;
                    }
                    break;
                case FbxGeometryElement::eByPolygon:
                case FbxGeometryElement::eByEdge:
                case FbxGeometryElement::eAllSame:
                case FbxGeometryElement::eNone:
                    std::print("UV none");
                    break;
                }

                std::println();
            }
        }

        for (int i = 0; i < polygon_size; ++i)
        {
            for (int j = 0; j < pMesh->GetElementNormalCount(); j++)
            {
                auto* normal = pMesh->GetElementNormal(j);

                switch (normal->GetMappingMode())
                {
                case FbxGeometryElement::eByControlPoint:
                    std::print("    Normal by control point  ");
                    switch (normal->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        {
                            std::print("direct  ");
                            FbxVector4 norm = normal->GetDirectArray().GetAt(pMesh->GetTextureUVIndex(p, i));
                            std::print("Normal {}: ({}, {}, {}, {}), ", pMesh->GetTextureUVIndex(p, i),  norm[0], norm[1], norm[2], norm[3]);
                        }
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                        {
                            std::print("index to direct  ");
                            int id = normal->GetIndexArray().GetAt(pMesh->GetTextureUVIndex(p, i));
                            FbxVector4 norm = normal->GetDirectArray().GetAt(id);
                            std::print("Normal {}: ({}, {}, {}, {}), ", pMesh->GetTextureUVIndex(p, i), norm[0], norm[1], norm[2], norm[3]);
                        }
                        break;
                    case FbxGeometryElement::eIndex:
                        break;
                    }
                    break;
                case FbxGeometryElement::eByPolygonVertex:
                    std::print("    Normal by polygon vertex  ");
                    switch (normal->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                    case FbxGeometryElement::eIndexToDirect:
                        {
                            std::print("direct  ");
                            FbxVector4 norm = normal->GetDirectArray().GetAt(pMesh->GetTextureUVIndex(p, i));
                            std::print("Normal {}: ({}, {}, {}, {}), ", pMesh->GetTextureUVIndex(p, i), norm[0], norm[1], norm[2], norm[3]);
                        }
                        break;
                    case FbxGeometryElement::eIndex:
                        break;
                    }
                    break;
                case FbxGeometryElement::eByPolygon:
                case FbxGeometryElement::eByEdge:
                case FbxGeometryElement::eAllSame:
                case FbxGeometryElement::eNone:
                    std::print("Normal none");
                    break;
                }

                std::println();
            }
        }


    }
}

void DisplayPosition(FbxMesh* pMesh)
{
    int vertex_count = pMesh->GetControlPointsCount();
    FbxVector4* position = pMesh->GetControlPoints();

    for (int i = 0; i < vertex_count; ++i)
    {
        std::println("Vertex {}: ({}, {}, {})",
            i, position[i][0], position[i][1], position[i][2], position[i][3]);

        for (int j = 0; j < pMesh->GetElementUVCount(); ++j)
            {
                auto* elementUV = pMesh->GetElementUV(j);

                switch (elementUV->GetMappingMode())
                {
                case FbxGeometryElement::eByControlPoint:
                    std::print("    UV by control point  ");
                    switch (elementUV->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                        {
                            std::print("direct  ");
                            FbxVector2 uv = elementUV->GetDirectArray().GetAt(i);
                            std::print("UV: ({}, {}), ", uv[0], uv[1]);
                        }
                        break;
                    case FbxGeometryElement::eIndexToDirect:
                        {
                            std::print("index to direct  ");
                            int id = elementUV->GetIndexArray().GetAt(i);
                            FbxVector2 uv = elementUV->GetDirectArray().GetAt(id);
                            std::print("UV: ({}, {}), ", uv[0], uv[1]);
                        }
                        break;
                    case FbxGeometryElement::eIndex:
                        break;
                    }
                    break;
                case FbxGeometryElement::eByPolygonVertex:
                    std::print("    UV by polygon vertex  ");
                    switch (elementUV->GetReferenceMode())
                    {
                    case FbxGeometryElement::eDirect:
                    case FbxGeometryElement::eIndexToDirect:
                        {
                            std::print("direct  ");
                            FbxVector2 uv = elementUV->GetDirectArray().GetAt(i);
                            std::print("UV {}: ({}, {}), ", i, uv[0], uv[1]);
                        }
                        break;
                    case FbxGeometryElement::eIndex:
                        break;
                    }
                    break;
                case FbxGeometryElement::eByPolygon:
                case FbxGeometryElement::eByEdge:
                case FbxGeometryElement::eAllSame:
                case FbxGeometryElement::eNone:
                    std::print("UV none");
                    break;
                }

                std::println();
            }
    }
}

void DisplayMesh(FbxNode* pNode)
{
    FbxMesh* mesh = pNode->GetMesh();

    std::println("Mesh name: {}", pNode->GetName());

    DisplayIndex(mesh);
    //DisplayPosition(mesh);
}

void DisplayContent(FbxNode* pNode)
{
    FbxNodeAttribute::EType type = pNode->GetNodeAttribute()->GetAttributeType();

    if (type == FbxNodeAttribute::eMesh)
    {
        DisplayMesh(pNode);
    }

    for (int i = 0; i < pNode->GetChildCount(); ++i)
    {
        DisplayContent(pNode->GetChild(i));
    }
}

int main(int argc, char** argv)
{
    const char* lFilename = "isu.fbx";
    if (!FbxFileUtils::Exist(lFilename))
    {
        std::println("file not found: {}", lFilename);
        system("dir");
        exit(-1);
    }

    FbxManager* lSdkManager = FbxManager::Create();

    FbxIOSettings* ios = FbxIOSettings::Create(lSdkManager, IOSROOT);
    lSdkManager->SetIOSettings(ios);

    FbxImporter* lImporter = FbxImporter::Create(lSdkManager, "");

    if (!lImporter->Initialize(lFilename, -1, lSdkManager->GetIOSettings()))
    {
        std::println("fbx importer initialization failed.");
        std::println("Error: {}", lImporter->GetStatus().GetErrorString());
        exit(-1);
    }

    FbxScene* lScene = FbxScene::Create(lSdkManager, "myScene");
    lImporter->Import(lScene);
    lImporter->Destroy();

    FbxGeometryConverter lGeomConverter(lSdkManager);
    lGeomConverter.Triangulate(lScene, true);

    FbxNode* lRootNode = lScene->GetRootNode();
    if (lRootNode)
    {
        for (int i = 0; i < lRootNode->GetChildCount(); ++i)
        {
            DisplayContent(lRootNode->GetChild(i));
        }
    }

    lSdkManager->Destroy();
    return 0;
}
