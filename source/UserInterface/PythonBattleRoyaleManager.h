#pragma once

//ENABLE_BATTLE_ROYALE

class PythonBattleRoyaleManager : public CSingleton<PythonBattleRoyaleManager>
{
public:
	enum
	{
		BATTLE_ROYALE_CHAT_DISABLED = 1,
		BATTLE_ROYALE_INVENTORY_EXTEND_BUTTON_MIN_RANGE = 3,
		BATTLE_ROYALE_INVENTORY_EXTEND_BUTTON_MAX_RANGE = 9,
		ITEM_VNUM_BATTLE_ROYALE_INFINITE_STAMINA = 91059,
	};

	enum
	{
		BATTLE_ROYALE_GC_PROCESS_CATEGORY_MINI_GAME,
		BATTLE_ROYALE_GC_PROCESS_CATEGORY_BATTLE_ROYALE_INTERFACE,
	};

	enum
	{
		BATTLE_ROYALE_GC_INSERT_APPLICANT,
		BATTLE_ROYALE_GC_REMOVE_APPLICANT,
		BATTLE_ROYALE_GC_STARTABLE,
		BATTLE_ROYALE_GC_ENTRY_CLOSED,
		BATTLE_ROYALE_GC_START,
		BATTLE_ROYALE_GC_EXIT,
		BATTLE_ROYALE_GC_OPEN_KILL_INFO,
		BATTLE_ROYALE_GC_KILL_INFO,
		BATTLE_ROYALE_GC_DEAD,
		BATTLE_ROYALE_GC_WHITE_CIRCLE,
		BATTLE_ROYALE_GC_RED_CIRCLE,
		BATTLE_ROYALE_GC_SHRINK,
		BATTLE_ROYALE_GC_UNKNOWN,
		BATTLE_ROYALE_GC_WINNER,
		BATTLE_ROYALE_GC_HAS_REWARD,
	};

	enum
	{
		BATTLE_ROYALE_CG_APPLICATION,
		BATTLE_ROYALE_CG_APPLICATION_CANCEL,
		BATTLE_ROYALE_CG_EXIT,
		BATTLE_ROYALE_CG_RANKING,
		BATTLE_ROYALE_CG_START,
		BATTLE_ROYALE_CG_CLAIM_REWARD,
	};

public:
	PythonBattleRoyaleManager();
	~PythonBattleRoyaleManager();

	void BATTLE_ROYALE_Unknown_1();
	void SetWhiteCirclePos(float fX, float fY, float fRadius);
	void SetRedCirclePos(float fX, float fY, float fRadius);
	void Shrink(float fTime);
	void BATTLE_ROYALE_Unknown_5();
	void BATTLE_ROYALE_Unknown_6();
	void CreateEffect();

	void Update();

	const std::string& GetCrownImagePath() const { return m_sCrownImage; }
	const std::string& GetAtlasElectricFieldImagePath() const { return m_sAtlasElectricFieldImage; }
	bool IsDisablePickedInstanceInfo() const { return m_bIsDisablePickedInstanceInfo; }
	bool IsDropItemDialogDisabled() const { return m_bIsDropItemDialogDisabled; }
	bool IsDisableOpenCharacterMenu() const { return m_bIsDisableOpenCharacterMenu; }
	bool IsPCTextTailHided() const { return m_bIsPCTextTailHided; }
	bool IsPlaying() const { return m_bIsPlaying; }
	void SetPlaying(bool bPlay) { m_bIsPlaying = bPlay; }
	const D3DXVECTOR4& GetWhiteCirclePos() const { return m_v4WhiteCirclePos; }
	const D3DXVECTOR4& GetRedCirclePos() const { return m_v4RedCirclePos; }
	float GetUnknown0() const { return m_Unknown_0; }

private:
	std::string m_sCrownImage;
	std::string m_sAtlasElectricFieldImage;
	bool m_bIsDisablePickedInstanceInfo;
	bool m_bIsDropItemDialogDisabled;
	bool m_bIsDisableOpenCharacterMenu;
	int m_iBattleyEffectIndex;
	bool m_bIsPCTextTailHided;
	bool m_bIsPlaying;
	bool m_bUpdate;
	D3DXVECTOR4 m_v4RedCirclePos;
	D3DXVECTOR4 m_v4WhiteCirclePos;
	D3DXVECTOR4 m_v4Unknown_1;
	float m_Unknown_0;
	float m_fUnknown_2;
	float m_fUnknown_3;
	float m_fUnknown_4;
	float m_fUnknown_5;
};