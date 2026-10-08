#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <vector>
#include <string>
#include <unordered_map>

struct CachedTree {
    char name[64];
    float boundingBox[6];
    char branchTexture[64];
    char compositeTexture[64];
    char shadowTexture[64];
    float branchMaterial[12];
    float frondMaterial[12];
    float leafMaterial[12];
    float leafLightingAdjustment;

    struct ColObj {
        uint32_t type;
        float pos[3];
        float dim[3];
    };
    std::vector<ColObj> collisions;

    uint32_t branchVertexCount;
    std::vector<float> branchCoords;
    std::vector<uint32_t> branchColors;
    std::vector<float> branchTex0;
    std::vector<float> branchTex1;
    uint32_t branchIndexCount;
    std::vector<uint16_t> branchIndices;

    uint32_t frondVertexCount;
    std::vector<float> frondCoords;
    std::vector<uint32_t> frondColors;
    std::vector<float> frondTex0;
    std::vector<float> frondTex1;
    uint32_t frondIndexCount;
    std::vector<uint16_t> frondIndices;

    uint32_t leafCount;
    std::vector<float> leafCenters;
    std::vector<uint32_t> leafColors;
    std::vector<float> leafMapCoords;
    std::vector<float> leafMapTexCoords;
};

static bool ParseCache(const unsigned char* pData, size_t nSize, std::unordered_map<std::string, CachedTree>& outMap)
{
    if (!pData || nSize < 8) return false;
    if (memcmp(pData, "STC1", 4) != 0) return false;

    size_t offset = 4;
    uint32_t numTrees = *(const uint32_t*)(pData + offset);
    offset += 4;

    printf("Header: numTrees = %u\n", numTrees);

    for (uint32_t i = 0; i < numTrees; ++i)
    {
        CachedTree t;
        memcpy(t.name, pData + offset, 64); offset += 64;
        memcpy(t.boundingBox, pData + offset, 6 * sizeof(float)); offset += 6 * sizeof(float);
        memcpy(t.branchTexture, pData + offset, 64); offset += 64;
        memcpy(t.compositeTexture, pData + offset, 64); offset += 64;
        memcpy(t.shadowTexture, pData + offset, 64); offset += 64;
        memcpy(t.branchMaterial, pData + offset, 12 * sizeof(float)); offset += 12 * sizeof(float);
        memcpy(t.frondMaterial, pData + offset, 12 * sizeof(float)); offset += 12 * sizeof(float);
        memcpy(t.leafMaterial, pData + offset, 12 * sizeof(float)); offset += 12 * sizeof(float);
        memcpy(&t.leafLightingAdjustment, pData + offset, sizeof(float)); offset += sizeof(float);

        uint32_t numCol = *(const uint32_t*)(pData + offset); offset += 4;
        if (numCol > 0)
        {
            t.collisions.resize(numCol);
            memcpy(t.collisions.data(), pData + offset, numCol * sizeof(CachedTree::ColObj));
            offset += numCol * sizeof(CachedTree::ColObj);
        }

        // Branches
        t.branchVertexCount = *(const uint32_t*)(pData + offset); offset += 4;
        if (t.branchVertexCount > 0)
        {
            t.branchCoords.resize(t.branchVertexCount * 3);
            memcpy(t.branchCoords.data(), pData + offset, t.branchVertexCount * 3 * sizeof(float));
            offset += t.branchVertexCount * 3 * sizeof(float);

            t.branchColors.resize(t.branchVertexCount);
            memcpy(t.branchColors.data(), pData + offset, t.branchVertexCount * sizeof(uint32_t));
            offset += t.branchVertexCount * sizeof(uint32_t);

            t.branchTex0.resize(t.branchVertexCount * 2);
            memcpy(t.branchTex0.data(), pData + offset, t.branchVertexCount * 2 * sizeof(float));
            offset += t.branchVertexCount * 2 * sizeof(float);

            t.branchTex1.resize(t.branchVertexCount * 2);
            memcpy(t.branchTex1.data(), pData + offset, t.branchVertexCount * 2 * sizeof(float));
            offset += t.branchVertexCount * 2 * sizeof(float);
        }

        t.branchIndexCount = *(const uint32_t*)(pData + offset); offset += 4;
        if (t.branchIndexCount > 0)
        {
            t.branchIndices.resize(t.branchIndexCount);
            memcpy(t.branchIndices.data(), pData + offset, t.branchIndexCount * sizeof(uint16_t));
            offset += t.branchIndexCount * sizeof(uint16_t);
        }

        // Fronds
        t.frondVertexCount = *(const uint32_t*)(pData + offset); offset += 4;
        if (t.frondVertexCount > 0)
        {
            t.frondCoords.resize(t.frondVertexCount * 3);
            memcpy(t.frondCoords.data(), pData + offset, t.frondVertexCount * 3 * sizeof(float));
            offset += t.frondVertexCount * 3 * sizeof(float);

            t.frondColors.resize(t.frondVertexCount);
            memcpy(t.frondColors.data(), pData + offset, t.frondVertexCount * sizeof(uint32_t));
            offset += t.frondVertexCount * sizeof(uint32_t);

            t.frondTex0.resize(t.frondVertexCount * 2);
            memcpy(t.frondTex0.data(), pData + offset, t.frondVertexCount * 2 * sizeof(float));
            offset += t.frondVertexCount * 2 * sizeof(float);

            t.frondTex1.resize(t.frondVertexCount * 2);
            memcpy(t.frondTex1.data(), pData + offset, t.frondVertexCount * 2 * sizeof(float));
            offset += t.frondVertexCount * 2 * sizeof(float);
        }

        t.frondIndexCount = *(const uint32_t*)(pData + offset); offset += 4;
        if (t.frondIndexCount > 0)
        {
            t.frondIndices.resize(t.frondIndexCount);
            memcpy(t.frondIndices.data(), pData + offset, t.frondIndexCount * sizeof(uint16_t));
            offset += t.frondIndexCount * sizeof(uint16_t);
        }

        // Leaves
        t.leafCount = *(const uint32_t*)(pData + offset); offset += 4;
        if (t.leafCount > 0)
        {
            t.leafCenters.resize(t.leafCount * 3);
            memcpy(t.leafCenters.data(), pData + offset, t.leafCount * 3 * sizeof(float));
            offset += t.leafCount * 3 * sizeof(float);

            t.leafColors.resize(t.leafCount);
            memcpy(t.leafColors.data(), pData + offset, t.leafCount * sizeof(uint32_t));
            offset += t.leafCount * sizeof(uint32_t);

            t.leafMapCoords.resize(t.leafCount * 16);
            memcpy(t.leafMapCoords.data(), pData + offset, t.leafCount * 16 * sizeof(float));
            offset += t.leafCount * 16 * sizeof(float);

            t.leafMapTexCoords.resize(t.leafCount * 8);
            memcpy(t.leafMapTexCoords.data(), pData + offset, t.leafCount * 8 * sizeof(float));
            offset += t.leafCount * 8 * sizeof(float);
        }

        outMap[t.name] = t;
    }

    printf("Parsed %zu trees successfully (offset = %zu, total = %zu)!\n", outMap.size(), offset, nSize);
    return offset == nSize;
}

int main()
{
    FILE* fp = fopen("speedtree_cache.dat", "rb");
    if (!fp) { printf("Failed to open cache\n"); return 1; }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    std::vector<unsigned char> buf(sz);
    fread(buf.data(), 1, sz, fp);
    fclose(fp);

    std::unordered_map<std::string, CachedTree> map;
    bool ok = ParseCache(buf.data(), sz, map);
    printf("Result: %s\n", ok ? "VALID & EXACT MATCH" : "CORRUPT");

    auto it = map.find("b1_pagodatree_rt3.spt");
    if (it != map.end())
    {
        printf("b1_pagodatree_rt3.spt:\n");
        printf("  BBox: [%.1f, %.1f, %.1f] to [%.1f, %.1f, %.1f]\n",
               it->second.boundingBox[0], it->second.boundingBox[1], it->second.boundingBox[2],
               it->second.boundingBox[3], it->second.boundingBox[4], it->second.boundingBox[5]);
        printf("  Bark: %s, Composite: %s, Shadow: %s\n",
               it->second.branchTexture, it->second.compositeTexture, it->second.shadowTexture);
        printf("  Branches: %u verts, %u indices\n", it->second.branchVertexCount, it->second.branchIndexCount);
        printf("  Fronds: %u verts, %u indices\n", it->second.frondVertexCount, it->second.frondIndexCount);
        printf("  Leaves: %u leaves\n", it->second.leafCount);
        printf("  Collisions: %zu\n", it->second.collisions.size());
    }
    return ok ? 0 : 1;
}
