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

#include <iostream>
#include <vector>

namespace nodejsgl {

#if DEBUG
void LogExtensions(const char* extensions_name, const char* extensions) {
  std::string s(extensions);
  std::string delim = " ";
  size_t pos = 0;
  std::string token;
  std::cout << "---- " << extensions_name << "----" << std::endl;
  while ((pos = s.find(delim)) != std::string::npos) {
    token = s.substr(0, pos);
    std::cout << token << std::endl;
    s.erase(0, pos + delim.length());
  }
  std::cout << s;
  std::cout << "-------------------------" << std::endl;
}
#endif

EGLContextWrapper::EGLContextWrapper(napi_env env) {
  InitEGL(env);
  BindProcAddresses();

#if DEBUG
  // LogExtensions("GL_EXTENSIONS",
  //               reinterpret_cast<const char*>(glGetString(GL_EXTENSIONS)));
#endif
}

void EGLContextWrapper::InitEGL(napi_env env) {
  // TODO(kreeger): Figure out how to make this work headless
  display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
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

#if DEBUG
  // TODO(kreeger): Clean this up.
  // std::cerr << "major: " << major << std::endl;
  // std::cerr << "minor: " << minor << std::endl;
#endif

  extensions_wrapper = new EGLExtensionsWrapper(display);

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

#if DEBUG
  // LogExtensions("EGL_EXTENSIONS", eglQueryString(display, EGL_EXTENSIONS));
#endif

  // Append attributes based on available features
  std::vector<EGLint> context_attributes;

  // TODO(kreeger): Add the ability to define or look this up!
  // Hard-code version 2 for now
  // context_attributes.push_back(EGL_CONTEXT_CLIENT_VERSION);
  // context_attributes.push_back(2);

  // context_attributes.push_back(EGL_CONTEXT_WEBGL_COMPATIBILITY_ANGLE);
  // context_attributes.push_back(EGL_TRUE);

  context_attributes.push_back(EGL_CONTEXT_OPENGL_DEBUG);
#if DEBUG
  context_attributes.push_back(EGL_TRUE);
#else
  context_attributes.push_back(EGL_FALSE);
#endif

  context_attributes.push_back(EGL_CONTEXT_OPENGL_NO_ERROR_KHR);
#if DEBUG
  context_attributes.push_back(EGL_FALSE);
#else
  context_attributes.push_back(EGL_TRUE);
#endif

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
  glDrawElements = reinterpret_cast<PFNGLDRAWELEMENTSPROC>(
      eglGetProcAddress("glDrawElements"));
  glDisable =
      reinterpret_cast<PFNGLDISABLEPROC>(eglGetProcAddress("glDisable"));
  glEnable = reinterpret_cast<PFNGLENABLEPROC>(eglGetProcAddress("glEnable"));
  glEnableVertexAttribArray =
      reinterpret_cast<PFNGLENABLEVERTEXATTRIBARRAYPROC>(
          eglGetProcAddress("glEnableVertexAttribArray"));
  glFinish = reinterpret_cast<PFNGLFINISHPROC>(eglGetProcAddress("glFinish"));
  glFlush = reinterpret_cast<PFNGLFLUSHPROC>(eglGetProcAddress("glFlush"));
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
}

EGLContextWrapper::~EGLContextWrapper() {
  // TODO(kreeger): Close context attributes.
  // TODO(kreeger): Cleanup global objects.
  if (extensions_wrapper != nullptr) {
    delete extensions_wrapper;
  }
}

EGLContextWrapper* EGLContextWrapper::Create(napi_env env) {
  return new EGLContextWrapper(env);
}

}  // namespace nodejsgl
