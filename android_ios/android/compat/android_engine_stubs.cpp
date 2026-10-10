// Android engine stubs for Metin2 Mobile
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <windows.h>
#ifdef __ANDROID__
#include <android/log.h>
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO, "Metin2Engine", __VA_ARGS__)
#else
#define LOGI(...)
#endif

// =================================================================
// LZO Compression (C linkage)
// =================================================================
#ifdef __cplusplus
extern "C" {
#endif

typedef unsigned char lzo_byte;
typedef unsigned int  lzo_uint;

int (*AdhocTest)(int argc, char *argv[]) = NULL;

#define LZO_E_OK 0

int lzo_init(void) { return LZO_E_OK; }
int __lzo_init_v2(unsigned v, int s1, int s2, int s3, int s4, int s5,
                  int s6, int s7, int s8, int s9) { return LZO_E_OK; }

// =================================================================
// WebBrowser (CEF-based, Windows-only)
// =================================================================
int  WebBrowser_Startup(HINSTANCE hInstance) { return 1; }
void WebBrowser_Cleanup() {}
void WebBrowser_Destroy() {}
int  WebBrowser_Show(HWND parent, const char* addr, const RECT* rcWebBrowser) { return 0; }
void WebBrowser_Hide() {}
void WebBrowser_Move(const RECT* rcWebBrowser) {}
int  WebBrowser_IsVisible() { return 0; }
static RECT s_emptyRect = {0, 0, 0, 0};
const RECT& WebBrowser_GetRect() { return s_emptyRect; }

#ifdef __cplusplus
}
#endif

// =================================================================
// Anti-cheat / CRC stubs (Windows-only)
// =================================================================
void BuildProcessCRC() {}
BYTE GetProcessCRCMagicCubePiece() { return 0; }

// =================================================================
// Test Server Mode flag
// =================================================================
bool __IS_TEST_SERVER_MODE__ = false;

// =================================================================
// ApplicationStringTable (UserInterface.cpp stubs)
// =================================================================
static std::string s_emptyString = "";

const std::string& ApplicationStringTable_GetString(DWORD dwID, LPCSTR szKey)
{
    static std::string s_str;
    s_str = szKey ? szKey : "";
    return s_str;
}

const std::string& ApplicationStringTable_GetString(DWORD dwID)
{
    return s_emptyString;
}

const char* ApplicationStringTable_GetStringz(DWORD dwID, LPCSTR szKey)
{
    return ApplicationStringTable_GetString(dwID, szKey).c_str();
}

const char* ApplicationStringTable_GetStringz(DWORD dwID)
{
    return ApplicationStringTable_GetString(dwID).c_str();
}


// =================================================================
// SDL2 Stubs for Android
// =================================================================
#ifdef __cplusplus
extern "C" {
#endif

int SDL_PollEvent(void* event) { return 0; }
uint32_t SDL_GetMouseState(int* x, int* y) { if (x) *x = 0; if (y) *y = 0; return 0; }
int SDL_GetNumDisplayModes(int displayIndex) { return 1; }
int SDL_GetDisplayMode(int displayIndex, int modeIndex, void* mode) { return 0; }

void SDL_DestroyWindow(void* window) {}
void SDL_ShowWindow(void* window) {}
void SDL_HideWindow(void* window) {}
void SDL_SetWindowPosition(void* window, int x, int y) {}
void SDL_SetWindowSize(void* window, int w, int h) {}
void SDL_SetWindowTitle(void* window, const char* title) {}
void* SDL_GetKeyboardFocus(void) { return nullptr; }

#ifdef __cplusplus
}
#endif

// =================================================================
// SpeedTreeRT Implementation for Android
// =================================================================
#include <SpeedTreeRT.h>
#include <cmath>
#include <vector>
#include <string>
#include <algorithm>
#include <unordered_map>

#ifdef __ANDROID__
#include <android/asset_manager.h>
extern AAssetManager* g_pAssetManager;
#endif

extern void TraceError(const char * c_szFormat, ...);

bool CSpeedTreeRT::m_bTextureFlip = false;
bool CSpeedTreeRT::m_bDropToBillboard = false;

struct CachedTreeEntry {
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

    uint32_t branchVertexCount = 0;
    std::vector<float> branchCoords;
    std::vector<unsigned long> branchColors;
    std::vector<float> branchTex0;
    std::vector<float> branchTex1;
    uint32_t branchIndexCount = 0;
    std::vector<unsigned short> branchIndices;

    uint32_t frondVertexCount = 0;
    std::vector<float> frondCoords;
    std::vector<unsigned long> frondColors;
    std::vector<float> frondTex0;
    std::vector<float> frondTex1;
    uint32_t frondIndexCount = 0;
    std::vector<unsigned short> frondIndices;

    uint32_t leafCount = 0;
    std::vector<float> leafCenters;
    std::vector<unsigned long> leafColors;
    std::vector<float> leafMapCoordsData;
    std::vector<float> leafMapTexCoordsData;
};

struct InternalTreeData
{
    InternalTreeData* m_pBaseTree = nullptr;
    std::string m_strBark;
    std::string m_strComposite;
    std::string m_strShadow;

    float m_fSize = 800.0f;
    float m_fVariance = 0.0f;
    float m_afPos[3] = {0.0f, 0.0f, 0.0f};

    float m_afBranchMaterial[12] = {1,1,1,1, 1,1,1,1, 1,1,1,1};
    float m_afFrondMaterial[12]  = {1,1,1,1, 1,1,1,1, 1,1,1,1};
    float m_afLeafMaterial[12]   = {1,1,1,1, 1,1,1,1, 1,1,1,1};
    float m_fLeafLightingAdjustment = 1.0f;

    struct ColObj {
        uint32_t type;
        float pos[3];
        float dim[3];
    };
    std::vector<ColObj> m_vCollisions;

    // Branch geometry
    std::vector<float> m_vBranchCoords;
    std::vector<unsigned long> m_vBranchColors;
    std::vector<float> m_vBranchTexCoords0;
    std::vector<float> m_vBranchTexCoords1;
    std::vector<unsigned short> m_vBranchIndices;
    unsigned short m_usBranchStripLen = 0;
    const unsigned short* m_pBranchStripLenPtr = nullptr;
    const unsigned short* m_pBranchIndicesPtr = nullptr;

    // Frond geometry
    std::vector<float> m_vFrondCoords;
    std::vector<unsigned long> m_vFrondColors;
    std::vector<float> m_vFrondTexCoords0;
    std::vector<float> m_vFrondTexCoords1;
    std::vector<unsigned short> m_vFrondIndices;
    unsigned short m_usFrondStripLen = 0;
    const unsigned short* m_pFrondStripLenPtr = nullptr;
    const unsigned short* m_pFrondIndicesPtr = nullptr;

    // Leaf geometry
    unsigned short m_usLeafCount = 0;
    std::vector<float> m_vLeafCenters;
    std::vector<unsigned long> m_vLeafColors;
    std::vector<float> m_vLeafCoordsData;
    std::vector<const float*> m_vLeafCoordsPtrs;
    std::vector<float> m_vLeafTexCoordsData;
    std::vector<const float*> m_vLeafTexCoordsPtrs;

    // Bounding box
    float m_afBoundingBox[6] = {-500.0f, -500.0f, 0.0f, 500.0f, 500.0f, 1500.0f};

    // Parsed Leaf UV rectangles (fallback)
    std::vector<std::vector<float>> m_vParsedLeafRects;

    bool m_bLoadedFromCache = false;
    int m_nRefCount = 1;

    void SetupPointers()
    {
        m_usBranchStripLen = (unsigned short)m_vBranchIndices.size();
        m_pBranchStripLenPtr = &m_usBranchStripLen;
        m_pBranchIndicesPtr = m_vBranchIndices.data();

        m_usFrondStripLen = (unsigned short)m_vFrondIndices.size();
        m_pFrondStripLenPtr = &m_usFrondStripLen;
        m_pFrondIndicesPtr = m_vFrondIndices.data();

        m_vLeafCoordsPtrs.resize(m_usLeafCount);
        m_vLeafTexCoordsPtrs.resize(m_usLeafCount);
        for (unsigned short l = 0; l < m_usLeafCount; ++l)
        {
            m_vLeafCoordsPtrs[l] = &m_vLeafCoordsData[l * 16];
            m_vLeafTexCoordsPtrs[l] = &m_vLeafTexCoordsData[l * 8];
        }
    }
};

static std::unordered_map<std::string, CachedTreeEntry> s_treeCache;
static bool s_treeCacheLoaded = false;

static bool ParseTreeCacheBuffer(const unsigned char* pData, size_t nSize)
{
    if (!pData || nSize < 8) return false;
    if (memcmp(pData, "STC1", 4) != 0) return false;

    size_t offset = 4;
    uint32_t numTrees = *(const uint32_t*)(pData + offset);
    offset += 4;

    for (uint32_t i = 0; i < numTrees; ++i)
    {
        CachedTreeEntry t;
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
            memcpy(t.collisions.data(), pData + offset, numCol * sizeof(CachedTreeEntry::ColObj));
            offset += numCol * sizeof(CachedTreeEntry::ColObj);
        }

        // Branches
        t.branchVertexCount = *(const uint32_t*)(pData + offset); offset += 4;
        if (t.branchVertexCount > 0)
        {
            t.branchCoords.resize(t.branchVertexCount * 3);
            memcpy(t.branchCoords.data(), pData + offset, t.branchVertexCount * 3 * sizeof(float));
            offset += t.branchVertexCount * 3 * sizeof(float);

            t.branchColors.resize(t.branchVertexCount);
            const uint32_t* pCols = (const uint32_t*)(pData + offset);
            for (uint32_t v = 0; v < t.branchVertexCount; ++v)
                t.branchColors[v] = (unsigned long)pCols[v];
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
            const uint32_t* pCols = (const uint32_t*)(pData + offset);
            for (uint32_t v = 0; v < t.frondVertexCount; ++v)
                t.frondColors[v] = (unsigned long)pCols[v];
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
            const uint32_t* pCols = (const uint32_t*)(pData + offset);
            for (uint32_t l = 0; l < t.leafCount; ++l)
                t.leafColors[l] = (unsigned long)pCols[l];
            offset += t.leafCount * sizeof(uint32_t);

            t.leafMapCoordsData.resize(t.leafCount * 16);
            memcpy(t.leafMapCoordsData.data(), pData + offset, t.leafCount * 16 * sizeof(float));
            offset += t.leafCount * 16 * sizeof(float);

            t.leafMapTexCoordsData.resize(t.leafCount * 8);
            memcpy(t.leafMapTexCoordsData.data(), pData + offset, t.leafCount * 8 * sizeof(float));
            offset += t.leafCount * 8 * sizeof(float);
        }

        std::string lowerName = t.name;
        for (char& c : lowerName) c = (char)tolower((unsigned char)c);
        s_treeCache[lowerName] = t;
    }

    s_treeCacheLoaded = true;
    return true;
}

static bool EnsureTreeCacheLoaded()
{
    if (s_treeCacheLoaded) return true;

#ifdef __ANDROID__
    if (g_pAssetManager)
    {
        AAsset* asset = AAssetManager_open(g_pAssetManager, "speedtree_cache.dat", AASSET_MODE_BUFFER);
        if (asset)
        {
            size_t sz = (size_t)AAsset_getLength(asset);
            const unsigned char* buf = (const unsigned char*)AAsset_getBuffer(asset);
            if (buf && sz > 0)
            {
                if (ParseTreeCacheBuffer(buf, sz))
                {
                    LOGI("SpeedTree: Successfully loaded %zu trees from APK assets/speedtree_cache.dat (direct buffer)", s_treeCache.size());
                    AAsset_close(asset);
                    return true;
                }
            }
            else if (sz > 0)
            {
                std::vector<unsigned char> readBuf(sz);
                int readBytes = AAsset_read(asset, readBuf.data(), sz);
                if (readBytes > 0 && ParseTreeCacheBuffer(readBuf.data(), (size_t)readBytes))
                {
                    LOGI("SpeedTree: Successfully loaded %zu trees from APK assets/speedtree_cache.dat (stream buffer)", s_treeCache.size());
                    AAsset_close(asset);
                    return true;
                }
            }
            AAsset_close(asset);
        }
    }
#endif

    const char* fallbackPaths[] = {
        "/sdcard/aspar2/speedtree_cache.dat",
        "/storage/emulated/0/aspar2/speedtree_cache.dat",
        "/sdcard/metin2/speedtree_cache.dat",
        "/storage/emulated/0/metin2/speedtree_cache.dat",
        "pack/speedtree_cache.dat",
        "speedtree_cache.dat",
        "d:/ymir work/tree/speedtree_cache.dat",
        "Data_Pack/tree/speedtree_cache.dat"
    };

    for (const char* path : fallbackPaths)
    {
        FILE* fp = fopen(path, "rb");
        if (fp)
        {
            fseek(fp, 0, SEEK_END);
            long sz = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            if (sz > 0)
            {
                std::vector<unsigned char> buf(sz);
                fread(buf.data(), 1, sz, fp);
                fclose(fp);
                if (ParseTreeCacheBuffer(buf.data(), (size_t)sz))
                {
                    LOGI("SpeedTree: Successfully loaded %zu trees from fallback file '%s'", s_treeCache.size(), path);
                    return true;
                }
            }
            else
            {
                fclose(fp);
            }
        }
    }

    TraceError("SpeedTree: Warning - speedtree_cache.dat not found! Will use procedural fallback.");
    return false;
}

static void ApplyCachedTree(InternalTreeData* pTree, const CachedTreeEntry& entry)
{
    memcpy(pTree->m_afBoundingBox, entry.boundingBox, sizeof(pTree->m_afBoundingBox));
    pTree->m_strBark = entry.branchTexture;
    pTree->m_strComposite = entry.compositeTexture;
    pTree->m_strShadow = entry.shadowTexture;

    memcpy(pTree->m_afBranchMaterial, entry.branchMaterial, sizeof(pTree->m_afBranchMaterial));
    memcpy(pTree->m_afFrondMaterial, entry.frondMaterial, sizeof(pTree->m_afFrondMaterial));
    memcpy(pTree->m_afLeafMaterial, entry.leafMaterial, sizeof(pTree->m_afLeafMaterial));
    pTree->m_fLeafLightingAdjustment = entry.leafLightingAdjustment;

    pTree->m_vCollisions.clear();
    for (const auto& col : entry.collisions)
    {
        InternalTreeData::ColObj obj;
        obj.type = col.type;
        memcpy(obj.pos, col.pos, sizeof(obj.pos));
        memcpy(obj.dim, col.dim, sizeof(obj.dim));
        pTree->m_vCollisions.push_back(obj);
    }

    pTree->m_vBranchCoords = entry.branchCoords;
    pTree->m_vBranchColors = entry.branchColors;
    pTree->m_vBranchTexCoords0 = entry.branchTex0;
    pTree->m_vBranchTexCoords1 = entry.branchTex1;
    pTree->m_vBranchIndices = entry.branchIndices;

    pTree->m_vFrondCoords = entry.frondCoords;
    pTree->m_vFrondColors = entry.frondColors;
    pTree->m_vFrondTexCoords0 = entry.frondTex0;
    pTree->m_vFrondTexCoords1 = entry.frondTex1;
    pTree->m_vFrondIndices = entry.frondIndices;

    pTree->m_usLeafCount = (unsigned short)entry.leafCount;
    pTree->m_vLeafCenters = entry.leafCenters;
    pTree->m_vLeafColors = entry.leafColors;
    pTree->m_vLeafCoordsData = entry.leafMapCoordsData;
    pTree->m_vLeafTexCoordsData = entry.leafMapTexCoordsData;

    pTree->SetupPointers();
    pTree->m_bLoadedFromCache = true;
}

static void ParseSptTextures(const unsigned char* pData, unsigned int nSize, InternalTreeData* pTree)
{
    if (!pData || nSize == 0 || !pTree)
        return;

    const std::string sData((const char*)pData, nSize);
    std::string lowerData = sData;
    for (char& c : lowerData) c = (char)tolower((unsigned char)c);

    // 1. Bark texture
    size_t barkPos = lowerData.find("bark");
    if (barkPos != std::string::npos)
    {
        size_t start = barkPos;
        while (start > 0 && isalnum((unsigned char)sData[start - 1]))
            --start;
        size_t end = barkPos;
        while (end < sData.size() && isalnum((unsigned char)sData[end]))
            ++end;
        pTree->m_strBark = sData.substr(start, end - start);
    }
    if (pTree->m_strBark.empty())
        pTree->m_strBark = "pagodatreebark";

    // 2. Composite texture
    size_t compPos = lowerData.find("compositemap");
    if (compPos != std::string::npos)
    {
        size_t start = compPos;
        size_t end = compPos;
        while (end < sData.size() && isalnum((unsigned char)sData[end]))
            ++end;
        pTree->m_strComposite = sData.substr(start, end - start);
    }
    if (pTree->m_strComposite.empty())
        pTree->m_strComposite = "compositemapb1";

    // 3. Shadow texture
    size_t shadowPos = lowerData.find("compositeshadowmap");
    if (shadowPos != std::string::npos)
    {
        size_t start = shadowPos;
        size_t end = shadowPos;
        while (end < sData.size() && isalnum((unsigned char)sData[end]))
            ++end;
        pTree->m_strShadow = sData.substr(start, end - start);
    }
    if (pTree->m_strShadow.empty())
        pTree->m_strShadow = "compositeshadowmapb1";

    for (char& c : pTree->m_strBark) c = (char)tolower((unsigned char)c);
    for (char& c : pTree->m_strComposite) c = (char)tolower((unsigned char)c);
    for (char& c : pTree->m_strShadow) c = (char)tolower((unsigned char)c);

    // 4. Parse leaf UV rectangles byte-by-byte
    pTree->m_vParsedLeafRects.clear();
    for (size_t i = 0; i + 32 <= nSize; ++i)
    {
        const float* f8 = (const float*)(pData + i);
        bool inRange = true;
        for (int k = 0; k < 8; ++k)
        {
            if (std::isnan(f8[k]) || (f8[k] != 0.0f && (f8[k] < 0.04f || f8[k] > 1.0f)))
            {
                inRange = false;
                break;
            }
        }
        if (!inRange) continue;

        float u_vals[4] = { f8[0], f8[2], f8[4], f8[6] };
        float v_vals[4] = { f8[1], f8[3], f8[5], f8[7] };

        float u_min = u_vals[0], u_max = u_vals[0];
        float v_min = v_vals[0], v_max = v_vals[0];
        for (int k = 1; k < 4; ++k)
        {
            if (u_vals[k] < u_min) u_min = u_vals[k];
            if (u_vals[k] > u_max) u_max = u_vals[k];
            if (v_vals[k] < v_min) v_min = v_vals[k];
            if (v_vals[k] > v_max) v_max = v_vals[k];
        }

        float du = u_max - u_min;
        float dv = v_max - v_min;
        if (du < 0.08f || du > 0.50f || dv < 0.08f || dv > 0.50f)
            continue;

        int count_umin = 0, count_umax = 0;
        int count_vmin = 0, count_vmax = 0;
        for (int k = 0; k < 4; ++k)
        {
            if (fabsf(u_vals[k] - u_min) < 0.005f) count_umin++;
            if (fabsf(u_vals[k] - u_max) < 0.005f) count_umax++;
            if (fabsf(v_vals[k] - v_min) < 0.005f) count_vmin++;
            if (fabsf(v_vals[k] - v_max) < 0.005f) count_vmax++;
        }
        if (count_umin != 2 || count_umax != 2 || count_vmin != 2 || count_vmax != 2)
            continue;

        bool dup = false;
        for (const auto& r : pTree->m_vParsedLeafRects)
        {
            if (fabsf(r[0] - u_max) < 0.005f && fabsf(r[1] - v_max) < 0.005f &&
                fabsf(r[2] - u_min) < 0.005f && fabsf(r[5] - v_min) < 0.005f)
            {
                dup = true;
                break;
            }
        }
        if (dup) continue;

        // Normalized CCW rectangle: (u_max, v_max), (u_min, v_max), (u_min, v_min), (u_max, v_min)
        std::vector<float> rect = { u_max, v_max, u_min, v_max, u_min, v_min, u_max, v_min };
        pTree->m_vParsedLeafRects.push_back(rect);
    }

    if (pTree->m_vParsedLeafRects.empty())
    {
        pTree->m_vParsedLeafRects.push_back({ 0.5f, 0.5f, 0.25f, 0.5f, 0.25f, 0.25f, 0.5f, 0.25f });
        pTree->m_vParsedLeafRects.push_back({ 0.25f, 0.75f, 0.0f, 0.75f, 0.0f, 0.5f, 0.25f, 0.5f });
        pTree->m_vParsedLeafRects.push_back({ 0.75f, 0.3691f, 0.625f, 0.3691f, 0.625f, 0.2471f, 0.75f, 0.2471f });
        pTree->m_vParsedLeafRects.push_back({ 0.625f, 0.8584f, 0.5f, 0.8584f, 0.5f, 0.7354f, 0.625f, 0.7354f });
    }
}

static void GenerateTreeGeometry(InternalTreeData* pTree, unsigned int nSeed)
{
    float H = pTree->m_fSize;
    if (H <= 10.0f) H = 800.0f;

    float R = H * 0.045f;
    float canopyRadius = H * 0.35f;

    pTree->m_afBoundingBox[0] = -canopyRadius * 1.2f;
    pTree->m_afBoundingBox[1] = -canopyRadius * 1.2f;
    pTree->m_afBoundingBox[2] = 0.0f;
    pTree->m_afBoundingBox[3] = canopyRadius * 1.2f;
    pTree->m_afBoundingBox[4] = canopyRadius * 1.2f;
    pTree->m_afBoundingBox[5] = H * 1.1f;

    // 1. Trunk Cylinder (6 rings of 8 vertices)
    const int NUM_RINGS = 6;
    const int NUM_SEGS = 8;
    const float ringHeights[NUM_RINGS] = { 0.0f, 0.15f * H, 0.35f * H, 0.55f * H, 0.75f * H, 0.95f * H };
    const float ringRadii[NUM_RINGS]   = { R * 1.4f, R * 1.0f, R * 0.85f, R * 0.70f, R * 0.45f, R * 0.10f };

    pTree->m_vBranchCoords.clear();
    pTree->m_vBranchColors.clear();
    pTree->m_vBranchTexCoords0.clear();
    pTree->m_vBranchTexCoords1.clear();
    pTree->m_vBranchIndices.clear();

    const float TWO_PI = 6.28318530718f;

    for (int r = 0; r < NUM_RINGS; ++r)
    {
        float z = ringHeights[r];
        float rad = ringRadii[r];
        for (int s = 0; s < NUM_SEGS; ++s)
        {
            float theta = TWO_PI * ((float)s / (float)NUM_SEGS);
            float x = rad * cosf(theta);
            float y = rad * sinf(theta);

            pTree->m_vBranchCoords.push_back(x);
            pTree->m_vBranchCoords.push_back(y);
            pTree->m_vBranchCoords.push_back(z);

            pTree->m_vBranchColors.push_back(0xFFDCDCDC);

            pTree->m_vBranchTexCoords0.push_back((float)s / (float)NUM_SEGS);
            pTree->m_vBranchTexCoords0.push_back(z / 200.0f);

            pTree->m_vBranchTexCoords1.push_back((float)s / (float)NUM_SEGS);
            pTree->m_vBranchTexCoords1.push_back(z / H);
        }
    }

    for (int r = 0; r < NUM_RINGS - 1; ++r)
    {
        int ringA = r * NUM_SEGS;
        int ringB = (r + 1) * NUM_SEGS;

        if (r > 0)
        {
            pTree->m_vBranchIndices.push_back((unsigned short)pTree->m_vBranchIndices.back());
            pTree->m_vBranchIndices.push_back((unsigned short)ringA);
        }

        for (int s = 0; s <= NUM_SEGS; ++s)
        {
            int idxA = ringA + (s % NUM_SEGS);
            int idxB = ringB + (s % NUM_SEGS);
            pTree->m_vBranchIndices.push_back((unsigned short)idxA);
            pTree->m_vBranchIndices.push_back((unsigned short)idxB);
        }
    }

    pTree->m_usBranchStripLen = (unsigned short)pTree->m_vBranchIndices.size();
    pTree->m_pBranchStripLenPtr = &pTree->m_usBranchStripLen;
    pTree->m_pBranchIndicesPtr = pTree->m_vBranchIndices.data();

    // 2. Leaf Foliage Clusters (60 clusters x 2 crossing quads = 120 quads)
    const int NUM_CLUSTERS = 60;
    const int QUADS_PER_CLUSTER = 2;
    const int TOTAL_LEAVES = NUM_CLUSTERS * QUADS_PER_CLUSTER;

    pTree->m_usLeafCount = TOTAL_LEAVES;
    pTree->m_vLeafCenters.resize(TOTAL_LEAVES * 3);
    pTree->m_vLeafColors.assign(TOTAL_LEAVES, 0xFFFFFFFF);
    pTree->m_vLeafCoordsData.resize(TOTAL_LEAVES * 16);
    pTree->m_vLeafCoordsPtrs.resize(TOTAL_LEAVES);
    pTree->m_vLeafTexCoordsData.resize(TOTAL_LEAVES * 8);
    pTree->m_vLeafTexCoordsPtrs.resize(TOTAL_LEAVES);

    float leafW = H * 0.30f;
    float leafH = H * 0.30f;

    size_t numRects = pTree->m_vParsedLeafRects.size();
    if (numRects == 0)
    {
        pTree->m_vParsedLeafRects.push_back({ 0.5f, 0.5f, 0.25f, 0.5f, 0.25f, 0.25f, 0.5f, 0.25f });
        numRects = 1;
    }

    int leafIdx = 0;
    for (int c = 0; c < NUM_CLUSTERS; ++c)
    {
        float frac = (float)c / (float)NUM_CLUSTERS;
        float z = H * (0.35f + 0.65f * frac);
        float radiusFactor = sinf(frac * 3.14159265f);
        float r = canopyRadius * (0.30f + 0.70f * sqrtf(radiusFactor));
        float phi = (float)c * 2.399963229728f; // Golden angle

        float cx = r * cosf(phi);
        float cy = r * sinf(phi);

        float tilt = 0.20f + 0.30f * (1.0f - frac);

        // Quad A orientation
        float ux1 = -sinf(phi) * (leafW * 0.5f);
        float uy1 =  cosf(phi) * (leafW * 0.5f);
        float uz1 = 0.0f;

        float vx1 = -cosf(phi) * tilt * (leafH * 0.5f);
        float vy1 = -sinf(phi) * tilt * (leafH * 0.5f);
        float vz1 = (1.0f - tilt * 0.5f) * (leafH * 0.5f);

        // Quad B orientation (crossing at ~90 degrees)
        float ux2 =  cosf(phi) * (leafW * 0.5f);
        float uy2 =  sinf(phi) * (leafW * 0.5f);
        float uz2 = 0.0f;

        float vx2 =  sinf(phi) * tilt * (leafH * 0.5f);
        float vy2 = -cosf(phi) * tilt * (leafH * 0.5f);
        float vz2 = (1.0f - tilt * 0.5f) * (leafH * 0.5f);

        const std::vector<float>& rectUV1 = pTree->m_vParsedLeafRects[(c * 2) % numRects];
        const std::vector<float>& rectUV2 = pTree->m_vParsedLeafRects[(c * 2 + 1) % numRects];

        // Store Quad A
        {
            int l = leafIdx++;
            pTree->m_vLeafCenters[l * 3 + 0] = cx;
            pTree->m_vLeafCenters[l * 3 + 1] = cy;
            pTree->m_vLeafCenters[l * 3 + 2] = z;

            float* pCoord = &pTree->m_vLeafCoordsData[l * 16];
            pCoord[0]  = -ux1 - vx1; pCoord[1]  = -uy1 - vy1; pCoord[2]  = -uz1 - vz1; pCoord[3]  = 0.0f;
            pCoord[4]  =  ux1 - vx1; pCoord[5]  =  uy1 - vy1; pCoord[6]  =  uz1 - vz1; pCoord[7]  = 0.0f;
            pCoord[8]  =  ux1 + vx1; pCoord[9]  =  uy1 + vy1; pCoord[10] =  uz1 + vz1; pCoord[11] = 0.0f;
            pCoord[12] = -ux1 + vx1; pCoord[13] = -uy1 + vy1; pCoord[14] = -uz1 + vz1; pCoord[15] = 0.0f;
            pTree->m_vLeafCoordsPtrs[l] = pCoord;

            float* pTex = &pTree->m_vLeafTexCoordsData[l * 8];
            memcpy(pTex, rectUV1.data(), 8 * sizeof(float));
            pTree->m_vLeafTexCoordsPtrs[l] = pTex;
        }

        // Store Quad B (crossed)
        {
            int l = leafIdx++;
            pTree->m_vLeafCenters[l * 3 + 0] = cx;
            pTree->m_vLeafCenters[l * 3 + 1] = cy;
            pTree->m_vLeafCenters[l * 3 + 2] = z;

            float* pCoord = &pTree->m_vLeafCoordsData[l * 16];
            pCoord[0]  = -ux2 - vx2; pCoord[1]  = -uy2 - vy2; pCoord[2]  = -uz2 - vz2; pCoord[3]  = 0.0f;
            pCoord[4]  =  ux2 - vx2; pCoord[5]  =  uy2 - vy2; pCoord[6]  =  uz2 - vz2; pCoord[7]  = 0.0f;
            pCoord[8]  =  ux2 + vx2; pCoord[9]  =  uy2 + vy2; pCoord[10] =  uz2 + vz2; pCoord[11] = 0.0f;
            pCoord[12] = -ux2 + vx2; pCoord[13] = -uy2 + vy2; pCoord[14] = -uz2 + vz2; pCoord[15] = 0.0f;
            pTree->m_vLeafCoordsPtrs[l] = pCoord;

            float* pTex = &pTree->m_vLeafTexCoordsData[l * 8];
            memcpy(pTex, rectUV2.data(), 8 * sizeof(float));
            pTree->m_vLeafTexCoordsPtrs[l] = pTex;
        }
    }
}

// CSpeedTreeRT Lifecycle & Memory
CSpeedTreeRT::CSpeedTreeRT() {
    m_pEngine = (CTreeEngine*) new InternalTreeData;
}

CSpeedTreeRT::~CSpeedTreeRT() {
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (pTree)
    {
        if (pTree->m_pBaseTree)
        {
            pTree->m_pBaseTree->m_nRefCount--;
            if (pTree->m_pBaseTree->m_nRefCount <= 0)
                delete pTree->m_pBaseTree;
            pTree->m_pBaseTree = nullptr;
        }
        pTree->m_nRefCount--;
        if (pTree->m_nRefCount <= 0)
            delete pTree;
        m_pEngine = nullptr;
    }
}

void* CSpeedTreeRT::operator new(size_t nSize) { return malloc(nSize); }
void CSpeedTreeRT::operator delete(void* pRawMemory) { free(pRawMemory); }

// Tree Loading & Instances
bool CSpeedTreeRT::LoadTree(const char* pszFilename)
{
    if (!m_pEngine)
        m_pEngine = (CTreeEngine*) new InternalTreeData;
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;

    EnsureTreeCacheLoaded();

    if (pszFilename)
    {
        std::string fn = pszFilename;
        size_t slash = fn.find_last_of("/\\");
        if (slash != std::string::npos) fn = fn.substr(slash + 1);
        for (char& c : fn) c = (char)tolower((unsigned char)c);

        auto it = s_treeCache.find(fn);
        if (it != s_treeCache.end())
        {
            ApplyCachedTree(pTree, it->second);
            // LOGI("SpeedTree: Loaded authentic PC tree geometry for '%s'...", fn.c_str());
            return true;
        }
    }

    FILE* fp = fopen(pszFilename, "rb");
    if (fp)
    {
        fseek(fp, 0, SEEK_END);
        long sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        if (sz > 0)
        {
            std::vector<unsigned char> buf(sz);
            fread(buf.data(), 1, sz, fp);
            ParseSptTextures(buf.data(), sz, pTree);
        }
        fclose(fp);
    }
    return true;
}

bool CSpeedTreeRT::LoadTree(const unsigned char* pBlock, unsigned int nNumBytes)
{
    if (!m_pEngine)
        m_pEngine = (CTreeEngine*) new InternalTreeData;
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;

    ParseSptTextures(pBlock, nNumBytes, pTree);

    EnsureTreeCacheLoaded();

    if (!pTree->m_bLoadedFromCache && !s_treeCache.empty())
    {
        for (const auto& kv : s_treeCache)
        {
            std::string cachedBark = kv.second.branchTexture;
            for (char& c : cachedBark) c = (char)tolower((unsigned char)c);
            if (!pTree->m_strBark.empty() && cachedBark.find(pTree->m_strBark) != std::string::npos)
            {
                ApplyCachedTree(pTree, kv.second);
                // LOGI("SpeedTree: Matched cached tree '%s' via bark '%s'!", kv.first.c_str(), pTree->m_strBark.c_str());
                return true;
            }
        }
    }

    return true;
}

CSpeedTreeRT* CSpeedTreeRT::MakeInstance()
{
    CSpeedTreeRT* pInst = new CSpeedTreeRT();
    InternalTreeData* pBase = (InternalTreeData*)m_pEngine;
    InternalTreeData* pInstData = (InternalTreeData*)pInst->m_pEngine;
    if (pBase && pInstData)
    {
        if (pBase->m_pBaseTree)
            pBase = pBase->m_pBaseTree;

        pInstData->m_pBaseTree = pBase;
        pBase->m_nRefCount++;

        pInstData->m_fSize = pBase->m_fSize;
        pInstData->m_fVariance = pBase->m_fVariance;
        memcpy(pInstData->m_afBoundingBox, pBase->m_afBoundingBox, sizeof(pBase->m_afBoundingBox));
        pInstData->m_strBark = pBase->m_strBark;
        pInstData->m_strComposite = pBase->m_strComposite;
        pInstData->m_strShadow = pBase->m_strShadow;
        pInstData->m_bLoadedFromCache = pBase->m_bLoadedFromCache;
        pInstData->m_afPos[0] = 0.0f;
        pInstData->m_afPos[1] = 0.0f;
        pInstData->m_afPos[2] = 0.0f;
    }
    return pInst;
}

void CSpeedTreeRT::DeleteTransientData() {}

// Positioning & Sizing
static float s_treePos[3] = {0.0f, 0.0f, 0.0f};

const float* CSpeedTreeRT::GetTreePosition() const
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (pTree)
        return pTree->m_afPos;
    return s_treePos;
}

void CSpeedTreeRT::SetTreePosition(float x, float y, float z)
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (pTree)
    {
        pTree->m_afPos[0] = x;
        pTree->m_afPos[1] = y;
        pTree->m_afPos[2] = z;
    }
    s_treePos[0] = x;
    s_treePos[1] = y;
    s_treePos[2] = z;
}

void CSpeedTreeRT::GetTreeSize(float& fSize, float& fVariance) const
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (pTree)
    {
        if (pTree->m_pBaseTree) pTree = pTree->m_pBaseTree;
        fSize = pTree->m_fSize;
        fVariance = pTree->m_fVariance;
    }
    else
    {
        fSize = 800.0f;
        fVariance = 0.0f;
    }
}

void CSpeedTreeRT::SetTreeSize(float fSize, float fVariance)
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (pTree)
    {
        pTree->m_fSize = fSize;
        pTree->m_fVariance = fVariance;
    }
}

void CSpeedTreeRT::GetBoundingBox(float* pBounds) const
{
    if (pBounds)
    {
        InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
        if (pTree)
        {
            if (pTree->m_pBaseTree)
                memcpy(pBounds, pTree->m_pBaseTree->m_afBoundingBox, 6 * sizeof(float));
            else
                memcpy(pBounds, pTree->m_afBoundingBox, 6 * sizeof(float));
        }
        else
        {
            pBounds[0] = -500.0f; pBounds[1] = -500.0f; pBounds[2] = 0.0f;
            pBounds[3] =  500.0f; pBounds[4] =  500.0f; pBounds[5] = 1500.0f;
        }
    }
}

// Lighting & Materials
static float s_defaultMaterial[4] = {1.0f, 1.0f, 1.0f, 1.0f};
void CSpeedTreeRT::SetBranchLightingMethod(ELightingMethod) {}
void CSpeedTreeRT::SetFrondLightingMethod(ELightingMethod) {}
void CSpeedTreeRT::SetLeafLightingMethod(ELightingMethod) {}

float CSpeedTreeRT::GetLeafLightingAdjustment() const
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (pTree) {
        if (pTree->m_pBaseTree) pTree = pTree->m_pBaseTree;
        return pTree->m_fLeafLightingAdjustment;
    }
    return 1.0f;
}

const float* CSpeedTreeRT::GetBranchMaterial() const
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (pTree) {
        if (pTree->m_pBaseTree) pTree = pTree->m_pBaseTree;
        return pTree->m_afBranchMaterial;
    }
    return s_defaultMaterial;
}

const float* CSpeedTreeRT::GetFrondMaterial() const
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (pTree) {
        if (pTree->m_pBaseTree) pTree = pTree->m_pBaseTree;
        return pTree->m_afFrondMaterial;
    }
    return s_defaultMaterial;
}

const float* CSpeedTreeRT::GetLeafMaterial() const
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (pTree) {
        if (pTree->m_pBaseTree) pTree = pTree->m_pBaseTree;
        return pTree->m_afLeafMaterial;
    }
    return s_defaultMaterial;
}

void CSpeedTreeRT::SetLightState(unsigned int, bool) {}
void CSpeedTreeRT::SetLightAttributes(unsigned int, const float*) {}

// Camera & Wind
void CSpeedTreeRT::SetCamera(const float*, const float*) {}
void CSpeedTreeRT::SetTime(float) {}
void CSpeedTreeRT::SetBranchWindMethod(EWindMethod) {}
void CSpeedTreeRT::SetFrondWindMethod(EWindMethod) {}
void CSpeedTreeRT::SetLeafWindMethod(EWindMethod) {}
void CSpeedTreeRT::SetLeafRockingState(bool) {}
void CSpeedTreeRT::SetNumLeafRockingGroups(unsigned int) {}
void CSpeedTreeRT::SetNumWindMatrices(unsigned int) {}
float CSpeedTreeRT::SetWindStrength(float, float, float) { return 0.0f; }

// LOD
void CSpeedTreeRT::SetLodLevel(float) {}
void CSpeedTreeRT::SetDropToBillboard(bool b) { m_bDropToBillboard = b; }
unsigned short CSpeedTreeRT::GetNumBranchLodLevels() const { return 1; }
unsigned short CSpeedTreeRT::GetNumLeafLodLevels() const { return 1; }
unsigned short CSpeedTreeRT::GetNumFrondLodLevels() const { return 1; }

// Geometry & Textures & Computation
bool CSpeedTreeRT::Compute(const float*, unsigned int nSeed, bool)
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (!pTree)
    {
        m_pEngine = (CTreeEngine*) new InternalTreeData;
        pTree = (InternalTreeData*)m_pEngine;
    }
    if (pTree->m_pBaseTree)
        pTree = pTree->m_pBaseTree;

    if (!pTree->m_bLoadedFromCache)
    {
        GenerateTreeGeometry(pTree, nSeed);
    }
    return true;
}

void CSpeedTreeRT::GetGeometry(SGeometry& sGeometry, unsigned long ulBitVector, short sOverrideBranchLodValue, short sOverrideFrondLodValue, short sOverrideLeafLodValue)
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (!pTree)
        return;
    if (pTree->m_pBaseTree)
        pTree = pTree->m_pBaseTree;

    // Branches
    if (ulBitVector & SpeedTree_BranchGeometry)
    {
        sGeometry.m_fBranchAlphaTestValue = 84.0f;
        sGeometry.m_sBranches.m_nDiscreteLodLevel = 0;
        sGeometry.m_sBranches.m_usVertexCount = (unsigned short)(pTree->m_vBranchCoords.size() / 3);
        sGeometry.m_sBranches.m_pCoords = pTree->m_vBranchCoords.data();
        sGeometry.m_sBranches.m_pColors = pTree->m_vBranchColors.data();
        sGeometry.m_sBranches.m_pTexCoords0 = pTree->m_vBranchTexCoords0.data();
        sGeometry.m_sBranches.m_pTexCoords1 = pTree->m_vBranchTexCoords1.data();
        sGeometry.m_sBranches.m_usNumStrips = (pTree->m_usBranchStripLen > 0) ? 1 : 0;
        sGeometry.m_sBranches.m_pStripLengths = pTree->m_pBranchStripLenPtr;
        sGeometry.m_sBranches.m_pStrips = &pTree->m_pBranchIndicesPtr;
    }

    // Fronds
    if (ulBitVector & SpeedTree_FrondGeometry)
    {
        sGeometry.m_fFrondAlphaTestValue = 84.0f;
        sGeometry.m_sFronds.m_nDiscreteLodLevel = (pTree->m_usFrondStripLen > 0) ? 0 : -1;
        sGeometry.m_sFronds.m_usVertexCount = (unsigned short)(pTree->m_vFrondCoords.size() / 3);
        sGeometry.m_sFronds.m_pCoords = pTree->m_vFrondCoords.data();
        sGeometry.m_sFronds.m_pColors = pTree->m_vFrondColors.data();
        sGeometry.m_sFronds.m_pTexCoords0 = pTree->m_vFrondTexCoords0.data();
        sGeometry.m_sFronds.m_pTexCoords1 = pTree->m_vFrondTexCoords1.data();
        sGeometry.m_sFronds.m_usNumStrips = (pTree->m_usFrondStripLen > 0) ? 1 : 0;
        sGeometry.m_sFronds.m_pStripLengths = pTree->m_pFrondStripLenPtr;
        sGeometry.m_sFronds.m_pStrips = &pTree->m_pFrondIndicesPtr;
    }

    // Leaves
    if (ulBitVector & SpeedTree_LeafGeometry)
    {
        sGeometry.m_sLeaves0.m_bIsActive = (pTree->m_usLeafCount > 0);
        sGeometry.m_sLeaves0.m_fAlphaTestValue = 84.0f;
        sGeometry.m_sLeaves0.m_nDiscreteLodLevel = 0;
        sGeometry.m_sLeaves0.m_usLeafCount = pTree->m_usLeafCount;
        sGeometry.m_sLeaves0.m_pCenterCoords = pTree->m_vLeafCenters.data();
        sGeometry.m_sLeaves0.m_pColors = pTree->m_vLeafColors.data();
        sGeometry.m_sLeaves0.m_pLeafMapCoords = (const float**)pTree->m_vLeafCoordsPtrs.data();
        sGeometry.m_sLeaves0.m_pLeafMapTexCoords = (const float**)pTree->m_vLeafTexCoordsPtrs.data();

        sGeometry.m_sLeaves1.m_bIsActive = false;
        sGeometry.m_sLeaves1.m_usLeafCount = 0;
    }
}

void CSpeedTreeRT::GetTextures(STextures& sTextures) const
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (pTree)
    {
        if (pTree->m_pBaseTree)
            pTree = pTree->m_pBaseTree;
        sTextures.m_pBranchTextureFilename = pTree->m_strBark.c_str();
        sTextures.m_pCompositeFilename = pTree->m_strComposite.c_str();
        sTextures.m_pSelfShadowFilename = pTree->m_strShadow.c_str();
    }
    else
    {
        sTextures.m_pBranchTextureFilename = "pagodatreebark";
        sTextures.m_pCompositeFilename = "compositemapb1";
        sTextures.m_pSelfShadowFilename = "compositeshadowmapb1";
    }
    sTextures.m_uiLeafTextureCount = 0;
    sTextures.m_pLeafTextureFilenames = nullptr;
    sTextures.m_uiFrondTextureCount = 0;
    sTextures.m_pFrondTextureFilenames = nullptr;
}

void CSpeedTreeRT::SetTextureFlip(bool b) { m_bTextureFlip = b; }

// Collision & Error
unsigned int CSpeedTreeRT::GetCollisionObjectCount()
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (!pTree) return 0;
    if (pTree->m_pBaseTree) pTree = pTree->m_pBaseTree;
    return (unsigned int)pTree->m_vCollisions.size();
}

void CSpeedTreeRT::GetCollisionObject(unsigned int nIndex, ECollisionObjectType& eType, float* pPosition, float* pDimensions)
{
    InternalTreeData* pTree = (InternalTreeData*)m_pEngine;
    if (!pTree) return;
    if (pTree->m_pBaseTree) pTree = pTree->m_pBaseTree;
    if (nIndex < pTree->m_vCollisions.size())
    {
        eType = (ECollisionObjectType)pTree->m_vCollisions[nIndex].type;
        if (pPosition) memcpy(pPosition, pTree->m_vCollisions[nIndex].pos, 3 * sizeof(float));
        if (pDimensions) memcpy(pDimensions, pTree->m_vCollisions[nIndex].dim, 3 * sizeof(float));
    }
}

const char* CSpeedTreeRT::GetCurrentError() { return ""; }

// Geometry Struct Constructors / Destructors
CSpeedTreeRT::SGeometry::SGeometry() { memset(this, 0, sizeof(*this)); }
CSpeedTreeRT::SGeometry::~SGeometry() {}
CSpeedTreeRT::SGeometry::SIndexed::SIndexed() { memset(this, 0, sizeof(*this)); }
CSpeedTreeRT::SGeometry::SIndexed::~SIndexed() {}
CSpeedTreeRT::SGeometry::SLeaf::SLeaf() { memset(this, 0, sizeof(*this)); }
CSpeedTreeRT::SGeometry::SLeaf::~SLeaf() {}
CSpeedTreeRT::SGeometry::SBillboard::SBillboard() { memset(this, 0, sizeof(*this)); }
CSpeedTreeRT::SGeometry::SBillboard::~SBillboard() {}
CSpeedTreeRT::STextures::STextures() { memset(this, 0, sizeof(*this)); }
CSpeedTreeRT::STextures::~STextures() {}
