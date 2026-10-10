#include "StdAfx.h"
#include "GL_Loader.h"
#include "../EterBase/Debug.h"

#if defined(USE_OPENGL_ES)

#if defined(__ANDROID__) || defined(__APPLE__)
bool InitGLLoader()
{
	const char* szVersion = (const char*)glGetString(GL_VERSION);
	const char* szRenderer = (const char*)glGetString(GL_RENDERER);
	const char* szVendor = (const char*)glGetString(GL_VENDOR);
	const char* szGLSL = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	// OpenGL ES initialized quietly
	return true;
}
#else
// Define all function pointers
#define X(type, name) type gles_##name = nullptr;
GLES_FUNCTIONS
#undef X

bool InitGLLoader()
{
	int loadedCount = 0;
	int totalCount = 0;

#define X(type, name) \
	gles_##name = (type)SDL_GL_GetProcAddress(#name); \
	totalCount++; \
	if (gles_##name) { \
		loadedCount++; \
	} else { \
		TraceError("OpenGL ES: Failed to load %s", #name); \
	}
	GLES_FUNCTIONS
#undef X

	const char* szVersion = (const char*)glGetString(GL_VERSION);
	const char* szRenderer = (const char*)glGetString(GL_RENDERER);
	const char* szVendor = (const char*)glGetString(GL_VENDOR);
	const char* szGLSL = (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

	TraceError("========================================");
	TraceError(" [OpenGL ES 3.0] Initialized Successfully");
	TraceError(" Loaded functions: %d / %d", loadedCount, totalCount);
	TraceError(" GL_VERSION:  %s", szVersion ? szVersion : "UNKNOWN");
	TraceError(" GL_RENDERER: %s", szRenderer ? szRenderer : "UNKNOWN");
	TraceError(" GL_VENDOR:   %s", szVendor ? szVendor : "UNKNOWN");
	TraceError(" GL_GLSL:     %s", szGLSL ? szGLSL : "UNKNOWN");
	TraceError("========================================");

	return (loadedCount > 0);
}
#endif // !__ANDROID__ && !__APPLE__

#endif // USE_OPENGL_ES
