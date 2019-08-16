/**
 * @license
 * Copyright 2018 Google Inc. All Rights Reserved.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * =============================================================================
 */

#include "egl_context_wrapper.h"

#include "utils.h"

#include "angle/include/EGL/egl.h"
#include "angle/include/EGL/eglext.h"

#include <vector>

namespace nodejsgl {

EGLContextWrapper::EGLContextWrapper(napi_env env,
                                     const GLContextOptions& context_options) {
  InitEGL(env, context_options);
  BindProcAddresses();
  RefreshGLExtensions();

#if DEBUG
  std::cerr << "** GL_EXTENSIONS:" << std::endl;
  gl_extensions->LogExtensions();
  std::cerr << std::endl;

  std::cerr << "** REQUESTABLE_EXTENSIONS:" << std::endl;
  angle_requestable_extensions->LogExtensions();
  std::cerr << std::endl;
#endif
}

void EGLContextWrapper::InitEGL(napi_env env,
                                const GLContextOptions& context_options) {
  std::vector<EGLAttrib> display_attributes;
  display_attributes.push_back(EGL_PLATFORM_ANGLE_TYPE_ANGLE);
  // Most NVIDIA drivers will not work properly with
  // EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE, only enable this option on ARM
  // devices for now:
#if defined(__arm__)
  display_attributes.push_back(EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE);
#else
  display_attributes.push_back(EGL_PLATFORM_ANGLE_TYPE_DEFAULT_ANGLE);
#endif

  display_attributes.push_back(EGL_NONE);

  display = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE, nullptr,
                                  &display_attributes[0]);
  if (display == EGL_NO_DISPLAY) {
    // TODO(kreeger): This is the default path for Mac OS. Determine why egl has
    // to be initialized this way on Mac OS.
    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY) {
      NAPI_THROW_ERROR(env, "No display");
      return;
    }
  }

  EGLint major;
  EGLint minor;
  if (!eglInitialize(display, &major, &minor)) {
    NAPI_THROW_ERROR(env, "Could not initialize display");
    return;
  }

  egl_extensions = std::unique_ptr<GLExtensionsWrapper>(
      new GLExtensionsWrapper(eglQueryString(display, EGL_EXTENSIONS)));
#if DEBUG
  std::cerr << "** EGL_EXTENSIONS:" << std::endl;
  egl_extensions->LogExtensions();
  std::cerr << std::endl;
#endif

  EGLint attrib_list[] = {EGL_SURFACE_TYPE, EGL_PBUFFER_BIT,
                          EGL_RED_SIZE,     8,
                          EGL_GREEN_SIZE,   8,
                          EGL_BLUE_SIZE,    8,
                          EGL_ALPHA_SIZE,   8,
                          EGL_DEPTH_SIZE,   24,
                          EGL_STENCIL_SIZE, 8,
                          EGL_NONE};

  EGLint num_config;
  if (!eglChooseConfig(display, attrib_list, &config, 1, &num_config)) {
    NAPI_THROW_ERROR(env, "Failed creating a config");
    return;
  }

  eglBindAPI(EGL_OPENGL_ES_API);
  if (eglGetError() != EGL_SUCCESS) {
    NAPI_THROW_ERROR(env, "Failed to set OpenGL ES API");
    return;
  }

  EGLint config_renderable_type;
  if (!eglGetConfigAttrib(display, config, EGL_RENDERABLE_TYPE,
                          &config_renderable_type)) {
    NAPI_THROW_ERROR(env, "Failed to get EGL_RENDERABLE_TYPE");
    return;
  }

  // If the requested context is ES3 but the config cannot support ES3, request
  // ES2 instead.
  EGLint major_version = context_options.client_major_es_version;
  EGLint minor_version = context_options.client_minor_es_version;
  if ((config_renderable_type & EGL_OPENGL_ES3_BIT) == 0 &&
      major_version >= 3) {
    major_version = 2;
    minor_version = 0;
  }

  // Append attributes based on available features
  std::vector<EGLint> context_attributes;

  context_attributes.push_back(EGL_CONTEXT_MAJOR_VERSION_KHR);
  context_attributes.push_back(major_version);

  context_attributes.push_back(EGL_CONTEXT_MINOR_VERSION_KHR);
  context_attributes.push_back(minor_version);

  if (context_options.webgl_compatibility) {
    context_attributes.push_back(EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE);
    context_attributes.push_back(EGL_TRUE);
  }

  // TODO(kreeger): This is only needed to avoid validation.
  // This is needed for OES_TEXTURE_HALF_FLOAT textures uploading as FLOAT
  context_attributes.push_back(EGL_CONTEXT_OPENGL_NO_ERROR_KHR);
  context_attributes.push_back(EGL_TRUE);

  context_attributes.push_back(EGL_NONE);

  context = eglCreateContext(display, config, EGL_NO_CONTEXT,
                             context_attributes.data());
  if (context == EGL_NO_CONTEXT) {
    NAPI_THROW_ERROR(env, "Could not create context");
    return;
  }

  EGLint surface_attribs[] = {EGL_WIDTH, (EGLint)context_options.width,
                              EGL_HEIGHT, (EGLint)context_options.height,
                              EGL_NONE};
  surface = eglCreatePbufferSurface(display, config, surface_attribs);
  if (surface == EGL_NO_SURFACE) {
    NAPI_THROW_ERROR(env, "Could not create surface");
    return;
  }

  if (!eglMakeCurrent(display, surface, surface, context)) {
    NAPI_THROW_ERROR(env, "Could not make context current");
    return;
  }
}

void EGLContextWrapper::BindProcAddresses() {
  // Bind runtime function pointers.
  glActiveTexture = reinterpret_cast<PFNGLACTIVETEXTUREPROC>(
      eglGetProcAddress("glActiveTexture"));
  glAttachShader = reinterpret_cast<PFNGLATTACHSHADERPROC>(
      eglGetProcAddress("glAttachShader"));
  glBindAttribLocation = reinterpret_cast<PFNGLBINDATTRIBLOCATIONPROC>(
      eglGetProcAddress("glBindAttribLocation"));
  glBindBuffer =
      reinterpret_cast<PFNGLBINDBUFFERPROC>(eglGetProcAddress("glBindBuffer"));
  glBindFramebuffer = reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(
      eglGetProcAddress("glBindFramebuffer"));
  glBindRenderbuffer = reinterpret_cast<PFNGLBINDRENDERBUFFERPROC>(
      eglGetProcAddress("glBindRenderbuffer"));
  glBindTexture = reinterpret_cast<PFNGLBINDTEXTUREPROC>(
      eglGetProcAddress("glBindTexture"));
  glBlendColor =
      reinterpret_cast<PFNGLBLENDCOLORPROC>(eglGetProcAddress("glBlendColor"));
  glBlendEquation = reinterpret_cast<PFNGLBLENDEQUATIONPROC>(
      eglGetProcAddress("glBlendEquation"));
  glBlendEquationSeparate = reinterpret_cast<PFNGLBLENDEQUATIONSEPARATEPROC>(
      eglGetProcAddress("glBlendEquationSeparate"));
  glBlendFunc =
      reinterpret_cast<PFNGLBLENDFUNCPROC>(eglGetProcAddress("glBlendFunc"));
  glBlendFuncSeparate = reinterpret_cast<PFNGLBLENDFUNCSEPARATEPROC>(
      eglGetProcAddress("glBlendFuncSeparate"));
  glBufferData =
      reinterpret_cast<PFNGLBUFFERDATAPROC>(eglGetProcAddress("glBufferData"));
  glBufferSubData = reinterpret_cast<PFNGLBUFFERSUBDATAPROC>(
      eglGetProcAddress("glBufferSubData"));
  glCheckFramebufferStatus = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSPROC>(
      eglGetProcAddress("glCheckFramebufferStatus"));
  glClear = reinterpret_cast<PFNGLCLEARPROC>(eglGetProcAddress("glClear"));
  glClearColor =
      reinterpret_cast<PFNGLCLEARCOLORPROC>(eglGetProcAddress("glClearColor"));
  glClearDepthf = reinterpret_cast<PFNGLCLEARDEPTHFPROC>(
      eglGetProcAddress("glClearDepthf"));
  glClearStencil = reinterpret_cast<PFNGLCLEARSTENCILPROC>(
      eglGetProcAddress("glClearStencil"));
  glClientWaitSync = reinterpret_cast<PFNGLCLIENTWAITSYNCPROC>(
      eglGetProcAddress("glClientWaitSync"));
  glColorMask =
      reinterpret_cast<PFNGLCOLORMASKPROC>(eglGetProcAddress("glColorMask"));
  glCompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>(
      eglGetProcAddress("glCompileShader"));
  glCompressedTexImage2D = reinterpret_cast<PFNGLCOMPRESSEDTEXIMAGE2DPROC>(
      eglGetProcAddress("glCompressedTexImage2D"));
  glCompressedTexSubImage2D =
      reinterpret_cast<PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC>(
          eglGetProcAddress("glCompressedTexSubImage2D"));
  glCopyTexImage2D = reinterpret_cast<PFNGLCOPYTEXIMAGE2DPROC>(
      eglGetProcAddress("glCopyTexImage2D"));
  glCopyTexSubImage2D = reinterpret_cast<PFNGLCOPYTEXSUBIMAGE2DPROC>(
      eglGetProcAddress("glCopyTexSubImage2D"));
  glCreateProgram = reinterpret_cast<PFNGLCREATEPROGRAMPROC>(
      eglGetProcAddress("glCreateProgram"));
  glCreateShader = reinterpret_cast<PFNGLCREATESHADERPROC>(
      eglGetProcAddress("glCreateShader"));
  glCullFace =
      reinterpret_cast<PFNGLCULLFACEPROC>(eglGetProcAddress("glCullFace"));
  glDeleteBuffers = reinterpret_cast<PFNGLDELETEBUFFERSPROC>(
      eglGetProcAddress("glDeleteBuffers"));
  glDeleteFramebuffers = reinterpret_cast<PFNGLDELETEFRAMEBUFFERSPROC>(
      eglGetProcAddress("glDeleteFramebuffers"));
  glDeleteRenderbuffers = reinterpret_cast<PFNGLDELETERENDERBUFFERSPROC>(
      eglGetProcAddress("glDeleteRenderbuffers"));
  glDeleteProgram = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(
      eglGetProcAddress("glDeleteProgram"));
  glDeleteShader = reinterpret_cast<PFNGLDELETESHADERPROC>(
      eglGetProcAddress("glDeleteShader"));
  glDeleteSync =
      reinterpret_cast<PFNGLDELETESYNCPROC>(eglGetProcAddress("glDeleteSync"));
  glDeleteTextures = reinterpret_cast<PFNGLDELETETEXTURESPROC>(
      eglGetProcAddress("glDeleteTextures"));
  glDepthFunc =
      reinterpret_cast<PFNGLDEPTHFUNCPROC>(eglGetProcAddress("glDepthFunc"));
  glDepthMask =
      reinterpret_cast<PFNGLDEPTHMASKPROC>(eglGetProcAddress("glDepthMask"));
  glDepthRangef = reinterpret_cast<PFNGLDEPTHRANGEFPROC>(
      eglGetProcAddress("glDepthRangef"));
  glDrawArrays =
      reinterpret_cast<PFNGLDRAWARRAYSPROC>(eglGetProcAddress("glDrawArrays"));
  glDrawElements = reinterpret_cast<PFNGLDRAWELEMENTSPROC>(
      eglGetProcAddress("glDrawElements"));
  glDetachShader = reinterpret_cast<PFNGLDETACHSHADERPROC>(
      eglGetProcAddress("glDetachShader"));
  glDisable =
      reinterpret_cast<PFNGLDISABLEPROC>(eglGetProcAddress("glDisable"));
  glDisableVertexAttribArray =
      reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAYPROC>(
          eglGetProcAddress("glDisableVertexAttribArray"));
  glEnable = reinterpret_cast<PFNGLENABLEPROC>(eglGetProcAddress("glEnable"));
  glEnableVertexAttribArray =
      reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(
          eglGetProcAddress("glEnableVertexAttribArray"));
  glFenceSync =
      reinterpret_cast<PFNGLFENCESYNCPROC>(eglGetProcAddress("glFenceSync"));
  glFinish = reinterpret_cast<PFNGLFINISHPROC>(eglGetProcAddress("glFinish"));
  glFlush = reinterpret_cast<PFNGLFLUSHPROC>(eglGetProcAddress("glFlush"));
  glFramebufferRenderbuffer =
      reinterpret_cast<PFNGLFRAMEBUFFERRENDERBUFFERPROC>(
          eglGetProcAddress("glFramebufferRenderbuffer"));
  glFramebufferTexture2D = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(
      eglGetProcAddress("glFramebufferTexture2D"));
  glFrontFace =
      reinterpret_cast<PFNGLFRONTFACEPROC>(eglGetProcAddress("glFrontFace"));
  glGenerateMipmap = reinterpret_cast<PFNGLGENERATEMIPMAPPROC>(
      eglGetProcAddress("glGenerateMipmap"));
  glGenBuffers =
      reinterpret_cast<PFNGLGENBUFFERSPROC>(eglGetProcAddress("glGenBuffers"));
  glGenFramebuffers = reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(
      eglGetProcAddress("glGenFramebuffers"));
  glGenRenderbuffers = reinterpret_cast<PFNGLGENRENDERBUFFERSPROC>(
      eglGetProcAddress("glGenRenderbuffers"));
  glGetAttribLocation = reinterpret_cast<PFNGLGETATTRIBLOCATIONPROC>(
      eglGetProcAddress("glGetAttribLocation"));
  glGetBufferParameteriv = reinterpret_cast<PFNGLGETBUFFERPARAMETERIVPROC>(
      eglGetProcAddress("glGetBufferParameteriv"));
  glGetError =
      reinterpret_cast<PFNGLGETERRORPROC>(eglGetProcAddress("glGetError"));
  glGetFramebufferAttachmentParameteriv =
      reinterpret_cast<PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC>(
          eglGetProcAddress("glGetFramebufferAttachmentParameteriv"));
  glGetIntegerv = reinterpret_cast<PFNGLGETINTEGERVPROC>(
      eglGetProcAddress("glGetIntegerv"));
  glGenTextures = reinterpret_cast<PFNGLGENTEXTURESPROC>(
      eglGetProcAddress("glGenTextures"));
  glGetActiveAttrib = reinterpret_cast<PFNGLGETACTIVEATTRIBPROC>(
      eglGetProcAddress("glGetActiveAttrib"));
  glGetActiveUniform = reinterpret_cast<PFNGLGETACTIVEUNIFORMPROC>(
      eglGetProcAddress("glGetActiveUniform"));
  glGetAttachedShaders = reinterpret_cast<PFNGLGETATTACHEDSHADERSPROC>(
      eglGetProcAddress("glGetAttachedShaders"));
  glGetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(
      eglGetProcAddress("glGetProgramiv"));
  glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(
      eglGetProcAddress("glGetProgramInfoLog"));
  glGetRenderbufferParameteriv =
      reinterpret_cast<PFNGLGETRENDERBUFFERPARAMETERIVPROC>(
          eglGetProcAddress("glGetRenderbufferParameteriv"));
  glGetShaderiv = reinterpret_cast<PFNGLGETSHADERIVPROC>(
      eglGetProcAddress("glGetShaderiv"));
  glGetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(
      eglGetProcAddress("glGetShaderInfoLog"));
  glGetShaderPrecisionFormat =
      reinterpret_cast<PFNGLGETSHADERPRECISIONFORMATPROC>(
          eglGetProcAddress("glGetShaderPrecisionFormat"));
  glGetString =
      reinterpret_cast<PFNGLGETSTRINGPROC>(eglGetProcAddress("glGetString"));
  glGetTexParameterfv = reinterpret_cast<PFNGLGETTEXPARAMETERFVPROC>(
      eglGetProcAddress("glGetTexParameterfv"));
  glGetTexParameteriv = reinterpret_cast<PFNGLGETTEXPARAMETERIVPROC>(
      eglGetProcAddress("glGetTexParameteriv"));
  glGetUniformLocation = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(
      eglGetProcAddress("glGetUniformLocation"));
  glHint = reinterpret_cast<PFNGLHINTPROC>(eglGetProcAddress("glHint"));
  glIsBuffer =
      reinterpret_cast<PFNGLISBUFFERPROC>(eglGetProcAddress("glIsBuffer"));
  glIsEnabled =
      reinterpret_cast<PFNGLISENABLEDPROC>(eglGetProcAddress("glIsEnabled"));
  glIsFramebuffer = reinterpret_cast<PFNGLISFRAMEBUFFERPROC>(
      eglGetProcAddress("glIsFramebuffer"));
  glIsProgram =
      reinterpret_cast<PFNGLISPROGRAMPROC>(eglGetProcAddress("glIsProgram"));
  glIsRenderbuffer = reinterpret_cast<PFNGLISRENDERBUFFERPROC>(
      eglGetProcAddress("glIsRenderbuffer"));
  glIsShader =
      reinterpret_cast<PFNGLISSHADERPROC>(eglGetProcAddress("glIsShader"));
  glIsTexture =
      reinterpret_cast<PFNGLISTEXTUREPROC>(eglGetProcAddress("glIsTexture"));
  glLineWidth =
      reinterpret_cast<PFNGLLINEWIDTHPROC>(eglGetProcAddress("glLineWidth"));
  glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>(
      eglGetProcAddress("glLinkProgram"));
  glMapBufferRange = reinterpret_cast<PFNGLMAPBUFFERRANGEPROC>(
      eglGetProcAddress("glMapBufferRange"));
  glPixelStorei = reinterpret_cast<PFNGLPIXELSTOREIPROC>(
      eglGetProcAddress("glPixelStorei"));
  glPolygonOffset = reinterpret_cast<PFNGLPOLYGONOFFSETPROC>(
      eglGetProcAddress("glPolygonOffset"));
  glReadPixels =
      reinterpret_cast<PFNGLREADPIXELSPROC>(eglGetProcAddress("glReadPixels"));
  glRenderbufferStorage = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEPROC>(
      eglGetProcAddress("glRenderbufferStorage"));
  glSampleCoverage = reinterpret_cast<PFNGLSAMPLECOVERAGEPROC>(
      eglGetProcAddress("glSampleCoverage"));
  glScissor =
      reinterpret_cast<PFNGLSCISSORPROC>(eglGetProcAddress("glScissor"));
  glShaderSource = reinterpret_cast<PFNGLSHADERSOURCEPROC>(
      eglGetProcAddress("glShaderSource"));
  glStencilMask = reinterpret_cast<PFNGLSTENCILMASKPROC>(
      eglGetProcAddress("glStencilMask"));
  glStencilMaskSeparate = reinterpret_cast<PFNGLSTENCILMASKSEPARATEPROC>(
      eglGetProcAddress("glStencilMaskSeparate"));
  glStencilFunc = reinterpret_cast<PFNGLSTENCILFUNCPROC>(
      eglGetProcAddress("glStencilFunc"));
  glStencilFuncSeparate = reinterpret_cast<PFNGLSTENCILFUNCSEPARATEPROC>(
      eglGetProcAddress("glStencilFuncSeparate"));
  glStencilOp =
      reinterpret_cast<PFNGLSTENCILOPPROC>(eglGetProcAddress("glStencilOp"));
  glStencilOpSeparate = reinterpret_cast<PFNGLSTENCILOPSEPARATEPROC>(
      eglGetProcAddress("glStencilOpSeparate"));
  glTexImage2D =
      reinterpret_cast<PFNGLTEXIMAGE2DPROC>(eglGetProcAddress("glTexImage2D"));
  glTexParameteri = reinterpret_cast<PFNGLTEXPARAMETERIPROC>(
      eglGetProcAddress("glTexParameteri"));
  glTexParameterf = reinterpret_cast<PFNGLTEXPARAMETERFPROC>(
      eglGetProcAddress("glTexParameterf"));
  glTexSubImage2D = reinterpret_cast<PFNGLTEXSUBIMAGE2DPROC>(
      eglGetProcAddress("glTexSubImage2D"));
  glUniform1f =
      reinterpret_cast<PFNGLUNIFORM1FPROC>(eglGetProcAddress("glUniform1f"));
  glUniform1fv =
      reinterpret_cast<PFNGLUNIFORM1FVPROC>(eglGetProcAddress("glUniform1fv"));
  glUniform1i =
      reinterpret_cast<PFNGLUNIFORM1IPROC>(eglGetProcAddress("glUniform1i"));
  glUniform1iv =
      reinterpret_cast<PFNGLUNIFORM1IVPROC>(eglGetProcAddress("glUniform1iv"));
  glUniform2f =
      reinterpret_cast<PFNGLUNIFORM2FPROC>(eglGetProcAddress("glUniform2f"));
  glUniform2fv =
      reinterpret_cast<PFNGLUNIFORM2FVPROC>(eglGetProcAddress("glUniform2fv"));
  glUniform2i =
      reinterpret_cast<PFNGLUNIFORM2IPROC>(eglGetProcAddress("glUniform2i"));
  glUniform2iv =
      reinterpret_cast<PFNGLUNIFORM2IVPROC>(eglGetProcAddress("glUniform2iv"));
  glUniform3f =
      reinterpret_cast<PFNGLUNIFORM3FPROC>(eglGetProcAddress("glUniform3f"));
  glUniform3fv =
      reinterpret_cast<PFNGLUNIFORM3FVPROC>(eglGetProcAddress("glUniform3fv"));
  glUniform3i =
      reinterpret_cast<PFNGLUNIFORM3IPROC>(eglGetProcAddress("glUniform3i"));
  glUniform3iv =
      reinterpret_cast<PFNGLUNIFORM3IVPROC>(eglGetProcAddress("glUniform3iv"));
  glUniform4f =
      reinterpret_cast<PFNGLUNIFORM4FPROC>(eglGetProcAddress("glUniform4f"));
  glUniform4fv =
      reinterpret_cast<PFNGLUNIFORM4FVPROC>(eglGetProcAddress("glUniform4fv"));
  glUniform4i =
      reinterpret_cast<PFNGLUNIFORM4IPROC>(eglGetProcAddress("glUniform4i"));
  glUniform4iv =
      reinterpret_cast<PFNGLUNIFORM4IVPROC>(eglGetProcAddress("glUniform4iv"));
  glUniformMatrix2fv = reinterpret_cast<PFNGLUNIFORMMATRIX2FVPROC>(
      eglGetProcAddress("glUniformMatrix2fv"));
  glUniformMatrix3fv = reinterpret_cast<PFNGLUNIFORMMATRIX3FVPROC>(
      eglGetProcAddress("glUniformMatrix3fv"));
  glUniformMatrix4fv = reinterpret_cast<PFNGLUNIFORMMATRIX4FVPROC>(
      eglGetProcAddress("glUniformMatrix4fv"));
  glUnmapBuffer = reinterpret_cast<PFNGLUNMAPBUFFERPROC>(
      eglGetProcAddress("glUnmapBuffer"));
  glUseProgram =
      reinterpret_cast<PFNGLUSEPROGRAMPROC>(eglGetProcAddress("glUseProgram"));
  glValidateProgram = reinterpret_cast<PFNGLVALIDATEPROGRAMPROC>(
      eglGetProcAddress("glValidateProgram"));
  glVertexAttrib1f = reinterpret_cast<PFNGLVERTEXATTRIB1FPROC>(
      eglGetProcAddress("glVertexAttrib1f"));
  glVertexAttrib1fv = reinterpret_cast<PFNGLVERTEXATTRIB1FVPROC>(
      eglGetProcAddress("glVertexAttrib1fv"));
  glVertexAttrib2f = reinterpret_cast<PFNGLVERTEXATTRIB2FPROC>(
      eglGetProcAddress("glVertexAttrib2f"));
  glVertexAttrib2fv = reinterpret_cast<PFNGLVERTEXATTRIB2FVPROC>(
      eglGetProcAddress("glVertexAttrib2fv"));
  glVertexAttrib3f = reinterpret_cast<PFNGLVERTEXATTRIB3FPROC>(
      eglGetProcAddress("glVertexAttrib3f"));
  glVertexAttrib3fv = reinterpret_cast<PFNGLVERTEXATTRIB3FVPROC>(
      eglGetProcAddress("glVertexAttrib3fv"));
  glVertexAttrib4f = reinterpret_cast<PFNGLVERTEXATTRIB4FPROC>(
      eglGetProcAddress("glVertexAttrib4f"));
  glVertexAttrib4fv = reinterpret_cast<PFNGLVERTEXATTRIB4FVPROC>(
      eglGetProcAddress("glVertexAttrib4fv"));
  glVertexAttribPointer = reinterpret_cast<PFNGLVERTEXATTRIBPOINTERPROC>(
      eglGetProcAddress("glVertexAttribPointer"));
  glViewport =
      reinterpret_cast<PFNGLVIEWPORTPROC>(eglGetProcAddress("glViewport"));

  // ANGLE specific
  glRequestExtensionANGLE = reinterpret_cast<PFNGLREQUESTEXTENSIONANGLEPROC>(
      eglGetProcAddress("glRequestExtensionANGLE"));
}

void EGLContextWrapper::RefreshGLExtensions() {
  gl_extensions = std::unique_ptr<GLExtensionsWrapper>(new GLExtensionsWrapper(
      reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS))));

  angle_requestable_extensions = std::unique_ptr<GLExtensionsWrapper>(
      new GLExtensionsWrapper(reinterpret_cast<const char*>(
          glGetString(GL_REQUESTABLE_EXTENSIONS_ANGLE))));
}

EGLContextWrapper::~EGLContextWrapper() {
  if (context) {
    if (!eglDestroyContext(display, context)) {
      std::cerr << "Failed to delete EGL context: " << std::endl;
    }
    context = nullptr;
  }

  // TODO(kreeger): Close context attributes.
  // TODO(kreeger): Cleanup global objects.
}

EGLContextWrapper* EGLContextWrapper::Create(
    napi_env env, const GLContextOptions& context_options) {
  return new EGLContextWrapper(env, context_options);
}

}  // namespace nodejsgl
