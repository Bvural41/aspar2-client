// SpeedTreeRT Complete Stubs for Android ARM64 and x86_64
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "SpeedTreeRT.h"

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

bool CSpeedTreeRT::m_bTextureFlip = false;
bool CSpeedTreeRT::m_bDropToBillboard = false;

CSpeedTreeRT::CSpeedTreeRT() {
    m_pEngine = NULL;
    m_pBranchGeometry = NULL;
    m_pLeafGeometry = NULL;
    m_pLightingEngine = NULL;
    m_pWindEngine = NULL;
    m_pSimpleBillboard = NULL;
    m_pInstanceData = NULL;
    m_pInstanceList = NULL;
    m_pRefCount = NULL;
    m_pTreeSizes = NULL;
    m_pEmbeddedTexCoords = NULL;
    m_pProjectedShadow = NULL;
    m_pCollisionObjects = NULL;
    m_pFrondEngine = NULL;
    m_pFrondGeometry = NULL;
    m_pUserData = NULL;
    m_pLeafLodSizeFactors = NULL;
}

CSpeedTreeRT::~CSpeedTreeRT() {}

void* CSpeedTreeRT::operator new(size_t nSize) { return calloc(1, nSize); }
void* CSpeedTreeRT::operator new[](size_t nSize) { return calloc(1, nSize); }
void CSpeedTreeRT::operator delete(void* pRawMemory) { if (pRawMemory) free(pRawMemory); }
void CSpeedTreeRT::operator delete[](void* pRawMemory) { if (pRawMemory) free(pRawMemory); }

bool CSpeedTreeRT::Compute(const float* pTransform, unsigned int nSeed, bool bCompositeStrips) { return true; }
CSpeedTreeRT* CSpeedTreeRT::Clone(float x, float y, float z, unsigned int nSeed) const { return new CSpeedTreeRT(); }
const CSpeedTreeRT* CSpeedTreeRT::InstanceOf(void) const { return this; }
CSpeedTreeRT* CSpeedTreeRT::MakeInstance(void) { return new CSpeedTreeRT(); }
void CSpeedTreeRT::DeleteTransientData(void) {}

bool CSpeedTreeRT::LoadTree(const char* pFilename) { return true; }
bool CSpeedTreeRT::LoadTree(const unsigned char* pBlock, unsigned int nNumBytes) { return true; }
unsigned char* CSpeedTreeRT::SaveTree(unsigned int& nNumBytes, bool bSaveLeaves) const { nNumBytes = 0; return NULL; }

void CSpeedTreeRT::GetTreeSize(float& fSize, float& fVariance) const { fSize = 100.0f; fVariance = 0.0f; }
void CSpeedTreeRT::SetTreeSize(float fNewSize, float fNewVariance) {}
unsigned int CSpeedTreeRT::GetSeed() const { return 1; }

const float* CSpeedTreeRT::GetTreePosition(void) const { static float s_pos[3] = {0,0,0}; return s_pos; }
void CSpeedTreeRT::SetTreePosition(float x, float y, float z) {}
void CSpeedTreeRT::SetLeafTargetAlphaMask(unsigned char ucMask) {}

CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetBranchLightingMethod(void) const { return LIGHT_DYNAMIC; }
void CSpeedTreeRT::SetBranchLightingMethod(ELightingMethod eMethod) {}
CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetLeafLightingMethod(void) const { return LIGHT_DYNAMIC; }
void CSpeedTreeRT::SetLeafLightingMethod(ELightingMethod eMethod) {}
CSpeedTreeRT::ELightingMethod CSpeedTreeRT::GetFrondLightingMethod(void) const { return LIGHT_DYNAMIC; }
void CSpeedTreeRT::SetFrondLightingMethod(ELightingMethod eMethod) {}

CSpeedTreeRT::EStaticLightingStyle CSpeedTreeRT::GetStaticLightingStyle(void) const { return SLS_BASIC; }
void CSpeedTreeRT::SetStaticLightingStyle(EStaticLightingStyle eStyle) {}
float CSpeedTreeRT::GetLeafLightingAdjustment() const { return 1.0f; }
void CSpeedTreeRT::SetLeafLightingAdjustment(float fScalar) {}

bool CSpeedTreeRT::GetLightState(unsigned int nLightIndex) { return true; }
void CSpeedTreeRT::SetLightState(unsigned int nLightIndex, bool bLightOn) {}
const float* CSpeedTreeRT::GetLightAttributes(unsigned int nLightIndex) { static float s_attr[4] = {0,0,0,1}; return s_attr; }
void CSpeedTreeRT::SetLightAttributes(unsigned int nLightIndex, const float* pLightAttributes) {}

const float* CSpeedTreeRT::GetBranchMaterial(void) const { static float s_mat[4] = {1,1,1,1}; return s_mat; }
void CSpeedTreeRT::SetBranchMaterial(const float* pMaterial) {}
const float* CSpeedTreeRT::GetLeafMaterial(void) const { static float s_mat[4] = {1,1,1,1}; return s_mat; }
void CSpeedTreeRT::SetLeafMaterial(const float* pMaterial) {}
const float* CSpeedTreeRT::GetFrondMaterial(void) const { static float s_mat[4] = {1,1,1,1}; return s_mat; }
void CSpeedTreeRT::SetFrondMaterial(const float* pMaterial) {}

void CSpeedTreeRT::GetCamera(float* pPosition, float* pDirection) {}
void CSpeedTreeRT::SetCamera(const float* pPosition, const float* pDirection) {}

void CSpeedTreeRT::SetTime(float fTime) {}
void CSpeedTreeRT::ComputeWindEffects(bool bBranches, bool bLeaves, bool bFronds) {}
void CSpeedTreeRT::ResetLeafWindState(void) {}

bool CSpeedTreeRT::GetLeafRockingState(void) const { return false; }
void CSpeedTreeRT::SetLeafRockingState(bool bFlag) {}
void CSpeedTreeRT::SetNumLeafRockingGroups(unsigned int nRockingGroups) {}

CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetLeafWindMethod(void) const { return WIND_NONE; }
void CSpeedTreeRT::SetLeafWindMethod(EWindMethod eMethod) {}
CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetBranchWindMethod(void) const { return WIND_NONE; }
void CSpeedTreeRT::SetBranchWindMethod(EWindMethod eMethod) {}
CSpeedTreeRT::EWindMethod CSpeedTreeRT::GetFrondWindMethod(void) const { return WIND_NONE; }
void CSpeedTreeRT::SetFrondWindMethod(EWindMethod eMethod) {}

float CSpeedTreeRT::GetWindStrength(void) const { return 0.0f; }
float CSpeedTreeRT::SetWindStrength(float fNewStrength, float fOldStrength, float fFrequencyTimeOffset) { return 0.0f; }

void CSpeedTreeRT::SetNumWindMatrices(unsigned int nNumMatrices) {}
void CSpeedTreeRT::SetWindMatrix(unsigned int nMatrixIndex, const float* pMatrix) {}
void CSpeedTreeRT::GetLocalMatrices(unsigned int& nStartingIndex, unsigned int& nMatrixSpan) { nStartingIndex = 0; nMatrixSpan = 0; }
void CSpeedTreeRT::SetLocalMatrices(unsigned int nStartingMatrix, unsigned int nMatrixSpan) {}

void CSpeedTreeRT::ComputeLodLevel(void) {}
float CSpeedTreeRT::GetLodLevel(void) const { return 1.0f; }
void CSpeedTreeRT::SetLodLevel(float fLodLevel) {}
void CSpeedTreeRT::SetDropToBillboard(bool bFlag) { m_bDropToBillboard = bFlag; }

void CSpeedTreeRT::GetLodLimits(float& fNear, float& fFar) const { fNear = 100.0f; fFar = 10000.0f; }
void CSpeedTreeRT::SetLodLimits(float fNear, float fFar) {}

short CSpeedTreeRT::GetDiscreteBranchLodLevel(float fLodLevel) const { return 0; }
unsigned short CSpeedTreeRT::GetDiscreteLeafLodLevel(float fLodLevel) const { return 0; }
short CSpeedTreeRT::GetDiscreteFrondLodLevel(float fLodLevel) const { return 0; }

unsigned short CSpeedTreeRT::GetNumBranchLodLevels(void) const { return 1; }
unsigned short CSpeedTreeRT::GetNumLeafLodLevels(void) const { return 1; }
unsigned short CSpeedTreeRT::GetNumFrondLodLevels(void) const { return 1; }

void CSpeedTreeRT::DeleteBranchGeometry(void) {}
void CSpeedTreeRT::DeleteFrondGeometry(void) {}
unsigned char* CSpeedTreeRT::GetFrondGeometryMapIndexes(int nLodLevel) const { return NULL; }
const float* CSpeedTreeRT::GetLeafBillboardTable(unsigned int& nEntryCount) const { nEntryCount = 0; return NULL; }
const float* CSpeedTreeRT::GetLeafLodSizeAdjustments(void) { return NULL; }
void CSpeedTreeRT::GetGeometry(SGeometry& sGeometry, unsigned long ulBitVector, short sOverrideBranchLodValue, short sOverrideFrondLodValue, short sOverrideLeafLodValue) {}

void CSpeedTreeRT::GetTextures(STextures& sTextures) const {}
void CSpeedTreeRT::SetLeafTextureCoords(unsigned int nLeafMapIndex, const float* pTexCoords) {}
void CSpeedTreeRT::SetFrondTextureCoords(unsigned int nFrondMapIndex, const float* pTexCoords) {}
bool CSpeedTreeRT::GetTextureFlip(void) { return m_bTextureFlip; }
void CSpeedTreeRT::SetTextureFlip(bool bFlag) { m_bTextureFlip = bFlag; }
void CSpeedTreeRT::SetBranchTextureFilename(const char* pFilename) {}
void CSpeedTreeRT::SetLeafTextureFilename(unsigned int nLeafMapIndex, const char* pFilename) {}
void CSpeedTreeRT::SetFrondTextureFilename(unsigned int nFrondMapIndex, const char* pFilename) {}

void CSpeedTreeRT::Authorize(const char* pKey) {}
bool CSpeedTreeRT::IsAuthorized(void) { return true; }
const char* CSpeedTreeRT::GetCurrentError(void) { return ""; }
void CSpeedTreeRT::ResetError(void) {}

void CSpeedTreeRT::GetBoundingBox(float* pBounds) const { if (pBounds) memset(pBounds, 0, sizeof(float)*6); }
unsigned int CSpeedTreeRT::GetLeafTriangleCount(float fLodLevel) const { return 0; }
unsigned int CSpeedTreeRT::GetBranchTriangleCount(float fLodLevel) const { return 0; }
unsigned int CSpeedTreeRT::GetFrondTriangleCount(float fLodLevel) const { return 0; }

unsigned int CSpeedTreeRT::GetCollisionObjectCount(void) { return 0; }
void CSpeedTreeRT::GetCollisionObject(unsigned int nIndex, ECollisionObjectType& eType, float* pPosition, float* pDimensions) {}

const char* CSpeedTreeRT::GetUserData(void) const { return ""; }
