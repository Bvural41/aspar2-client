#pragma once

#include "Packet.h"
#include "AbstractChat.h"
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
#include "PythonSystem.h"
#endif

class CWhisper
{
	public:
		typedef struct SChatLine
		{
			CGraphicTextInstance Instance;

			SChatLine()
			{
			}
			~SChatLine()
			{
				Instance.Destroy();
			}

			static void DestroySystem();

			static SChatLine* New();
			static void Delete(SChatLine* pkChatLine);

			static CDynamicPool<SChatLine> ms_kPool;
		} TChatLine;

		typedef std::deque<TChatLine*> TChatLineDeque;
		typedef std::list<TChatLine*> TChatLineList;

	public:
		CWhisper();
		~CWhisper();

		void Destroy();

		void SetPosition(float fPosition);
		void SetBoxSize(float fWidth, float fHeight);
		void AppendChat(int iType, const char* c_szChat);
		void Render(float fx, float fy);

	protected:
		void __Initialize();
		void __ArrangeChat();

	protected:
		float m_fLineStep;
		float m_fWidth;
		float m_fHeight;
		float m_fcurPosition;

		TChatLineDeque m_ChatLineDeque;
		TChatLineList m_ShowingChatLineList;

	public:
		static void DestroySystem();

		static CWhisper* New();
		static void Delete(CWhisper* pkWhisper);

		static CDynamicPool<CWhisper>		ms_kPool;
};

class CPythonChat : public CSingleton<CPythonChat>, public IAbstractChat
{
	public:
		enum EWhisperType
		{
			WHISPER_TYPE_CHAT               = 0,
			WHISPER_TYPE_NOT_EXIST          = 1,
			WHISPER_TYPE_TARGET_BLOCKED     = 2,
			WHISPER_TYPE_SENDER_BLOCKED     = 3,
			WHISPER_TYPE_ERROR              = 4,
			WHISPER_TYPE_GM                 = 5,
			WHISPER_TYPE_SYSTEM             = 0xFF
		};

		enum EBoardState
		{
			BOARD_STATE_VIEW,
			BOARD_STATE_EDIT,
			BOARD_STATE_LOG,
		};

		enum
		{
			CHAT_LINE_MAX_NUM = 300,
#ifdef ENABLE_CHAT_STACK
			CHAT_STACK_MAX_NUM = 200,
#endif
			CHAT_LINE_COLOR_ARRAY_MAX_NUM = 3,
		};

		enum EHyperlinkType
		{
			HYPER_LINK_ITEM_KEYWORD = 0,  // Başlangıç değerini belirtebilirsin
			HYPER_LINK_ITEM_VNUM,
			HYPER_LINK_ITEM_FLAGS,
			HYPER_LINK_ITEM_SOCKET0,
			HYPER_LINK_ITEM_SOCKET1,
			HYPER_LINK_ITEM_SOCKET2,
			HYPER_LINK_ITEM_SOCKET3,
			HYPER_LINK_ITEM_SOCKET4,
			HYPER_LINK_ITEM_SOCKET5,
			HYPER_LINK_ITEM_CHANGE_ITEM_VNUM,
			HYPER_LINK_ITEM_ELEMENT_ITEM_VNUM,
#ifdef ENABLE_SET_ITEM
			HYPER_LINK_ITEM_PRE_SET_VALUE,
#endif
			HYPER_LINK_ITEM_APPLY_RANDOM_TYPE0,
			HYPER_LINK_ITEM_APPLY_RANDOM_VALUE0,
			HYPER_LINK_ITEM_APPLY_RANDOM_TYPE1,
			HYPER_LINK_ITEM_APPLY_RANDOM_VALUE1,
			HYPER_LINK_ITEM_APPLY_RANDOM_TYPE2,
			HYPER_LINK_ITEM_APPLY_RANDOM_VALUE2,
			HYPER_LINK_ITEM_APPLY_RANDOM_TYPE3,
			HYPER_LINK_ITEM_APPLY_RANDOM_VALUE3,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE0,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE0,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE1,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE1,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE2,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE2,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE3,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE3,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE4,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE4,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE5,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE5,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE6,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE6,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE7,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE7,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE8,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE8,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE9,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE9,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE10,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE10,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE11,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE11,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE12,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE12,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE13,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE13,
			HYPER_LINK_ITEM_ATTRIBUTE_TYPE14,
			HYPER_LINK_ITEM_ATTRIBUTE_VALUE14,

			HYPER_LINK_ITEM_MAX,
		};

		typedef struct SChatLine
		{
			int iType;
			float fAppendedTime;
			D3DXCOLOR aColor[CHAT_LINE_COLOR_ARRAY_MAX_NUM];
			CGraphicTextInstance Instance;
#ifdef ENABLE_NEW_CHAT_VIEW
			CGraphicImageInstance InstanceEmpire;
#endif
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			CGraphicImageInstance InstanceLang;
#endif
			SChatLine();
			virtual ~SChatLine();

#ifdef ENABLE_CHAT_SETTINGS
			std::string text;
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
			uint8_t countryIndex;
			std::string	countryFile;
#endif
#endif

			void SetColor(DWORD dwID, DWORD dwColor);
			void SetColorAll(DWORD dwColor);
			D3DXCOLOR & GetColorRef(DWORD dwID);
			static void DestroySystem();

			static SChatLine* New();
			static void Delete(SChatLine* pkChatLine);

			static CDynamicPool<SChatLine> ms_kPool;
		} TChatLine;

		typedef struct SWaitChat
		{
			int iType;
			std::string strChat;

			DWORD dwAppendingTime;
		} TWaitChat;

		typedef std::deque<TChatLine*> TChatLineDeque;
		typedef std::list<TChatLine*> TChatLineList;

		typedef std::map<std::string, CWhisper*> TWhisperMap;
		typedef std::set<std::string> TIgnoreCharacterSet;
		typedef std::list<TWaitChat> TWaitChatList;

		typedef struct SChatSet
		{
			int					m_ix;
			int					m_iy;
			int					m_iHeight;
			int					m_iStep;
			float				m_fEndPos;

			int					m_iBoardState;
			std::vector<int>	m_iMode;

			TChatLineList		m_ShowingChatLineList;

			bool CheckMode(DWORD dwType)
			{
				if (dwType >= m_iMode.size())
					return false;

				return m_iMode[dwType] ? true : false;
			}

#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
			std::vector<int> m_iCountryMode;
			bool CheckCountryMode(uint32_t dwType)
			{
				if (dwType >= m_iCountryMode.size())
					return false;
				return m_iCountryMode[dwType] ? true : false;
			}
#endif

			SChatSet()
			{
				m_iBoardState = BOARD_STATE_VIEW;

				m_ix = 0;
				m_iy = 0;
				m_fEndPos = 1.0f;
				m_iHeight = 0;
				m_iStep = 15;

				m_iMode.clear();
				m_iMode.resize(ms_iChatModeSize, 1);

#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
				m_iCountryMode.clear();
				m_iCountryMode.resize(15, 1);
#endif

			}

			static int ms_iChatModeSize;
		} TChatSet;

		typedef std::map<int, TChatSet> TChatSetMap;

#ifdef ENABLE_CHAT_STACK
		std::vector<std::string>	m_WritedChatStack;
#endif

	public:
		CPythonChat();
		virtual ~CPythonChat();

		void SetChatColor(UINT eType, UINT r, UINT g, UINT b);

		void Destroy();
		void Close();

		int CreateChatSet(DWORD dwID);
		void Update(DWORD dwID);
		void Render(DWORD dwID);
		void RenderWhisper(const char * c_szName, float fx, float fy);

		void SetBoardState(DWORD dwID, int iState);
		void SetPosition(DWORD dwID, int ix, int iy);
		void SetHeight(DWORD dwID, int iHeight);
		void SetStep(DWORD dwID, int iStep);
		void ToggleChatMode(DWORD dwID, int iMode);
		void EnableChatMode(DWORD dwID, int iMode);
		void DisableChatMode(DWORD dwID, int iMode);
		void SetEndPos(DWORD dwID, float fPos);

#ifdef ENABLE_CHAT_SETTINGS
		void DeleteChatSet(uint32_t dwID);
#endif
#if defined (ENABLE_CHAT_SETTINGS) && defined(ENABLE_MULTI_LANGUAGE_SYSTEM)
		void EnableCountryMode(uint32_t dwID, int iMode);
		void DisableCountryMode(uint32_t dwID, int iMode);
#endif

		int  GetVisibleLineCount(DWORD dwID);
		int  GetEditableLineCount(DWORD dwID);
		int  GetLineCount(DWORD dwID);
		int  GetLineStep(DWORD dwID);

		// Chat
		void AppendChat(int iType, const char * c_szChat);
		void AppendChatWithDelay(int iType, const char * c_szChat, int iDelay);
		void ArrangeShowingChat(DWORD dwID);
#ifdef ENABLE_CHAT_STOP_SYSTEM
		void SetChatStop(bool bStop) { m_bChatStop = bStop; }
		bool IsChatStop() const { return m_bChatStop; }
#endif

#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
		bool IsFilteredLanguage(std::string szLanguage);
#endif

#ifdef ENABLE_CHAT_STACK
		void AddChatStack(const char * c_szChat);
		const char* GetChatStack(BYTE index);
		BYTE GetChatStackSize();
#endif

		// Ignore
		void IgnoreCharacter(const char * c_szName);
		BOOL IsIgnoreCharacter(const char * c_szName);

		// Whisper
		CWhisper * CreateWhisper(const char * c_szName);
		void AppendWhisper(int iType, const char * c_szName, const char * c_szChat);
		void ClearWhisper(const char * c_szName);
		BOOL GetWhisper(const char * c_szName, CWhisper ** ppWhisper);
		void InitWhisper(PyObject * ppyObject);

	protected:
		void __Initialize();
		void __DestroyWhisperMap();

		TChatLineList * GetChatLineListPtr(DWORD dwID);
		TChatSet * GetChatSetPtr(DWORD dwID);

		void UpdateViewMode(DWORD dwID);
		void UpdateEditMode(DWORD dwID);
		void UpdateLogMode(DWORD dwID);

		DWORD GetChatColor(int iType);

	protected:
		TChatLineDeque						m_ChatLineDeque;
#ifdef ENABLE_CHAT_SETTINGS
		std::map<uint8_t, TChatLineDeque> m_ChatLineNewDeque;
#endif
		TChatLineList						m_ShowingChatLineList;
#ifdef ENABLE_CHAT_STOP_SYSTEM
		bool								m_bChatStop;
#endif
		TChatSetMap							m_ChatSetMap;
		TWhisperMap							m_WhisperMap;
		TIgnoreCharacterSet					m_IgnoreCharacterSet;
		TWaitChatList						m_WaitChatList;

		D3DXCOLOR m_akD3DXClrChat[CHAT_TYPE_MAX_NUM];
};