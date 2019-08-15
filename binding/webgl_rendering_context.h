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

#ifndef NODEJS_GL_WEBGL_RENDERING_CONTEXT_H_
#define NODEJS_GL_WEBGL_RENDERING_CONTEXT_H_

#include <node_api.h>

#include <atomic>

#include "egl_context_wrapper.h"

namespace nodejsgl {

class WebGLRenderingContext {
 public:
  static napi_status Register(napi_env env, napi_value exports);
  static napi_status NewInstance(napi_env env, napi_value* instance,
                                 napi_callback_info info);

 private:
  WebGLRenderingContext(napi_env env, GLContextOptions opts);
  ~WebGLRenderingContext();

  static napi_value InitInternal(napi_env env, napi_callback_info info);
  static void Cleanup(napi_env env, void* native, void* hint);

  // User facing methods:
  static napi_value ActiveTexture(napi_env env, napi_callback_info info);
  static napi_value AttachShader(napi_env env, napi_callback_info info);
  static napi_value BindAttribLocation(napi_env env, napi_callback_info info);
  static napi_value BindBuffer(napi_env env, napi_callback_info info);
  static napi_value BindFramebuffer(napi_env env, napi_callback_info info);
  static napi_value BindRenderbuffer(napi_env env, napi_callback_info info);
  static napi_value BindTexture(napi_env env, napi_callback_info info);
  static napi_value BlendColor(napi_env env, napi_callback_info info);
  static napi_value BlendEquation(napi_env env, napi_callback_info info);
  static napi_value BlendEquationSeparate(napi_env env,
                                          napi_callback_info info);
  static napi_value BlendFunc(napi_env env, napi_callback_info info);
  static napi_value BlendFuncSeparate(napi_env env, napi_callback_info info);
  static napi_value BufferData(napi_env env, napi_callback_info info);
  static napi_value BufferSubData(napi_env env, napi_callback_info info);
  static napi_value CheckFramebufferStatus(napi_env env,
                                           napi_callback_info info);
  static napi_value Clear(napi_env env, napi_callback_info info);
  static napi_value ClearColor(napi_env env, napi_callback_info info);
  static napi_value ClearDepth(napi_env env, napi_callback_info info);
  static napi_value ClearStencil(napi_env env, napi_callback_info info);
  static napi_value ClientWaitSync(napi_env env, napi_callback_info info);
  static napi_value ColorMask(napi_env env, napi_callback_info info);
  static napi_value CompileShader(napi_env env, napi_callback_info info);
  static napi_value CompressedTexImage2D(napi_env env, napi_callback_info info);
  static napi_value CompressedTexSubImage2D(napi_env env,
                                            napi_callback_info info);
  static napi_value CopyTexImage2D(napi_env env, napi_callback_info info);
  static napi_value CopyTexSubImage2D(napi_env env, napi_callback_info info);
  static napi_value CreateBuffer(napi_env env, napi_callback_info info);
  static napi_value CreateFramebuffer(napi_env env, napi_callback_info info);
  static napi_value CreateProgram(napi_env env, napi_callback_info info);
  static napi_value CreateRenderbuffer(napi_env env, napi_callback_info info);
  static napi_value CreateShader(napi_env env, napi_callback_info info);
  static napi_value CreateTexture(napi_env env, napi_callback_info info);
  static napi_value CullFace(napi_env env, napi_callback_info info);
  static napi_value DeleteBuffer(napi_env env, napi_callback_info info);
  static napi_value DeleteFramebuffer(napi_env env, napi_callback_info info);
  static napi_value DeleteProgram(napi_env env, napi_callback_info info);
  static napi_value DeleteRenderbuffer(napi_env env, napi_callback_info info);
  static napi_value DeleteShader(napi_env env, napi_callback_info info);
  static napi_value DeleteTexture(napi_env env, napi_callback_info info);
  static napi_value DepthFunc(napi_env env, napi_callback_info info);
  static napi_value DepthMask(napi_env env, napi_callback_info info);
  static napi_value DepthRange(napi_env env, napi_callback_info info);
  static napi_value DetachShader(napi_env env, napi_callback_info info);
  static napi_value Disable(napi_env env, napi_callback_info info);
  static napi_value DisableVertexAttribArray(napi_env env,
                                             napi_callback_info info);
  static napi_value DrawArrays(napi_env env, napi_callback_info info);
  static napi_value DrawElements(napi_env env, napi_callback_info info);
  static napi_value Enable(napi_env env, napi_callback_info info);
  static napi_value EnableVertexAttribArray(napi_env env,
                                            napi_callback_info info);
  static napi_value FenceSynce(napi_env env, napi_callback_info info);
  // TODO(kreeger): Check alignment in CC file here
  static napi_value Finish(napi_env env, napi_callback_info info);
  static napi_value Flush(napi_env env, napi_callback_info info);
  static napi_value FramebufferRenderbuffer(napi_env env,
                                            napi_callback_info info);
  static napi_value FramebufferTexture2D(napi_env env, napi_callback_info info);
  static napi_value FrontFace(napi_env env, napi_callback_info info);
  static napi_value GenerateMipmap(napi_env env, napi_callback_info info);
  static napi_value GetAttachedShaders(napi_env env, napi_callback_info info);
  static napi_value GetAttribLocation(napi_env env, napi_callback_info info);
  static napi_value GetActiveAttrib(napi_env env, napi_callback_info info);
  static napi_value GetActiveUniform(napi_env env, napi_callback_info info);
  static napi_value GetBufferParameter(napi_env env, napi_callback_info info);
  static napi_value GetBufferSubData(napi_env env, napi_callback_info info);
  static napi_value GetContextAttributes(napi_env env, napi_callback_info info);
  static napi_value GetError(napi_env env, napi_callback_info info);
  static napi_value GetFramebufferAttachmentParameter(napi_env env,
                                                      napi_callback_info info);
  static napi_value GetExtension(napi_env env, napi_callback_info info);
  static napi_value GetParameter(napi_env env, napi_callback_info info);
  static napi_value GetProgramInfoLog(napi_env env, napi_callback_info info);
  static napi_value GetProgramParameter(napi_env env, napi_callback_info info);
  static napi_value GetRenderbufferParameter(napi_env env,
                                             napi_callback_info info);
  static napi_value GetShaderPrecisionFormat(napi_env env,
                                             napi_callback_info info);
  static napi_value GetShaderInfoLog(napi_env env, napi_callback_info info);
  static napi_value GetShaderParameter(napi_env env, napi_callback_info info);
  static napi_value GetSupportedExtensions(napi_env env,
                                           napi_callback_info info);
  static napi_value GetTexParameter(napi_env env, napi_callback_info info);
  static napi_value GetUniformLocation(napi_env env, napi_callback_info info);
  static napi_value Hint(napi_env env, napi_callback_info info);
  static napi_value IsBuffer(napi_env env, napi_callback_info info);
  static napi_value IsContextLost(napi_env env, napi_callback_info info);
  static napi_value IsEnabled(napi_env env, napi_callback_info info);
  static napi_value IsFramebuffer(napi_env env, napi_callback_info info);
  static napi_value IsProgram(napi_env env, napi_callback_info info);
  static napi_value IsRenderbuffer(napi_env env, napi_callback_info info);
  static napi_value IsShader(napi_env env, napi_callback_info info);
  static napi_value IsTexture(napi_env env, napi_callback_info info);
  static napi_value LineWidth(napi_env env, napi_callback_info info);
  static napi_value LinkProgram(napi_env env, napi_callback_info info);
  static napi_value PixelStorei(napi_env env, napi_callback_info info);
  static napi_value PolygonOffset(napi_env env, napi_callback_info info);
  static napi_value ReadPixels(napi_env env, napi_callback_info info);
  static napi_value RenderbufferStorage(napi_env env, napi_callback_info info);
  static napi_value SampleCoverage(napi_env env, napi_callback_info info);
  static napi_value Scissor(napi_env env, napi_callback_info info);
  static napi_value ShaderSource(napi_env env, napi_callback_info info);
  static napi_value StencilFunc(napi_env env, napi_callback_info info);
  static napi_value StencilFuncSeparate(napi_env env, napi_callback_info info);
  static napi_value StencilMask(napi_env env, napi_callback_info info);
  static napi_value StencilMaskSeparate(napi_env env, napi_callback_info info);
  static napi_value StencilOp(napi_env env, napi_callback_info info);
  static napi_value StencilOpSeparate(napi_env env, napi_callback_info info);
  static napi_value TexImage2D(napi_env env, napi_callback_info info);
  static napi_value TexParameteri(napi_env env, napi_callback_info info);
  static napi_value TexParameterf(napi_env env, napi_callback_info info);
  static napi_value TexSubImage2D(napi_env env, napi_callback_info info);
  static napi_value Uniform1i(napi_env env, napi_callback_info info);
  static napi_value Uniform1iv(napi_env env, napi_callback_info info);
  static napi_value Uniform1f(napi_env env, napi_callback_info info);
  static napi_value Uniform1fv(napi_env env, napi_callback_info info);
  static napi_value Uniform2i(napi_env env, napi_callback_info info);
  static napi_value Uniform2f(napi_env env, napi_callback_info info);
  static napi_value Uniform2fv(napi_env env, napi_callback_info info);
  static napi_value Uniform2iv(napi_env env, napi_callback_info info);
  static napi_value Uniform3i(napi_env env, napi_callback_info info);
  static napi_value Uniform3iv(napi_env env, napi_callback_info info);
  static napi_value Uniform3f(napi_env env, napi_callback_info info);
  static napi_value Uniform3fv(napi_env env, napi_callback_info info);
  static napi_value Uniform4fv(napi_env env, napi_callback_info info);
  static napi_value Uniform4i(napi_env env, napi_callback_info info);
  static napi_value Uniform4iv(napi_env env, napi_callback_info info);
  static napi_value Uniform4f(napi_env env, napi_callback_info info);
  static napi_value UniformMatrix2fv(napi_env env, napi_callback_info info);
  static napi_value UniformMatrix3fv(napi_env env, napi_callback_info info);
  static napi_value UniformMatrix4fv(napi_env env, napi_callback_info info);
  static napi_value UseProgram(napi_env env, napi_callback_info info);
  static napi_value ValidateProgram(napi_env env, napi_callback_info info);
  static napi_value VertexAttrib1f(napi_env env, napi_callback_info info);
  static napi_value VertexAttrib1fv(napi_env env, napi_callback_info info);
  static napi_value VertexAttrib2f(napi_env env, napi_callback_info info);
  static napi_value VertexAttrib2fv(napi_env env, napi_callback_info info);
  static napi_value VertexAttrib3f(napi_env env, napi_callback_info info);
  static napi_value VertexAttrib3fv(napi_env env, napi_callback_info info);
  static napi_value VertexAttrib4f(napi_env env, napi_callback_info info);
  static napi_value VertexAttrib4fv(napi_env env, napi_callback_info info);
  static napi_value VertexAttribPointer(napi_env env, napi_callback_info info);
  static napi_value Viewport(napi_env env, napi_callback_info info);

  static napi_ref constructor_ref_;

  bool CheckForErrors();

  napi_env env_;
  napi_ref ref_;
  EGLContextWrapper* eglContextWrapper_;

  std::atomic<size_t> alloc_count_;
};

}  // namespace nodejsgl

#endif  // NODEJS_GL_WEBGL_RENDERING_CONTEXT_H_
