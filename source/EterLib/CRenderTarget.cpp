#include "StdAfx.h"
#include "CRenderTarget.h"
#include "../EterLib/Camera.h"
#include "../EterLib/CRenderTargetManager.h"
#include "../EterPythonLib/PythonGraphic.h"
#include "../EterBase/CRC32.h"
#include "../GameLib/GameType.h"
#include "../GameLib/MapType.h"
#include "../GameLib/ItemData.h"
#include "../GameLib/ActorInstance.h"
#include "../UserInterface/InstanceBase.h"
#include "ResourceManager.h"

CRenderTarget::CRenderTarget(const DWORD width, const DWORD height) : m_pModel(nullptr),
                                 m_background(nullptr),
                                 m_modelRotation(0),
                                 m_visible(false) 
{
	auto pTex = new CGraphicRenderTargetTexture;
	if (!pTex->Create(width, height, D3DFMT_X8R8G8B8, D3DFMT_D16)) {
		delete pTex;
		TraceError("CRenderTarget::CRenderTarget: Could not create CGraphicRenderTargetTexture %dx%d", width, height);
		return;
	}

	m_renderTargetTexture = std::unique_ptr<CGraphicRenderTargetTexture>(pTex);
}

CRenderTarget::~CRenderTarget()
{

}

void CRenderTarget::SetVisibility(bool isShow)
{
	m_visible = isShow;
}
void CRenderTarget::SetArmor(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
	m_pModel->ChangeArmor(vnum);
}

void CRenderTarget::SetWeapon(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
	m_pModel->ChangeWeapon(vnum);
}

void CRenderTarget::SetAcce(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
}

void CRenderTarget::ChangeHair(DWORD vnum)
{
	if (!m_visible || !m_pModel)
		return;
	m_pModel->ChangeHair(vnum);
}
void CRenderTarget::RenderTexture() const
{
	m_renderTargetTexture->Render();
}

void CRenderTarget::SetRenderingRect(RECT* rect) const
{
	m_renderTargetTexture->SetRenderingRect(rect);
}

void CRenderTarget::CreateTextures() const
{
	m_renderTargetTexture->CreateTextures();
}

void CRenderTarget::ReleaseTextures() const
{
	m_renderTargetTexture->ReleaseTextures();
}

void CRenderTarget::SelectModel(const DWORD index)
{
	CInstanceBase::SCreateData kCreateData{};


	kCreateData.m_bType = CActorInstance::TYPE_PC;
	kCreateData.m_dwRace = index;

	auto model = std::make_unique<CInstanceBase>();
	if (!model->Create(kCreateData))
	{
		if (m_pModel)
		{
			m_pModel.reset();
		}
		return;
	}

	m_pModel = std::move(model);
	m_pModel->NEW_SetPixelPosition(TPixelPosition(0,0,0));
	m_pModel->GetGraphicThingInstancePtr()->ClearAttachingEffect();
	m_modelRotation = 0.0f;
	m_pModel->Refresh(CRaceMotionData::NAME_WAIT, true);
	m_pModel->SetLoopMotion(CRaceMotionData::NAME_WAIT);
	m_pModel->SetAlwaysRender(true);
	m_pModel->SetRotation(0.0f);

	auto& camera_manager = CCameraManager::instance();
	camera_manager.SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
	camera_manager.GetCurrentCamera()->SetTargetHeight(110.0);
	camera_manager.ResetToPreviousCamera();
}

bool CRenderTarget::CreateBackground(const char* imgPath, const DWORD width, const DWORD height)
{
	if (m_background)
		return false;

	m_background = std::make_unique<CGraphicImageInstance>();

	const auto graphic_image = dynamic_cast<CGraphicImage*>(CResourceManager::instance().GetResourcePointer(imgPath));
	if (!graphic_image)
	{
		m_background.reset();
		return false;
	}

	m_background->SetImagePointer(graphic_image);

	const float imgW = graphic_image->GetWidth();
	const float imgH = graphic_image->GetHeight();

	const float scale = static_cast<float>(width) / imgW;
	m_background->SetScale(scale, scale);

	const float posY = (height - imgH * scale) / 2.0f;
	m_background->SetPosition(0.0f, posY);

	return true;
}

void CRenderTarget::RenderBackground() const
{
	if (!m_visible)
		return;

	if (!m_background)
		return;

	auto& rectRender = *m_renderTargetTexture->GetRenderingRect();
	m_renderTargetTexture->SetRenderTarget();

	CGraphicRenderTargetTexture::Clear();
	CPythonGraphic::Instance().SetInterfaceRenderState();

	m_background->Render();
	m_renderTargetTexture->ResetRenderTarget();
}

void CRenderTarget::UpdateModel()
{
	if (!m_visible || !m_pModel)
		return;

	if (m_modelRotation < 360.0f)
		m_modelRotation += 1.0f;
	else
		m_modelRotation = 0.0f;

	m_pModel->SetRotation(m_modelRotation);
	m_pModel->Transform();
	m_pModel->GetGraphicThingInstanceRef().RotationProcess();
}

void CRenderTarget::DeformModel() const
{
	if (!m_visible)
		return;

	if (m_pModel)
		m_pModel->Deform();
}

void CRenderTarget::RenderModel() const
{
	if (!m_visible)
	{
		return;
	}

	auto& python_graphic = CPythonGraphic::Instance();
	auto& camera_manager = CCameraManager::instance();
	auto& state_manager = CStateManager::Instance();

	auto& rectRender = *m_renderTargetTexture->GetRenderingRect();

	if (!m_pModel)
	{
		return;
	}

	m_renderTargetTexture->SetRenderTarget();

	if (!m_background)
	{
		m_renderTargetTexture->Clear();
	}

	python_graphic.ClearDepthBuffer();

#if defined(USE_OPENGL_ES)
	// Ensure depth state is correct for 3D rendering after 2D background pass
	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glClear(GL_DEPTH_BUFFER_BIT);
#endif

	const auto fov = python_graphic.GetFOV();
	const auto aspect = python_graphic.GetAspect();
	const auto near_y = python_graphic.GetNear();
	const auto far_y = python_graphic.GetFar();

	const auto width = static_cast<float>(rectRender.right - rectRender.left);
	const auto height = static_cast<float>(rectRender.bottom - rectRender.top);

	if (width <= 0.0f || height <= 0.0f)
	{
		m_renderTargetTexture->ResetRenderTarget();
		return;
	}

	state_manager.SetRenderState(D3DRS_FOGENABLE, FALSE);
#if defined(USE_OPENGL_ES)
	state_manager.SetRenderState(D3DRS_ZENABLE, TRUE);
	state_manager.SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	state_manager.SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	state_manager.SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
#endif

	python_graphic.SetViewport(0.0f, 0.0f, width, height);

	python_graphic.PushState();

	camera_manager.SetCurrentCamera(CCameraManager::SHOPDECO_CAMERA);
	camera_manager.GetCurrentCamera()->SetViewParams(
		D3DXVECTOR3{ 0.0f, -1500.0f, 600.0f },
		D3DXVECTOR3{ 0.0f, 0.0f, 95.0f },
		D3DXVECTOR3{ 0.0f, 0.0f, 1.0f }
	);

	python_graphic.UpdateViewMatrix();

	python_graphic.SetPerspective(10.0f, width / height, 100.0f, 3000.0f);

	m_pModel->Render();
	camera_manager.ResetToPreviousCamera();
	python_graphic.RestoreViewport();
	python_graphic.PopState();
	python_graphic.SetPerspective(fov, aspect, near_y, far_y);
	m_renderTargetTexture->ResetRenderTarget();
	state_manager.SetRenderState(D3DRS_FOGENABLE, FALSE);
}
