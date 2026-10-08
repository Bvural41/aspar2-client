#include "StdAfx.h"
#include "PythonBattleRoyaleManager.h"
#include "PythonApplication.h"
#include "../EffectLib/EffectManager.h"

const char* sBattleyEffectFileName = "d:/ymir work/effect/battleroyale/battley_01_64.mse";

PythonBattleRoyaleManager::PythonBattleRoyaleManager()
{
	m_sCrownImage = "D:/ymir work/effect/battleroyale/crown01.tga";
	m_sAtlasElectricFieldImage = "d:/ymir work/ui/game/battleroyale/electric_field_in_atlas_2.sub";
	m_bIsDisableOpenCharacterMenu = true;
	m_bIsPCTextTailHided = true;
	m_bIsDisablePickedInstanceInfo = true;
	m_bIsPlaying = false;
	m_iBattleyEffectIndex = 0;
	m_bIsDropItemDialogDisabled = true;
	m_bUpdate = true;
	m_v4WhiteCirclePos = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
	m_v4RedCirclePos = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
	m_Unknown_0 = 82.0f;
	m_v4Unknown_1 = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
	m_fUnknown_2 = 0.0f;
	m_fUnknown_3 = 0.0f;
	m_fUnknown_4 = 0.0f;
	m_fUnknown_5 = 0.0f;
}

PythonBattleRoyaleManager::~PythonBattleRoyaleManager()
{
}

void PythonBattleRoyaleManager::BATTLE_ROYALE_Unknown_1()
{
	m_iBattleyEffectIndex = 0;
	m_bIsPlaying = false;
	m_bUpdate = false;

	m_v4RedCirclePos = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
	m_v4WhiteCirclePos = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);

	m_fUnknown_2 = 0.0f;
	m_fUnknown_3 = 0.0f;
	m_fUnknown_4 = 0.0f;
	m_fUnknown_5 = 0.0f;

	CEffectManager::Instance().RegisterEffect(sBattleyEffectFileName);
}

void PythonBattleRoyaleManager::SetWhiteCirclePos(float fX, float fY, float fRadius)
{
	m_v4WhiteCirclePos = D3DXVECTOR4(fX, fY, 0.0f, fRadius);
}

void PythonBattleRoyaleManager::SetRedCirclePos(float fX, float fY, float fRadius)
{
	m_v4RedCirclePos = D3DXVECTOR4(fX, fY, 0.0f, fRadius);

	if (!m_iBattleyEffectIndex)
		CreateEffect();
}

void PythonBattleRoyaleManager::Shrink(float fTime)
{
	m_bUpdate = true;
	m_v4Unknown_1 = m_v4RedCirclePos;
	m_fUnknown_2 = fTime;
	m_fUnknown_3 = fTime;
}

void PythonBattleRoyaleManager::BATTLE_ROYALE_Unknown_5()
{
	m_v4RedCirclePos = m_v4WhiteCirclePos;
	m_v4WhiteCirclePos = D3DXVECTOR4(0.0f, 0.0f, 0.0f, 0.0f);
	m_bUpdate = false;
	m_fUnknown_4 = 0.0f;
	m_fUnknown_5 = 0.0f;
}

void PythonBattleRoyaleManager::BATTLE_ROYALE_Unknown_6()
{
	BATTLE_ROYALE_Unknown_1();
}

void PythonBattleRoyaleManager::CreateEffect()
{
	m_iBattleyEffectIndex = CEffectManager::Instance().CreateEffect(sBattleyEffectFileName, D3DXVECTOR3(0.0f, 0.0f, 0.0f), D3DXVECTOR3(0.0f, 0.0f, 0.0f));
	
	if (CEffectManager::Instance().SelectEffectInstance(m_iBattleyEffectIndex))
		CEffectManager::Instance().SetEffectInstanceAlwaysRender(true);
}

void PythonBattleRoyaleManager::Update()
{
	if (!m_bUpdate)
		return;

	const float fGlobalTime = CPythonApplication::Instance().GetGlobalTime();

	if (m_fUnknown_5 >= fGlobalTime)
		return;

	if (m_fUnknown_5 == 0.0f)
		m_fUnknown_4 = 0.0f;
	else
		m_fUnknown_4 = fGlobalTime - m_fUnknown_5;
	
	m_fUnknown_3 = m_fUnknown_3 - m_fUnknown_4;

	if (m_fUnknown_3 > 0.0f)
	{
		m_v4RedCirclePos.w = m_v4RedCirclePos.w - m_fUnknown_4 / m_fUnknown_2 * (m_v4Unknown_1.w - m_v4WhiteCirclePos.w);
		if (CEffectManager::Instance().SelectEffectInstance(m_iBattleyEffectIndex))
		{
			if (m_v4RedCirclePos.w >= 5.0)
			{
				const float fScale = m_v4RedCirclePos.w / 64.0f;
				CEffectManager::Instance().SetEffectInstanceMeshScale(D3DXVECTOR3(fScale, fScale, 10.0f));
			}
			else
			{
				CEffectManager::Instance().SetEffectInstanceMeshScale(D3DXVECTOR3(0.0f, 0.0f, 0.0f));
			}

			const float fWidth = m_v4WhiteCirclePos.x - m_v4Unknown_1.x;
			const float fHeight = m_v4WhiteCirclePos.y - m_v4Unknown_1.y;

			m_v4RedCirclePos.x = m_fUnknown_4 / m_fUnknown_2 * fWidth + m_v4RedCirclePos.x;
			m_v4RedCirclePos.y = m_fUnknown_4 / m_fUnknown_2 * fHeight + m_v4RedCirclePos.y;

			CEffectManager::Instance().SetEffectInstanceGlobalMatrixPosition(D3DXVECTOR3(m_v4RedCirclePos.x * 100.0f, m_v4RedCirclePos.y * -100.0f, 10000.0f));

			m_fUnknown_5 = fGlobalTime;
		}
	}
	else
	{
		m_v4RedCirclePos = m_v4WhiteCirclePos;
		m_bUpdate = false;
	}
}

/*MODULE*/
PyObject* battleRoyaleMgrIsPCTextTailHided(PyObject* poTarget, PyObject* poArgs)
{
	return Py_BuildValue("i", PythonBattleRoyaleManager::Instance().IsPCTextTailHided());
}

PyObject* battleRoyaleMgrIsDropItemDialogDisabled(PyObject* poTarget, PyObject* poArgs)
{
	return Py_BuildValue("i", PythonBattleRoyaleManager::Instance().IsDropItemDialogDisabled());
}

void initbattleRoyaleMgr()
{
	static PyMethodDef s_methods[] =
	{
		{	"IsPCTextTailHided",			battleRoyaleMgrIsPCTextTailHided,			METH_VARARGS	},
		{	"IsDropItemDialogDisabled",		battleRoyaleMgrIsDropItemDialogDisabled,	METH_VARARGS	},
		{	NULL,							NULL,										NULL			},
	};

	PyObject* poModule = Py_InitModule("battleRoyaleMgr", s_methods);
	PyModule_AddIntConstant(poModule, "BATTLE_ROYALE_CHAT_DISABLED", PythonBattleRoyaleManager::BATTLE_ROYALE_CHAT_DISABLED);
	PyModule_AddIntConstant(poModule, "BATTLE_ROYALE_INVENTORY_EXTEND_BUTTON_MIN_RANGE", PythonBattleRoyaleManager::BATTLE_ROYALE_INVENTORY_EXTEND_BUTTON_MIN_RANGE);
	PyModule_AddIntConstant(poModule, "BATTLE_ROYALE_INVENTORY_EXTEND_BUTTON_MAX_RANGE", PythonBattleRoyaleManager::BATTLE_ROYALE_INVENTORY_EXTEND_BUTTON_MAX_RANGE);
	PyModule_AddIntConstant(poModule, "ITEM_VNUM_BATTLE_ROYALE_INFINITE_STAMINA", PythonBattleRoyaleManager::ITEM_VNUM_BATTLE_ROYALE_INFINITE_STAMINA);
}