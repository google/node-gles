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

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>

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
  display_attributes.push_back(EGL_PLATFORM_ANGLE_TYPE_OPENGLES_ANGLE);
  display_attributes.push_back(EGL_NONE);

  display = eglGetPlatformDisplay(EGL_PLATFORM_ANGLE_ANGLE, nullptr,
                                  &display_attributes[0]);
  if (display == EGL_NO_DISPLAY) {
    NAPI_THROW_ERROR(env, "No display");
    return;
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

  EGLint surface_attribs[] = {EGL_WIDTH, (EGLint)1, EGL_HEIGHT, (EGLint)1,
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
  glBindBuffer =
      reinterpret_cast<PFNGLBINDBUFFERPROC>(eglGetProcAddress("glBindBuffer"));
  glBindFramebuffer = reinterpret_cast<PFNGLBINDFRAMEBUFFERPROC>(
      eglGetProcAddress("glBindFramebuffer"));
  glBindRenderbuffer = reinterpret_cast<PFNGLBINDRENDERBUFFERPROC>(
      eglGetProcAddress("glBindRenderbuffer"));
  glBindTexture = reinterpret_cast<PFNGLBINDTEXTUREPROC>(
      eglGetProcAddress("glBindTexture"));
  glBufferData =
      reinterpret_cast<PFNGLBUFFERDATAPROC>(eglGetProcAddress("glBufferData"));
  glCheckFramebufferStatus = reinterpret_cast<PFNGLCHECKFRAMEBUFFERSTATUSPROC>(
      eglGetProcAddress("glCheckFramebufferStatus"));
  glCompileShader = reinterpret_cast<PFNGLCOMPILESHADERPROC>(
      eglGetProcAddress("glCompileShader"));
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
  glDeleteProgram = reinterpret_cast<PFNGLDELETEPROGRAMPROC>(
      eglGetProcAddress("glDeleteProgram"));
  glDeleteShader = reinterpret_cast<PFNGLDELETESHADERPROC>(
      eglGetProcAddress("glDeleteShader"));
  glDeleteTextures = reinterpret_cast<PFNGLDELETETEXTURESPROC>(
      eglGetProcAddress("glDeleteTextures"));
  glDrawArrays =
      reinterpret_cast<PFNGLDRAWARRAYSPROC>(eglGetProcAddress("glDrawArrays"));
  glDrawElements = reinterpret_cast<PFNGLDRAWELEMENTSPROC>(
      eglGetProcAddress("glDrawElements"));
  glDisable =
      reinterpret_cast<PFNGLDISABLEPROC>(eglGetProcAddress("glDisable"));
  glDisableVertexAttribArray =
      reinterpret_cast<PFNGLDISABLEVERTEXATTRIBARRAYPROC>(
          eglGetProcAddress("glDisableVertexAttribArray"));
  glEnable = reinterpret_cast<PFNGLENABLEPROC>(eglGetProcAddress("glEnable"));
  glEnableVertexAttribArray =
      reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(
          eglGetProcAddress("glEnableVertexAttribArray"));
  glFinish = reinterpret_cast<PFNGLFINISHPROC>(eglGetProcAddress("glFinish"));
  glFlush = reinterpret_cast<PFNGLFLUSHPROC>(eglGetProcAddress("glFlush"));
  glFramebufferRenderbuffer =
      reinterpret_cast<PFNGLFRAMEBUFFERRENDERBUFFERPROC>(
          eglGetProcAddress("glFramebufferRenderbuffer"));
  glFramebufferTexture2D = reinterpret_cast<PFNGLFRAMEBUFFERTEXTURE2DPROC>(
      eglGetProcAddress("glFramebufferTexture2D"));
  glGenBuffers =
      reinterpret_cast<PFNGLGENBUFFERSPROC>(eglGetProcAddress("glGenBuffers"));
  glGenFramebuffers = reinterpret_cast<PFNGLGENFRAMEBUFFERSPROC>(
      eglGetProcAddress("glGenFramebuffers"));
  glGenRenderbuffers = reinterpret_cast<PFNGLGENRENDERBUFFERSPROC>(
      eglGetProcAddress("glGenRenderbuffers"));
  glGetAttribLocation = reinterpret_cast<PFNGLGETATTRIBLOCATIONPROC>(
      eglGetProcAddress("glGetAttribLocation"));
  glGetError =
      reinterpret_cast<PFNGLGETERRORPROC>(eglGetProcAddress("glGetError"));
  glGetIntegerv = reinterpret_cast<PFNGLGETINTEGERVPROC>(
      eglGetProcAddress("glGetIntegerv"));
  glGenTextures = reinterpret_cast<PFNGLGENTEXTURESPROC>(
      eglGetProcAddress("glGenTextures"));
  glGetProgramiv = reinterpret_cast<PFNGLGETPROGRAMIVPROC>(
      eglGetProcAddress("glGetProgramiv"));
  glGetProgramInfoLog = reinterpret_cast<PFNGLGETPROGRAMINFOLOGPROC>(
      eglGetProcAddress("glGetProgramInfoLog"));
  glGetShaderiv = reinterpret_cast<PFNGLGETSHADERIVPROC>(
      eglGetProcAddress("glGetShaderiv"));
  glGetShaderInfoLog = reinterpret_cast<PFNGLGETSHADERINFOLOGPROC>(
      eglGetProcAddress("glGetShaderInfoLog"));
  glGetString =
      reinterpret_cast<PFNGLGETSTRINGPROC>(eglGetProcAddress("glGetString"));
  glGetUniformLocation = reinterpret_cast<PFNGLGETUNIFORMLOCATIONPROC>(
      eglGetProcAddress("glGetUniformLocation"));
  glLinkProgram = reinterpret_cast<PFNGLLINKPROGRAMPROC>(
      eglGetProcAddress("glLinkProgram"));
  glReadPixels =
      reinterpret_cast<PFNGLREADPIXELSPROC>(eglGetProcAddress("glReadPixels"));
  glRenderbufferStorage = reinterpret_cast<PFNGLRENDERBUFFERSTORAGEPROC>(
      eglGetProcAddress("glRenderbufferStorage"));
  glScissor =
      reinterpret_cast<PFNGLSCISSORPROC>(eglGetProcAddress("glScissor"));
  glShaderSource = reinterpret_cast<PFNGLSHADERSOURCEPROC>(
      eglGetProcAddress("glShaderSource"));
  glTexImage2D =
      reinterpret_cast<PFNGLTEXIMAGE2DPROC>(eglGetProcAddress("glTexImage2D"));
  glTexParameteri = reinterpret_cast<PFNGLTEXPARAMETERIPROC>(
      eglGetProcAddress("glTexParameteri"));
  glTexSubImage2D = reinterpret_cast<PFNGLTEXSUBIMAGE2DPROC>(
      eglGetProcAddress("glTexSubImage2D"));
  glUniform1i =
      reinterpret_cast<PFNGLUNIFORM1IPROC>(eglGetProcAddress("glUniform1i"));
  glUniform1f =
      reinterpret_cast<PFNGLUNIFORM1FPROC>(eglGetProcAddress("glUniform1f"));
  glUniform1fv =
      reinterpret_cast<PFNGLUNIFORM1FVPROC>(eglGetProcAddress("glUniform1fv"));
  glUniform2i =
      reinterpret_cast<PFNGLUNIFORM2IPROC>(eglGetProcAddress("glUniform2i"));
  glUniform4fv =
      reinterpret_cast<PFNGLUNIFORM4FVPROC>(eglGetProcAddress("glUniform4fv"));
  glUniform4i =
      reinterpret_cast<PFNGLUNIFORM4IPROC>(eglGetProcAddress("glUniform4i"));
  glUseProgram =
      reinterpret_cast<PFNGLUSEPROGRAMPROC>(eglGetProcAddress("glUseProgram"));
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
