/****************************************
* title_name		: System Info Client
* filename			: ../../UserInterface/Locale_inc.h
* author			: Bvural41
* version			: Version 3.0.2
* date				: 2015 04 11
* update			: 2025 11 06
****************************************/

/*** YMIR Services System ***/
#define LOCALE_SERVICE_SINGAPORE					//Oyun Coğrafyası Singapur
/*** YMIR Services System End ***/

/*** Graphics Engine Configuration (DirectX 9 vs OpenGL ES 3.0) ***/
#if defined(USE_OPENGL_ES) || defined(ENABLE_OPENGL_ES)
#ifndef USE_OPENGL_ES
#define USE_OPENGL_ES
#endif
#ifndef ENABLE_OPENGL_ES
#define ENABLE_OPENGL_ES
#endif
#endif

/*** YMIR System ***/
#define ENABLE_QUICKSLOT_IMPROVE					//Tr QuikSlot Güncellemesi
#define ENABLE_METIN_ELEMENTAL						//Tr Element Taş Sistemi
#define ENABLE_FOOTS_SOCKET							//Tr Ayakakbı Soket Sistemi
#define ENABLE_GREEDY_ROOM							//Tr Aamon Hirs Zindani Sistemi
#define ENABLE_SHAKE_CAMERA							//Tr Kamera Sallantı Efekti
#define ENABLE_MOONLIGHT_VALLEY						//Tr Karanlık Plato Zindanı
#define ENABLE_BALATHOR_DUNGEON						//Tr Lonca Balathor Zindanı
#define ENABLE_SLOT_TEXT_SIZE						//Tr Yazı Büyütme Sistemi
#define ENABLE_MYSTERY_DUNGEON						//Tr Gizemli Yankı Zindan/Bonus
#define ENABLE_ITEM_ATTR_COSTUME					//Tr Kostüm Efsun Ekleme Sistemi
#define ENABLE_SUNGMA_PREMIUM_BUFF					//Tr Sungma Premium Buff Sistemi
#define ENABLE_SELECT_REWARD_BOX					//Tr Seçmeli Sandık Sistemi
#define ENABLE_NEWWORLD_LEVEL						//Tr Nesne İçin Yeni Level
#define ENABLE_TITLE_SYSTEM							//Tr Ünvan Sistemi
#define ENABLE_INPUT_CANCEL							//Tr Quest Giriş Ayarı
#define ENABLE_QUEST_WIDTH_EXPANSION				//Tr Quest Güncellemesi
#define ENABLE_GPU_CONFIG							//Tr Ekran Kartı Optimizasyonu
#define ENABLE_BUTTON_TOOLTIP_RENEWAL				//Tr Buton Tooltip Güncellemesi
#define ENABLE_ANIMATION_OPTIMIZATION				//Tr Efekt Optimizasyon Sistemi
#define ENABLE_CLIENT_TIMER							//Tr Zindan Süre Sistemi
#define ENABLE_SUNG_MAHI_TOWER						//Tr Cehennem Kulesi Sistemi
#define ENABLE_WHITE_DRAGON							//Tr Beyaz Ejderha Zindanı
#define ENABLE_MOB_HP_EXTENDED						//Tr Mob HP Sınır Güncellemesi
#define ENABLE_VNUM_EXTENDED						//Tr Vnum Yükseltme Güncellemesi
#define ENABLE_BATTLE_ROYALE						//Tr Kraliyet Savaşı Sistemi
#define ENABLE_USER_REPORT_SYSTEM 					//Tr Oyuncu Raporlama Sistemi
#define ENABLE_SAVE_CAMERA_MODE						//Tr Kamera Modu Kayıt Sistemi
#define ENABLE_EXTENDED_AFFECT_FLAG					//Tr Efekt Sınırı Genişletmesi 64>96
#define ENABLE_BELT_INVENTORY_RENEWAL				//Tr Kemer Envanteri Güncellemesi
#define ENABLE_OFFICAL_FEATURES						//Tr Skill Özellik Sistemi
#define ENABLE_SET_ITEM								//Tr Set İtem Sistemi
#define ENABLE_NEW_GAMEOPTION						//Tr Yeni Sistem Seçenekleri Sistemi
#define ENABLE_LEFT_SEAT							//Tr AFK Sistemi
#define ENABLE_GROWTH_PET_SYSTEM					//Tr Levelli Pet Sistemi
#define ENABLE_ITEM_APPLY4							//Tr ITEM_APPLY(4) Genişletme Sistemi
#define ENABLE_RIDING_EXTENDED						//Tr Yeni Binicilik Sistemi
#define ENABLE_ADDITIONAL_EQUIPMENT_PAGE			//Tr Yeni Envanter Görünümü
#define ENABLE_PRECISION							//Tr Tamlık Bonus Sistemi
#define ENABLE_DRAGON_SOUL_7_SLOT					//Tr Ametist Simya Sistemi
#define ENABLE_MOVE_RING_SYSTEM						//Tr Işınlanma Yüzüğü Sistemi
#define ENABLE_ELEMENTAL_WORLD						//Tr Elementer Haritası Sistemi
#define ENABLE_ELEMENT_ALL_BONUSES					//Tr Element Gücü Sistemi
#define ENABLE_UNBLOCK_SYSTEM						//Tr Serbest Bırak Sistemi
#define ENABLE_REFINE_ELEMENT						//Tr Element Sistemi
#define ENABLE_GUILD_DONATE_SYSTEM					//Tr Lonca Puan Sistemi
#define ENABLE_PASSIVE_SYSTEM						//Tr Kalıntı Bonusu Sistemi
#define ENABLE_CONQUEROR_LEVEL						//Tr Yohara Sistemi
#define ENABLE_GLOVE_SYSTEM							//Tr Güç Eldiveni Sistemi
#define ENABLE_FOG_FIX								//Tr Sis Kapatma Sistemi
#define ENABLE_QUEST_RENEWAL						//Tr Görev Kategori Sistemi
#define ENABLE_DETAILS_UI							//Tr Efsun Detayları Sistemi
#define ENABLE_KEYBOARD_SETTINGS_SYSTEM				//Tr Klavye Sistemi
#define ENABLE_TAB_NEXT_TARGET						//Tr Bir Sonraki Slot Sistemi
#define ENABLE_NINJA_MAP_FIX						//Tr Ninja MiniMap Hide Sistemi
#define ENABLE_OX_RENEWAL							//Tr Ox Mesaj Sistemi
#define ENABLE_DICE_SYSTEM							//Tr Zar Sistemi
#define ENABLE_PENDANT								//Tr Tılsım Sistemi
#define ENABLE_COSTUME_SYSTEM						//Tr Kostüm Sistemi
#define ENABLE_ENERGY_SYSTEM						//Tr Enerji Sistemi
#define ENABLE_SOUL_SYSTEM							//Tr Rüya Ruhu Sistemi
#define ENABLE_DRAGON_SOUL_SYSTEM					//Tr Simya Sistemi
#define ENABLE_NEW_EQUIPMENT_SYSTEM					//Tr Kemer Envanteri
#define ENABLE_SASH_SYSTEM							//Tr Kuşak Güncellemesi
#define ENABLE_HIGHLIGHT_SYSTEM						//Tr Yeni Eşya Efekti
#define ENABLE_ANTI_RESIST_MAGIC_BONUS_SYSTEM		//Tr Yeni Efsun
#define ENABLE_AURA_SYSTEM							//Tr Aura Sistemi
#define ENABLE_CHEQUE_SYSTEM						//Tr Yeni Para Birimi Won
#define ENABLE_MOUNT_COSTUME_SYSTEM					//Tr Kostüm Binek Sistemi
#define ENABLE_DS_GRADE_MYTH						//Tr Mitsi Simya Güncellemesi
#define ENABLE_DS_SET								//Tr Simya Set Bonus Güncellemesi
#define ENABLE_NEW_ARROW_SYSTEM						//Tr Zırh Güncellemesi
#define ENABLE_COSTUME_WEAPON_SYSTEM				//Tr Kostüm Silah Sistemi
#define ENABLE_7AND8TH_SKILLS						//Tr 7-8 Skill Güncellemesi
#define ENABLE_NEW_GYEONGGONG_SKILL					//Tr Ninja Skill Güncellemesi
#define ENABLE_AGGREGATE_MONSTER_EFFECT				//Tr Pelerin Efekti
#define ENABLE_CHANGELOOK_SYSTEM					//Tr Yansıtma Sistemi
#define ENABLE_MOVE_CHANNEL							//Tr Kanal Değiştirme Sistemi
#define ENABLE_OFFICAL_CHARACTER_SCREEN				//Tr Karakter Seçme Ekranı
#define ENABLE_MESSENGER_BLOCK						//Tr Engelleme Sistemi
#define ENABLE_MESSENGER_RENEWAL					//Tr Gelişmiş Mesaj Sistemi
#ifdef ENABLE_MESSENGER_RENEWAL
	#define ENABLE_COMMUNITY_GUILD_RENEWAL			//Tr Topluluk lonca sekmesi uye listesi / konum
	#define ENABLE_MESSENGER_FAVORITE				//Tr Topluluk favori ekleme
#endif
#define ENABLE_SAFEBOX_EX_SYSTEM					//Tr 5'li Oyuncu Deposu
#define ENABLE_VIEW_EQUIPMENT_SYSTEM				//Tr Profil Görüntüleme
#define ENABLE_BOOK_TRADE_SYSTEM					//Tr Beceri Kitabı Takas Sistemi
#define ENABLE_MOUNT_CHANGELOOK_SYSTEM				//Tr Binek Dönüşüm Güncellemesi
#define ENABLE_TRADABLE_ICON						//Tr Satılamayan Nesne Efekti
#define ENABLE_ELEMENT_TARGET						//Tr Element Sistemi
#define ENABLE_ELEMENT_NEW_BONUSES					//Tr Tılsım Efsunları
#define ENABLE_ATTR_6TH_7TH_SYSTEM					//Tr 6/7 Efsun Ekleme
#define ENABLE_CUBE_RENEWAL							//Tr Yeni Arındırma Sistemi
#define ENABLE_NEW_EMOTION							//Tr Yeni Emoji Sistemi
#define ENABLE_EXPRESSING_EMOTION					//Tr Yeni Emojiler
#define ENABLE_SKILLBOOK_COMB_SYSTEM				//Tr Beceri Kitap Takas Sistemi
#define ENABLE_GAYA_SYSTEM							//Tr Gaya Sistemi
#define ENABLE_FISHING_RENEWAL						//Tr Balık Yakalama Sistemi
#define ENABLE_SHADOW_RENDER_QUALITY_OPTION			//Tr Gölge Kalitesi Sistemi
#define ENABLE_LOADING_TIP_INFO						//Tr Loading İnfo Sistemi
#define ENABLE_SOULBIND_SYSTEM						//Tr Nesne Ruha Bağlama Sistemi
#define ENABLE_PET_ATTR_DETERMINE					//Tr Yeni Pet Güncellemesi
#define ENABLE_EXTEND_INVEN_SYSTEM					//Tr Tipi Envanter Genişletme Sistemi
#define ENABLE_SHOW_MOB_INFO						//Tr Mob Seviye-Agresif Güncellemesi
#define ENABLE_MAILBOX_SYSTEM						//Tr Mail Kutusu Sistemi
#define ENABLE_CHAT_SETTINGS						//Tr Yazışma Sistemi
#define ENABLE_CHAT_SETTINGS_EXTEND					//Tr Yazışma Pencere Sistemi
#define ENABLE_BATTLE_FIELD							//Tr Savaş Bölgesi
#define ENABLE_DEFENSE_WAVE							//Tr Gemi Savunması
#ifdef ENABLE_DEFENSE_WAVE
	#define ENABLE_HYDRA_MOVE_RING					//Tr Hidra'nın Yüzüğü Sistemi
#endif
#define ENABLE_12ZI									//Tr Zodyak Tapınağı
#define ENABLE_OFFICIAL_19_3_ITEMS					//Tr Yeni +15 Nesneler
#define ENABLE_FAST_ATTACH_ITEMS_SYSTEM				//Tr Depo Hızlı Alma
#define ENABLE_ITEM_CHECKINOUT_UPDATE				//Tr Sağ Tık Ticaret Güncellemesi
#define ENABLE_DS_CHANGE_ATTR						//Tr Simya Efsunlama Güncellemesi
#define ENABLE_WOLFMAN_CHARACTER					//Tr Lycan Karakteri
#define ENABLE_BUTTON_FLASH							//Tr Flash Buton Sistemi
#define ENABLE_LUCKY_BOX							//Tr Şans Kutusu Sistemi
#define ENABLE_REFINE_FAIL_TYPE						//Tr Genişletilmiş Arındırma Mesajı
#define ENABLE_ITEM_TYPE_GACHA						//Tr Yeni Sandık Tipi Sistemi
#define ENABLE_QUEEN_NETHIS							//Tr Yılan Zindanı
#define VERSION_162_ENABLED							//Tr Büyülü Orman Tapınağı
#define ENABLE_MELEY_LAIR_DUNGEON					//Tr Meley Zindanı Sistemi
#ifdef ENABLE_MELEY_LAIR_DUNGEON
	#define ENABLE_MELEY_LAIR_DUNGEON_PARTY			//Tr Meley Zindanı Grup Güncellemesi
	#define MELEY_LAIR_DUNGEON_STATUE 6118			//Tr Meley Zindanı Kule Kodu
#endif
#define ENABLE_STANDING_MOUNT						//Tr Yeni Binek Sistemi
#ifdef ENABLE_STANDING_MOUNT
	#define SURFBOARD 20280							//Tr Sörf Tahtası mob_proto Kodu
	#define WUKONG_1 20281							//Tr Wukong'un Fırtınası mob_proto Kodu
	#define WUKONG_2 20282							//Tr Wukong'un Gürlemesi mob_proto Kodu
	#define DRAKKAR_1 20283							//Tr Mavi kadırga mob_proto Kodu
	#define DRAKKAR_2 20284							//Tr Kırmızı kadırga mob_proto Kodu
#endif
#define ENABLE_AUTO_SYSTEM							//Tr Otomatik Av Sistemi
#ifdef ENABLE_AUTO_SYSTEM
	#define ENABLE_AUTO_RESTART_EVENT
	#define ENABLE_IMPROVED_AUTOMATIC_HUNTING_SYSTEM
	#define EVENT_HANDLER_MASTER
#endif
/*** YMIR System End ***/

/*** Offline Shop System ***/
#define ENABLE_OFFLINESHOP_SYSTEM					//Premium Çevrimdışı Pazar
#ifdef ENABLE_OFFLINESHOP_SYSTEM
	#define ENABLE_SHOP_SEARCH_SYSTEM				//Premium Çevrimdışı Pazar Arama Camı
#endif
/*** Offline Shop System End ***/

/*** Enable System ***/
#define ENABLE_VOTE4BUFF							//Vote Sistemi
#define ENABLE_NEW_RING_EQUIPMENT					//Yeni Yüzük Slotları
#define ENABLE_DAMAGE_LIMIT_RENEWAL					//Damage limit yükseltmesi(long long)
#define ENABLE_MULTI_FARM_BLOCK						//Farm Engel Sistemi
#define ENABLE_COSTUME_SET_ITEM						//Kostüm Set Bonus Sistemi
#define ENABLE_BOSS_DEDECTOR_SYSTEM					//Patron Dedektör Sistemi
#if !defined(__ANDROID__) && !defined(__APPLE__)
#define CEF_BROWSER									//CEF Browser Modülü
#endif
#define ENABLE_USE_CLIP_MASK 						//Pencere SCROLL Modülü
#define ENABLE_BATTLEPASS							//Savaş Görevleri Sistemi
#define ENABLE_TELEPORT_SCROLL_IMAGE				//Teleport Scroll Image Sistemi
#define ENABLE_MANUEL_SWITCH_CHANGE					//Manuel Bonus Değiştirme Sistemi
#define ENABLE_MULTI_TEXTLINE						//Çoklu Yazı Sistemi
#define ENABLE_AUTO_QUQUE_ATTACK					//Metin Taşı İşaretleme Sistemi
#define ENABLE_GIFTBOX_MULTI_OPEN					//Hızlı Sandık Açma Sistemi
#ifdef ENABLE_GIFTBOX_MULTI_OPEN
#define ICOUNT unsigned short						//ICOUNT=>unsigned short Değiştirme Modülü
#endif
#define ENABLE_REMOTE_SHOP							//Uzaktan Npc Sistemi
#define ENABLE_FAST_ITEM_DELETE_SYSTEM				//Hızlı Nesne Silme Sistemi
#define ENABLE_MINIMAP_SMOOTH_ZOOM					//MiniMap Hareketli Yakınlaştırma Sistemi
#define ENABLE_FATE_ROULETTE_SYSTEM					//Kader Çarkı Sistemi
#define ENABLE_STONE_BOSS_BONUS						//Metinlere ve Patronlara Güçlü Efsun
#define ENABLE_HYPERLINK_ITEM_ICON					//Sohbette Nesne Yansıtma Resmi
#define ENABLE_CHAT_STOP_SYSTEM						//Sohbet Durdurma Sistemi
#define ENABLE_RENDER_TARGET						//Mob Önizleme Sistemi
#define ENABLE_ATTRACT_RANGER_SYSYTEM				//Okçuları Yanına Çekme Sistemi
#if !defined(__ANDROID__) && !defined(__APPLE__)
#define ENABLE_TRANSLATOR_GOOGLE_SYSTEM				//Otomatik Çeviri Sistemi
#endif

#define ENABLE_KILL_STATISTICS						//Oyuncu İstatistik Sistemi
#define ENABLE_COINS_SYSTEM							//Ep Sistemi
#define ENABLE_SALES_SYSTEM							//Fırsatı Yakala Sistemi
#define ENABLE_REFINE_ABILITY						//Nesne Geliştirme Sistemi
#define ENABLE_ITEM_DROP_RENEWAL					//Yere Düşen İtemlerin İsmini Görme Sistemi
#define ENABLE_ATLAS_WARPING						//GM Map Işınlanma Sistemi
#define ENABLE_FIX_D_YMIR_WORK						//Ymir Work Engel Sistemi
#define ENABLE_PETS_WITHOUT_COLLISIONS				//Petlerin İçinden Geçme Sistemi
#define ENABLE_MOUNTS_WITHOUT_COLLISIONS			//Bineklerin İçinden Geçme Sistemi
#define ENABLE_SHAMAN_AND_LYCAN_DAMAGE_FIX			//Saman ve Lycan Damage Ayarı
#define ENABLE_AUTO_CHAT_SYSTEM						//Otomatik Bağırma Sistemi
#define ENABLE_DAMAGE_BAR							//Hasar Ayırma Sistemi
#define ENABLE_EXTENDED_ITEM_COUNT					//İtem Sınırı Sistemi
#define ENABLE_FOV_OPTION							//Görüntüleme Açısı Sistemi
#define ENABLE_DRAGON_SOUL_EFFECT					//Simya Slot Efekti Sistemi
#define ENABLE_NEW_USER_CARE						//Kullanıcı Seçeneklerini Değiştirme
#define ENABLE_TEXT_LEVEL_REFRESH					//Level Yenileme Sistemi
#define ENABLE_OBJ_SCALLING							//Obje Görüntü Ayar Sistemi
#define ENABLE_EQUIPMENT_AFFECT						//Ekipman Efekti Sistemi
#define ENABLE_STATUS_UP_RENEWAL					//Hızlı Statü Sistemi
#define ENABLE_SKILL_SELECT_SYSTEM					//Skill Seçme Sistemi
#define ENABLE_SWITCHBOT							//Efsun Botu
#define ENABLE_PACKET_INFO_SYSTEM					//Bildirim Sistemi
#define ENABLE_EXTENDED_PET_SYSTEM					//Kostüm Pet Sistemi
#define ENABLE_INVENTORY_ADDITION					//Envanter Yanı Butonlar
#define ENABLE_TEXT_IMAGE_LINE						//Yazı Emoji Sistemi
#define ENABLE_INDEX_FILE							//Pack Index Modulü
#define ENABLE_ETER_PACK_OBSCURING					//Yeni Pack Koruması
#define ENABLE_BOSS_EFFECT_SYSTEM					//Patron Efekt Sistemi
#define ENABLE_SUPPORT_SYSTEM						//Yarcımcı Şaman Sistemi
#define ENABLE_EXTENDING_ITEM_BUFF_TIME				//Şaman Buff İtem Süre Uzatma Sistemi
#define ENABLE_WON_EXCHANGE_WINDOW					//Won-Yang Bozdurma Sistemi
#define ENABLE_SPIRIT_STONE_READING					//Ruh Taşı Okuma Sistemi
#define ENABLE_SKILL_BOOK_READING					//Beceri Kitabı Okuma Sistemi
#define ENABLE_BIOLOG_SYSTEM						//Biyolog Sistemi
#define ENABLE_MAX_RED_BUFF_EFFECT					//Maksimum Kırmızı Buff Efekti
#define ENABLE_FPSTIME_SYSTEM						//Fps Gösterme Sistemi
#define ENABLE_TIME_SYSTEM							//Saat Gösterme Sistemi
#define ENABLE_CHEQUE_DESK_SYSTEM					//Won Masası Sistemi
#define ENABLE_STONE_POINT_SYSTEM					//Metin Puanı Sistemi
#define ENABLE_TYPE_SHOPEX_SYSTEM					//Çok Fonksiyonlu Market Sistemi
#define ENABLE_CHAT_STACK							//Mesaj Kaydetme Sistemi
#define ENABLE_ATLASINFO_FROM_ROOT					//AtlasInfo Root'dan Çektirme
#define ENABLE_CHEQUE_COUPON_SYSTEM					//Won Çeki Sistemi
#define ENABLE_GAYA_TICKET_SYSTEM					//Gaya Çeki Sistemi
#define ENABLE_BLOCK_EXP							//Deneyim Bloklama Sistemi
#define ENABLE_NEW_CHAT_VIEW						//Konuşmada Ülke Bayrağı
#define ENABLE_LINK_IN_CHAT							//Konusmada Link Paylaşma
#define ENABLE_HIDE_COSTUME_SYSTEM					//Kostüm Gizleme Sistemi
#define ENABLE_POISON_GAUGE_SYSTEM					//Yaratık Zehir Göstergesi
#define ENABLE_HEALTH_PERCENT_SYSTEM				//Canı Yüzdeli Şekilde Görme Sistemi
#define ENABLE_VIEW_TARGET_DECIMAL_HP				//Yaratığın Canını Yüzdeli Şekilde Görme
#ifdef ENABLE_VIEW_TARGET_DECIMAL_HP
	#define ENABLE_VIEW_TARGET_PLAYER_HP			//Karakterin Canını Yüzdeli Şekilde Görme
#endif
#define ENABLE_MOB_APPEARANCE						//Mob Görünüm Sistemi
#define ENABLE_CRITICAL_VALUE						//Kritik Vuruş Renklendirme
#define ENABLE_DUNGEON_INFO_SYSTEM					//Zindan Bilgi Sistemi
#define ENABLE_SHOW_CHEST_DROP						//Sandık İçgörü Sistemi
#define ENABLE_INSTANT_PICKUP_SYSTEM				//Hızlı Nesne Toplama
#define ENABLE_AUTO_PICKUP_SYSTEM					//Otomatik Nesne Toplama
#define ENABLE_REFINE_RENEWAL						//Hızlı Yükseltme Sistemi
#define ENABLE_SPLIT_INVENTORY_SYSTEM				//Ek Envanter Sistemi
#define ENABLE_INVENTORY_SHOW						//Envanter İle Ek Envanter Açma
#define ENABLE_NEW_EXCHANGE_WINDOW					//Yeni Pencere Sistemi
#define ENABLE_MOBLARA_YUZDE						//Moblarda Yüzde Gösterimi
#define ENABLE_WINDOW_MESSAGE						//Mesaj Geldiğinde Pencere Uyarı
#define ENABLE_OX_INVISIBILITY_SYSTEM				//Ox Görünmezlik Sistemi
#define ENABLE_BUY_WITH_ITEM						//Nesne İle Eşya Alma Sistemi
#define ENABLE_NEW_AFFECT_POTION					//Yeni İksir Türleri
#define ENABLE_ADMIN_BAN_MANAGER					//Admin Ban Sistemi
#define ENABLE_WHISPER_ADMIN_SYSTEM					//Admin Mesaj Sistemi
#define ENABLE_TARGET_INFORMATION_SYSTEM			//Mob Düşen Nesneleri Görme Sistemi
#define ENABLE_AFFECT_POLYMORPH_REMOVE				//Dönüşümden Çıkma Sistemi
#define ENABLE_MINIMAP_WHITEMARK_NEW				//Yuvarlak Mini Map Görseli
#define ENABLE_SAFEZONE_COLLISION					//Npc İçinden Geçme
#define ENABLE_SHOP_COLLISION						//Market  İçinden Geçme
#define ENABLE_HORSE_COLLISION						//Binek/At  İçinden Geçme
#define ENABLE_OX_PLAYER_COLLISION					//OX Map İçinden Geçme
#define ENABLE_IMPROVED_LOGOUT_POINTS				//Karakter Ekranı Yenileme
#define ENABLE_DISABLE_SOFTWARE_TILING				//Tr Tiling İptal Güncellemesi
#define ENABLE_ENVIRONMENT_EFFECT_OPTION			//Tr Efekt Gizleme Sistemi
#define ENABLE_EFFECT_CAMERA_VIEW_FIX				//Efekt Görüntü Düzenlemesi
#define ENABLE_GRAPHIC_ON_OFF						//Tr Grafik Ayar Güncellemesi
#define ENABLE_MULTI_LANGUAGE_SYSTEM				//Çoklu Dil Sistemi
#ifdef ENABLE_MULTI_LANGUAGE_SYSTEM
	#define ENABLE_MULTI_LANGUAGE_WHISPER_DETAILS	//Çoklu Dil Mesaj Göstergesi
#endif
#define ENABLE_MOUSEWHEEL_EVENT						//Mause Döndürme Sistemi
#define ENABLE_PREMIUM_AFFECT_SYSTEM				//Premium Efekt Engel Sistemi
#define ENABLE_SEQUENCE_SYSTEM						//Sequence İptali
#define ENABLE_INGAME_WIKI							//Oyun İçi Wiki Sistemi
/*** Enable System End ***/

/*** Event System ***/
#define ENABLE_TREASURE_EVENT						//Tr Goblin Etkinliği
#define ENABLE_FLOWER_EVENT							//Tr Çiçek Çocukları Etkinliği
#define ENABLE_MINI_GAME_OKEY						//Tr Okey Kart Etkinliği
#define ENABLE_EVENT_BANNER_FLAG					//Tr Event Bayrak Sistemi
#define ENABLE_SOUL_ROULETTE_SYSTEM					//Tr Kan Ritüeli Etkinliği
#define ENABLE_EVENT_SYSTEM							//Tr Event Sistemi
#define ENABLE_FISH_EVENT_SYSTEM					//Tr Balık Etkinliği
#define ENABLE_MINI_GAME							//Tr Mini Oyunlar
#define ENABLE_MINI_GAME_CATCH_KING					//Tr Kralı Yakala Etkinliği
#define ENABLE_WORD_GAME_EVENT						//Kelime Etkinliği
#define ENABLE_AUTO_EVENTS							//Otomatik Etkinlikler
/*** Event System End ***/

/*** Performance Fix System ***/
#define YMIR_WORK_FPS_DROP_FIX						//Pack Log Fix
#define ENABLE_FIX_MOBS_LAG							//Mob Lag Fix Sistemi
#if defined(ENABLE_FIX_MOBS_LAG)
#define FIX_MOBS_LAG_FIX							//Stream Lag Fix
#endif
#define ENABLE_MAP_PERFORMANCE						//Dosya Yazma Fix + Map Yüklenme Lag + Map Performans Fix
#define ENABLE_MINIMIZED_EFFECT_FIX					//Client Minimize Efekt Fix
#define ENABLE_FIX_EFFECT_MOVEMENT					//Karakter Hareket Görüntü Fix
#define ENABLE_DAMAGE_QUEUE_FIX						//Damage Birikme Fix
/*** Performance Fix System End ***/

/*** AntiCheat System ***/
#define ENABLE_SVSIDE_ANTI_CHEAT					/SvSide Anti Cheat
/*** AntiCheat System End ***/

/*** Debug  System ***/
#define ENABLE_PACKET_DESYNC_LOG					//Paket Kayması & Desync Debug Log Sistemi
/*** Debug  System End ***/