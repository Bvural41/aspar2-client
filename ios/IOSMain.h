#pragma once

#ifdef __cplusplus
extern "C" {
#endif

void IOS_Init(const char* bundlePath, const char* docsPath, int screenWidth, int screenHeight);
void IOS_Render();
void IOS_ActionTouch(int action, float x, float y, float deltaX, float deltaY, bool isDrag);
void IOS_JoystickTouch(int action, float x, float y);
void IOS_SecondaryTouch(int action, float x, float y, float deltaX, float deltaY);
void IOS_OnKeyboardText(const char* text);
void IOS_OnKeyboardEnter();

void IOS_ShowKeyboard(const char* initialText);
void IOS_HideKeyboard();

void IOS_ShowWebPage(const char* url);
void IOS_HideWebPage();
bool IOS_IsWebShowing();

bool IOS_IsGamePhase();
void IOS_PauseAudio();
void IOS_ResumeAudio();
void IOS_SaveConfig();

#ifdef __cplusplus
}
#endif
