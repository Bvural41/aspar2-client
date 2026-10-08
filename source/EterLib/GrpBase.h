#pragma once

#include "GrpDetector.h"
#include "Ray.h"
#include "../UserInterface/Locale_inc.h"

#ifdef ENABLE_FIX_MOBS_LAG
static const std::size_t SMALL_PDT_VERTEX_BUFFER_SIZE = 16;
static const std::size_t LARGE_PDT_VERTEX_BUFFER_SIZE = 200;
#endif

#include <vector>

void PixelPositionToD3DXVECTOR3(const D3DXVECTOR3& c_rkPPosSrc, D3DXVECTOR3* pv3Dst);
void D3DXVECTOR3ToPixelPosition(const D3DXVECTOR3& c_rv3Src, D3DXVECTOR3* pv3Dst);

class CGraphicTexture;

typedef WORD TIndex;

typedef struct SFace
{
	TIndex indices[3];
} TFace;

typedef D3DXVECTOR3 TPosition;

typedef D3DXVECTOR3 TNormal;

typedef D3DXVECTOR2 TTextureCoordinate;

typedef DWORD TDiffuse;
typedef DWORD TAmbient;
typedef DWORD TSpecular;

typedef union UDepth
{
	float	f;
	long	l;
	DWORD	dw;
} TDepth;

typedef struct SVertex
{
	float x, y, z;
	DWORD color;
	float u, v;
} TVertex;

struct STVertex
{
	float x, y, z, rhw;
};

struct SPVertex
{
	float x, y, z;
};

typedef struct SPDVertex
{
	float x, y, z;
	DWORD color;
} TPDVertex;

struct SPDTVertexRaw
{
	float px, py, pz;
	DWORD diffuse;
	float u, v;
};

typedef struct SPTVertex
{
	TPosition position;
	TTextureCoordinate texCoord;
} TPTVertex;

typedef struct SPDTVertex
{
	TPosition	position;
	TDiffuse	diffuse;
	TTextureCoordinate texCoord;
#ifdef ENABLE_FIX_MOBS_LAG
	static const DWORD kFVF = D3DFVF_XYZ | D3DFVF_DIFFUSE | D3DFVF_TEX1;
#endif
} TPDTVertex;

typedef struct SPNTVertex
{
	TPosition			position;
	TNormal				normal;
	TTextureCoordinate	texCoord;
} TPNTVertex;

typedef struct SPNT2Vertex
{
	TPosition	position;
	TNormal		normal;
	TTextureCoordinate texCoord;
	TTextureCoordinate texCoord2;
} TPNT2Vertex;

typedef struct SPDT2Vertex
{
	TPosition	position;
	DWORD		diffuse;
	TTextureCoordinate texCoord;
	TTextureCoordinate texCoord2;
} TPDT2Vertex;

typedef struct SNameInfo
{
	DWORD	name;
	TDepth	depth;
} TNameInfo;

typedef struct SBoundBox
{
	float sx, sy, sz;
	float ex, ey, ez;
	int meshIndex;
	int boneIndex;
} TBoundBox;

const WORD c_FillRectIndices[6] = { 0, 2, 1, 2, 3, 1 };

/*
enum EIndexCount
{
	LINE_INDEX_COUNT = 2,
	TRIANGLE_INDEX_COUNT = 2*3,
	RECTANGLE_INDEX_COUNT = 2*4,
	CUBE_INDEX_COUNT = 2*4*3,
	FILLED_TRIANGLE_INDEX_COUNT = 3,
	FILLED_RECTANGLE_INDEX_COUNT = 3*2,
	FILLED_CUBE_INDEX_COUNT = 3*2*6,
};
*/

class CGraphicBase
{
	public:
		static DWORD GetAvailableTextureMemory();
		static const D3DXMATRIX& GetViewMatrix();
		static const D3DXMATRIX & GetIdentityMatrix();

		enum
		{
			DEFAULT_IB_LINE,
			DEFAULT_IB_LINE_TRI,
			DEFAULT_IB_LINE_RECT,
			DEFAULT_IB_LINE_CUBE,
			DEFAULT_IB_FILL_TRI,
			DEFAULT_IB_FILL_RECT,
			DEFAULT_IB_FILL_CUBE,
			DEFAULT_IB_NUM,
		};

	public:
		CGraphicBase();
		virtual	~CGraphicBase();
#ifdef ENABLE_FIX_MOBS_LAG
		static LPDIRECT3DVERTEXBUFFER9 GetSmallPdtVertexBuffer() { return m_smallPdtVertexBuffer; }
		static LPDIRECT3DVERTEXBUFFER9 GetLargePdtVertexBuffer() { return m_largePdtVertexBuffer; }
#endif

#if defined(USE_OPENGL_ES)
		static void SetSDLWindow(struct SDL_Window* pWin) { ms_pSDLWindow = pWin; }
		static struct SDL_Window* GetSDLWindow() { return ms_pSDLWindow; }
		static void SetGLContext(void* pCtx) { ms_hGLContext = pCtx; }
		static void* GetGLContext() { return ms_hGLContext; }
#endif


		void		SetSimpleCamera(float x, float y, float z, float pitch, float roll) const;
		void		SetEyeCamera(float xEye, float yEye, float zEye, float xCenter, float yCenter, float zCenter, float xUp, float yUp, float zUp) const;
		void		SetAroundCamera(float distance, float pitch, float roll, float lookAtZ = 0.0f) const;
		void		SetPositionCamera(float fx, float fy, float fz, float fDistance, float fPitch, float fRotation) const;
		void		MoveCamera(float fdeltax, float fdeltay, float fdeltaz);

		void		GetTargetPosition(float * px, float * py, float * pz) const;
		void		GetCameraPosition(float * px, float * py, float * pz) const;
		void		SetOrtho2D(float hres, float vres, float zres) const;
		void		SetOrtho3D(float hres, float vres, float zmin, float zmax) const;
		void		SetPerspective(float fov, float aspect, float nearz, float farz) const;
				float		GetFOV() const;
		float		GetAspect() const;
		float		GetNear() const;
		float		GetFar() const;
		void		GetClipPlane(float * fNearY, float * fFarY)
		{
			*fNearY = ms_fNearY;
			*fFarY = ms_fFarY;
		}

		void		GetClipPlane(float * fNearY, float * fFarY) const
		{
			*fNearY = ms_fNearY;
			*fFarY = ms_fFarY;
		}

		////////////////////////////////////////////////////////////////////////
		void		PushMatrix() const;

		void		MultMatrix( const D3DXMATRIX* pMat ) const;
		void		MultMatrixLocal( const D3DXMATRIX* pMat ) const;

		void		Translate(float x, float y, float z) const;
		void		Rotate(float degree, float x, float y, float z) const;
		void		RotateLocal(float degree, float x, float y, float z) const;
		void		RotateYawPitchRollLocal(float fYaw, float fPitch, float fRoll) const;
		void		Scale(float x, float y, float z) const;
		void		PopMatrix() const;
		void		LoadMatrix(const D3DXMATRIX & c_rSrcMatrix) const;
		void		GetMatrix(D3DXMATRIX * pRetMatrix) const;
		const		D3DXMATRIX * GetMatrixPointer() const;

		// Special Routine
		void		GetSphereMatrix(D3DXMATRIX * pMatrix, float fValue = 0.1f) const;

		////////////////////////////////////////////////////////////////////////
		void		InitScreenEffect() const;
		void		SetScreenEffectWaving(float fDuringTime, int iPower) const;
		void		SetScreenEffectFlashing(float fDuringTime, const D3DXCOLOR & c_rColor) const;

		////////////////////////////////////////////////////////////////////////
		DWORD		GetColor(float r, float g, float b, float a = 1.0f) const;

		DWORD		GetFaceCount() const;
		void		ResetFaceCount() const;
		HRESULT		GetLastResult() const;

		void		UpdateProjMatrix() const;
		void		UpdateViewMatrix() const;

		void		SetViewport(DWORD dwX, DWORD dwY, DWORD dwWidth, DWORD dwHeight, float fMinZ, float fMaxZ) const;
		static void		GetBackBufferSize(UINT* puWidth, UINT* puHeight);
		static bool		IsTLVertexClipping();
		static bool		IsFastTNL();
		static bool		IsLowTextureMemory();
		static bool		IsHighTextureMemory();

#if defined(USE_OPENGL_ES)
		static unsigned int				ms_uPDTVBO;
		static unsigned int				ms_auDefEBO[DEFAULT_IB_NUM];
		static unsigned int				ms_uUIShaderProgram;
		static int						ms_iLocProjMatrix;
		static int						ms_iLocTexture;
		static int						ms_iLocUseTexture;
		static int						ms_iLocUIWorld;
		static int						ms_iLocUITexMat1;
		static int						ms_iLocUIUseMask;
		static int						ms_iLocUIMaskTexture;
		static int						ms_iLocUIUseTFactor;
		static int						ms_iLocUITFactorColor;

		static unsigned int				ms_uMeshShaderProgram;
		static int						ms_iLocMeshWVP;
		static int						ms_iLocMeshWorld;
		static int						ms_iLocMeshTexture;
		static int						ms_iLocMeshTexture2;
		static int						ms_iLocMeshUseTexture;
		static int						ms_iLocMeshUseTexture2;
		static int						ms_iLocMeshLightDir;
		static int						ms_iLocMeshLightColor;
		static int						ms_iLocMeshAmbientColor;
		static int						ms_iLocMeshAlphaTest;
		static int						ms_iLocMeshAlphaRef;
		static int						ms_iLocMeshFogColor;
		static int						ms_iLocMeshFogRange;
		static int						ms_iLocMeshFogEnable;
		static int						ms_iLocMeshCameraPos;
		static int						ms_iLocMeshSunDir;
		static int						ms_iLocMeshIsDungeonMap;
		static int						ms_iLocMeshView;
		static int						ms_iLocMeshTexMat1;
		static int						ms_iLocMeshIsShadowPass;
		static int						ms_iLocMeshIsPCBlockerPass;

		static unsigned int				ms_uTerrainShaderProgram;
		static int						ms_iLocTerrainWVP;
		static int						ms_iLocTerrainView;
		static int						ms_iLocTerrainWorld;
		static int						ms_iLocTerrainTexMat0;
		static int						ms_iLocTerrainTexMat1;
		static int						ms_iLocTerrainTileTexture;
		static int						ms_iLocTerrainSplatTexture;
		static int						ms_iLocTerrainUseTileTexture;
		static int						ms_iLocTerrainUseSplatAlpha;
		static int						ms_iLocTerrainIsShadowPass;
		static int						ms_iLocTerrainLightDir;
		static int						ms_iLocTerrainLightColor;
		static int						ms_iLocTerrainAmbientColor;
		static int						ms_iLocTerrainFogColor;
		static int						ms_iLocTerrainFogRange;
		static int						ms_iLocTerrainFogEnable;
		static int						ms_iLocTerrainCameraPos;
		static int						ms_iLocTerrainSunDir;
		static int						ms_iLocTerrainIsDungeonMap;

		static unsigned int				ms_uParticleShaderProgram;
		static int						ms_iLocParticleWVP;
		static int						ms_iLocParticleTexture;
		static int						ms_iLocParticleUseTexture;
		static int						ms_iLocParticleColor;
		static int						ms_iLocParticleUseVertexColor;
		static int						ms_iLocParticleColorOp;

		static unsigned int				ms_uSkyShaderProgram;
		static int						ms_iLocSkyWVP;
		static int						ms_iLocSkyWorld;
		static int						ms_iLocSkyTexMat0;
		static int						ms_iLocSkyIsCloud;
		static int						ms_iLocSkyUseTexture;
		static int						ms_iLocSkyTime;
		static int						ms_iLocSkyCameraPos;
		static int						ms_iLocSkyTexture;
		static int						ms_iLocSkyColors;
		static int						ms_iLocSkyColorCount;
		static int						ms_iLocSkyMode;
		static int						ms_iLocSkySunDir;
		static int						ms_iLocSkySunIntensity;

		static unsigned int				ms_uWaterShaderProgram;
		static int						ms_iLocWaterWVP;
		static int						ms_iLocWaterWorld;
		static int						ms_iLocWaterScale;
		static int						ms_iLocWaterCameraPos;
		static int						ms_iLocWaterFogColor;
		static int						ms_iLocWaterFogRange;
		static int						ms_iLocWaterFogEnable;
		static int						ms_iLocWaterTime;
		static int						ms_iLocWaterTexture;

		static unsigned int				ms_uSpeedTreeShaderProgram;
		static int						ms_iLocSpeedTreeWVP;
		static int						ms_iLocSpeedTreeWorld;
		static int						ms_iLocSpeedTreeTexture;
		static int						ms_iLocSpeedTreeAlphaTest;
		static int						ms_iLocSpeedTreeAlphaRef;
		static int						ms_iLocSpeedTreeFogColor;
		static int						ms_iLocSpeedTreeFogRange;
		static int						ms_iLocSpeedTreeFogEnable;
		static int						ms_iLocSpeedTreeCameraPos;
		static int						ms_iLocSpeedTreeSunDir;

		static bool						ms_bSkyBoxMode;
		static bool						ms_bSkyCloudMode;
		static bool						ms_bWaterMode;
		static bool						ms_bSpeedTreeMode;
		static bool						ms_bDungeonMapMode;
#endif
	public:
#if defined(USE_OPENGL_ES)
		static void SetSkyBoxMode(bool bEnable, bool bIsCloud = false) { ms_bSkyBoxMode = bEnable; ms_bSkyCloudMode = bIsCloud; }
		static bool IsSkyBoxMode() { return ms_bSkyBoxMode; }
		static bool IsSkyCloudMode() { return ms_bSkyCloudMode; }
		static void SetWaterMode(bool bEnable) { ms_bWaterMode = bEnable; }
		static bool IsWaterMode() { return ms_bWaterMode; }
		static void SetSpeedTreeMode(bool bEnable) { ms_bSpeedTreeMode = bEnable; }
		static bool IsSpeedTreeMode() { return ms_bSpeedTreeMode; }
		static void SetDungeonMapMode(bool bEnable) { ms_bDungeonMapMode = bEnable; }
		static bool IsDungeonMapMode() { return ms_bDungeonMapMode; }
#else
		static void SetSkyBoxMode(bool, bool = false) {}
		static bool IsSkyBoxMode() { return false; }
		static bool IsSkyCloudMode() { return false; }
		static void SetWaterMode(bool) {}
		static bool IsWaterMode() { return false; }
		static void SetSpeedTreeMode(bool) {}
		static bool IsSpeedTreeMode() { return false; }
		static void SetDungeonMapMode(bool) {}
		static bool IsDungeonMapMode() { return false; }
#endif
		static void SetDefaultIndexBuffer(UINT eDefIB);
		static bool SetPDTStream(SPDTVertexRaw* pVertices, UINT uVtxCount);
		static bool SetPDTStream(SPDTVertex* pVertices, UINT uVtxCount);
		static bool SetPDTTextLineStream(SPDTVertexRaw* pVertices, UINT uVtxCount);

	protected:
		static D3DXMATRIX				ms_matIdentity;

		static D3DXMATRIX				ms_matView;
		static D3DXMATRIX				ms_matProj;
		static D3DXMATRIX				ms_matInverseView;
		static D3DXMATRIX				ms_matInverseViewYAxis;

		static D3DXMATRIX				ms_matWorld;
		static D3DXMATRIX				ms_matWorldView;

	protected:
		void		UpdatePipeLineMatrix() const;

	protected:
		static LPD3DXMESH				ms_lpSphereMesh;
		static LPD3DXMESH				ms_lpCylinderMesh;

	protected:
		static HRESULT					ms_hLastResult;

		static int						ms_iWidth;
		static int						ms_iHeight;

		static UINT						ms_iD3DAdapterInfo;
		static UINT						ms_iD3DDevInfo;
		static UINT						ms_iD3DModeInfo;
		static D3D_CDisplayModeAutoDetector				ms_kD3DDetector;

		static HWND						ms_hWnd;
		static HDC						ms_hDC;
#if defined(USE_OPENGL_ES)
		static struct SDL_Window*		ms_pSDLWindow;
		static void*					ms_hGLContext;
#endif
		static LPDIRECT3D9EX			ms_lpd3d;
		static LPDIRECT3DDEVICE9EX		ms_lpd3dDevice;
		static ID3DXMatrixStack*		ms_lpd3dMatStack;
		static D3DVIEWPORT9				ms_Viewport;

		static DWORD					ms_faceCount;
		static D3DCAPS9					ms_d3dCaps;
		static D3DPRESENT_PARAMETERS	ms_d3dPresentParameter;

		static DWORD					ms_dwD3DBehavior;
		static LPDIRECT3DVERTEXDECLARATION9					ms_ptVS;
		static LPDIRECT3DVERTEXDECLARATION9					ms_pntVS;
		static LPDIRECT3DVERTEXDECLARATION9					ms_pnt2VS;
		static LPDIRECT3DVERTEXDECLARATION9					ms_doublePntVS;

		static D3DXMATRIX				ms_matScreen0;
		static D3DXMATRIX				ms_matScreen1;
		static D3DXMATRIX				ms_matScreen2;
		//static D3DXMATRIX				ms_matPrePipeLine;

		static D3DXVECTOR3				ms_vtPickRayOrig;
		static D3DXVECTOR3				ms_vtPickRayDir;

		static float					ms_fFieldOfView;
		static float					ms_fAspect;
		static float					ms_fNearY;
		static float					ms_fFarY;

		/*
		static std::vector<TIndex>		ms_lineIdxVector;
		static std::vector<TIndex>		ms_lineTriIdxVector;
		static std::vector<TIndex>		ms_lineRectIdxVector;
		static std::vector<TIndex>		ms_lineCubeIdxVector;

		static std::vector<TIndex>		ms_fillTriIdxVector;
		static std::vector<TIndex>		ms_fillRectIdxVector;
		static std::vector<TIndex>		ms_fillCubeIdxVector;
		*/

		// Screen Effect - Waving, Flashing and so on..
		static DWORD					ms_dwWavingEndTime;
		static int						ms_iWavingPower;
		static DWORD					ms_dwFlashingEndTime;
		static D3DXCOLOR				ms_FlashingColor;

 		static CRay						ms_Ray;

		static bool						ms_bSupportDXT;
		static bool						ms_isLowTextureMemory;
		static bool						ms_isHighTextureMemory;

	public:
		enum
		{
			PDT_VERTEX_NUM = 16,
			PDT_VERTEXBUFFER_NUM = 100,
			PDT_TEXTLINE_VERTEX_NUM = 1024 * 10,
		};

	protected:

#ifdef ENABLE_FIX_MOBS_LAG
		static LPDIRECT3DVERTEXBUFFER9 m_smallPdtVertexBuffer;
		static LPDIRECT3DVERTEXBUFFER9 m_largePdtVertexBuffer;
#else
		static LPDIRECT3DVERTEXBUFFER9 ms_alpd3dPDTVB[PDT_VERTEXBUFFER_NUM];
#endif

		static LPDIRECT3DVERTEXBUFFER9	ms_alpd3dTextLinePDTVB;
		static LPDIRECT3DINDEXBUFFER9	ms_alpd3dDefIB[DEFAULT_IB_NUM];
};

