#include <windows.h>
#include <stdio.h>
#include <stdint.h>
#include <string>
#include <vector>
#include <algorithm>

#define SPEEDTREERT_DYNAMIC_LIB
#define WRAPPER_FLIP_T_TEXCOORD
#include "../../extern/include/SpeedTreeRT.h"

struct TreeExportData {
    char name[64];
    float boundingBox[6];
    char branchTexture[64];
    char compositeTexture[64];
    char shadowTexture[64];
    float branchMaterial[12];
    float frondMaterial[12];
    float leafMaterial[12];
    float leafLightingAdjustment;

    uint32_t branchVertexCount;
    std::vector<float> branchCoords;     // 3 * count
    std::vector<uint32_t> branchColors;  // count
    std::vector<float> branchTex0;       // 2 * count
    std::vector<float> branchTex1;       // 2 * count
    uint32_t branchIndexCount;
    std::vector<uint16_t> branchIndices;

    uint32_t frondVertexCount;
    std::vector<float> frondCoords;      // 3 * count
    std::vector<uint32_t> frondColors;   // count
    std::vector<float> frondTex0;        // 2 * count
    std::vector<float> frondTex1;        // 2 * count
    uint32_t frondIndexCount;
    std::vector<uint16_t> frondIndices;

    uint32_t leafCount;
    std::vector<float> leafCenters;      // 3 * count
    std::vector<uint32_t> leafColors;    // count
    std::vector<float> leafMapCoords;    // 16 * count
    std::vector<float> leafMapTexCoords; // 8 * count

    struct ColObj {
        uint32_t type;
        float pos[3];
        float dim[3];
    };
    std::vector<ColObj> collisions;
};

static std::string CleanTextureName(const char* pTex)
{
    if (!pTex) return "";
    std::string s = pTex;
    size_t lastSlash = s.find_last_of("/\\");
    if (lastSlash != std::string::npos) s = s.substr(lastSlash + 1);
    size_t lastDot = s.find_last_of('.');
    if (lastDot != std::string::npos) s = s.substr(0, lastDot);
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s + ".dds";
}

int main(int argc, char** argv)
{
    printf("==================================================\n");
    printf(" SpeedTreeRT PC Geometry Exporter for Android\n");
    printf("==================================================\n");

    const char* inputDir = "D:/01NewFiles/Data_Pack/tree/ymir work/tree/";
    const char* outputCache = "speedtree_cache.dat";

    std::string searchMask = std::string(inputDir) + "*.spt";
    WIN32_FIND_DATAA fd;
    HANDLE hFind = FindFirstFileA(searchMask.c_str(), &fd);
    if (hFind == INVALID_HANDLE_VALUE)
    {
        printf("Error: Cannot find any .spt files in '%s'\n", inputDir);
        return 1;
    }

    CSpeedTreeRT::SetTextureFlip(true);
    std::vector<TreeExportData> exportedTrees;

    do
    {
        if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            continue;

        std::string filename = fd.cFileName;
        std::string fullPath = std::string(inputDir) + filename;

        CSpeedTreeRT* pTree = new CSpeedTreeRT();
        if (!pTree->LoadTree(fullPath.c_str()))
        {
            printf("FAIL load: %s (%s)\n", filename.c_str(), CSpeedTreeRT::GetCurrentError());
            delete pTree;
            continue;
        }

        pTree->SetBranchLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
        pTree->SetLeafLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
        pTree->SetFrondLightingMethod(CSpeedTreeRT::LIGHT_STATIC);
        pTree->SetNumLeafRockingGroups(1);

        if (!pTree->Compute(nullptr, 1))
        {
            printf("FAIL compute: %s (%s)\n", filename.c_str(), CSpeedTreeRT::GetCurrentError());
            delete pTree;
            continue;
        }

        TreeExportData data;
        memset(&data, 0, sizeof(data));

        std::string lowerName = filename;
        std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
        strncpy(data.name, lowerName.c_str(), sizeof(data.name) - 1);

        pTree->GetBoundingBox(data.boundingBox);

        CSpeedTreeRT::STextures tex;
        pTree->GetTextures(tex);

        std::string branchTex = CleanTextureName(tex.m_pBranchTextureFilename);
        std::string compTex = CleanTextureName(tex.m_pCompositeFilename);
        std::string shadowTex = CleanTextureName(tex.m_pSelfShadowFilename);

        strncpy(data.branchTexture, branchTex.c_str(), sizeof(data.branchTexture) - 1);
        strncpy(data.compositeTexture, compTex.c_str(), sizeof(data.compositeTexture) - 1);
        strncpy(data.shadowTexture, shadowTex.c_str(), sizeof(data.shadowTexture) - 1);

        const float* bMat = pTree->GetBranchMaterial();
        if (bMat) memcpy(data.branchMaterial, bMat, 12 * sizeof(float));

        const float* fMat = pTree->GetFrondMaterial();
        if (fMat) memcpy(data.frondMaterial, fMat, 12 * sizeof(float));

        const float* lMat = pTree->GetLeafMaterial();
        if (lMat) memcpy(data.leafMaterial, lMat, 12 * sizeof(float));

        data.leafLightingAdjustment = pTree->GetLeafLightingAdjustment();

        // Collisions
        unsigned int numCol = pTree->GetCollisionObjectCount();
        for (unsigned int c = 0; c < numCol; ++c)
        {
            CSpeedTreeRT::ECollisionObjectType colType;
            TreeExportData::ColObj obj;
            pTree->GetCollisionObject(c, colType, obj.pos, obj.dim);
            obj.type = (uint32_t)colType;
            data.collisions.push_back(obj);
        }

        // Geometry (LOD 0)
        pTree->SetLodLevel(1.0f);
        CSpeedTreeRT::SGeometry geo;
        pTree->GetGeometry(geo, SpeedTree_AllGeometry, 0, 0, 0);

        // 1. Branches
        data.branchVertexCount = geo.m_sBranches.m_usVertexCount;
        if (data.branchVertexCount > 0 && geo.m_sBranches.m_pCoords)
        {
            data.branchCoords.assign(geo.m_sBranches.m_pCoords, geo.m_sBranches.m_pCoords + (data.branchVertexCount * 3));
            if (geo.m_sBranches.m_pColors)
                data.branchColors.assign(geo.m_sBranches.m_pColors, geo.m_sBranches.m_pColors + data.branchVertexCount);
            else
                data.branchColors.assign(data.branchVertexCount, 0xFFFFFFFF);

            if (geo.m_sBranches.m_pTexCoords0)
                data.branchTex0.assign(geo.m_sBranches.m_pTexCoords0, geo.m_sBranches.m_pTexCoords0 + (data.branchVertexCount * 2));
            else
                data.branchTex0.assign(data.branchVertexCount * 2, 0.0f);

            if (geo.m_sBranches.m_pTexCoords1)
                data.branchTex1.assign(geo.m_sBranches.m_pTexCoords1, geo.m_sBranches.m_pTexCoords1 + (data.branchVertexCount * 2));
            else
                data.branchTex1.assign(data.branchVertexCount * 2, 0.0f);
        }

        if (geo.m_sBranches.m_usNumStrips > 0 && geo.m_sBranches.m_pStripLengths && geo.m_sBranches.m_pStrips)
        {
            data.branchIndexCount = geo.m_sBranches.m_pStripLengths[0];
            if (data.branchIndexCount > 0 && geo.m_sBranches.m_pStrips[0])
            {
                data.branchIndices.assign(geo.m_sBranches.m_pStrips[0], geo.m_sBranches.m_pStrips[0] + data.branchIndexCount);
            }
        }

        // 2. Fronds
        data.frondVertexCount = geo.m_sFronds.m_usVertexCount;
        if (data.frondVertexCount > 0 && geo.m_sFronds.m_pCoords)
        {
            data.frondCoords.assign(geo.m_sFronds.m_pCoords, geo.m_sFronds.m_pCoords + (data.frondVertexCount * 3));
            if (geo.m_sFronds.m_pColors)
                data.frondColors.assign(geo.m_sFronds.m_pColors, geo.m_sFronds.m_pColors + data.frondVertexCount);
            else
                data.frondColors.assign(data.frondVertexCount, 0xFFFFFFFF);

            if (geo.m_sFronds.m_pTexCoords0)
                data.frondTex0.assign(geo.m_sFronds.m_pTexCoords0, geo.m_sFronds.m_pTexCoords0 + (data.frondVertexCount * 2));
            else
                data.frondTex0.assign(data.frondVertexCount * 2, 0.0f);

            if (geo.m_sFronds.m_pTexCoords1)
                data.frondTex1.assign(geo.m_sFronds.m_pTexCoords1, geo.m_sFronds.m_pTexCoords1 + (data.frondVertexCount * 2));
            else
                data.frondTex1.assign(data.frondVertexCount * 2, 0.0f);
        }

        if (geo.m_sFronds.m_usNumStrips > 0 && geo.m_sFronds.m_pStripLengths && geo.m_sFronds.m_pStrips)
        {
            data.frondIndexCount = geo.m_sFronds.m_pStripLengths[0];
            if (data.frondIndexCount > 0 && geo.m_sFronds.m_pStrips[0])
            {
                data.frondIndices.assign(geo.m_sFronds.m_pStrips[0], geo.m_sFronds.m_pStrips[0] + data.frondIndexCount);
            }
        }

        // 3. Leaves
        data.leafCount = geo.m_sLeaves0.m_usLeafCount;
        if (data.leafCount > 0 && geo.m_sLeaves0.m_pCenterCoords)
        {
            data.leafCenters.assign(geo.m_sLeaves0.m_pCenterCoords, geo.m_sLeaves0.m_pCenterCoords + (data.leafCount * 3));
            if (geo.m_sLeaves0.m_pColors)
                data.leafColors.assign(geo.m_sLeaves0.m_pColors, geo.m_sLeaves0.m_pColors + data.leafCount);
            else
                data.leafColors.assign(data.leafCount, 0xFFFFFFFF);

            data.leafMapCoords.resize(data.leafCount * 16);
            data.leafMapTexCoords.resize(data.leafCount * 8);

            for (uint32_t l = 0; l < data.leafCount; ++l)
            {
                if (geo.m_sLeaves0.m_pLeafMapCoords && geo.m_sLeaves0.m_pLeafMapCoords[l])
                {
                    memcpy(&data.leafMapCoords[l * 16], geo.m_sLeaves0.m_pLeafMapCoords[l], 16 * sizeof(float));
                }
                if (geo.m_sLeaves0.m_pLeafMapTexCoords && geo.m_sLeaves0.m_pLeafMapTexCoords[l])
                {
                    memcpy(&data.leafMapTexCoords[l * 8], geo.m_sLeaves0.m_pLeafMapTexCoords[l], 8 * sizeof(float));
                }
            }
        }

        exportedTrees.push_back(data);
        delete pTree;

        printf("  Exported %-32s (B_verts:%4u, F_verts:%4u, Leaves:%4u)\n",
               data.name, data.branchVertexCount, data.frondVertexCount, data.leafCount);

    } while (FindNextFileA(hFind, &fd));

    FindClose(hFind);

    printf("\nWriting binary cache file: %s ...\n", outputCache);
    FILE* fp = fopen(outputCache, "wb");
    if (!fp)
    {
        printf("Error: Failed to open '%s' for writing!\n", outputCache);
        return 1;
    }

    // Header: Magic "STC1", count
    const char magic[4] = { 'S', 'T', 'C', '1' };
    fwrite(magic, 1, 4, fp);
    uint32_t numTrees = (uint32_t)exportedTrees.size();
    fwrite(&numTrees, sizeof(uint32_t), 1, fp);

    for (const auto& t : exportedTrees)
    {
        fwrite(t.name, 1, 64, fp);
        fwrite(t.boundingBox, sizeof(float), 6, fp);
        fwrite(t.branchTexture, 1, 64, fp);
        fwrite(t.compositeTexture, 1, 64, fp);
        fwrite(t.shadowTexture, 1, 64, fp);
        fwrite(t.branchMaterial, sizeof(float), 12, fp);
        fwrite(t.frondMaterial, sizeof(float), 12, fp);
        fwrite(t.leafMaterial, sizeof(float), 12, fp);
        fwrite(&t.leafLightingAdjustment, sizeof(float), 1, fp);

        // Collisions
        uint32_t numCol = (uint32_t)t.collisions.size();
        fwrite(&numCol, sizeof(uint32_t), 1, fp);
        if (numCol > 0)
        {
            fwrite(t.collisions.data(), sizeof(TreeExportData::ColObj), numCol, fp);
        }

        // Branches
        fwrite(&t.branchVertexCount, sizeof(uint32_t), 1, fp);
        if (t.branchVertexCount > 0)
        {
            fwrite(t.branchCoords.data(), sizeof(float), t.branchVertexCount * 3, fp);
            fwrite(t.branchColors.data(), sizeof(uint32_t), t.branchVertexCount, fp);
            fwrite(t.branchTex0.data(), sizeof(float), t.branchVertexCount * 2, fp);
            fwrite(t.branchTex1.data(), sizeof(float), t.branchVertexCount * 2, fp);
        }
        fwrite(&t.branchIndexCount, sizeof(uint32_t), 1, fp);
        if (t.branchIndexCount > 0)
        {
            fwrite(t.branchIndices.data(), sizeof(uint16_t), t.branchIndexCount, fp);
        }

        // Fronds
        fwrite(&t.frondVertexCount, sizeof(uint32_t), 1, fp);
        if (t.frondVertexCount > 0)
        {
            fwrite(t.frondCoords.data(), sizeof(float), t.frondVertexCount * 3, fp);
            fwrite(t.frondColors.data(), sizeof(uint32_t), t.frondVertexCount, fp);
            fwrite(t.frondTex0.data(), sizeof(float), t.frondVertexCount * 2, fp);
            fwrite(t.frondTex1.data(), sizeof(float), t.frondVertexCount * 2, fp);
        }
        fwrite(&t.frondIndexCount, sizeof(uint32_t), 1, fp);
        if (t.frondIndexCount > 0)
        {
            fwrite(t.frondIndices.data(), sizeof(uint16_t), t.frondIndexCount, fp);
        }

        // Leaves
        fwrite(&t.leafCount, sizeof(uint32_t), 1, fp);
        if (t.leafCount > 0)
        {
            fwrite(t.leafCenters.data(), sizeof(float), t.leafCount * 3, fp);
            fwrite(t.leafColors.data(), sizeof(uint32_t), t.leafCount, fp);
            fwrite(t.leafMapCoords.data(), sizeof(float), t.leafCount * 16, fp);
            fwrite(t.leafMapTexCoords.data(), sizeof(float), t.leafCount * 8, fp);
        }
    }

    long totalSize = ftell(fp);
    fclose(fp);

    printf("\nSuccess! Exported %u trees into '%s' (Total size: %.2f MB / %ld bytes)\n",
           numTrees, outputCache, (float)totalSize / (1024.0f * 1024.0f), totalSize);

    return 0;
}
