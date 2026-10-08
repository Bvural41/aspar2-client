#pragma once

#if defined(USE_OPENGL_ES)

#ifdef __ANDROID__
#include <GLES3/gl3.h>
#include <GLES3/gl3ext.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_opengles2.h>
#endif

// GLES 3.0 Specific Definitions & Types
#ifndef GL_DEPTH24_STENCIL8
#define GL_DEPTH24_STENCIL8 0x88F0
#endif

#ifndef GL_HALF_FLOAT
#define GL_HALF_FLOAT 0x140B
#endif

#ifndef GL_RGBA8
#define GL_RGBA8 0x8058
#endif

#ifndef GL_SHADING_LANGUAGE_VERSION
#define GL_SHADING_LANGUAGE_VERSION 0x8B8C
#endif

// VAO Types for GLES 3.0
typedef void (GL_APIENTRYP PFNGLGENVERTEXARRAYSPROC) (GLsizei n, GLuint *arrays);
typedef void (GL_APIENTRYP PFNGLBINDVERTEXARRAYPROC) (GLuint array);
typedef void (GL_APIENTRYP PFNGLDELETEVERTEXARRAYSPROC) (GLsizei n, const GLuint *arrays);
typedef GLboolean (GL_APIENTRYP PFNGLISVERTEXARRAYPROC) (GLuint array);

// Function Pointer List (X-Macro)
#define GLES_FUNCTIONS \
	X(PFNGLACTIVETEXTUREPROC, glActiveTexture) \
	X(PFNGLATTACHSHADERPROC, glAttachShader) \
	X(PFNGLBINDBUFFERPROC, glBindBuffer) \
	X(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) \
	X(PFNGLBINDRENDERBUFFERPROC, glBindRenderbuffer) \
	X(PFNGLBINDTEXTUREPROC, glBindTexture) \
	X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
	X(PFNGLBLENDEQUATIONPROC, glBlendEquation) \
	X(PFNGLBLENDEQUATIONSEPARATEPROC, glBlendEquationSeparate) \
	X(PFNGLBLENDFUNCPROC, glBlendFunc) \
	X(PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate) \
	X(PFNGLBUFFERDATAPROC, glBufferData) \
	X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
	X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) \
	X(PFNGLCLEARPROC, glClear) \
	X(PFNGLCLEARCOLORPROC, glClearColor) \
	X(PFNGLCLEARDEPTHFPROC, glClearDepthf) \
	X(PFNGLCLEARSTENCILPROC, glClearStencil) \
	X(PFNGLCOLORMASKPROC, glColorMask) \
	X(PFNGLCOMPILESHADERPROC, glCompileShader) \
	X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
	X(PFNGLCREATESHADERPROC, glCreateShader) \
	X(PFNGLCULLFACEPROC, glCullFace) \
	X(PFNGLDELETEBUFFERSPROC, glDeleteBuffers) \
	X(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers) \
	X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
	X(PFNGLDELETERENDERBUFFERSPROC, glDeleteRenderbuffers) \
	X(PFNGLDELETESHADERPROC, glDeleteShader) \
	X(PFNGLDELETETEXTURESPROC, glDeleteTextures) \
	X(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays) \
	X(PFNGLDEPTHFUNCPROC, glDepthFunc) \
	X(PFNGLDEPTHMASKPROC, glDepthMask) \
	X(PFNGLDETACHSHADERPROC, glDetachShader) \
	X(PFNGLDISABLEPROC, glDisable) \
	X(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray) \
	X(PFNGLDRAWARRAYSPROC, glDrawArrays) \
	X(PFNGLDRAWELEMENTSPROC, glDrawElements) \
	X(PFNGLENABLEPROC, glEnable) \
	X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
	X(PFNGLFINISHPROC, glFinish) \
	X(PFNGLFLUSHPROC, glFlush) \
	X(PFNGLFRAMEBUFFERRENDERBUFFERPROC, glFramebufferRenderbuffer) \
	X(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) \
	X(PFNGLFRONTFACEPROC, glFrontFace) \
	X(PFNGLGENBUFFERSPROC, glGenBuffers) \
	X(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) \
	X(PFNGLGENRENDERBUFFERSPROC, glGenRenderbuffers) \
	X(PFNGLGENTEXTURESPROC, glGenTextures) \
	X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
	X(PFNGLGENERATEMIPMAPPROC, glGenerateMipmap) \
	X(PFNGLGETATTRIBLOCATIONPROC, glGetAttribLocation) \
	X(PFNGLGETBOOLEANVPROC, glGetBooleanv) \
	X(PFNGLGETERRORPROC, glGetError) \
	X(PFNGLGETFLOATVPROC, glGetFloatv) \
	X(PFNGLGETINTEGERVPROC, glGetIntegerv) \
	X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
	X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
	X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
	X(PFNGLGETSHADERSOURCEPROC, glGetShaderSource) \
	X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
	X(PFNGLGETSTRINGPROC, glGetString) \
	X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
	X(PFNGLISENABLEDPROC, glIsEnabled) \
	X(PFNGLLINEWIDTHPROC, glLineWidth) \
	X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
	X(PFNGLPIXELSTOREIPROC, glPixelStorei) \
	X(PFNGLPOLYGONOFFSETPROC, glPolygonOffset) \
	X(PFNGLREADPIXELSPROC, glReadPixels) \
	X(PFNGLRENDERBUFFERSTORAGEPROC, glRenderbufferStorage) \
	X(PFNGLSCISSORPROC, glScissor) \
	X(PFNGLSHADERSOURCEPROC, glShaderSource) \
	X(PFNGLSTENCILFUNCPROC, glStencilFunc) \
	X(PFNGLSTENCILMASKPROC, glStencilMask) \
	X(PFNGLSTENCILOPPROC, glStencilOp) \
	X(PFNGLTEXIMAGE2DPROC, glTexImage2D) \
	X(PFNGLTEXPARAMETERFPROC, glTexParameterf) \
	X(PFNGLTEXPARAMETERIPROC, glTexParameteri) \
	X(PFNGLTEXSUBIMAGE2DPROC, glTexSubImage2D) \
	X(PFNGLUNIFORM1FPROC, glUniform1f) \
	X(PFNGLUNIFORM1FVPROC, glUniform1fv) \
	X(PFNGLUNIFORM1IPROC, glUniform1i) \
	X(PFNGLUNIFORM1IVPROC, glUniform1iv) \
	X(PFNGLUNIFORM2FPROC, glUniform2f) \
	X(PFNGLUNIFORM2FVPROC, glUniform2fv) \
	X(PFNGLUNIFORM2IPROC, glUniform2i) \
	X(PFNGLUNIFORM2IVPROC, glUniform2iv) \
	X(PFNGLUNIFORM3FPROC, glUniform3f) \
	X(PFNGLUNIFORM3FVPROC, glUniform3fv) \
	X(PFNGLUNIFORM3IPROC, glUniform3i) \
	X(PFNGLUNIFORM3IVPROC, glUniform3iv) \
	X(PFNGLUNIFORM4FPROC, glUniform4f) \
	X(PFNGLUNIFORM4FVPROC, glUniform4fv) \
	X(PFNGLUNIFORM4IPROC, glUniform4i) \
	X(PFNGLUNIFORM4IVPROC, glUniform4iv) \
	X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv) \
	X(PFNGLUSEPROGRAMPROC, glUseProgram) \
	X(PFNGLVERTEXATTRIB4FPROC, glVertexAttrib4f) \
	X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer) \
	X(PFNGLVIEWPORTPROC, glViewport)

#ifndef __ANDROID__
// Declare extern pointers
#define X(type, name) extern type gles_##name;
GLES_FUNCTIONS
#undef X

// Transparent alias defines so standard gl* calls invoke function pointers
#define glActiveTexture gles_glActiveTexture
#define glAttachShader gles_glAttachShader
#define glBindBuffer gles_glBindBuffer
#define glBindFramebuffer gles_glBindFramebuffer
#define glBindRenderbuffer gles_glBindRenderbuffer
#define glBindTexture gles_glBindTexture
#define glBindVertexArray gles_glBindVertexArray
#define glBlendEquation gles_glBlendEquation
#define glBlendEquationSeparate gles_glBlendEquationSeparate
#define glBlendFunc gles_glBlendFunc
#define glBlendFuncSeparate gles_glBlendFuncSeparate
#define glBufferData gles_glBufferData
#define glBufferSubData gles_glBufferSubData
#define glCheckFramebufferStatus gles_glCheckFramebufferStatus
#define glClear gles_glClear
#define glClearColor gles_glClearColor
#define glClearDepthf gles_glClearDepthf
#define glClearStencil gles_glClearStencil
#define glColorMask gles_glColorMask
#define glCompileShader gles_glCompileShader
#define glCreateProgram gles_glCreateProgram
#define glCreateShader gles_glCreateShader
#define glCullFace gles_glCullFace
#define glDeleteBuffers gles_glDeleteBuffers
#define glDeleteFramebuffers gles_glDeleteFramebuffers
#define glDeleteProgram gles_glDeleteProgram
#define glDeleteRenderbuffers gles_glDeleteRenderbuffers
#define glDeleteShader gles_glDeleteShader
#define glDeleteTextures gles_glDeleteTextures
#define glDeleteVertexArrays gles_glDeleteVertexArrays
#define glDepthFunc gles_glDepthFunc
#define glDepthMask gles_glDepthMask
#define glDetachShader gles_glDetachShader
#define glDisable gles_glDisable
#define glDisableVertexAttribArray gles_glDisableVertexAttribArray
#define glDrawArrays gles_glDrawArrays
#define glDrawElements gles_glDrawElements
#define glEnable gles_glEnable
#define glEnableVertexAttribArray gles_glEnableVertexAttribArray
#define glFinish gles_glFinish
#define glFlush gles_glFlush
#define glFramebufferRenderbuffer gles_glFramebufferRenderbuffer
#define glFramebufferTexture2D gles_glFramebufferTexture2D
#define glFrontFace gles_glFrontFace
#define glGenBuffers gles_glGenBuffers
#define glGenFramebuffers gles_glGenFramebuffers
#define glGenRenderbuffers gles_glGenRenderbuffers
#define glGenTextures gles_glGenTextures
#define glGenVertexArrays gles_glGenVertexArrays
#define glGenerateMipmap gles_glGenerateMipmap
#define glGetAttribLocation gles_glGetAttribLocation
#define glGetBooleanv gles_glGetBooleanv
#define glGetError gles_glGetError
#define glGetFloatv gles_glGetFloatv
#define glGetIntegerv gles_glGetIntegerv
#define glGetProgramInfoLog gles_glGetProgramInfoLog
#define glGetProgramiv gles_glGetProgramiv
#define glGetShaderInfoLog gles_glGetShaderInfoLog
#define glGetShaderSource gles_glGetShaderSource
#define glGetShaderiv gles_glGetShaderiv
#define glGetString gles_glGetString
#define glGetUniformLocation gles_glGetUniformLocation
#define glIsEnabled gles_glIsEnabled
#define glLineWidth gles_glLineWidth
#define glLinkProgram gles_glLinkProgram
#define glPixelStorei gles_glPixelStorei
#define glPolygonOffset gles_glPolygonOffset
#define glReadPixels gles_glReadPixels
#define glRenderbufferStorage gles_glRenderbufferStorage
#define glScissor gles_glScissor
#define glShaderSource gles_glShaderSource
#define glStencilFunc gles_glStencilFunc
#define glStencilMask gles_glStencilMask
#define glStencilOp gles_glStencilOp
#define glTexImage2D gles_glTexImage2D
#define glTexParameterf gles_glTexParameterf
#define glTexParameteri gles_glTexParameteri
#define glTexSubImage2D gles_glTexSubImage2D
#define glUniform1f gles_glUniform1f
#define glUniform1fv gles_glUniform1fv
#define glUniform1i gles_glUniform1i
#define glUniform1iv gles_glUniform1iv
#define glUniform2f gles_glUniform2f
#define glUniform2fv gles_glUniform2fv
#define glUniform2i gles_glUniform2i
#define glUniform2iv gles_glUniform2iv
#define glUniform3f gles_glUniform3f
#define glUniform3fv gles_glUniform3fv
#define glUniform3i gles_glUniform3i
#define glUniform3iv gles_glUniform3iv
#define glUniform4f gles_glUniform4f
#define glUniform4fv gles_glUniform4fv
#define glUniform4i gles_glUniform4i
#define glUniform4iv gles_glUniform4iv
#define glUniformMatrix4fv gles_glUniformMatrix4fv
#define glUseProgram gles_glUseProgram
#define glVertexAttrib4f gles_glVertexAttrib4f
#define glVertexAttribPointer gles_glVertexAttribPointer
#define glViewport gles_glViewport
#endif // !__ANDROID__

bool InitGLLoader();

#endif // USE_OPENGL_ES
