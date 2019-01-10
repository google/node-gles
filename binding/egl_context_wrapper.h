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

#ifndef NODEJS_GL_EGL_CONTEXT_WRAPPER_H_
#define NODEJS_GL_EGL_CONTEXT_WRAPPER_H_

#include <node_api.h>

#include <EGL/egl.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <string>

namespace nodejsgl {

// Provides lookup of EGL extensions.
class EGLExtensionsWrapper {
 public:
  EGLExtensionsWrapper(EGLDisplay display)
      : client_extensions_(eglQueryString(display, EGL_EXTENSIONS)) {}

  bool HasExtension(const char* name) {
    return client_extensions_.find(name) != std::string::npos;
  }

 private:
  std::string client_extensions_;
};

// Wraps an EGLContext instance for off screen usage.
class EGLContextWrapper {
 public:
  ~EGLContextWrapper();

  // Creates and in
  static EGLContextWrapper* Create(napi_env env);

  EGLContext context;
  EGLDisplay display;
  EGLConfig config;
  EGLSurface surface;

  EGLExtensionsWrapper* extensions_wrapper;

  // Function pointers
  PFNGLACTIVETEXTUREPROC glActiveTexture;
  PFNGLATTACHSHADERPROC glAttachShader;
  PFNGLBINDBUFFERPROC glBindBuffer;
  PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
  PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
  PFNGLBINDTEXTUREPROC glBindTexture;
  PFNGLBUFFERDATAPROC glBufferData;
  PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
  PFNGLCOMPILESHADERPROC glCompileShader;
  PFNGLCREATEPROGRAMPROC glCreateProgram;
  PFNGLCREATESHADERPROC glCreateShader;
  PFNGLCULLFACEPROC glCullFace;
  PFNGLDELETEBUFFERSPROC glDeleteBuffers;
  PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
  PFNGLDELETEPROGRAMPROC glDeleteProgram;
  PFNGLDELETESHADERPROC glDeleteShader;
  PFNGLDELETETEXTURESPROC glDeleteTextures;
  PFNGLDISABLEPROC glDisable;
  PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
  PFNGLDRAWARRAYSPROC glDrawArrays;
  PFNGLDRAWELEMENTSPROC glDrawElements;
  PFNGLENABLEPROC glEnable;
  PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
  PFNGLFINISHPROC glFinish;
  PFNGLFLUSHPROC glFlush;
  PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
  PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
  PFNGLGENBUFFERSPROC glGenBuffers;
  PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
  PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
  PFNGLGENTEXTURESPROC glGenTextures;
  PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
  PFNGLGETERRORPROC glGetError;
  PFNGLGETINTEGERVPROC glGetIntegerv;
  PFNGLGETPROGRAMIVPROC glGetProgramiv;
  PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
  PFNGLGETSHADERIVPROC glGetShaderiv;
  PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
  PFNGLGETSTRINGPROC glGetString;
  PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
  PFNGLLINKPROGRAMPROC glLinkProgram;
  PFNGLREADPIXELSPROC glReadPixels;
  PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
  PFNGLSCISSORPROC glScissor;
  PFNGLSHADERSOURCEPROC glShaderSource;
  PFNGLTEXIMAGE2DPROC glTexImage2D;
  PFNGLTEXPARAMETERIPROC glTexParameteri;
  PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D;
  PFNGLUNIFORM1IPROC glUniform1i;
  PFNGLUNIFORM1FPROC glUniform1f;
  PFNGLUNIFORM1FVPROC glUniform1fv;
  PFNGLUNIFORM2IPROC glUniform2i;
  PFNGLUNIFORM4FVPROC glUniform4fv;
  PFNGLUNIFORM4IPROC glUniform4i;
  PFNGLUSEPROGRAMPROC glUseProgram;
  PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
  PFNGLVIEWPORTPROC glViewport;

  // ANGLE specific
  PFNGLREQUESTEXTENSIONANGLEPROC glRequestExtensionANGLE;

 private:
  EGLContextWrapper(napi_env env);

  void InitEGL(napi_env env);
  void BindProcAddresses();
  void BindExtensions();
};

}  // namespace nodejsgl

#endif  // NODEJS_GL_EGL_CONTEXT_H_
