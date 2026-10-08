// D3DX9 Math + D3D9 + DevIL stub implementations for Android ARM64
#include <math.h>
#include <string.h>
#include <stdlib.h>

#include <d3d9.h>
#include <d3dx9.h>

// ===========================================================
// Direct3D9 and D3DX Creation Stubs
// ===========================================================
extern "C" {

HRESULT WINAPI D3DXCreateSphere(LPDIRECT3DDEVICE9 pDevice, FLOAT fRadius, UINT uSlices, UINT uStacks, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency)
{
    if (ppMesh) *ppMesh = NULL;
    return S_OK;
}

HRESULT WINAPI D3DXCreateCylinder(LPDIRECT3DDEVICE9 pDevice, FLOAT fRadius1, FLOAT fRadius2, FLOAT fLength, UINT uSlices, UINT uStacks, LPD3DXMESH* ppMesh, LPD3DXBUFFER* ppAdjacency)
{
    if (ppMesh) *ppMesh = NULL;
    return S_OK;
}

HRESULT WINAPI D3DXLoadSurfaceFromSurface(LPDIRECT3DSURFACE9 pDestSurface, CONST PALETTEENTRY* pDestPalette, CONST RECT* pDestRect, LPDIRECT3DSURFACE9 pSrcSurface, CONST PALETTEENTRY* pSrcPalette, CONST RECT* pSrcRect, DWORD Filter, D3DCOLOR ColorKey)
{
    return S_OK;
}

// ===========================================================
// D3DX Math Implementations (C linkage)
// ===========================================================

D3DXVECTOR2* WINAPI D3DXVec2Normalize(D3DXVECTOR2* pOut, const D3DXVECTOR2* pV)
{
    float len = sqrtf(pV->x*pV->x + pV->y*pV->y);
    if (len > 1e-10f) { pOut->x = pV->x/len; pOut->y = pV->y/len; }
    else { pOut->x = pOut->y = 0.0f; }
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixTranslation(D3DXMATRIX* pOut, FLOAT x, FLOAT y, FLOAT z)
{
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = 1.0f; pOut->_22 = 1.0f; pOut->_33 = 1.0f; pOut->_44 = 1.0f;
    pOut->_41 = x; pOut->_42 = y; pOut->_43 = z;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationX(D3DXMATRIX* pOut, FLOAT angle)
{
    float s = sinf(angle), c = cosf(angle);
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = 1.0f;
    pOut->_22 = c;  pOut->_23 = s;
    pOut->_32 = -s; pOut->_33 = c;
    pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationY(D3DXMATRIX* pOut, FLOAT angle)
{
    float s = sinf(angle), c = cosf(angle);
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = c; pOut->_13 = -s;
    pOut->_22 = 1.0f;
    pOut->_31 = s; pOut->_33 = c;
    pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationZ(D3DXMATRIX* pOut, FLOAT angle)
{
    float s = sinf(angle), c = cosf(angle);
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = c;  pOut->_12 = s;
    pOut->_21 = -s; pOut->_22 = c;
    pOut->_33 = 1.0f; pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixScaling(D3DXMATRIX* pOut, FLOAT sx, FLOAT sy, FLOAT sz)
{
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = sx; pOut->_22 = sy; pOut->_33 = sz; pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixMultiply(D3DXMATRIX* pOut, const D3DXMATRIX* pM1, const D3DXMATRIX* pM2)
{
    D3DXMATRIX tmp;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++) {
            float v = 0.0f;
            for (int k = 0; k < 4; k++)
                v += pM1->m[i][k] * pM2->m[k][j];
            tmp.m[i][j] = v;
        }
    *pOut = tmp;
    return pOut;
}

D3DXQUATERNION* WINAPI D3DXQuaternionMultiply(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ1, const D3DXQUATERNION* pQ2)
{
    float x = pQ1->w*pQ2->x + pQ1->x*pQ2->w + pQ1->y*pQ2->z - pQ1->z*pQ2->y;
    float y = pQ1->w*pQ2->y - pQ1->x*pQ2->z + pQ1->y*pQ2->w + pQ1->z*pQ2->x;
    float z = pQ1->w*pQ2->z + pQ1->x*pQ2->y - pQ1->y*pQ2->x + pQ1->z*pQ2->w;
    float w = pQ1->w*pQ2->w - pQ1->x*pQ2->x - pQ1->y*pQ2->y - pQ1->z*pQ2->z;
    pOut->x = x; pOut->y = y; pOut->z = z; pOut->w = w;
    return pOut;
}

D3DXQUATERNION* WINAPI D3DXQuaternionRotationAxis(D3DXQUATERNION* pOut, const D3DXVECTOR3* pV, FLOAT angle)
{
    float s = sinf(angle * 0.5f);
    pOut->x = pV->x * s;
    pOut->y = pV->y * s;
    pOut->z = pV->z * s;
    pOut->w = cosf(angle * 0.5f);
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationQuaternion(D3DXMATRIX* pOut, const D3DXQUATERNION* pQ)
{
    float xx = pQ->x*pQ->x, yy = pQ->y*pQ->y, zz = pQ->z*pQ->z;
    float xy = pQ->x*pQ->y, xz = pQ->x*pQ->z, yz = pQ->y*pQ->z;
    float wx = pQ->w*pQ->x, wy = pQ->w*pQ->y, wz = pQ->w*pQ->z;
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = 1-2*(yy+zz); pOut->_12 = 2*(xy+wz);  pOut->_13 = 2*(xz-wy);
    pOut->_21 = 2*(xy-wz);  pOut->_22 = 1-2*(xx+zz); pOut->_23 = 2*(yz+wx);
    pOut->_31 = 2*(xz+wy);  pOut->_32 = 2*(yz-wx);  pOut->_33 = 1-2*(xx+yy);
    pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixTranspose(D3DXMATRIX* pOut, const D3DXMATRIX* pM)
{
    D3DXMATRIX tmp;
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            tmp.m[i][j] = pM->m[j][i];
    *pOut = tmp;
    return pOut;
}

FLOAT WINAPI D3DXMatrixDeterminant(const D3DXMATRIX* pM)
{
    float d = 0;
    for (int j = 0; j < 4; j++) {
        float minor[3][3];
        for (int r = 1; r < 4; r++) {
            int cc = 0;
            for (int c = 0; c < 4; c++) { if (c == j) continue; minor[r-1][cc++] = pM->m[r][c]; }
        }
        float det3 = minor[0][0]*(minor[1][1]*minor[2][2]-minor[1][2]*minor[2][1])
                   - minor[0][1]*(minor[1][0]*minor[2][2]-minor[1][2]*minor[2][0])
                   + minor[0][2]*(minor[1][0]*minor[2][1]-minor[1][1]*minor[2][0]);
        d += (j%2==0?1.0f:-1.0f) * pM->m[0][j] * det3;
    }
    return d;
}

FLOAT WINAPI D3DXMatrixfDeterminant(const D3DXMATRIX* pM)
{
    return D3DXMatrixDeterminant(pM);
}

D3DXMATRIX* WINAPI D3DXMatrixInverse(D3DXMATRIX* pOut, FLOAT* pDeterminant, const D3DXMATRIX* pM)
{
    float a[4][8];
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) a[i][j] = pM->m[i][j];
        for (int j = 4; j < 8; j++) a[i][j] = (i == j-4) ? 1.0f : 0.0f;
    }
    for (int col = 0; col < 4; col++) {
        int pivot = col;
        for (int row = col+1; row < 4; row++)
            if (fabsf(a[row][col]) > fabsf(a[pivot][col])) pivot = row;
        if (pivot != col)
            for (int j = 0; j < 8; j++) { float t = a[col][j]; a[col][j] = a[pivot][j]; a[pivot][j] = t; }
        float d = a[col][col];
        if (fabsf(d) < 1e-10f) { D3DXMatrixIdentity(pOut); return pOut; }
        for (int j = 0; j < 8; j++) a[col][j] /= d;
        for (int row = 0; row < 4; row++) {
            if (row == col) continue;
            float f = a[row][col];
            for (int j = 0; j < 8; j++) a[row][j] -= f * a[col][j];
        }
    }
    for (int i = 0; i < 4; i++)
        for (int j = 0; j < 4; j++)
            pOut->m[i][j] = a[i][j+4];
    if (pDeterminant) *pDeterminant = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixLookAtLH(D3DXMATRIX* pOut, const D3DXVECTOR3* pEye, const D3DXVECTOR3* pAt, const D3DXVECTOR3* pUp)
{
    D3DXVECTOR3 zAxis, xAxis, yAxis, tmp;
    zAxis.x = pAt->x - pEye->x; zAxis.y = pAt->y - pEye->y; zAxis.z = pAt->z - pEye->z;
    D3DXVec3Normalize(&zAxis, &zAxis);
    tmp.x = pUp->y*zAxis.z - pUp->z*zAxis.y;
    tmp.y = pUp->z*zAxis.x - pUp->x*zAxis.z;
    tmp.z = pUp->x*zAxis.y - pUp->y*zAxis.x;
    D3DXVec3Normalize(&xAxis, &tmp);
    yAxis.x = zAxis.y*xAxis.z - zAxis.z*xAxis.y;
    yAxis.y = zAxis.z*xAxis.x - zAxis.x*xAxis.z;
    yAxis.z = zAxis.x*xAxis.y - zAxis.y*xAxis.x;
    pOut->_11 = xAxis.x; pOut->_12 = yAxis.x; pOut->_13 = zAxis.x; pOut->_14 = 0.0f;
    pOut->_21 = xAxis.y; pOut->_22 = yAxis.y; pOut->_23 = zAxis.y; pOut->_24 = 0.0f;
    pOut->_31 = xAxis.z; pOut->_32 = yAxis.z; pOut->_33 = zAxis.z; pOut->_34 = 0.0f;
    pOut->_41 = -(xAxis.x*pEye->x + xAxis.y*pEye->y + xAxis.z*pEye->z);
    pOut->_42 = -(yAxis.x*pEye->x + yAxis.y*pEye->y + yAxis.z*pEye->z);
    pOut->_43 = -(zAxis.x*pEye->x + zAxis.y*pEye->y + zAxis.z*pEye->z);
    pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixLookAtRH(D3DXMATRIX* pOut, const D3DXVECTOR3* pEye, const D3DXVECTOR3* pAt, const D3DXVECTOR3* pUp)
{
    D3DXVECTOR3 zAxis, xAxis, yAxis, tmp;
    zAxis.x = pEye->x - pAt->x; zAxis.y = pEye->y - pAt->y; zAxis.z = pEye->z - pAt->z;
    D3DXVec3Normalize(&zAxis, &zAxis);
    tmp.x = pUp->y*zAxis.z - pUp->z*zAxis.y;
    tmp.y = pUp->z*zAxis.x - pUp->x*zAxis.z;
    tmp.z = pUp->x*zAxis.y - pUp->y*zAxis.x;
    D3DXVec3Normalize(&xAxis, &tmp);
    yAxis.x = zAxis.y*xAxis.z - zAxis.z*xAxis.y;
    yAxis.y = zAxis.z*xAxis.x - zAxis.x*xAxis.z;
    yAxis.z = zAxis.x*xAxis.y - zAxis.y*xAxis.x;
    pOut->_11 = xAxis.x; pOut->_12 = yAxis.x; pOut->_13 = zAxis.x; pOut->_14 = 0.0f;
    pOut->_21 = xAxis.y; pOut->_22 = yAxis.y; pOut->_23 = zAxis.y; pOut->_24 = 0.0f;
    pOut->_31 = xAxis.z; pOut->_32 = yAxis.z; pOut->_33 = zAxis.z; pOut->_34 = 0.0f;
    pOut->_41 = -(xAxis.x*pEye->x + xAxis.y*pEye->y + xAxis.z*pEye->z);
    pOut->_42 = -(yAxis.x*pEye->x + yAxis.y*pEye->y + yAxis.z*pEye->z);
    pOut->_43 = -(zAxis.x*pEye->x + zAxis.y*pEye->y + zAxis.z*pEye->z);
    pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixPerspectiveFovLH(D3DXMATRIX* pOut, FLOAT fovY, FLOAT aspect, FLOAT zn, FLOAT zf)
{
    float h = 1.0f / tanf(fovY * 0.5f);
    float w = h / aspect;
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = w; pOut->_22 = h;
    pOut->_33 = zf / (zf - zn); pOut->_34 = 1.0f;
    pOut->_43 = -zn * zf / (zf - zn);
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixPerspectiveFovRH(D3DXMATRIX* pOut, FLOAT fovY, FLOAT aspect, FLOAT zn, FLOAT zf)
{
    float h = 1.0f / tanf(fovY * 0.5f);
    float w = h / aspect;
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = w; pOut->_22 = h;
    pOut->_33 = zf / (zn - zf); pOut->_34 = -1.0f;
    pOut->_43 = zn * zf / (zn - zf);
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixOrthoLH(D3DXMATRIX* pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf)
{
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = 2.0f/w; pOut->_22 = 2.0f/h;
    pOut->_33 = 1.0f/(zf-zn); pOut->_43 = -zn/(zf-zn); pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixOrthoRH(D3DXMATRIX* pOut, FLOAT w, FLOAT h, FLOAT zn, FLOAT zf)
{
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = 2.0f/w; pOut->_22 = 2.0f/h;
    pOut->_33 = 1.0f/(zn-zf); pOut->_43 = zn/(zn-zf); pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterLH(D3DXMATRIX* pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf)
{
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = 2.0f/(r-l); pOut->_22 = 2.0f/(t-b);
    pOut->_33 = 1.0f/(zf-zn);
    pOut->_41 = (l+r)/(l-r); pOut->_42 = (t+b)/(b-t);
    pOut->_43 = zn/(zn-zf); pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixOrthoOffCenterRH(D3DXMATRIX* pOut, FLOAT l, FLOAT r, FLOAT b, FLOAT t, FLOAT zn, FLOAT zf)
{
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = 2.0f/(r-l); pOut->_22 = 2.0f/(t-b);
    pOut->_33 = 1.0f/(zn-zf);
    pOut->_41 = (l+r)/(l-r); pOut->_42 = (t+b)/(b-t);
    pOut->_43 = zn/(zn-zf); pOut->_44 = 1.0f;
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationYawPitchRoll(D3DXMATRIX* pOut, FLOAT yaw, FLOAT pitch, FLOAT roll)
{
    D3DXMATRIX mY, mP, mR, tmp;
    D3DXMatrixRotationY(&mY, yaw);
    D3DXMatrixRotationX(&mP, pitch);
    D3DXMatrixRotationZ(&mR, roll);
    D3DXMatrixMultiply(&tmp, &mR, &mP);
    D3DXMatrixMultiply(pOut, &tmp, &mY);
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixRotationAxis(D3DXMATRIX* pOut, const D3DXVECTOR3* pV, FLOAT angle)
{
    float s = sinf(angle), c = cosf(angle), t = 1.0f - c;
    float x = pV->x, y = pV->y, z = pV->z;
    memset(pOut, 0, sizeof(D3DXMATRIX));
    pOut->_11 = t*x*x+c;   pOut->_12 = t*x*y+s*z; pOut->_13 = t*x*z-s*y;
    pOut->_21 = t*x*y-s*z; pOut->_22 = t*y*y+c;   pOut->_23 = t*y*z+s*x;
    pOut->_31 = t*x*z+s*y; pOut->_32 = t*y*z-s*x; pOut->_33 = t*z*z+c;
    pOut->_44 = 1.0f;
    return pOut;
}

// Vector3 functions
D3DXVECTOR3* WINAPI D3DXVec3Normalize(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV)
{
    float len = sqrtf(pV->x*pV->x + pV->y*pV->y + pV->z*pV->z);
    if (len > 1e-10f) { pOut->x = pV->x/len; pOut->y = pV->y/len; pOut->z = pV->z/len; }
    else { pOut->x = pOut->y = pOut->z = 0.0f; }
    return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3TransformCoord(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
    float x = pV->x, y = pV->y, z = pV->z;
    float w = x*pM->_14 + y*pM->_24 + z*pM->_34 + pM->_44;
    if (fabsf(w) < 1e-10f) w = 1.0f;
    pOut->x = (x*pM->_11 + y*pM->_21 + z*pM->_31 + pM->_41) / w;
    pOut->y = (x*pM->_12 + y*pM->_22 + z*pM->_32 + pM->_42) / w;
    pOut->z = (x*pM->_13 + y*pM->_23 + z*pM->_33 + pM->_43) / w;
    return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3TransformNormal(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
    float x = pV->x, y = pV->y, z = pV->z;
    pOut->x = x*pM->_11 + y*pM->_21 + z*pM->_31;
    pOut->y = x*pM->_12 + y*pM->_22 + z*pM->_32;
    pOut->z = x*pM->_13 + y*pM->_23 + z*pM->_33;
    return pOut;
}

// ===========================================================
// DevIL Image Library Stubs (C linkage)
// ===========================================================
typedef unsigned int ILenum;
typedef unsigned char ILboolean;
typedef unsigned int ILbitfield;
typedef signed char ILbyte;
typedef short ILshort;
typedef int ILint;
typedef size_t ILsizei;
typedef unsigned char ILubyte;
typedef unsigned short ILushort;
typedef unsigned int ILuint;
typedef float ILfloat;
typedef float ILclampf;
typedef double ILdouble;
typedef double ILclampd;
typedef void ILvoid;
typedef char ILchar;
typedef char* ILstring;
#define ILAPIENTRY
#define IL_TRUE 1
#define IL_FALSE 0
#define IL_NO_ERROR 0
typedef void (*IL_LOADPROC)(const ILstring);
typedef void (*IL_SAVEPROC)(const ILstring);

static ILuint s_ilCurrentImage = 0;

void ILAPIENTRY ilInit(void) {}
void ILAPIENTRY ilShutDown(void) {}

void ILAPIENTRY ilGenImages(ILsizei num, ILuint* images)
{
    static ILuint nextId = 1;
    for (ILsizei i = 0; i < num; i++) images[i] = nextId++;
}

void ILAPIENTRY ilBindImage(ILuint image) { s_ilCurrentImage = image; }

ILboolean ILAPIENTRY ilEnable(ILenum mode) { return IL_TRUE; }
ILboolean ILAPIENTRY ilDisable(ILenum mode) { return IL_TRUE; }
ILboolean ILAPIENTRY ilOriginFunc(ILenum mode) { return IL_TRUE; }

ILboolean ILAPIENTRY ilLoad(ILenum type, const ILstring fileName) { return IL_FALSE; }
ILboolean ILAPIENTRY ilLoadL(ILenum type, ILvoid* lump, ILuint size) { return IL_FALSE; }
ILboolean ILAPIENTRY ilSave(ILenum type, const ILstring fileName) { return IL_FALSE; }

ILint ILAPIENTRY ilGetInteger(ILenum mode) { return 0; }
void  ILAPIENTRY ilGetIntegerv(ILenum mode, ILint* param) { if (param) *param = 0; }

ILboolean ILAPIENTRY ilConvertImage(ILenum destFormat, ILenum destType) { return IL_FALSE; }

ILuint ILAPIENTRY ilCopyPixels(ILuint xoff, ILuint yoff, ILuint zoff,
                               ILuint width, ILuint height, ILuint depth,
                               ILenum format, ILenum type, void* data) { return 0; }

void ILAPIENTRY ilDeleteImages(ILsizei num, const ILuint* images) {}

ILenum ILAPIENTRY ilGetError(void) { return IL_NO_ERROR; }

ILboolean ILAPIENTRY ilTexImage(ILuint width, ILuint height, ILuint depth,
                             ILubyte numChannels, ILenum format, ILenum type,
                             void* data) { return IL_FALSE; }

void ILAPIENTRY ilSetString(ILenum mode, const char* string) {}
ILboolean ILAPIENTRY ilRegisterLoad(const ILstring ext, IL_LOADPROC load) { return IL_TRUE; }
ILboolean ILAPIENTRY ilRegisterSave(const ILstring ext, IL_SAVEPROC save) { return IL_TRUE; }

void ILAPIENTRY ilSetPixels(ILint xoff, ILint yoff, ILint zoff,
                            ILuint width, ILuint height, ILuint depth,
                            ILenum format, ILenum type, void* data) {}

// ===========================================================
// Additional D3DX Functions
// ===========================================================
HRESULT WINAPI D3DXAssembleShaderFromFileA(LPCSTR pSrcFile, CONST D3DXMACRO* pDefines, LPD3DXINCLUDE pInclude, DWORD Flags, LPD3DXBUFFER* ppShader, LPD3DXBUFFER* ppErrorMsgs)
{
    if (ppShader) *ppShader = NULL;
    return S_OK;
}

UINT WINAPI D3DXGetFVFVertexSize(DWORD FVF)
{
    UINT size = 0;
    if (FVF & D3DFVF_XYZ) size += sizeof(float) * 3;
    if (FVF & D3DFVF_XYZRHW) size += sizeof(float) * 4;
    if (FVF & D3DFVF_NORMAL) size += sizeof(float) * 3;
    if (FVF & D3DFVF_DIFFUSE) size += sizeof(DWORD);
    if (FVF & D3DFVF_SPECULAR) size += sizeof(DWORD);
    int numTex = (FVF >> 8) & 0x0F;
    size += numTex * sizeof(float) * 2;
    return size ? size : 32;
}

D3DXQUATERNION* WINAPI D3DXQuaternionRotationYawPitchRoll(D3DXQUATERNION* pOut, FLOAT yaw, FLOAT pitch, FLOAT roll)
{
    float cy = cosf(yaw * 0.5f), sy = sinf(yaw * 0.5f);
    float cp = cosf(pitch * 0.5f), sp = sinf(pitch * 0.5f);
    float cr = cosf(roll * 0.5f), sr = sinf(roll * 0.5f);
    pOut->w = cy * cp * cr + sy * sp * sr;
    pOut->x = cy * sp * cr + sy * cp * sr;
    pOut->y = sy * cp * cr - cy * sp * sr;
    pOut->z = cy * cp * sr - sy * sp * cr;
    return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3Project(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DVIEWPORT9* pViewport, const D3DXMATRIX* pProjection, const D3DXMATRIX* pView, const D3DXMATRIX* pWorld)
{
    D3DXMATRIX m;
    D3DXMatrixMultiply(&m, pWorld, pView);
    D3DXMatrixMultiply(&m, &m, pProjection);
    D3DXVec3TransformCoord(pOut, pV, &m);
    pOut->x = pViewport->X + (1.0f + pOut->x) * pViewport->Width / 2.0f;
    pOut->y = pViewport->Y + (1.0f - pOut->y) * pViewport->Height / 2.0f;
    pOut->z = pViewport->MinZ + pOut->z * (pViewport->MaxZ - pViewport->MinZ);
    return pOut;
}

D3DXVECTOR3* WINAPI D3DXVec3Unproject(D3DXVECTOR3* pOut, const D3DXVECTOR3* pV, const D3DVIEWPORT9* pViewport, const D3DXMATRIX* pProjection, const D3DXMATRIX* pView, const D3DXMATRIX* pWorld)
{
    D3DXMATRIX m, inv;
    D3DXMatrixMultiply(&m, pWorld, pView);
    D3DXMatrixMultiply(&m, &m, pProjection);
    D3DXMatrixInverse(&inv, NULL, &m);
    D3DXVECTOR3 in;
    in.x = (pV->x - pViewport->X) * 2.0f / pViewport->Width - 1.0f;
    in.y = 1.0f - (pV->y - pViewport->Y) * 2.0f / pViewport->Height;
    in.z = (pV->z - pViewport->MinZ) / (pViewport->MaxZ - pViewport->MinZ);
    return D3DXVec3TransformCoord(pOut, &in, &inv);
}

D3DXPLANE* WINAPI D3DXPlaneNormalize(D3DXPLANE* pOut, const D3DXPLANE* pP)
{
    float len = sqrtf(pP->a*pP->a + pP->b*pP->b + pP->c*pP->c);
    if (len > 1e-10f) { pOut->a = pP->a/len; pOut->b = pP->b/len; pOut->c = pP->c/len; pOut->d = pP->d/len; }
    else { memset(pOut, 0, sizeof(D3DXPLANE)); }
    return pOut;
}

D3DXVECTOR4* WINAPI D3DXVec3Transform(D3DXVECTOR4* pOut, const D3DXVECTOR3* pV, const D3DXMATRIX* pM)
{
    pOut->x = pV->x*pM->_11 + pV->y*pM->_21 + pV->z*pM->_31 + pM->_41;
    pOut->y = pV->x*pM->_12 + pV->y*pM->_22 + pV->z*pM->_32 + pM->_42;
    pOut->z = pV->x*pM->_13 + pV->y*pM->_23 + pV->z*pM->_33 + pM->_43;
    pOut->w = pV->x*pM->_14 + pV->y*pM->_24 + pV->z*pM->_34 + pM->_44;
    return pOut;
}

D3DXVECTOR4* WINAPI D3DXVec4Transform(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV, const D3DXMATRIX* pM)
{
    float x = pV->x, y = pV->y, z = pV->z, w = pV->w;
    pOut->x = x*pM->_11 + y*pM->_21 + z*pM->_31 + w*pM->_41;
    pOut->y = x*pM->_12 + y*pM->_22 + z*pM->_32 + w*pM->_42;
    pOut->z = x*pM->_13 + y*pM->_23 + z*pM->_33 + w*pM->_43;
    pOut->w = x*pM->_14 + y*pM->_24 + z*pM->_34 + w*pM->_44;
    return pOut;
}

D3DXVECTOR4* WINAPI D3DXVec4Normalize(D3DXVECTOR4* pOut, const D3DXVECTOR4* pV)
{
    float len = sqrtf(pV->x*pV->x + pV->y*pV->y + pV->z*pV->z + pV->w*pV->w);
    if (len > 1e-10f) { pOut->x = pV->x/len; pOut->y = pV->y/len; pOut->z = pV->z/len; pOut->w = pV->w/len; }
    else { memset(pOut, 0, sizeof(D3DXVECTOR4)); }
    return pOut;
}

D3DXVECTOR4* WINAPI D3DXVec2Transform(D3DXVECTOR4* pOut, const D3DXVECTOR2* pV, const D3DXMATRIX* pM)
{
    pOut->x = pV->x*pM->_11 + pV->y*pM->_21 + pM->_41;
    pOut->y = pV->x*pM->_12 + pV->y*pM->_22 + pM->_42;
    pOut->z = pV->x*pM->_13 + pV->y*pM->_23 + pM->_43;
    pOut->w = pV->x*pM->_14 + pV->y*pM->_24 + pM->_44;
    return pOut;
}

D3DXPLANE* WINAPI D3DXPlaneTransform(D3DXPLANE* pOut, const D3DXPLANE* pP, const D3DXMATRIX* pM)
{
    D3DXMATRIX inv;
    D3DXMatrixInverse(&inv, NULL, pM);
    pOut->a = pP->a*inv._11 + pP->b*inv._12 + pP->c*inv._13 + pP->d*inv._14;
    pOut->b = pP->a*inv._21 + pP->b*inv._22 + pP->c*inv._23 + pP->d*inv._24;
    pOut->c = pP->a*inv._31 + pP->b*inv._32 + pP->c*inv._33 + pP->d*inv._34;
    pOut->d = pP->a*inv._41 + pP->b*inv._42 + pP->c*inv._43 + pP->d*inv._44;
    return pOut;
}

D3DXPLANE* WINAPI D3DXPlaneFromPointNormal(D3DXPLANE* pOut, const D3DXVECTOR3* pV, const D3DXVECTOR3* pNormal)
{
    pOut->a = pNormal->x;
    pOut->b = pNormal->y;
    pOut->c = pNormal->z;
    pOut->d = -(pNormal->x*pV->x + pNormal->y*pV->y + pNormal->z*pV->z);
    return pOut;
}

D3DXPLANE* WINAPI D3DXPlaneFromPoints(D3DXPLANE* pOut, const D3DXVECTOR3* p1, const D3DXVECTOR3* p2, const D3DXVECTOR3* p3)
{
    D3DXVECTOR3 v1 = *p2 - *p1;
    D3DXVECTOR3 v2 = *p3 - *p1;
    D3DXVECTOR3 n;
    D3DXVec3Cross(&n, &v1, &v2);
    D3DXVec3Normalize(&n, &n);
    return D3DXPlaneFromPointNormal(pOut, p1, &n);
}

D3DXVECTOR3* WINAPI D3DXPlaneIntersectLine(D3DXVECTOR3* pOut, const D3DXPLANE* pP, const D3DXVECTOR3* p1, const D3DXVECTOR3* p2)
{
    D3DXVECTOR3 dir = *p2 - *p1;
    float denom = pP->a * dir.x + pP->b * dir.y + pP->c * dir.z;
    if (fabsf(denom) < 1e-6f) return NULL;
    float t = -(pP->a * p1->x + pP->b * p1->y + pP->c * p1->z + pP->d) / denom;
    *pOut = *p1 + dir * t;
    return pOut;
}

D3DXQUATERNION* WINAPI D3DXQuaternionSlerp(D3DXQUATERNION* pOut, const D3DXQUATERNION* pQ1, const D3DXQUATERNION* pQ2, float t)
{
    float cosTheta = pQ1->x*pQ2->x + pQ1->y*pQ2->y + pQ1->z*pQ2->z + pQ1->w*pQ2->w;
    float q2Array[4] = { pQ2->x, pQ2->y, pQ2->z, pQ2->w };
    if (cosTheta < 0.0f) {
        cosTheta = -cosTheta;
        q2Array[0] = -q2Array[0]; q2Array[1] = -q2Array[1]; q2Array[2] = -q2Array[2]; q2Array[3] = -q2Array[3];
    }
    float scale0, scale1;
    if (1.0f - cosTheta > 0.001f) {
        float theta = acosf(cosTheta);
        float sinTheta = sinf(theta);
        scale0 = sinf((1.0f - t) * theta) / sinTheta;
        scale1 = sinf(t * theta) / sinTheta;
    } else {
        scale0 = 1.0f - t;
        scale1 = t;
    }
    pOut->x = scale0 * pQ1->x + scale1 * q2Array[0];
    pOut->y = scale0 * pQ1->y + scale1 * q2Array[1];
    pOut->z = scale0 * pQ1->z + scale1 * q2Array[2];
    pOut->w = scale0 * pQ1->w + scale1 * q2Array[3];
    return pOut;
}

D3DXQUATERNION* WINAPI D3DXQuaternionRotationMatrix(D3DXQUATERNION* pOut, const D3DXMATRIX* pM)
{
    float trace = pM->_11 + pM->_22 + pM->_33;
    if (trace > 0.0f) {
        float s = sqrtf(trace + 1.0f) * 2.0f;
        pOut->w = 0.25f * s;
        pOut->x = (pM->_23 - pM->_32) / s;
        pOut->y = (pM->_31 - pM->_13) / s;
        pOut->z = (pM->_12 - pM->_21) / s;
    } else if (pM->_11 > pM->_22 && pM->_11 > pM->_33) {
        float s = sqrtf(1.0f + pM->_11 - pM->_22 - pM->_33) * 2.0f;
        pOut->w = (pM->_23 - pM->_32) / s;
        pOut->x = 0.25f * s;
        pOut->y = (pM->_12 + pM->_21) / s;
        pOut->z = (pM->_13 + pM->_31) / s;
    } else if (pM->_22 > pM->_33) {
        float s = sqrtf(1.0f + pM->_22 - pM->_11 - pM->_33) * 2.0f;
        pOut->w = (pM->_31 - pM->_13) / s;
        pOut->x = (pM->_12 + pM->_21) / s;
        pOut->y = 0.25f * s;
        pOut->z = (pM->_23 + pM->_32) / s;
    } else {
        float s = sqrtf(1.0f + pM->_33 - pM->_11 - pM->_22) * 2.0f;
        pOut->w = (pM->_12 - pM->_21) / s;
        pOut->x = (pM->_13 + pM->_31) / s;
        pOut->y = (pM->_23 + pM->_32) / s;
        pOut->z = 0.25f * s;
    }
    return pOut;
}

D3DXMATRIX* WINAPI D3DXMatrixTransformation(
    D3DXMATRIX* pOut,
    const D3DXVECTOR3* pScalingCenter,
    const D3DXQUATERNION* pScalingRotation,
    const D3DXVECTOR3* pScaling,
    const D3DXVECTOR3* pRotationCenter,
    const D3DXQUATERNION* pRotation,
    const D3DXVECTOR3* pTranslation)
{
    D3DXMATRIX m1, m2;
    D3DXMatrixIdentity(pOut);

    if (pScaling) {
        D3DXMATRIX ms;
        D3DXMatrixScaling(&ms, pScaling->x, pScaling->y, pScaling->z);
        if (pScalingCenter) {
            D3DXMATRIX mc1, mc2;
            D3DXMatrixTranslation(&mc1, -pScalingCenter->x, -pScalingCenter->y, -pScalingCenter->z);
            D3DXMatrixTranslation(&mc2, pScalingCenter->x, pScalingCenter->y, pScalingCenter->z);
            D3DXMatrixMultiply(&m1, &mc1, &ms);
            D3DXMatrixMultiply(&ms, &m1, &mc2);
        }
        D3DXMatrixMultiply(pOut, pOut, &ms);
    }

    if (pRotation) {
        D3DXMATRIX mr;
        D3DXMatrixRotationQuaternion(&mr, pRotation);
        if (pRotationCenter) {
            D3DXMATRIX mc1, mc2;
            D3DXMatrixTranslation(&mc1, -pRotationCenter->x, -pRotationCenter->y, -pRotationCenter->z);
            D3DXMatrixTranslation(&mc2, pRotationCenter->x, pRotationCenter->y, pRotationCenter->z);
            D3DXMatrixMultiply(&m1, &mc1, &mr);
            D3DXMatrixMultiply(&mr, &m1, &mc2);
        }
        D3DXMatrixMultiply(pOut, pOut, &mr);
    }

    if (pTranslation) {
        D3DXMATRIX mt;
        D3DXMatrixTranslation(&mt, pTranslation->x, pTranslation->y, pTranslation->z);
        D3DXMatrixMultiply(pOut, pOut, &mt);
    }

    return pOut;
}

} // extern "C"

// ===========================================================
// libjpeg stubs
// ===========================================================
#include <stdio.h>
#include "libjpeg-9a/jpeglib.h"

extern "C" {
struct jpeg_error_mgr* jpeg_std_error(struct jpeg_error_mgr* err) { return err; }
void jpeg_CreateCompress(j_compress_ptr cinfo, int version, size_t structsize) {}
void jpeg_CreateDecompress(j_decompress_ptr cinfo, int version, size_t structsize) {}
void jpeg_destroy_compress(j_compress_ptr cinfo) {}
void jpeg_destroy_decompress(j_decompress_ptr cinfo) {}
void jpeg_stdio_dest(j_compress_ptr cinfo, FILE* outfile) {}
void jpeg_stdio_src(j_decompress_ptr cinfo, FILE* infile) {}
void jpeg_set_defaults(j_compress_ptr cinfo) {}
void jpeg_set_quality(j_compress_ptr cinfo, int quality, boolean force_baseline) {}
void jpeg_start_compress(j_compress_ptr cinfo, boolean write_all_tables) {}
JDIMENSION jpeg_write_scanlines(j_compress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION num_lines) { return num_lines; }
void jpeg_finish_compress(j_compress_ptr cinfo) {}
int jpeg_read_header(j_decompress_ptr cinfo, boolean require_image) { return JPEG_HEADER_OK; }
boolean jpeg_start_decompress(j_decompress_ptr cinfo) { return TRUE; }
JDIMENSION jpeg_read_scanlines(j_decompress_ptr cinfo, JSAMPARRAY scanlines, JDIMENSION max_lines) { return max_lines; }
boolean jpeg_finish_decompress(j_decompress_ptr cinfo) { return TRUE; }
}


// ===========================================================
// ID3DXMatrixStack & D3DXGetImageInfoFromFileInMemory
// ===========================================================
#include <vector>
#include "stb_image.h"

class CD3DXMatrixStack : public ID3DXMatrixStack
{
private:
    ULONG m_refCount;
    std::vector<D3DXMATRIX> m_stack;

public:
    CD3DXMatrixStack() : m_refCount(1) {
        D3DXMATRIX id;
        D3DXMatrixIdentity(&id);
        m_stack.push_back(id);
    }
    virtual ~CD3DXMatrixStack() {}

    STDMETHOD(QueryInterface)(REFIID riid, LPVOID * ppvObj) override {
        if (!ppvObj) return E_POINTER;
        *ppvObj = this;
        AddRef();
        return S_OK;
    }
    STDMETHOD_(ULONG, AddRef)() override {
        return ++m_refCount;
    }
    STDMETHOD_(ULONG, Release)() override {
        ULONG ref = --m_refCount;
        if (ref == 0) delete this;
        return ref;
    }

    STDMETHOD(Pop)() override {
        if (m_stack.size() > 1)
            m_stack.pop_back();
        return S_OK;
    }
    STDMETHOD(Push)() override {
        if (!m_stack.empty())
            m_stack.push_back(m_stack.back());
        return S_OK;
    }
    STDMETHOD(LoadIdentity)() override {
        if (!m_stack.empty())
            D3DXMatrixIdentity(&m_stack.back());
        return S_OK;
    }
    STDMETHOD(LoadMatrix)(CONST D3DXMATRIX* pM) override {
        if (!m_stack.empty() && pM)
            m_stack.back() = *pM;
        return S_OK;
    }
    STDMETHOD(MultMatrix)(CONST D3DXMATRIX* pM) override {
        if (!m_stack.empty() && pM) {
            D3DXMATRIX res;
            D3DXMatrixMultiply(&res, &m_stack.back(), pM);
            m_stack.back() = res;
        }
        return S_OK;
    }
    STDMETHOD(MultMatrixLocal)(CONST D3DXMATRIX* pM) override {
        if (!m_stack.empty() && pM) {
            D3DXMATRIX res;
            D3DXMatrixMultiply(&res, pM, &m_stack.back());
            m_stack.back() = res;
        }
        return S_OK;
    }
    STDMETHOD(RotateAxis)(CONST D3DXVECTOR3* pV, FLOAT Angle) override {
        if (!m_stack.empty() && pV) {
            D3DXMATRIX r;
            D3DXMatrixRotationAxis(&r, pV, Angle);
            MultMatrix(&r);
        }
        return S_OK;
    }
    STDMETHOD(RotateAxisLocal)(CONST D3DXVECTOR3* pV, FLOAT Angle) override {
        if (!m_stack.empty() && pV) {
            D3DXMATRIX r;
            D3DXMatrixRotationAxis(&r, pV, Angle);
            MultMatrixLocal(&r);
        }
        return S_OK;
    }
    STDMETHOD(RotateYawPitchRoll)(FLOAT Yaw, FLOAT Pitch, FLOAT Roll) override {
        if (!m_stack.empty()) {
            D3DXMATRIX r;
            D3DXMatrixRotationYawPitchRoll(&r, Yaw, Pitch, Roll);
            MultMatrix(&r);
        }
        return S_OK;
    }
    STDMETHOD(RotateYawPitchRollLocal)(FLOAT Yaw, FLOAT Pitch, FLOAT Roll) override {
        if (!m_stack.empty()) {
            D3DXMATRIX r;
            D3DXMatrixRotationYawPitchRoll(&r, Yaw, Pitch, Roll);
            MultMatrixLocal(&r);
        }
        return S_OK;
    }
    STDMETHOD(Scale)(FLOAT x, FLOAT y, FLOAT z) override {
        if (!m_stack.empty()) {
            D3DXMATRIX s;
            D3DXMatrixScaling(&s, x, y, z);
            MultMatrix(&s);
        }
        return S_OK;
    }
    STDMETHOD(ScaleLocal)(FLOAT x, FLOAT y, FLOAT z) override {
        if (!m_stack.empty()) {
            D3DXMATRIX s;
            D3DXMatrixScaling(&s, x, y, z);
            MultMatrixLocal(&s);
        }
        return S_OK;
    }
    STDMETHOD(Translate)(FLOAT x, FLOAT y, FLOAT z) override {
        if (!m_stack.empty()) {
            D3DXMATRIX t;
            D3DXMatrixTranslation(&t, x, y, z);
            MultMatrix(&t);
        }
        return S_OK;
    }
    STDMETHOD(TranslateLocal)(FLOAT x, FLOAT y, FLOAT z) override {
        if (!m_stack.empty()) {
            D3DXMATRIX t;
            D3DXMatrixTranslation(&t, x, y, z);
            MultMatrixLocal(&t);
        }
        return S_OK;
    }
    STDMETHOD_(D3DXMATRIX*, GetTop)() override {
        if (m_stack.empty()) return nullptr;
        return &m_stack.back();
    }
};

extern "C" HRESULT WINAPI D3DXCreateMatrixStack(DWORD Flags, LPD3DXMATRIXSTACK* ppStack)
{
    if (!ppStack) return D3DERR_INVALIDCALL;
    *ppStack = new CD3DXMatrixStack();
    return D3D_OK;
}

extern "C" HRESULT WINAPI D3DXGetImageInfoFromFileInMemory(
    LPCVOID                   pSrcData,
    UINT                      SrcDataSize,
    D3DXIMAGE_INFO*           pSrcInfo)
{
    if (!pSrcData || !SrcDataSize || !pSrcInfo)
        return D3DERR_INVALIDCALL;
    
    // Check DDS magic ('DDS ')
    if (SrcDataSize >= 128 && *(const DWORD*)pSrcData == 0x20534444)
    {
        const BYTE* b = (const BYTE*)pSrcData;
        DWORD h = *(const DWORD*)(b + 12);
        DWORD w = *(const DWORD*)(b + 16);
        DWORD mips = *(const DWORD*)(b + 28);
        DWORD pf_flags = *(const DWORD*)(b + 80);
        DWORD pf_fourcc = *(const DWORD*)(b + 84);
        DWORD pf_bits = *(const DWORD*)(b + 88);
        DWORD pf_g_mask = *(const DWORD*)(b + 96);
        DWORD pf_a_mask = *(const DWORD*)(b + 104);

        pSrcInfo->Width = w;
        pSrcInfo->Height = h;
        pSrcInfo->Depth = 1;
        pSrcInfo->MipLevels = mips > 0 ? mips : 1;
        pSrcInfo->ResourceType = D3DRTYPE_TEXTURE;
        pSrcInfo->ImageFileFormat = D3DXIFF_DDS;

        if (pf_flags & 0x4) // DDPF_FOURCC
        {
            if (pf_fourcc == 0x31545844) pSrcInfo->Format = D3DFMT_DXT1; // 'DXT1'
            else if (pf_fourcc == 0x33545844) pSrcInfo->Format = D3DFMT_DXT3; // 'DXT3'
            else if (pf_fourcc == 0x35545844) pSrcInfo->Format = D3DFMT_DXT5; // 'DXT5'
            else pSrcInfo->Format = D3DFMT_UNKNOWN;
        }
        else if (pf_bits == 16)
        {
            if (pf_a_mask == 0x8000) pSrcInfo->Format = D3DFMT_A1R5G5B5; // 25
            else if (pf_a_mask == 0xF000) pSrcInfo->Format = D3DFMT_A4R4G4B4; // 26
            else if (pf_g_mask == 0x07E0) pSrcInfo->Format = D3DFMT_R5G6B5; // 23
            else pSrcInfo->Format = D3DFMT_A1R5G5B5;
        }
        else if (pf_bits == 24)
        {
            pSrcInfo->Format = D3DFMT_R8G8B8; // 20
        }
        else if (pf_bits == 32)
        {
            pSrcInfo->Format = (pf_a_mask != 0) ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8;
        }
        else
        {
            pSrcInfo->Format = D3DFMT_UNKNOWN;
        }
        return D3D_OK;
    }

    int w = 0, h = 0, comp = 0;
    if (stbi_info_from_memory((const stbi_uc*)pSrcData, (int)SrcDataSize, &w, &h, &comp))
    {
        pSrcInfo->Width = w;
        pSrcInfo->Height = h;
        pSrcInfo->Depth = 1;
        pSrcInfo->MipLevels = 1;
        pSrcInfo->Format = D3DFMT_A8R8G8B8;
        pSrcInfo->ResourceType = D3DRTYPE_TEXTURE;
        pSrcInfo->ImageFileFormat = D3DXIFF_PNG;
        return D3D_OK;
    }
    return D3DXERR_INVALIDDATA;
}

