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

// Use generated EGL includes from ANGLE:
#define EGL_EGL_PROTOTYPES 1

#include "angle/include/EGL/egl.h"
#include "angle/include/GLES2/gl2.h"
#include "angle/include/GLES2/gl2ext.h"
#include "angle/include/GLES3/gl3.h"

#include <iostream>
#include <memory>
#include <string>

namespace nodejsgl {

// Provides initialization of EGL/GL context options.
struct GLContextOptions {
  bool webgl_compatibility = false;
  uint32_t client_major_es_version = 3;
  uint32_t client_minor_es_version = 0;
  uint32_t width = 1;
  uint32_t height = 1;
};

// Provides lookup of EGL/GL extensions.
class GLExtensionsWrapper {
 public:
  GLExtensionsWrapper(const char* extensions_str)
      : extensions_(extensions_str) {}

  bool HasExtension(const char* name) {
    return extensions_.find(name) != std::string::npos;
  }

  const char* GetExtensions() { return extensions_.c_str(); }

#if DEBUG
  void LogExtensions() {
    std::string s(extensions_);
    std::string delim = " ";
    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delim)) != std::string::npos) {
      token = s.substr(0, pos);
      s.erase(0, pos + delim.length());
      std::cerr << token << std::endl;
    }
  }
#endif

 private:
  std::string extensions_;
};

// Wraps an EGLContext instance for off screen usage.
class EGLContextWrapper {
 public:
  ~EGLContextWrapper();

  // Creates and in
  static EGLContextWrapper* Create(napi_env env,
                                   const GLContextOptions& context_options);

  EGLContext context;
  EGLDisplay display;
  EGLConfig config;
  EGLSurface surface;

  std::unique_ptr<GLExtensionsWrapper> egl_extensions;
  std::unique_ptr<GLExtensionsWrapper> gl_extensions;
  std::unique_ptr<GLExtensionsWrapper> angle_requestable_extensions;

  // Function pointers
  PFNGLACTIVETEXTUREPROC glActiveTexture;
  PFNGLATTACHSHADERPROC glAttachShader;
  PFNGLBINDATTRIBLOCATIONPROC glBindAttribLocation;
  PFNGLBINDBUFFERPROC glBindBuffer;
  PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
  PFNGLBINDRENDERBUFFERPROC glBindRenderbuffer;
  PFNGLBINDTEXTUREPROC glBindTexture;
  PFNGLBLENDCOLORPROC glBlendColor;
  PFNGLBLENDEQUATIONPROC glBlendEquation;
  PFNGLBLENDEQUATIONSEPARATEPROC glBlendEquationSeparate;
  PFNGLBLENDFUNCPROC glBlendFunc;
  PFNGLBLENDFUNCSEPARATEPROC glBlendFuncSeparate;
  PFNGLBUFFERDATAPROC glBufferData;
  PFNGLBUFFERSUBDATAPROC glBufferSubData;
  PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
  PFNGLCLEARPROC glClear;
  PFNGLCLEARCOLORPROC glClearColor;
  PFNGLCLEARDEPTHFPROC glClearDepthf;
  PFNGLCLEARSTENCILPROC glClearStencil;
  PFNGLCLIENTWAITSYNCPROC glClientWaitSync;
  PFNGLCOLORMASKPROC glColorMask;
  PFNGLCOMPILESHADERPROC glCompileShader;
  PFNGLCOMPRESSEDTEXIMAGE2DPROC glCompressedTexImage2D;
  PFNGLCOMPRESSEDTEXSUBIMAGE2DPROC glCompressedTexSubImage2D;
  PFNGLCOPYTEXIMAGE2DPROC glCopyTexImage2D;
  PFNGLCOPYTEXSUBIMAGE2DPROC glCopyTexSubImage2D;
  PFNGLCREATEPROGRAMPROC glCreateProgram;
  PFNGLCREATESHADERPROC glCreateShader;
  PFNGLCULLFACEPROC glCullFace;
  PFNGLDELETEBUFFERSPROC glDeleteBuffers;
  PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
  PFNGLDELETEPROGRAMPROC glDeleteProgram;
  PFNGLDELETERENDERBUFFERSPROC glDeleteRenderbuffers;
  PFNGLDELETESHADERPROC glDeleteShader;
  PFNGLDELETESYNCPROC glDeleteSync;
  PFNGLDELETETEXTURESPROC glDeleteTextures;
  PFNGLDEPTHFUNCPROC glDepthFunc;
  PFNGLDEPTHMASKPROC glDepthMask;
  PFNGLDEPTHRANGEFPROC glDepthRangef;
  PFNGLDETACHSHADERPROC glDetachShader;
  PFNGLDISABLEPROC glDisable;
  PFNGLDISABLEVERTEXATTRIBARRAYPROC glDisableVertexAttribArray;
  PFNGLDRAWARRAYSPROC glDrawArrays;
  PFNGLDRAWELEMENTSPROC glDrawElements;
  PFNGLENABLEPROC glEnable;
  PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;
  PFNGLFENCESYNCPROC glFenceSync;
  PFNGLFINISHPROC glFinish;
  PFNGLFLUSHPROC glFlush;
  PFNGLFRAMEBUFFERRENDERBUFFERPROC glFramebufferRenderbuffer;
  PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
  PFNGLFRONTFACEPROC glFrontFace;
  PFNGLGENERATEMIPMAPPROC glGenerateMipmap;
  PFNGLGENBUFFERSPROC glGenBuffers;
  PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
  PFNGLGENRENDERBUFFERSPROC glGenRenderbuffers;
  PFNGLGENTEXTURESPROC glGenTextures;
  PFNGLGETACTIVEATTRIBPROC glGetActiveAttrib;
  PFNGLGETACTIVEUNIFORMPROC glGetActiveUniform;
  PFNGLGETATTACHEDSHADERSPROC glGetAttachedShaders;
  PFNGLGETATTRIBLOCATIONPROC glGetAttribLocation;
  PFNGLGETBUFFERPARAMETERIVPROC glGetBufferParameteriv;
  /* PFNGLGETBUFFERSUBDATAPROC glGetBufferSubData; */
  PFNGLGETERRORPROC glGetError;
  PFNGLGETFRAMEBUFFERATTACHMENTPARAMETERIVPROC
  glGetFramebufferAttachmentParameteriv;
  PFNGLGETINTEGERVPROC glGetIntegerv;
  PFNGLGETPROGRAMIVPROC glGetProgramiv;
  PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
  PFNGLGETRENDERBUFFERPARAMETERIVPROC glGetRenderbufferParameteriv;
  PFNGLGETSHADERIVPROC glGetShaderiv;
  PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog;
  PFNGLGETSHADERPRECISIONFORMATPROC glGetShaderPrecisionFormat;
  PFNGLGETSTRINGPROC glGetString;
  PFNGLGETTEXPARAMETERFVPROC glGetTexParameterfv;
  PFNGLGETTEXPARAMETERIVPROC glGetTexParameteriv;
  PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
  PFNGLHINTPROC glHint;
  PFNGLISBUFFERPROC glIsBuffer;
  PFNGLISENABLEDPROC glIsEnabled;
  PFNGLISFRAMEBUFFERPROC glIsFramebuffer;
  PFNGLISPROGRAMPROC glIsProgram;
  PFNGLISRENDERBUFFERPROC glIsRenderbuffer;
  PFNGLISSHADERPROC glIsShader;
  PFNGLISTEXTUREPROC glIsTexture;
  PFNGLLINEWIDTHPROC glLineWidth;
  PFNGLLINKPROGRAMPROC glLinkProgram;
  PFNGLMAPBUFFERRANGEPROC glMapBufferRange;
  PFNGLPIXELSTOREIPROC glPixelStorei;
  PFNGLPOLYGONOFFSETPROC glPolygonOffset;
  PFNGLREADPIXELSPROC glReadPixels;
  PFNGLRENDERBUFFERSTORAGEPROC glRenderbufferStorage;
  PFNGLSAMPLECOVERAGEPROC glSampleCoverage;
  PFNGLSCISSORPROC glScissor;
  PFNGLSHADERSOURCEPROC glShaderSource;
  PFNGLSTENCILFUNCPROC glStencilFunc;
  PFNGLSTENCILFUNCSEPARATEPROC glStencilFuncSeparate;
  PFNGLSTENCILMASKPROC glStencilMask;
  PFNGLSTENCILMASKSEPARATEPROC glStencilMaskSeparate;
  PFNGLSTENCILOPPROC glStencilOp;
  PFNGLSTENCILOPSEPARATEPROC glStencilOpSeparate;
  PFNGLTEXIMAGE2DPROC glTexImage2D;
  PFNGLTEXPARAMETERIPROC glTexParameteri;
  PFNGLTEXPARAMETERFPROC glTexParameterf;
  PFNGLTEXSUBIMAGE2DPROC glTexSubImage2D;
  PFNGLUNIFORM1FPROC glUniform1f;
  PFNGLUNIFORM1FVPROC glUniform1fv;
  PFNGLUNIFORM1IPROC glUniform1i;
  PFNGLUNIFORM1IVPROC glUniform1iv;
  PFNGLUNIFORM2FPROC glUniform2f;
  PFNGLUNIFORM2FVPROC glUniform2fv;
  PFNGLUNIFORM2IPROC glUniform2i;
  PFNGLUNIFORM2IVPROC glUniform2iv;
  PFNGLUNIFORM3FPROC glUniform3f;
  PFNGLUNIFORM3FVPROC glUniform3fv;
  PFNGLUNIFORM3IPROC glUniform3i;
  PFNGLUNIFORM3IVPROC glUniform3iv;
  PFNGLUNIFORM4FPROC glUniform4f;
  PFNGLUNIFORM4FVPROC glUniform4fv;
  PFNGLUNIFORM4IPROC glUniform4i;
  PFNGLUNIFORM4IVPROC glUniform4iv;
  PFNGLUNIFORMMATRIX2FVPROC glUniformMatrix2fv;
  PFNGLUNIFORMMATRIX3FVPROC glUniformMatrix3fv;
  PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv;
  PFNGLUNMAPBUFFERPROC glUnmapBuffer;
  PFNGLUSEPROGRAMPROC glUseProgram;
  PFNGLVALIDATEPROGRAMPROC glValidateProgram;
  PFNGLVERTEXATTRIB1FPROC glVertexAttrib1f;
  PFNGLVERTEXATTRIB1FVPROC glVertexAttrib1fv;
  PFNGLVERTEXATTRIB2FPROC glVertexAttrib2f;
  PFNGLVERTEXATTRIB2FVPROC glVertexAttrib2fv;
  PFNGLVERTEXATTRIB3FPROC glVertexAttrib3f;
  PFNGLVERTEXATTRIB3FVPROC glVertexAttrib3fv;
  PFNGLVERTEXATTRIB4FPROC glVertexAttrib4f;
  PFNGLVERTEXATTRIB4FVPROC glVertexAttrib4fv;
  PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
  PFNGLVIEWPORTPROC glViewport;

  // ANGLE specific
  PFNGLREQUESTEXTENSIONANGLEPROC glRequestExtensionANGLE;

  // Refreshes extensions list:
  void RefreshGLExtensions();

 private:
  EGLContextWrapper(napi_env env, const GLContextOptions& context_options);

  void InitEGL(napi_env env, const GLContextOptions& context_options);
  void BindProcAddresses();
};

}  // namespace nodejsgl

#endif  // NODEJS_GL_EGL_CONTEXT_H_
