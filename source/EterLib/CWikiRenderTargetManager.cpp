#include "StdAfx.h"
#ifdef ENABLE_INGAME_WIKI
#include "../EterBase/Stl.h"
#include "CWikiRenderTargetManager.h"

CWikiRenderTargetManager::~CWikiRenderTargetManager() { InitializeData(); }
CWikiRenderTargetManager::CWikiRenderTargetManager() { InitializeData(); }

/*----------------------------
--------PUBLIC CLASS FUNCTIONS
-----------------------------*/

std::shared_ptr<CWikiRenderTarget> CWikiRenderTargetManager::GetRenderTarget(const int module_id)
{
	const auto it = m_renderTargets.find(module_id);
	if (it != m_renderTargets.end())
		return it->second;

	return nullptr;
}

bool CWikiRenderTargetManager::CreateRenderTarget(const int module_id, const int width, const int height)
{
	if (module_id < 1 || GetRenderTarget(module_id))
		return false;

	m_renderTargets.emplace(module_id, std::make_shared<CWikiRenderTarget>(width, height));
	return true;
}

void CWikiRenderTargetManager::DeleteRenderTarget(const int module_id)
{
	const auto it = m_renderTargets.find(module_id);
	if (it != m_renderTargets.end())
		m_renderTargets.erase(it);
}

void CWikiRenderTargetManager::CreateRenderTargetTextures() const
{
	for (const auto& kv : m_renderTargets)
		kv.second->CreateTextures();
}

void CWikiRenderTargetManager::ReleaseRenderTargetTextures() const
{
	for (const auto& kv : m_renderTargets)
		kv.second->ReleaseTextures();
}

void CWikiRenderTargetManager::DeformModels() const
{
	for (const auto& kv : m_renderTargets)
		kv.second->DeformModel();
}

void CWikiRenderTargetManager::UpdateModels() const
{
	for (const auto& kv : m_renderTargets)
		kv.second->UpdateModel();
}

void CWikiRenderTargetManager::RenderBackgrounds() const
{
	for (const auto& kv : m_renderTargets)
		kv.second->RenderBackground();
}

void CWikiRenderTargetManager::RenderModels() const
{
	for (const auto& kv : m_renderTargets)
		kv.second->RenderModel();
}
#endif
