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

#include "webgl_rendering_context.h"

#include "utils.h"
#include "webgl_extensions.h"

// TODO(kreeger): Fix this when angle is linked/imported.
#include <GLES2/gl2.h>
#include <GLES3/gl3.h>
#include <GLES3/gl32.h>

#include <cstring>
#include <iostream>
#include <string>

namespace nodejsgl {

bool WebGLRenderingContext::CheckForErrors() {
  GLenum error;
  bool had_error = false;
  while ((error = eglContextWrapper_->glGetError()) != GL_NO_ERROR) {
    fprintf(stderr, "HAS ERRORS()\n");
    switch (error) {
      case GL_INVALID_ENUM:
        fprintf(stderr, "Found unchecked GL error: GL_INVALID_ENUM\n");
        break;
      case GL_INVALID_VALUE:
        fprintf(stderr, "Found unchecked GL error: GL_INVALID_VALUE\n");
        break;
      case GL_INVALID_OPERATION:
        fprintf(stderr, "Found unchecked GL error: GL_INVALID_OPERATION\n");
        break;
      case GL_INVALID_FRAMEBUFFER_OPERATION:
        fprintf(stderr,
                "Found unchecked GL error: GL_INVALID_FRAMEBUFFER_OPERATION\n");
        break;
      case GL_OUT_OF_MEMORY:
        fprintf(stderr, "Found unchecked GL error: GL_OUT_OF_MEMORY\n");
        break;
      default:
        fprintf(stderr, "Found unchecked GL error: UNKNOWN ERROR\n");
        break;
    }
  }
  return had_error;
}

#define GL_BROWSER_DEFAULT_WEBGL 0x9244
#define GL_CONTEXT_LOST_WEBGL 0x9242
#define GL_UNPACK_COLORSPACE_CONVERSION_WEBGL 0x9243
#define GL_UNPACK_FLIP_Y_WEBGL 0x9240
#define GL_UNPACK_PREMULTIPLY_ALPHA_WEBGL 0x9241

// Returns wrapped context pointer only.
static napi_status GetContext(napi_env env, napi_callback_info info,
                              WebGLRenderingContext **context) {
  napi_status nstatus;

  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, nullptr, nullptr, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  ENSURE_VALUE_IS_OBJECT_RETVAL(env, js_this, napi_invalid_arg);

  nstatus = napi_unwrap(env, js_this, reinterpret_cast<void **>(context));
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

static napi_status UnwrapContext(napi_env env, napi_value js_this,
                                 WebGLRenderingContext **context) {
  ENSURE_VALUE_IS_OBJECT_RETVAL(env, js_this, napi_invalid_arg);
  return napi_unwrap(env, js_this, reinterpret_cast<void **>(context));
}

// Returns wrapped context pointer and uint32_t params.
static napi_status GetContextUint32Params(napi_env env, napi_callback_info info,
                                          WebGLRenderingContext **context,
                                          size_t param_length,
                                          uint32_t *params) {
  napi_status nstatus;

  size_t argc = param_length;
  napi_value args[param_length];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  ENSURE_ARGC_RETVAL(env, argc, param_length, napi_invalid_arg);

  nstatus = UnwrapContext(env, js_this, context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  for (size_t i = 0; i < param_length; ++i) {
    // Null-params get set to 0 in GL world.
    napi_valuetype value_type;
    nstatus = napi_typeof(env, args[i], &value_type);
    ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

    if (value_type == napi_null) {
      params[i] = 0;
    } else if (value_type == napi_number) {
      nstatus = napi_get_value_uint32(env, args[i], &params[i]);
      ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);
    } else {
      ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[i], nstatus);
    }
  }

  return napi_ok;
}

// TODO(kreeger): Cleanup and refactor with the unsigned version.
// Returns wrapped context pointer and uint32_t params.
static napi_status GetContextInt32Params(napi_env env, napi_callback_info info,
                                         WebGLRenderingContext **context,
                                         size_t param_length, int32_t *params) {
  napi_status nstatus;

  size_t argc = param_length;
  napi_value args[param_length];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  ENSURE_ARGC_RETVAL(env, argc, param_length, napi_invalid_arg);

  nstatus = UnwrapContext(env, js_this, context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  for (size_t i = 0; i < param_length; ++i) {
    // Null-params get set to 0 in GL world.
    napi_valuetype value_type;
    nstatus = napi_typeof(env, args[i], &value_type);
    ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

    if (value_type == napi_null) {
      params[i] = 0;
    } else if (value_type == napi_number) {
      nstatus = napi_get_value_int32(env, args[i], &params[i]);
      ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);
    } else {
      ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[i], nstatus);
    }
  }

  return napi_ok;
}

static napi_status GetStringParam(napi_env env, napi_value string_value,
                                  std::string &string) {
  ENSURE_VALUE_IS_STRING_RETVAL(env, string_value, napi_invalid_arg);

  napi_status nstatus;

  size_t str_length;
  nstatus =
      napi_get_value_string_utf8(env, string_value, nullptr, 0, &str_length);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  char buffer[str_length + 1];
  nstatus = napi_get_value_string_utf8(env, string_value, buffer,
                                       str_length + 1, &str_length);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  string.assign(buffer, str_length);
  return napi_ok;
}

napi_ref WebGLRenderingContext::constructor_ref_;

WebGLRenderingContext::WebGLRenderingContext(napi_env env)
    : env_(env), ref_(nullptr) {
  eglContextWrapper_ = EGLContextWrapper::Create(env);
  if (!eglContextWrapper_) {
    NAPI_THROW_ERROR(env, "Could not create EGL context");
    return;
  }
  alloc_count_ = 0;
}

WebGLRenderingContext::~WebGLRenderingContext() {
  if (eglContextWrapper_) {
    delete eglContextWrapper_;
  }

  napi_delete_reference(env_, ref_);
}

/* static */
napi_status WebGLRenderingContext::Register(napi_env env, napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      // WebGL methods:
      // clang-format off
// activeTexture(texture: number): void;
      NAPI_DEFINE_METHOD("attachShader", AttachShader),
// bindAttribLocation(program: WebGLProgram | null, index: number, name: string): void;
      NAPI_DEFINE_METHOD("bindBuffer", BindBuffer),
      NAPI_DEFINE_METHOD("bindFramebuffer", BindFramebuffer),
      NAPI_DEFINE_METHOD("bindRenderbuffer", BindRenderbuffer),
      NAPI_DEFINE_METHOD("bindTexture", BindTexture),
// blendColor(red: number, green: number, blue: number, alpha: number): void;
// blendEquation(mode: number): void;
// blendEquationSeparate(modeRGB: number, modeAlpha: number): void;
// blendFunc(sfactor: number, dfactor: number): void;
// blendFuncSeparate(srcRGB: number, dstRGB: number, srcAlpha: number, dstAlpha: number): void;
      NAPI_DEFINE_METHOD("bufferData", BufferData),
      NAPI_DEFINE_METHOD("bufferSubData", BufferSubData),
// bufferSubData(target: number, offset: number, data: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array | Uint8ClampedArray | Float32Array | Float64Array | DataView | ArrayBuffer | null): void;
      NAPI_DEFINE_METHOD("checkFramebufferStatus", CheckFramebufferStatus),
// clear(mask: number): void;
// clearColor(red: number, green: number, blue: number, alpha: number): void;
// clearDepth(depth: number): void;
// clearStencil(s: number): void;
// colorMask(red: boolean, green: boolean, blue: boolean, alpha: boolean): void;
      NAPI_DEFINE_METHOD("compileShader", CompileShader),
// compressedTexImage2D(target: number, level: number, internalformat: number, width: number, height: number, border: number, data: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array | Uint8ClampedArray | Float32Array | Float64Array | DataView | null): void;
// compressedTexSubImage2D(target: number, level: number, xoffset: number, yoffset: number, width: number, height: number, format: number, data: Int8Array | Int16Array | Int32Array | Uint8Array | Uint16Array | Uint32Array | Uint8ClampedArray | Float32Array | Float64Array | DataView | null): void;
// copyTexImage2D(target: number, level: number, internalformat: number, x: number, y: number, width: number, height: number, border: number): void;
// copyTexSubImage2D(target: number, level: number, xoffset: number, yoffset: number, x: number, y: number, width: number, height: number): void;
      NAPI_DEFINE_METHOD("createBuffer", CreateBuffer),
      NAPI_DEFINE_METHOD("createFramebuffer", CreateFramebuffer),
      NAPI_DEFINE_METHOD("createProgram", CreateProgram),
      NAPI_DEFINE_METHOD("createRenderbuffer", CreateRenderbuffer),
      NAPI_DEFINE_METHOD("createShader", CreateShader),
      NAPI_DEFINE_METHOD("createTexture", CreateTexture),
      NAPI_DEFINE_METHOD("cullFace", CullFace),
      NAPI_DEFINE_METHOD("deleteBuffer", DeleteBuffer),
      NAPI_DEFINE_METHOD("deleteFramebuffer", DeleteFramebuffer),
      NAPI_DEFINE_METHOD("deleteProgram", DeleteProgram),
// deleteRenderbuffker(renderbuffer: WebGLRenderbuffer | null): void;
      NAPI_DEFINE_METHOD("deleteShader", DeleteShader),
      NAPI_DEFINE_METHOD("deleteTexture", DeleteTexture),
// depthFunc(func: number): void;
// depthMask(flag: boolean): void;
// depthRange(zNear: number, zFar: number): void;
// detachShader(program: WebGLProgram | null, shader: WebGLShader | null): void;
      NAPI_DEFINE_METHOD("disable", Disable),
      NAPI_DEFINE_METHOD("disableVertexAttribArray", DisableVertexAttribArray),
      NAPI_DEFINE_METHOD("drawArrays", DrawArrays),
      NAPI_DEFINE_METHOD("drawElements", DrawElements),
      NAPI_DEFINE_METHOD("enable", Enable),
      NAPI_DEFINE_METHOD("enableVertexAttribArray", EnableVertexAttribArray),
      NAPI_DEFINE_METHOD("finish", Finish),
      NAPI_DEFINE_METHOD("flush", Flush),
      NAPI_DEFINE_METHOD("framebufferRenderbuffer", FramebufferRenderbuffer),
      NAPI_DEFINE_METHOD("framebufferTexture2D", FramebufferTexture2D),
// frontFace(mode: number): void;
// generateMipmap(target: number): void;
// getActiveAttrib(program: WebGLProgram | null, index: number): WebGLActiveInfo | null;
// getActiveUniform(program: WebGLProgram | null, index: number): WebGLActiveInfo | null;
// getAttachedShaders(program: WebGLProgram | null): WebGLShader[] | null;
      NAPI_DEFINE_METHOD("getAttribLocation", GetAttribLocation),
// getBufferParameter(target: number, pname: number): any;
// getContextAttributes(): WebGLContextAttributes;
      NAPI_DEFINE_METHOD("getError", GetError),
// getExtension(extensionName: "EXT_blend_minmax"): EXT_blend_minmax | null;
// getExtension(extensionName: "EXT_texture_filter_anisotropic"): EXT_texture_filter_anisotropic | null;
// getExtension(extensionName: "EXT_frag_depth"): EXT_frag_depth | null;
// getExtension(extensionName: "EXT_shader_texture_lod"): EXT_shader_texture_lod | null;
// getExtension(extensionName: "EXT_sRGB"): EXT_sRGB | null;
// getExtension(extensionName: "OES_vertex_array_object"): OES_vertex_array_object | null;
// getExtension(extensionName: "WEBGL_color_buffer_float"): WEBGL_color_buffer_float | null;
// getExtension(extensionName: "WEBGL_compressed_texture_astc"): WEBGL_compressed_texture_astc | null;
// getExtension(extensionName: "WEBGL_compressed_texture_s3tc_srgb"): WEBGL_compressed_texture_s3tc_srgb | null;
// getExtension(extensionName: "WEBGL_debug_shaders"): WEBGL_debug_shaders | null;
// getExtension(extensionName: "WEBGL_draw_buffers"): WEBGL_draw_buffers | null;
// getExtension(extensionName: "WEBGL_lose_context"): WEBGL_lose_context | null;
// getExtension(extensionName: "WEBGL_depth_texture"): WEBGL_depth_texture | null;
// getExtension(extensionName: "WEBGL_debug_renderer_info"): WEBGL_debug_renderer_info | null;
// getExtension(extensionName: "WEBGL_compressed_texture_s3tc"): WEBGL_compressed_texture_s3tc | null;
// getExtension(extensionName: "OES_texture_half_float_linear"): OES_texture_half_float_linear | null;
// getExtension(extensionName: "OES_texture_half_float"): OES_texture_half_float | null;
// getExtension(extensionName: "OES_texture_float_linear"): OES_texture_float_linear | null;
// getExtension(extensionName: "OES_texture_float"): OES_texture_float | null;
// getExtension(extensionName: "OES_standard_derivatives"): OES_standard_derivatives | null;
// getExtension(extensionName: "OES_element_index_uint"): OES_element_index_uint | null;
// getExtension(extensionName: "ANGLE_instanced_arrays"): ANGLE_instanced_arrays | null;
// getExtension(extensionName: string): any;
// getFramebufferAttachmentParameter(target: number, attachment: number, pname: number): any;
      NAPI_DEFINE_METHOD("getExtension", GetExtension),
      NAPI_DEFINE_METHOD("getParameter", GetParameter),
      NAPI_DEFINE_METHOD("getProgramInfoLog", GetProgramInfoLog),
      NAPI_DEFINE_METHOD("getProgramParameter", GetProgramParameter),
// getRenderbufferParameter(target: number, pname: number): any;
      NAPI_DEFINE_METHOD("getShaderInfoLog", GetShaderInfoLog),
      NAPI_DEFINE_METHOD("getShaderParameter", GetShaderParameter),
// getShaderPrecisionFormat(shadertype: number, precisiontype: number): WebGLShaderPrecisionFormat | null;
// getShaderSource(shader: WebGLShader | null): string | null;
// getSupportedExtensions(): string[] | null;
// getTexParameter(target: number, pname: number): any;
// getUniform(program: WebGLProgram | null, location: WebGLUniformLocation | null): any;
      NAPI_DEFINE_METHOD("getUniformLocation", GetUniformLocation),
// getVertexAttrib(index: number, pname: number): any;
// getVertexuniform1iAttribOffset(index: number, pname: number): number;
// hint(target: number, mode: number): void;
// isBuffer(buffer: WebGLBuffer | null): boolean;
// isContextLost(): boolean;
// isEnabled(cap: number): boolean;
// isFramebuffer(framebuffer: WebGLFramebuffer | null): boolean;
// isProgram(program: WebGLProgram | null): boolean;
// isRenderbuffer(renderbuffer: WebGLRenderbuffer | null): boolean;
// isShader(shader: WebGLShader | null): boolean;
// isTexture(texture: WebGLTexture | null): boolean;
// lineWidth(width: number): void;
      NAPI_DEFINE_METHOD("linkProgram", LinkProgram),
// pixelStorei(pname: number, param: number | boolean): void;
// polygonOffset(factor: number, units: number): void;
      NAPI_DEFINE_METHOD("readPixels", ReadPixels),
      NAPI_DEFINE_METHOD("renderbufferStorage", RenderbufferStorage),
// sampleCoverage(value: number, invert: boolean): void;
      NAPI_DEFINE_METHOD("scissor", Scissor),
      NAPI_DEFINE_METHOD("shaderSource", ShaderSource),
// stencilFunc(func: number, ref: number, mask: number): void;
// stencilFuncSeparate(face: number, func: number, ref: number, mask: number): void;
// stencilMask(mask: number): void;
// stencilMaskSeparate(face: number, mask: number): void;
// stencilOp(fail: number, zfail: number, zpass: number): void;
// stencilOpSeparate(face: number, fail: number, zfail: number, zpass: number): void;
// texImage2D(target: number, level: number, internalformat: number, width: number, height: number, border: number, format: number, type: number, pixels: ArrayBufferView | null): void;
// texImage2D(target: number, level: number, internalformat: number, format: number, type: number, pixels: ImageBitmap | ImageData | HTMLVideoElement | HTMLImageElement | HTMLCanvasElement): void;
      NAPI_DEFINE_METHOD("texImage2D", TexImage2D),
// texParameterf(target: number, pname: number, param: number): void;
      NAPI_DEFINE_METHOD("texParameteri", TexParameteri),
// texSubImage2D(target: number, level: number, xoffset: number, yoffset: number, width: number, height: number, format: number, type: number, pixels: ArrayBufferView | null): void;
// texSubImage2D(target: number, level: number, xoffset: number, yoffset: number, format: number, type: number, pixels: ImageBitmap | ImageData | HTMLVideoElement | HTMLImageElement | HTMLCanvasElement): void;
      NAPI_DEFINE_METHOD("texSubImage2D", TexSubImage2D),
// uniform1fv(location: WebGLUniformLocation | null, v: Float32Array | ArrayLike<number>): void;
      NAPI_DEFINE_METHOD("uniform1f", Uniform1f),
      NAPI_DEFINE_METHOD("uniform1fv", Uniform1fv),
      NAPI_DEFINE_METHOD("uniform1i", Uniform1i),
      NAPI_DEFINE_METHOD("uniform2f", Uniform2f),
// uniform1iv(location: WebGLUniformLocation | null, v: Int32Array | ArrayLike<number>): void;
// uniform2fv(location: WebGLUniformLocation | null, v: Float32Array | ArrayLike<number>): void;
      NAPI_DEFINE_METHOD("uniform2i", Uniform2i),
      NAPI_DEFINE_METHOD("uniform2iv", Uniform2iv),
      NAPI_DEFINE_METHOD("uniform3iv", Uniform3iv),
// uniform3f(location: WebGLUniformLocation | null, x: number, y: number, z: number): void;
// uniform3fv(location: WebGLUniformLocation | null, v: Float32Array | ArrayLike<number>): void;
// uniform3i(location: WebGLUniformLocation | null, x: number, y: number, z: number): void;
// uniform4f(location: WebGLUniformLocation | null, x: number, y: number, z: number, w: number): void;
// uniform4fv(location: WebGLUniformLocation | null, v: Float32Array | ArrayLike<number>): void;
      NAPI_DEFINE_METHOD("uniform4fv", Uniform4fv),
      NAPI_DEFINE_METHOD("uniform4i", Uniform4i),
// uniform4iv(location: WebGLUniformLocation | null, v: Int32Array | ArrayLike<number>): void;
// uniformMatrix2fv(location: WebGLUniformLocation | null, transpose: boolean, value: Float32Array | ArrayLike<number>): void;
// uniformMatrix3fv(location: WebGLUniformLocation | null, transpose: boolean, value: Float32Array | ArrayLike<number>): void;
// uniformMatrix4fv(location: WebGLUniformLocation | null, transpose: boolean, value: Float32Array | ArrayLike<number>): void;
      NAPI_DEFINE_METHOD("useProgram", UseProgram),
// validateProgram(program: WebGLProgram | null): void;
// vertexAttrib1f(indx: number, x: number): void;
// vertexAttrib1fv(indx: number, values: Float32Array | number[]): void;
// vertexAttrib2f(indx: number, x: number, y: number): void;
// vertexAttrib2fv(indx: number, values: Float32Array | number[]): void;
// vertexAttrib3f(indx: number, x: number, y: number, z: number): void;
// vertexAttrib3fv(indx: number, values: Float32Array | number[]): void;
// vertexAttrib4f(indx: number, x: number, y: number, z: number, w: number): void;
// vertexAttrib4fv(indx: number, values: Float32Array | number[]): void;
      NAPI_DEFINE_METHOD("vertexAttribPointer", VertexAttribPointer),
      NAPI_DEFINE_METHOD("viewport", Viewport),
      // clang-format on

      // WebGL attributes:
      NapiDefineIntProperty(env, GL_ACTIVE_ATTRIBUTES, "ACTIVE_ATTRIBUTES"),
      NapiDefineIntProperty(env, GL_ACTIVE_TEXTURE, "ACTIVE_TEXTURE"),
      NapiDefineIntProperty(env, GL_ACTIVE_UNIFORMS, "ACTIVE_UNIFORMS"),
      NapiDefineIntProperty(env, GL_ALIASED_LINE_WIDTH_RANGE,
                            "ALIASED_LINE_WIDTH_RANGE"),
      NapiDefineIntProperty(env, GL_ALIASED_POINT_SIZE_RANGE,
                            "ALIASED_POINT_SIZE_RANGE"),
      NapiDefineIntProperty(env, GL_ALPHA, "ALPHA"),
      NapiDefineIntProperty(env, GL_ALPHA_BITS, "ALPHA_BITS"),
      NapiDefineIntProperty(env, GL_ALWAYS, "ALWAYS"),
      NapiDefineIntProperty(env, GL_ARRAY_BUFFER, "ARRAY_BUFFER"),
      NapiDefineIntProperty(env, GL_ARRAY_BUFFER_BINDING,
                            "ARRAY_BUFFER_BINDING"),
      NapiDefineIntProperty(env, GL_ATTACHED_SHADERS, "ATTACHED_SHADERS"),
      NapiDefineIntProperty(env, GL_BACK, "BACK"),
      NapiDefineIntProperty(env, GL_BLEND, "BLEND"),
      NapiDefineIntProperty(env, GL_BLEND_COLOR, "BLEND_COLOR"),
      NapiDefineIntProperty(env, GL_BLEND_DST_ALPHA, "BLEND_DST_ALPHA"),
      NapiDefineIntProperty(env, GL_BLEND_DST_RGB, "BLEND_DST_RGB"),
      NapiDefineIntProperty(env, GL_BLEND_EQUATION, "BLEND_EQUATION"),
      NapiDefineIntProperty(env, GL_BLEND_EQUATION_ALPHA,
                            "BLEND_EQUATION_ALPHA"),
      NapiDefineIntProperty(env, GL_BLEND_EQUATION_RGB, "BLEND_EQUATION_RGB"),
      NapiDefineIntProperty(env, GL_BLEND_SRC_ALPHA, "BLEND_SRC_ALPHA"),
      NapiDefineIntProperty(env, GL_BLEND_SRC_RGB, "BLEND_SRC_RGB"),
      NapiDefineIntProperty(env, GL_BLUE_BITS, "BLUE_BITS"),
      NapiDefineIntProperty(env, GL_BOOL, "BOOL"),
      NapiDefineIntProperty(env, GL_BOOL_VEC2, "BOOL_VEC2"),
      NapiDefineIntProperty(env, GL_BOOL_VEC3, "BOOL_VEC3"),
      NapiDefineIntProperty(env, GL_BOOL_VEC4, "BOOL_VEC4"),
      NapiDefineIntProperty(env, GL_BROWSER_DEFAULT_WEBGL,
                            "BROWSER_DEFAULT_WEBGL"),
      NapiDefineIntProperty(env, GL_BUFFER_SIZE, "BUFFER_SIZE"),
      NapiDefineIntProperty(env, GL_BUFFER_USAGE, "BUFFER_USAGE"),
      NapiDefineIntProperty(env, GL_BYTE, "BYTE"),
      NapiDefineIntProperty(env, GL_CCW, "CCW"),
      NapiDefineIntProperty(env, GL_CLAMP_TO_EDGE, "CLAMP_TO_EDGE"),
      NapiDefineIntProperty(env, GL_COLOR_ATTACHMENT0, "COLOR_ATTACHMENT0"),
      NapiDefineIntProperty(env, GL_COLOR_BUFFER_BIT, "COLOR_BUFFER_BIT"),
      NapiDefineIntProperty(env, GL_COLOR_CLEAR_VALUE, "COLOR_CLEAR_VALUE"),
      NapiDefineIntProperty(env, GL_COLOR_WRITEMASK, "COLOR_WRITEMASK"),
      NapiDefineIntProperty(env, GL_COMPILE_STATUS, "COMPILE_STATUS"),
      NapiDefineIntProperty(env, GL_COMPRESSED_TEXTURE_FORMATS,
                            "COMPRESSED_TEXTURE_FORMATS"),
      NapiDefineIntProperty(env, GL_CONSTANT_ALPHA, "CONSTANT_ALPHA"),
      NapiDefineIntProperty(env, GL_CONSTANT_COLOR, "CONSTANT_COLOR"),
      NapiDefineIntProperty(env, GL_CONTEXT_LOST_WEBGL, "CONTEXT_LOST_WEBGL"),
      NapiDefineIntProperty(env, GL_CULL_FACE, "CULL_FACE"),
      NapiDefineIntProperty(env, GL_CULL_FACE_MODE, "CULL_FACE_MODE"),
      NapiDefineIntProperty(env, GL_CURRENT_PROGRAM, "CURRENT_PROGRAM"),
      NapiDefineIntProperty(env, GL_CURRENT_VERTEX_ATTRIB,
                            "CURRENT_VERTEX_ATTRIB"),
      NapiDefineIntProperty(env, GL_CW, "CW"),
      NapiDefineIntProperty(env, GL_DECR, "DECR"),
      NapiDefineIntProperty(env, GL_DECR_WRAP, "DECR_WRAP"),
      NapiDefineIntProperty(env, GL_DELETE_STATUS, "DELETE_STATUS"),
      NapiDefineIntProperty(env, GL_DEPTH_ATTACHMENT, "DEPTH_ATTACHMENT"),
      NapiDefineIntProperty(env, GL_DEPTH_BITS, "DEPTH_BITS"),
      NapiDefineIntProperty(env, GL_DEPTH_BUFFER_BIT, "DEPTH_BUFFER_BIT"),
      NapiDefineIntProperty(env, GL_DEPTH_CLEAR_VALUE, "DEPTH_CLEAR_VALUE"),
      NapiDefineIntProperty(env, GL_DEPTH_COMPONENT, "DEPTH_COMPONENT"),
      NapiDefineIntProperty(env, GL_DEPTH_COMPONENT16, "DEPTH_COMPONENT16"),
      NapiDefineIntProperty(env, GL_DEPTH_FUNC, "DEPTH_FUNC"),
      NapiDefineIntProperty(env, GL_DEPTH_RANGE, "DEPTH_RANGE"),
      NapiDefineIntProperty(env, GL_DEPTH_STENCIL, "DEPTH_STENCIL"),
      NapiDefineIntProperty(env, GL_DEPTH_STENCIL_ATTACHMENT,
                            "DEPTH_STENCIL_ATTACHMENT"),
      NapiDefineIntProperty(env, GL_DEPTH_TEST, "DEPTH_TEST"),
      NapiDefineIntProperty(env, GL_DEPTH_WRITEMASK, "DEPTH_WRITEMASK"),
      NapiDefineIntProperty(env, GL_DITHER, "DITHER"),
      NapiDefineIntProperty(env, GL_DONT_CARE, "DONT_CARE"),
      NapiDefineIntProperty(env, GL_DST_ALPHA, "DST_ALPHA"),
      NapiDefineIntProperty(env, GL_DST_COLOR, "DST_COLOR"),
      NapiDefineIntProperty(env, GL_DYNAMIC_DRAW, "DYNAMIC_DRAW"),
      NapiDefineIntProperty(env, GL_ELEMENT_ARRAY_BUFFER,
                            "ELEMENT_ARRAY_BUFFER"),
      NapiDefineIntProperty(env, GL_ELEMENT_ARRAY_BUFFER_BINDING,
                            "ELEMENT_ARRAY_BUFFER_BINDING"),
      NapiDefineIntProperty(env, GL_EQUAL, "EQUAL"),
      NapiDefineIntProperty(env, GL_FASTEST, "FASTEST"),
      NapiDefineIntProperty(env, GL_FLOAT, "FLOAT"),
      NapiDefineIntProperty(env, GL_FLOAT_MAT2, "FLOAT_MAT2"),
      NapiDefineIntProperty(env, GL_FLOAT_MAT3, "FLOAT_MAT3"),
      NapiDefineIntProperty(env, GL_FLOAT_MAT4, "FLOAT_MAT4"),
      NapiDefineIntProperty(env, GL_FLOAT_VEC2, "FLOAT_VEC2"),
      NapiDefineIntProperty(env, GL_FLOAT_VEC3, "FLOAT_VEC3"),
      NapiDefineIntProperty(env, GL_FLOAT_VEC4, "FLOAT_VEC4"),
      NapiDefineIntProperty(env, GL_FRAGMENT_SHADER, "FRAGMENT_SHADER"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER, "FRAMEBUFFER"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME,
                            "FRAMEBUFFER_ATTACHMENT_OBJECT_NAME"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
                            "FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE"),
      NapiDefineIntProperty(env,
                            GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE,
                            "FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL,
                            "FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER_BINDING, "FRAMEBUFFER_BINDING"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER_COMPLETE,
                            "FRAMEBUFFER_COMPLETE"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT,
                            "FRAMEBUFFER_INCOMPLETE_ATTACHMENT"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS,
                            "FRAMEBUFFER_INCOMPLETE_DIMENSIONS"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT,
                            "FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER_UNSUPPORTED,
                            "FRAMEBUFFER_UNSUPPORTED"),
      NapiDefineIntProperty(env, GL_FRONT, "FRONT"),
      NapiDefineIntProperty(env, GL_FRONT_AND_BACK, "FRONT_AND_BACK"),
      NapiDefineIntProperty(env, GL_FRONT_FACE, "FRONT_FACE"),
      NapiDefineIntProperty(env, GL_FUNC_ADD, "FUNC_ADD"),
      NapiDefineIntProperty(env, GL_FUNC_REVERSE_SUBTRACT,
                            "FUNC_REVERSE_SUBTRACT"),
      NapiDefineIntProperty(env, GL_FUNC_SUBTRACT, "FUNC_SUBTRACT"),
      NapiDefineIntProperty(env, GL_GENERATE_MIPMAP_HINT,
                            "GENERATE_MIPMAP_HINT"),
      NapiDefineIntProperty(env, GL_GEQUAL, "GEQUAL"),
      NapiDefineIntProperty(env, GL_GREATER, "GREATER"),
      NapiDefineIntProperty(env, GL_GREEN_BITS, "GREEN_BITS"),
      NapiDefineIntProperty(env, GL_HIGH_FLOAT, "HIGH_FLOAT"),
      NapiDefineIntProperty(env, GL_HIGH_INT, "HIGH_INT"),
      NapiDefineIntProperty(env, GL_IMPLEMENTATION_COLOR_READ_FORMAT,
                            "IMPLEMENTATION_COLOR_READ_FORMAT"),
      NapiDefineIntProperty(env, GL_IMPLEMENTATION_COLOR_READ_TYPE,
                            "IMPLEMENTATION_COLOR_READ_TYPE"),
      NapiDefineIntProperty(env, GL_INCR, "INCR"),
      NapiDefineIntProperty(env, GL_INCR_WRAP, "INCR_WRAP"),
      NapiDefineIntProperty(env, GL_INT, "INT"),
      NapiDefineIntProperty(env, GL_INT_VEC2, "INT_VEC2"),
      NapiDefineIntProperty(env, GL_INT_VEC3, "INT_VEC3"),
      NapiDefineIntProperty(env, GL_INT_VEC4, "INT_VEC4"),
      NapiDefineIntProperty(env, GL_INVALID_ENUM, "INVALID_ENUM"),
      NapiDefineIntProperty(env, GL_INVALID_FRAMEBUFFER_OPERATION,
                            "INVALID_FRAMEBUFFER_OPERATION"),
      NapiDefineIntProperty(env, GL_INVALID_OPERATION, "INVALID_OPERATION"),
      NapiDefineIntProperty(env, GL_INVALID_VALUE, "INVALID_VALUE"),
      NapiDefineIntProperty(env, GL_INVERT, "INVERT"),
      NapiDefineIntProperty(env, GL_KEEP, "KEEP"),
      NapiDefineIntProperty(env, GL_LEQUAL, "LEQUAL"),
      NapiDefineIntProperty(env, GL_LESS, "LESS"),
      NapiDefineIntProperty(env, GL_LINEAR, "LINEAR"),
      NapiDefineIntProperty(env, GL_LINEAR_MIPMAP_LINEAR,
                            "LINEAR_MIPMAP_LINEAR"),
      NapiDefineIntProperty(env, GL_LINEAR_MIPMAP_NEAREST,
                            "LINEAR_MIPMAP_NEAREST"),
      NapiDefineIntProperty(env, GL_LINES, "LINES"),
      NapiDefineIntProperty(env, GL_LINE_LOOP, "LINE_LOOP"),
      NapiDefineIntProperty(env, GL_LINE_STRIP, "LINE_STRIP"),
      NapiDefineIntProperty(env, GL_LINE_WIDTH, "LINE_WIDTH"),
      NapiDefineIntProperty(env, GL_LINK_STATUS, "LINK_STATUS"),
      NapiDefineIntProperty(env, GL_LOW_FLOAT, "LOW_FLOAT"),
      NapiDefineIntProperty(env, GL_LOW_INT, "LOW_INT"),
      NapiDefineIntProperty(env, GL_LUMINANCE, "LUMINANCE"),
      NapiDefineIntProperty(env, GL_LUMINANCE_ALPHA, "LUMINANCE_ALPHA"),
      NapiDefineIntProperty(env, GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS,
                            "MAX_COMBINED_TEXTURE_IMAGE_UNITS"),
      NapiDefineIntProperty(env, GL_MAX_CUBE_MAP_TEXTURE_SIZE,
                            "MAX_CUBE_MAP_TEXTURE_SIZE"),
      NapiDefineIntProperty(env, GL_MAX_FRAGMENT_UNIFORM_VECTORS,
                            "MAX_FRAGMENT_UNIFORM_VECTORS"),
      NapiDefineIntProperty(env, GL_MAX_RENDERBUFFER_SIZE,
                            "MAX_RENDERBUFFER_SIZE"),
      NapiDefineIntProperty(env, GL_MAX_TEXTURE_IMAGE_UNITS,
                            "MAX_TEXTURE_IMAGE_UNITS"),
      NapiDefineIntProperty(env, GL_MAX_TEXTURE_SIZE, "MAX_TEXTURE_SIZE"),
      NapiDefineIntProperty(env, GL_MAX_VARYING_VECTORS, "MAX_VARYING_VECTORS"),
      NapiDefineIntProperty(env, GL_MAX_VERTEX_ATTRIBS, "MAX_VERTEX_ATTRIBS"),
      NapiDefineIntProperty(env, GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS,
                            "MAX_VERTEX_TEXTURE_IMAGE_UNITS"),
      NapiDefineIntProperty(env, GL_MAX_VERTEX_UNIFORM_VECTORS,
                            "MAX_VERTEX_UNIFORM_VECTORS"),
      NapiDefineIntProperty(env, GL_MAX_VIEWPORT_DIMS, "MAX_VIEWPORT_DIMS"),
      NapiDefineIntProperty(env, GL_MEDIUM_FLOAT, "MEDIUM_FLOAT"),
      NapiDefineIntProperty(env, GL_MEDIUM_INT, "MEDIUM_INT"),
      NapiDefineIntProperty(env, GL_MIRRORED_REPEAT, "MIRRORED_REPEAT"),
      NapiDefineIntProperty(env, GL_NEAREST, "NEAREST"),
      NapiDefineIntProperty(env, GL_NEAREST_MIPMAP_LINEAR,
                            "NEAREST_MIPMAP_LINEAR"),
      NapiDefineIntProperty(env, GL_NEAREST_MIPMAP_NEAREST,
                            "NEAREST_MIPMAP_NEAREST"),
      NapiDefineIntProperty(env, GL_NEVER, "NEVER"),
      NapiDefineIntProperty(env, GL_NICEST, "NICEST"),
      NapiDefineIntProperty(env, GL_NONE, "NONE"),
      NapiDefineIntProperty(env, GL_NOTEQUAL, "NOTEQUAL"),
      NapiDefineIntProperty(env, GL_NO_ERROR, "NO_ERROR"),
      NapiDefineIntProperty(env, GL_ONE, "ONE"),
      NapiDefineIntProperty(env, GL_ONE_MINUS_CONSTANT_ALPHA,
                            "ONE_MINUS_CONSTANT_ALPHA"),
      NapiDefineIntProperty(env, GL_ONE_MINUS_CONSTANT_COLOR,
                            "ONE_MINUS_CONSTANT_COLOR"),
      NapiDefineIntProperty(env, GL_ONE_MINUS_DST_ALPHA, "ONE_MINUS_DST_ALPHA"),
      NapiDefineIntProperty(env, GL_ONE_MINUS_DST_COLOR, "ONE_MINUS_DST_COLOR"),
      NapiDefineIntProperty(env, GL_ONE_MINUS_SRC_ALPHA, "ONE_MINUS_SRC_ALPHA"),
      NapiDefineIntProperty(env, GL_ONE_MINUS_SRC_COLOR, "ONE_MINUS_SRC_COLOR"),
      NapiDefineIntProperty(env, GL_OUT_OF_MEMORY, "OUT_OF_MEMORY"),
      NapiDefineIntProperty(env, GL_PACK_ALIGNMENT, "PACK_ALIGNMENT"),
      NapiDefineIntProperty(env, GL_POINTS, "POINTS"),
      NapiDefineIntProperty(env, GL_POLYGON_OFFSET_FACTOR,
                            "POLYGON_OFFSET_FACTOR"),
      NapiDefineIntProperty(env, GL_POLYGON_OFFSET_FILL, "POLYGON_OFFSET_FILL"),
      NapiDefineIntProperty(env, GL_POLYGON_OFFSET_UNITS,
                            "POLYGON_OFFSET_UNITS"),
      NapiDefineIntProperty(env, GL_RED_BITS, "RED_BITS"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER, "RENDERBUFFER"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER_ALPHA_SIZE,
                            "RENDERBUFFER_ALPHA_SIZE"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER_BINDING,
                            "RENDERBUFFER_BINDING"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER_BLUE_SIZE,
                            "RENDERBUFFER_BLUE_SIZE"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER_DEPTH_SIZE,
                            "RENDERBUFFER_DEPTH_SIZE"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER_GREEN_SIZE,
                            "RENDERBUFFER_GREEN_SIZE"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER_HEIGHT, "RENDERBUFFER_HEIGHT"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER_INTERNAL_FORMAT,
                            "RENDERBUFFER_INTERNAL_FORMAT"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER_RED_SIZE,
                            "RENDERBUFFER_RED_SIZE"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER_STENCIL_SIZE,
                            "RENDERBUFFER_STENCIL_SIZE"),
      NapiDefineIntProperty(env, GL_RENDERBUFFER_WIDTH, "RENDERBUFFER_WIDTH"),
      NapiDefineIntProperty(env, GL_RENDERER, "RENDERER"),
      NapiDefineIntProperty(env, GL_REPEAT, "REPEAT"),
      NapiDefineIntProperty(env, GL_REPLACE, "REPLACE"),
      NapiDefineIntProperty(env, GL_RGB, "RGB"),
      NapiDefineIntProperty(env, GL_RGB565, "RGB565"),
      NapiDefineIntProperty(env, GL_RGB5_A1, "RGB5_A1"),
      NapiDefineIntProperty(env, GL_RGBA, "RGBA"),
      NapiDefineIntProperty(env, GL_RGBA4, "RGBA4"),
      NapiDefineIntProperty(env, GL_SAMPLER_2D, "SAMPLER_2D"),
      NapiDefineIntProperty(env, GL_SAMPLER_CUBE, "SAMPLER_CUBE"),
      NapiDefineIntProperty(env, GL_SAMPLES, "SAMPLES"),
      NapiDefineIntProperty(env, GL_SAMPLE_ALPHA_TO_COVERAGE,
                            "SAMPLE_ALPHA_TO_COVERAGE"),
      NapiDefineIntProperty(env, GL_SAMPLE_BUFFERS, "SAMPLE_BUFFERS"),
      NapiDefineIntProperty(env, GL_SAMPLE_COVERAGE, "SAMPLE_COVERAGE"),
      NapiDefineIntProperty(env, GL_SAMPLE_COVERAGE_INVERT,
                            "SAMPLE_COVERAGE_INVERT"),
      NapiDefineIntProperty(env, GL_SAMPLE_COVERAGE_VALUE,
                            "SAMPLE_COVERAGE_VALUE"),
      NapiDefineIntProperty(env, GL_SCISSOR_BOX, "SCISSOR_BOX"),
      NapiDefineIntProperty(env, GL_SCISSOR_TEST, "SCISSOR_TEST"),
      NapiDefineIntProperty(env, GL_SHADER_TYPE, "SHADER_TYPE"),
      NapiDefineIntProperty(env, GL_SHADING_LANGUAGE_VERSION,
                            "SHADING_LANGUAGE_VERSION"),
      NapiDefineIntProperty(env, GL_SHORT, "SHORT"),
      NapiDefineIntProperty(env, GL_SRC_ALPHA, "SRC_ALPHA"),
      NapiDefineIntProperty(env, GL_SRC_ALPHA_SATURATE, "SRC_ALPHA_SATURATE"),
      NapiDefineIntProperty(env, GL_SRC_COLOR, "SRC_COLOR"),
      NapiDefineIntProperty(env, GL_STATIC_DRAW, "STATIC_DRAW"),
      NapiDefineIntProperty(env, GL_STENCIL_ATTACHMENT, "STENCIL_ATTACHMENT"),
      NapiDefineIntProperty(env, GL_STENCIL_BACK_FAIL, "STENCIL_BACK_FAIL"),
      NapiDefineIntProperty(env, GL_STENCIL_BACK_FUNC, "STENCIL_BACK_FUNC"),
      NapiDefineIntProperty(env, GL_STENCIL_BACK_PASS_DEPTH_FAIL,
                            "STENCIL_BACK_PASS_DEPTH_FAIL"),
      NapiDefineIntProperty(env, GL_STENCIL_BACK_PASS_DEPTH_PASS,
                            "STENCIL_BACK_PASS_DEPTH_PASS"),
      NapiDefineIntProperty(env, GL_STENCIL_BACK_REF, "STENCIL_BACK_REF"),
      NapiDefineIntProperty(env, GL_STENCIL_BACK_VALUE_MASK,
                            "STENCIL_BACK_VALUE_MASK"),
      NapiDefineIntProperty(env, GL_STENCIL_BACK_WRITEMASK,
                            "STENCIL_BACK_WRITEMASK"),
      NapiDefineIntProperty(env, GL_STENCIL_BITS, "STENCIL_BITS"),
      NapiDefineIntProperty(env, GL_STENCIL_BUFFER_BIT, "STENCIL_BUFFER_BIT"),
      NapiDefineIntProperty(env, GL_STENCIL_CLEAR_VALUE, "STENCIL_CLEAR_VALUE"),
      NapiDefineIntProperty(env, GL_STENCIL_FAIL, "STENCIL_FAIL"),
      NapiDefineIntProperty(env, GL_STENCIL_FUNC, "STENCIL_FUNC"),
      NapiDefineIntProperty(env, GL_STENCIL_INDEX, "STENCIL_INDEX"),
      NapiDefineIntProperty(env, GL_STENCIL_INDEX8, "STENCIL_INDEX8"),
      NapiDefineIntProperty(env, GL_STENCIL_PASS_DEPTH_FAIL,
                            "STENCIL_PASS_DEPTH_FAIL"),
      NapiDefineIntProperty(env, GL_STENCIL_PASS_DEPTH_PASS,
                            "STENCIL_PASS_DEPTH_PASS"),
      NapiDefineIntProperty(env, GL_STENCIL_REF, "STENCIL_REF"),
      NapiDefineIntProperty(env, GL_STENCIL_TEST, "STENCIL_TEST"),
      NapiDefineIntProperty(env, GL_STENCIL_VALUE_MASK, "STENCIL_VALUE_MASK"),
      NapiDefineIntProperty(env, GL_STENCIL_WRITEMASK, "STENCIL_WRITEMASK"),
      NapiDefineIntProperty(env, GL_STREAM_DRAW, "STREAM_DRAW"),
      NapiDefineIntProperty(env, GL_SUBPIXEL_BITS, "SUBPIXEL_BITS"),
      NapiDefineIntProperty(env, GL_TEXTURE, "TEXTURE"),
      NapiDefineIntProperty(env, GL_TEXTURE0, "TEXTURE0"),
      NapiDefineIntProperty(env, GL_TEXTURE1, "TEXTURE1"),
      NapiDefineIntProperty(env, GL_TEXTURE10, "TEXTURE10"),
      NapiDefineIntProperty(env, GL_TEXTURE11, "TEXTURE11"),
      NapiDefineIntProperty(env, GL_TEXTURE12, "TEXTURE12"),
      NapiDefineIntProperty(env, GL_TEXTURE13, "TEXTURE13"),
      NapiDefineIntProperty(env, GL_TEXTURE14, "TEXTURE14"),
      NapiDefineIntProperty(env, GL_TEXTURE15, "TEXTURE15"),
      NapiDefineIntProperty(env, GL_TEXTURE16, "TEXTURE16"),
      NapiDefineIntProperty(env, GL_TEXTURE17, "TEXTURE17"),
      NapiDefineIntProperty(env, GL_TEXTURE18, "TEXTURE1"),
      NapiDefineIntProperty(env, GL_TEXTURE19, "TEXTURE19"),
      NapiDefineIntProperty(env, GL_TEXTURE2, "TEXTURE2"),
      NapiDefineIntProperty(env, GL_TEXTURE20, "TEXTURE20"),
      NapiDefineIntProperty(env, GL_TEXTURE21, "TEXTURE21"),
      NapiDefineIntProperty(env, GL_TEXTURE22, "TEXTURE22"),
      NapiDefineIntProperty(env, GL_TEXTURE23, "TEXTURE23"),
      NapiDefineIntProperty(env, GL_TEXTURE24, "TEXTURE24"),
      NapiDefineIntProperty(env, GL_TEXTURE25, "TEXTURE25"),
      NapiDefineIntProperty(env, GL_TEXTURE26, "TEXTURE26"),
      NapiDefineIntProperty(env, GL_TEXTURE27, "TEXTURE27"),
      NapiDefineIntProperty(env, GL_TEXTURE28, "TEXTURE28"),
      NapiDefineIntProperty(env, GL_TEXTURE29, "TEXTURE29"),
      NapiDefineIntProperty(env, GL_TEXTURE3, "TEXTURE3"),
      NapiDefineIntProperty(env, GL_TEXTURE30, "TEXTURE30"),
      NapiDefineIntProperty(env, GL_TEXTURE31, "TEXTURE31"),
      NapiDefineIntProperty(env, GL_TEXTURE4, "TEXTURE4"),
      NapiDefineIntProperty(env, GL_TEXTURE5, "TEXTURE5"),
      NapiDefineIntProperty(env, GL_TEXTURE6, "TEXTURE6"),
      NapiDefineIntProperty(env, GL_TEXTURE7, "TEXTURE7"),
      NapiDefineIntProperty(env, GL_TEXTURE8, "TEXTURE8"),
      NapiDefineIntProperty(env, GL_TEXTURE9, "TEXTURE9"),
      NapiDefineIntProperty(env, GL_TEXTURE_2D, "TEXTURE_2D"),
      NapiDefineIntProperty(env, GL_TEXTURE_BINDING_2D, "TEXTURE_BINDING_2D"),
      NapiDefineIntProperty(env, GL_TEXTURE_BINDING_CUBE_MAP,
                            "TEXTURE_BINDING_CUBE_MAP"),
      NapiDefineIntProperty(env, GL_TEXTURE_CUBE_MAP, "TEXTURE_CUBE_MAP"),
      NapiDefineIntProperty(env, GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
                            "TEXTURE_CUBE_MAP_NEGATIVE_X"),
      NapiDefineIntProperty(env, GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
                            "TEXTURE_CUBE_MAP_NEGATIVE_Y"),
      NapiDefineIntProperty(env, GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
                            "TEXTURE_CUBE_MAP_NEGATIVE_Z"),
      NapiDefineIntProperty(env, GL_TEXTURE_CUBE_MAP_POSITIVE_X,
                            "TEXTURE_CUBE_MAP_POSITIVE_X"),
      NapiDefineIntProperty(env, GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
                            "TEXTURE_CUBE_MAP_POSITIVE_Y"),
      NapiDefineIntProperty(env, GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
                            "TEXTURE_CUBE_MAP_POSITIVE_Z"),
      NapiDefineIntProperty(env, GL_TEXTURE_MAG_FILTER, "TEXTURE_MAG_FILTER"),
      NapiDefineIntProperty(env, GL_TEXTURE_MIN_FILTER, "TEXTURE_MIN_FILTER"),
      NapiDefineIntProperty(env, GL_TEXTURE_WRAP_S, "TEXTURE_WRAP_S"),
      NapiDefineIntProperty(env, GL_TEXTURE_WRAP_T, "TEXTURE_WRAP_T"),
      NapiDefineIntProperty(env, GL_TRIANGLES, "TRIANGLES"),
      NapiDefineIntProperty(env, GL_TRIANGLE_FAN, "TRIANGLE_FAN"),
      NapiDefineIntProperty(env, GL_TRIANGLE_STRIP, "TRIANGLE_STRIP"),
      NapiDefineIntProperty(env, GL_UNPACK_ALIGNMENT, "UNPACK_ALIGNMENT"),
      NapiDefineIntProperty(env, GL_UNPACK_COLORSPACE_CONVERSION_WEBGL,
                            "UNPACK_COLORSPACE_CONVERSION_WEBGL"),
      NapiDefineIntProperty(env, GL_UNPACK_FLIP_Y_WEBGL, "UNPACK_FLIP_Y_WEBGL"),
      NapiDefineIntProperty(env, GL_UNPACK_PREMULTIPLY_ALPHA_WEBGL,
                            "UNPACK_PREMULTIPLY_ALPHA_WEBGL"),
      NapiDefineIntProperty(env, GL_UNSIGNED_BYTE, "UNSIGNED_BYTE"),
      NapiDefineIntProperty(env, GL_UNSIGNED_INT, "UNSIGNED_INT"),
      NapiDefineIntProperty(env, GL_UNSIGNED_SHORT, "UNSIGNED_SHORT"),
      NapiDefineIntProperty(env, GL_UNSIGNED_SHORT_4_4_4_4,
                            "UNSIGNED_SHORT_4_4_4_4"),
      NapiDefineIntProperty(env, GL_UNSIGNED_SHORT_5_5_5_1,
                            "UNSIGNED_SHORT_5_5_5_1"),
      NapiDefineIntProperty(env, GL_UNSIGNED_SHORT_5_6_5,
                            "UNSIGNED_SHORT_5_6_5"),
      NapiDefineIntProperty(env, GL_VALIDATE_STATUS, "VALIDATE_STATUS"),
      NapiDefineIntProperty(env, GL_VENDOR, "VENDOR"),
      NapiDefineIntProperty(env, GL_VERSION, "VERSION"),
      NapiDefineIntProperty(env, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING,
                            "VERTEX_ATTRIB_ARRAY_BUFFER_BINDING"),
      NapiDefineIntProperty(env, GL_VERTEX_ATTRIB_ARRAY_ENABLED,
                            "VERTEX_ATTRIB_ARRAY_ENABLED"),
      NapiDefineIntProperty(env, GL_VERTEX_ATTRIB_ARRAY_NORMALIZED,
                            "VERTEX_ATTRIB_ARRAY_NORMALIZED"),
      NapiDefineIntProperty(env, GL_VERTEX_ATTRIB_ARRAY_POINTER,
                            "VERTEX_ATTRIB_ARRAY_POINTER"),
      NapiDefineIntProperty(env, GL_VERTEX_ATTRIB_ARRAY_SIZE,
                            "VERTEX_ATTRIB_ARRAY_SIZE"),
      NapiDefineIntProperty(env, GL_VERTEX_ATTRIB_ARRAY_STRIDE,
                            "VERTEX_ATTRIB_ARRAY_STRIDE"),
      NapiDefineIntProperty(env, GL_VERTEX_ATTRIB_ARRAY_TYPE,
                            "VERTEX_ATTRIB_ARRAY_TYPE"),
      NapiDefineIntProperty(env, GL_VERTEX_SHADER, "VERTEX_SHADER"),
      NapiDefineIntProperty(env, GL_VIEWPORT, "VIEWPORT"),
      NapiDefineIntProperty(env, GL_ZERO, "ZERO"),

      // WebGL2 methods:
      NAPI_DEFINE_METHOD("activeTexture", ActiveTexture),

      // WebGL2 attributes:
      NapiDefineIntProperty(env, GL_HALF_FLOAT, "HALF_FLOAT"),
      NapiDefineIntProperty(env, GL_R16F, "R16F"),
      NapiDefineIntProperty(env, GL_R32F, "R32F"),
      NapiDefineIntProperty(env, GL_RGBA32F, "RGBA32F"),
      NapiDefineIntProperty(env, GL_RGBA8, "RGBA8"),
      NapiDefineIntProperty(env, GL_RED, "RED"),
      NapiDefineIntProperty(env, GL_PIXEL_PACK_BUFFER, "PIXEL_PACK_BUFFER"),
  };

  // Create constructor
  napi_value ctor_value;
  nstatus = napi_define_class(env, "WebGLRenderingContext", NAPI_AUTO_LENGTH,
                              WebGLRenderingContext::InitInternal, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  // Bind reference
  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGLRenderingContext::NewInstance(napi_env env,
                                               napi_value *instance) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_get_reference_value(env, constructor_ref_, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_new_instance(env, ctor_value, 0, nullptr, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_value WebGLRenderingContext::InitInternal(napi_env env,
                                               napi_callback_info info) {
  napi_status nstatus;

  ENSURE_CONSTRUCTOR_CALL_RETVAL(env, info, nullptr);

  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, 0, nullptr, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = new WebGLRenderingContext(env);
  ENSURE_VALUE_IS_NOT_NULL_RETVAL(env, context, nullptr);

  nstatus = napi_wrap(env, js_this, context, Cleanup, nullptr, &context->ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  return js_this;
}

/* static */
void WebGLRenderingContext::Cleanup(napi_env env, void *native, void *hint) {
  WebGLRenderingContext *context = static_cast<WebGLRenderingContext *>(native);
  delete context;
}

/** Exported WebGL wrapper methods ********************************************/

/* static */
napi_value WebGLRenderingContext::ActiveTexture(napi_env env,
                                                napi_callback_info info) {
  LOG_CALL("ActiveTexture");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;

  GLenum texture;
  nstatus = GetContextUint32Params(env, info, &context, 1, &texture);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glActiveTexture(texture);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::AttachShader(napi_env env,
                                               napi_callback_info info) {
  LOG_CALL("AttachShader");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  uint32_t args[2];
  nstatus = GetContextUint32Params(env, info, &context, 2, args);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glAttachShader(args[0], args[1]);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::BindBuffer(napi_env env,
                                             napi_callback_info info) {
  LOG_CALL("BindBuffer");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  uint32_t args[2];
  nstatus = GetContextUint32Params(env, info, &context, 2, args);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glBindBuffer(args[0], args[1]);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::BindFramebuffer(napi_env env,
                                                  napi_callback_info info) {
  LOG_CALL("BindFramebuffer");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  uint32_t args[2];
  nstatus = GetContextUint32Params(env, info, &context, 2, args);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glBindFramebuffer(args[0], args[1]);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::BindRenderbuffer(napi_env env,
                                                   napi_callback_info info) {
  LOG_CALL("BindRenderbuffer");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  uint32_t args[2];
  nstatus = GetContextUint32Params(env, info, &context, 2, args);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glBindRenderbuffer(args[0], args[1]);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::BufferData(napi_env env,
                                             napi_callback_info info) {
  LOG_CALL("BufferData");
  napi_status nstatus;

  size_t argc = 3;
  napi_value args[3];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 3, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  GLenum target;
  nstatus = napi_get_value_uint32(env, args[0], &target);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);
  GLenum usage;
  nstatus = napi_get_value_uint32(env, args[2], &usage);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  napi_valuetype arg_valuetype;
  nstatus = napi_typeof(env, args[1], &arg_valuetype);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  if (arg_valuetype == napi_object) {
    bool is_typedarray;
    nstatus = napi_is_typedarray(env, args[1], &is_typedarray);
    ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

    if (is_typedarray) {
      void *data;
      napi_value arraybuffer_value;
      nstatus = napi_get_typedarray_info(env, args[1], nullptr, nullptr, &data,
                                         &arraybuffer_value, nullptr);
      ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

      size_t byte_length = 0;
      nstatus = napi_get_arraybuffer_info(env, arraybuffer_value, nullptr,
                                          &byte_length);
      ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

      context->eglContextWrapper_->glBufferData(target, byte_length, data,
                                                usage);
    } else {
      // TODO(kreeger): Handle this case.
      NAPI_THROW_ERROR(env, "Unsupported data type");
    }
  } else if (arg_valuetype == napi_number) {
    context->eglContextWrapper_->glBufferData(target, 0, nullptr, usage);
  } else {
    NAPI_THROW_ERROR(env, "Invalid argument");
  }

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::BufferSubData(napi_env env,
                                                napi_callback_info info) {
  LOG_CALL("BufferSubData");
  napi_status nstatus;

  size_t argc = 3;
  napi_value args[3];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 3, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  GLenum target;
  nstatus = napi_get_value_uint32(env, args[0], &target);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  // start here
  uint32_t offset;
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  nstatus = napi_get_value_uint32(env, args[1], &offset);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  napi_valuetype arg_valuetype;
  nstatus = napi_typeof(env, args[2], &arg_valuetype);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  if (arg_valuetype == napi_object) {
    bool is_typedarray;
    nstatus = napi_is_typedarray(env, args[2], &is_typedarray);
    ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

    if (is_typedarray) {
      void *data;
      napi_value arraybuffer_value;
      nstatus = napi_get_typedarray_info(env, args[2], nullptr, nullptr, &data,
                                         &arraybuffer_value, nullptr);
      ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

      size_t byte_length = 0;
      nstatus = napi_get_arraybuffer_info(env, arraybuffer_value, nullptr,
                                          &byte_length);
      ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

      context->eglContextWrapper_->glBufferSubData(target, offset, byte_length,
                                                   data);
    } else {
      // TODO(kreeger): Handle this case.
      NAPI_THROW_ERROR(env, "Unsupported data type");
    }
  } else if (arg_valuetype == napi_number) {
    NAPI_THROW_ERROR(env, "Unsupported data type");
  } else {
    NAPI_THROW_ERROR(env, "Invalid argument");
  }

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::BindTexture(napi_env env,
                                              napi_callback_info info) {
  LOG_CALL("BindTexture");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;

  uint32_t args[2];
  nstatus = GetContextUint32Params(env, info, &context, 2, args);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glBindTexture(args[0], args[1]);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::CheckFramebufferStatus(
    napi_env env, napi_callback_info info) {
  LOG_CALL("CheckFramebufferStatus");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  uint32_t arg_value;
  nstatus = GetContextUint32Params(env, info, &context, 1, &arg_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLuint status =
      context->eglContextWrapper_->glCheckFramebufferStatus(arg_value);

  napi_value status_value;
  nstatus = napi_create_uint32(env, status, &status_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return status_value;
}

/* static */
napi_value WebGLRenderingContext::CompileShader(napi_env env,
                                                napi_callback_info info) {
  LOG_CALL("CompileShader");
  WebGLRenderingContext *context = nullptr;
  GLuint shader;
  napi_status nstatus = GetContextUint32Params(env, info, &context, 1, &shader);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glCompileShader(shader);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::CreateBuffer(napi_env env,
                                               napi_callback_info info) {
  LOG_CALL("CreateBuffer");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  nstatus = GetContext(env, info, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLuint buffer;
  context->eglContextWrapper_->glGenBuffers(1, &buffer);

  // TODO(kreeger): Keep track of global objects.
  context->alloc_count_++;

  napi_value buffer_value;
  nstatus = napi_create_uint32(env, buffer, &buffer_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return buffer_value;
}

/* static */
napi_value WebGLRenderingContext::CreateFramebuffer(napi_env env,
                                                    napi_callback_info info) {
  LOG_CALL("CreateFrameBuffer");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  nstatus = GetContext(env, info, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLuint buffer;
  context->eglContextWrapper_->glGenFramebuffers(1, &buffer);

  // TODO(kreeger): Keep track of global objects.
  context->alloc_count_++;

  napi_value frame_buffer_value;
  nstatus = napi_create_uint32(env, buffer, &frame_buffer_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return frame_buffer_value;
}

/* static */
napi_value WebGLRenderingContext::CreateProgram(napi_env env,
                                                napi_callback_info info) {
  LOG_CALL("CreateProgram");
  WebGLRenderingContext *context = nullptr;
  napi_status nstatus = GetContext(env, info, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLuint program = context->eglContextWrapper_->glCreateProgram();

  // TODO(kreeger): Keep track of global objects.
  context->alloc_count_++;

  napi_value program_value;
  nstatus = napi_create_uint32(env, program, &program_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return program_value;
}

/* static */
napi_value WebGLRenderingContext::CreateRenderbuffer(napi_env env,
                                                     napi_callback_info info) {
  LOG_CALL("CreateRenderBuffer");

  WebGLRenderingContext *context = nullptr;
  napi_status nstatus = GetContext(env, info, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLuint renderbuffer;
  context->eglContextWrapper_->glGenRenderbuffers(1, &renderbuffer);

  napi_value renderbuffer_value;
  nstatus = napi_create_uint32(env, renderbuffer, &renderbuffer_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return renderbuffer_value;
}

/* static */
napi_value WebGLRenderingContext::CreateShader(napi_env env,
                                               napi_callback_info info) {
  LOG_CALL("CreateShader");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  GLenum shader_type;
  nstatus = GetContextUint32Params(env, info, &context, 1, &shader_type);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLuint shader = context->eglContextWrapper_->glCreateShader(shader_type);

  // TODO(kreeger): Keep track of global objects.
  context->alloc_count_++;

  napi_value shader_value;
  nstatus = napi_create_uint32(env, shader, &shader_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return shader_value;
}

/* static */
napi_value WebGLRenderingContext::CreateTexture(napi_env env,
                                                napi_callback_info info) {
  LOG_CALL("CreateTexture");

  WebGLRenderingContext *context = nullptr;
  napi_status nstatus = GetContext(env, info, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLuint texture;
  context->eglContextWrapper_->glGenTextures(1, &texture);

  // TODO(kreeger): Keep track of global objects.
  context->alloc_count_++;

  napi_value texture_value;
  nstatus = napi_create_uint32(env, texture, &texture_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return texture_value;
}

/* static */
napi_value WebGLRenderingContext::CullFace(napi_env env,
                                           napi_callback_info info) {
  LOG_CALL("CullFace");

  WebGLRenderingContext *context = nullptr;
  GLenum mode;
  napi_status nstatus = GetContextUint32Params(env, info, &context, 1, &mode);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glCullFace(mode);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::DeleteBuffer(napi_env env,
                                               napi_callback_info info) {
  LOG_CALL("DeleteBuffer");

  WebGLRenderingContext *context = nullptr;
  GLuint buffer;
  napi_status nstatus = GetContextUint32Params(env, info, &context, 1, &buffer);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glDeleteBuffers(1, &buffer);

  // TODO(kreeger): Keep track of global objects.
  context->alloc_count_--;
#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::DeleteProgram(napi_env env,
                                                napi_callback_info info) {
  LOG_CALL("DeleteProgram");

  WebGLRenderingContext *context = nullptr;
  GLuint program;
  napi_status nstatus =
      GetContextUint32Params(env, info, &context, 1, &program);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glDeleteProgram(program);

  // TODO(kreeger): Keep track of global objects.
  context->alloc_count_--;
#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::DeleteFramebuffer(napi_env env,
                                                    napi_callback_info info) {
  LOG_CALL("DeleteFramebuffer");

  WebGLRenderingContext *context = nullptr;
  GLuint frame_buffer;
  napi_status nstatus =
      GetContextUint32Params(env, info, &context, 1, &frame_buffer);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glDeleteFramebuffers(1, &frame_buffer);

  // TODO(kreeger): Keep track of global objects.
  context->alloc_count_--;
#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::DeleteShader(napi_env env,
                                               napi_callback_info info) {
  LOG_CALL("DeleteTexture");

  WebGLRenderingContext *context = nullptr;
  GLuint shader;
  napi_status nstatus = GetContextUint32Params(env, info, &context, 1, &shader);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glDeleteShader(shader);

  // TODO(kreeger): Keep track of global objects.
  context->alloc_count_--;
#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::DeleteTexture(napi_env env,
                                                napi_callback_info info) {
  LOG_CALL("DeleteTexture");

  WebGLRenderingContext *context = nullptr;
  GLuint texture;
  napi_status nstatus =
      GetContextUint32Params(env, info, &context, 1, &texture);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glDeleteTextures(1, &texture);

  // TODO(kreeger): Keep track of global objects.
  context->alloc_count_--;
#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Disable(napi_env env,
                                          napi_callback_info info) {
  LOG_CALL("Disable");

  WebGLRenderingContext *context = nullptr;
  GLenum cap;
  napi_status nstatus = GetContextUint32Params(env, info, &context, 1, &cap);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glDisable(cap);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::DisableVertexAttribArray(
    napi_env env, napi_callback_info info) {
  LOG_CALL("DisableVertexAttribArray");

  WebGLRenderingContext *context = nullptr;
  GLuint index;
  napi_status nstatus = GetContextUint32Params(env, info, &context, 1, &index);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glDisableVertexAttribArray(index);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::DrawArrays(napi_env env,
                                             napi_callback_info info) {
  LOG_CALL("DrawArrays");

  napi_status nstatus;

  size_t argc = 3;
  napi_value args[3];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 3, nullptr);

  for (size_t i = 0; i < 3; i++) {
    ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[i], nullptr);
  }

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum mode;
  nstatus = napi_get_value_uint32(env, args[0], &mode);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint first;
  nstatus = napi_get_value_int32(env, args[1], &first);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei count;
  nstatus = napi_get_value_int32(env, args[2], &count);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glDrawArrays(mode, first, count);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::DrawElements(napi_env env,
                                               napi_callback_info info) {
  LOG_CALL("DrawElements");

  napi_status nstatus;

  size_t argc = 4;
  napi_value args[4];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 4, nullptr);

  for (size_t i = 0; i < 4; i++) {
    ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[i], nullptr);
  }

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum mode;
  nstatus = napi_get_value_uint32(env, args[0], &mode);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei count;
  nstatus = napi_get_value_int32(env, args[1], &count);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum type;
  nstatus = napi_get_value_uint32(env, args[2], &type);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  uint32_t offset;
  nstatus = napi_get_value_uint32(env, args[3], &offset);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glDrawElements(
      mode, count, type, reinterpret_cast<GLvoid *>(offset));

#if DEBUG
  context->CheckForErrors();
#endif

  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Enable(napi_env env,
                                         napi_callback_info info) {
  LOG_CALL("Enable");

  WebGLRenderingContext *context = nullptr;
  GLenum cap;
  napi_status nstatus = GetContextUint32Params(env, info, &context, 1, &cap);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glEnable(cap);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::EnableVertexAttribArray(
    napi_env env, napi_callback_info info) {
  LOG_CALL("EnableVertexAttribArray");

  WebGLRenderingContext *context = nullptr;
  GLenum index;
  napi_status nstatus = GetContextUint32Params(env, info, &context, 1, &index);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glEnableVertexAttribArray(index);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Finish(napi_env env,
                                         napi_callback_info info) {
  LOG_CALL("Finish");

  WebGLRenderingContext *context = nullptr;
  napi_status nstatus = GetContext(env, info, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glFinish();

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::GetError(napi_env env,
                                           napi_callback_info info) {
  LOG_CALL("GetError");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  nstatus = GetContext(env, info, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum error = context->eglContextWrapper_->glGetError();

  napi_value error_value;
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  nstatus = napi_create_uint32(env, error, &error_value);

#if DEBUG
  context->CheckForErrors();
#endif
  return error_value;
}

/* static */
napi_value WebGLRenderingContext::GetExtension(napi_env env,
                                               napi_callback_info info) {
  LOG_CALL("GetExtension");
  napi_status nstatus;

  size_t argc = 1;
  napi_value extension_value;
  napi_value js_this;
  nstatus =
      napi_get_cb_info(env, info, &argc, &extension_value, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 1, nullptr);

  std::string extension_name;
  nstatus = GetStringParam(env, extension_value, extension_name);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  // TODO(kreeger): Add more extension support here.

  if (strcmp(extension_name.c_str(), "EXT_color_buffer_float") == 0) {
    // TODO(kreeger): Determine if we need to actually look up the ability to
    // render a variety of floating point format.
    napi_value stub_object;
    nstatus = napi_create_object(env, &stub_object);
    ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

    return stub_object;
  } else if (strcmp(extension_name.c_str(), "WEBGL_lose_context") == 0) {
    napi_value lose_context_value;
    nstatus = WebGL_LoseContextExtension::NewInstance(env, &lose_context_value);
    ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

    return lose_context_value;
  } else if (strcmp(extension_name.c_str(), "OES_texture_half_float") == 0) {
    napi_value oes_texture_half_float_value;
    nstatus = WebGL_OESTextureHalfFloatExtension::NewInstance(
        env, &oes_texture_half_float_value);
    ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

    return oes_texture_half_float_value;
  }

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::GetParameter(napi_env env,
                                               napi_callback_info info) {
  LOG_CALL("GetParameter");
  napi_status nstatus;

  GLenum name;
  WebGLRenderingContext *context = nullptr;
  nstatus = GetContextUint32Params(env, info, &context, 1, &name);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  switch (name) {
    case GL_MAX_TEXTURE_SIZE:
      GLint params;
      context->eglContextWrapper_->glGetIntegerv(name, &params);

      napi_value params_value;
      nstatus = napi_create_int32(env, params, &params_value);
      ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

      return params_value;

    case GL_VERSION: {
      const GLubyte *str = context->eglContextWrapper_->glGetString(name);
      if (str) {
        const char *str_c_str = reinterpret_cast<const char *>(str);
        napi_value str_value;
        nstatus = napi_create_string_utf8(env, str_c_str, strlen(str_c_str),
                                          &str_value);
        ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

        return str_value;
      }
      break;
    }

    case GL_ARRAY_BUFFER_BINDING: {
      GLint previous_buffer = 0;
      context->eglContextWrapper_->glGetIntegerv(GL_ARRAY_BUFFER_BINDING,
                                                 &previous_buffer);

      napi_value previous_buffer_value;
      nstatus = napi_create_int32(env, previous_buffer, &previous_buffer_value);
      ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

      return previous_buffer_value;
    }

      // TODO(kreeger): Add more of these

    default:
      NAPI_THROW_ERROR(env, "Unsupported getParameter() option");
  }

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::FramebufferRenderbuffer(
    napi_env env, napi_callback_info info) {
  LOG_CALL("FramebufferRenderbuffer");

  napi_status nstatus;

  uint32_t args[4];
  WebGLRenderingContext *context = nullptr;
  nstatus = GetContextUint32Params(env, info, &context, 4, args);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glFramebufferRenderbuffer(args[0], args[1],
                                                         args[2], args[3]);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::FramebufferTexture2D(
    napi_env env, napi_callback_info info) {
  LOG_CALL("FramebufferTexture2D");
  napi_status nstatus;

  size_t argc = 5;
  napi_value args[5];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 5, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[3], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[4], nullptr);

  GLenum target;
  nstatus = napi_get_value_uint32(env, args[0], &target);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum attachment;
  nstatus = napi_get_value_uint32(env, args[1], &attachment);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum textarget;
  nstatus = napi_get_value_uint32(env, args[2], &textarget);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLuint texture;
  nstatus = napi_get_value_uint32(env, args[3], &texture);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint level;
  nstatus = napi_get_value_int32(env, args[4], &level);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glFramebufferTexture2D(
      target, attachment, textarget, texture, level);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Flush(napi_env env, napi_callback_info info) {
  LOG_CALL("Flush");

  WebGLRenderingContext *context = nullptr;
  napi_status nstatus = GetContext(env, info, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glFlush();

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::GetAttribLocation(napi_env env,
                                                    napi_callback_info info) {
  LOG_CALL("GetAttribLocation");
  napi_status nstatus;

  size_t argc = 2;
  napi_value args[2];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 2, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  GLuint program;
  nstatus = napi_get_value_uint32(env, args[0], &program);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  std::string attrib_name;
  nstatus = GetStringParam(env, args[1], attrib_name);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint location = context->eglContextWrapper_->glGetAttribLocation(
      program, attrib_name.c_str());

  napi_value location_value;
  nstatus = napi_create_int32(env, location, &location_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return location_value;
}

/* static */
napi_value WebGLRenderingContext::GetProgramInfoLog(napi_env env,
                                                    napi_callback_info info) {
  LOG_CALL("GetProgramInfoLog");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  GLuint program;
  nstatus = GetContextUint32Params(env, info, &context, 1, &program);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint log_length;
  context->eglContextWrapper_->glGetProgramiv(program, GL_INFO_LOG_LENGTH,
                                              &log_length);

  char *error = new char[log_length + 1];
  context->eglContextWrapper_->glGetProgramInfoLog(program, log_length + 1,
                                                   &log_length, error);

  napi_value error_value;
  nstatus = napi_create_string_utf8(env, error, log_length + 1, &error_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return error_value;
}

/* static */
napi_value WebGLRenderingContext::GetProgramParameter(napi_env env,
                                                      napi_callback_info info) {
  LOG_CALL("GetProgramParameter");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  uint32_t args[2];
  nstatus = GetContextUint32Params(env, info, &context, 2, args);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint param;
  context->eglContextWrapper_->glGetProgramiv(args[0], args[1], &param);

  napi_value param_value;
  nstatus = napi_create_int32(env, param, &param_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return param_value;
}

/* static */
napi_value WebGLRenderingContext::GetShaderInfoLog(napi_env env,
                                                   napi_callback_info info) {
  LOG_CALL("GetShaderInfoLog");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  GLuint shader;
  nstatus = GetContextUint32Params(env, info, &context, 1, &shader);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint log_length;
  context->eglContextWrapper_->glGetShaderiv(shader, GL_INFO_LOG_LENGTH,
                                             &log_length);

  char *error = new char[log_length + 1];
  context->eglContextWrapper_->glGetShaderInfoLog(shader, log_length + 1,
                                                  &log_length, error);

  napi_value error_value;
  nstatus = napi_create_string_utf8(env, error, log_length + 1, &error_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return error_value;
}

/* static */
napi_value WebGLRenderingContext::GetShaderParameter(napi_env env,
                                                     napi_callback_info info) {
  LOG_CALL("GetShaderParameter");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  uint32_t arg_values[2];
  nstatus = GetContextUint32Params(env, info, &context, 2, arg_values);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint value;
  context->eglContextWrapper_->glGetShaderiv(arg_values[0], arg_values[1],
                                             &value);

  napi_value out_value;
  nstatus = napi_create_uint32(env, value, &out_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return out_value;
}

/* static */
napi_value WebGLRenderingContext::GetUniformLocation(napi_env env,
                                                     napi_callback_info info) {
  LOG_CALL("GetUniformLocation");
  napi_status nstatus;

  size_t argc = 2;
  napi_value args[2];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 2, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  GLuint program;
  nstatus = napi_get_value_uint32(env, args[0], &program);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  std::string uniform_name;
  nstatus = GetStringParam(env, args[1], uniform_name);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint location = context->eglContextWrapper_->glGetUniformLocation(
      program, uniform_name.c_str());

  napi_value location_value;
  nstatus = napi_create_int32(env, location, &location_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

#if DEBUG
  context->CheckForErrors();
#endif
  return location_value;
}

/* static */
napi_value WebGLRenderingContext::LinkProgram(napi_env env,
                                              napi_callback_info info) {
  LOG_CALL("LinkProgram");

  WebGLRenderingContext *context = nullptr;
  GLuint program;
  napi_status nstatus =
      GetContextUint32Params(env, info, &context, 1, &program);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glLinkProgram(program);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::ReadPixels(napi_env env,
                                             napi_callback_info info) {
  LOG_CALL("ReadPixels");

  napi_status nstatus;

  size_t argc = 7;
  napi_value args[7];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 7, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[3], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[4], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[5], nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint x;
  nstatus = napi_get_value_int32(env, args[0], &x);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint y;
  nstatus = napi_get_value_int32(env, args[1], &y);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei width;
  nstatus = napi_get_value_int32(env, args[2], &width);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei height;
  nstatus = napi_get_value_int32(env, args[3], &height);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum format;
  nstatus = napi_get_value_uint32(env, args[4], &format);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum type;
  nstatus = napi_get_value_uint32(env, args[5], &type);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  size_t length = 0;
  void *buffer = nullptr;

  napi_valuetype valuetype;
  napi_typeof(env, args[6], &valuetype);

  if (valuetype == napi_object) {
    nstatus = napi_get_typedarray_info(env, args[6], nullptr, &length, &buffer,
                                       nullptr, nullptr);
    ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  } else if (valuetype == napi_number) {
    // No op for now.
    // TODO(kreeger): Determine how to handle this.
  } else {
    fprintf(stderr, "---> Object type: %u\n", valuetype);
    NAPI_THROW_ERROR(env, "Unsupported object type");
    return nullptr;
  }

  context->eglContextWrapper_->glReadPixels(x, y, width, height, format, type,
                                            buffer);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::RenderbufferStorage(napi_env env,
                                                      napi_callback_info info) {
  LOG_CALL("RenderbufferStorage");

  napi_status nstatus;

  size_t argc = 4;
  napi_value args[4];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 4, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[3], nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum target;
  nstatus = napi_get_value_uint32(env, args[0], &target);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum internal_format;
  nstatus = napi_get_value_uint32(env, args[1], &internal_format);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei width;
  nstatus = napi_get_value_int32(env, args[2], &width);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei height;
  nstatus = napi_get_value_int32(env, args[3], &height);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glRenderbufferStorage(target, internal_format,
                                                     width, height);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Scissor(napi_env env,
                                          napi_callback_info info) {
  LOG_CALL("Scissor");
  napi_status nstatus;

  size_t argc = 4;
  napi_value args[4];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 4, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[3], nullptr);

  GLint x;
  nstatus = napi_get_value_int32(env, args[0], &x);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint y;
  nstatus = napi_get_value_int32(env, args[1], &y);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei width;
  nstatus = napi_get_value_int32(env, args[2], &width);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei height;
  nstatus = napi_get_value_int32(env, args[3], &height);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glScissor(x, y, width, height);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::TexImage2D(napi_env env,
                                             napi_callback_info info) {
  LOG_CALL("TexImage2D");

  napi_status nstatus;

  size_t argc = 9;
  napi_value args[9];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 9, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[3], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[4], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[5], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[6], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[7], nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum target;
  nstatus = napi_get_value_uint32(env, args[0], &target);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint level;
  nstatus = napi_get_value_int32(env, args[1], &level);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum internal_format;
  nstatus = napi_get_value_uint32(env, args[2], &internal_format);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei width;
  nstatus = napi_get_value_int32(env, args[3], &width);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei height;
  nstatus = napi_get_value_int32(env, args[4], &height);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint border;
  nstatus = napi_get_value_int32(env, args[5], &border);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum format;
  nstatus = napi_get_value_uint32(env, args[6], &format);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint type;
  nstatus = napi_get_value_int32(env, args[7], &type);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  napi_valuetype value_type;
  nstatus = napi_typeof(env, args[8], &value_type);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  if (value_type == napi_null) {
    context->eglContextWrapper_->glTexImage2D(target, level, internal_format,
                                              width, height, border, format,
                                              type, nullptr);
  } else {
    void *data = nullptr;
    nstatus = napi_get_typedarray_info(env, args[8], nullptr, nullptr, &data,
                                       nullptr, nullptr);
    ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

    context->eglContextWrapper_->glTexImage2D(target, level, internal_format,
                                              width, height, border, format,
                                              type, data);
  }

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::ShaderSource(napi_env env,
                                               napi_callback_info info) {
  LOG_CALL("ShaderSource");
  napi_status nstatus;

  size_t argc = 2;
  napi_value args[2];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 2, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  GLuint shader;
  nstatus = napi_get_value_uint32(env, args[0], &shader);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  std::string source;
  nstatus = GetStringParam(env, args[1], source);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint length = source.size();
  const char *codes[] = {source.c_str()};
  context->eglContextWrapper_->glShaderSource(shader, 1, codes, &length);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::TexParameteri(napi_env env,
                                                napi_callback_info info) {
  LOG_CALL("TexParameteri");
  napi_status nstatus;

  size_t argc = 3;
  napi_value args[3];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 3, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);

  GLenum target;
  nstatus = napi_get_value_uint32(env, args[0], &target);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum pname;
  nstatus = napi_get_value_uint32(env, args[1], &pname);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint param;
  nstatus = napi_get_value_int32(env, args[2], &param);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glTexParameteri(target, pname, param);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

napi_value WebGLRenderingContext::TexSubImage2D(napi_env env,
                                                napi_callback_info info) {
  LOG_CALL("TexSubImage2D");
  napi_status nstatus;

  size_t argc = 9;
  napi_value args[9];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 9, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[3], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[4], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[5], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[6], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[7], nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum target;
  nstatus = napi_get_value_uint32(env, args[0], &target);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint level;
  nstatus = napi_get_value_int32(env, args[1], &level);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint xoffset;
  nstatus = napi_get_value_int32(env, args[2], &xoffset);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint yoffset;
  nstatus = napi_get_value_int32(env, args[3], &yoffset);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei width;
  nstatus = napi_get_value_int32(env, args[4], &width);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei height;
  nstatus = napi_get_value_int32(env, args[5], &height);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum format;
  nstatus = napi_get_value_uint32(env, args[6], &format);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLenum type;
  nstatus = napi_get_value_uint32(env, args[7], &type);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  napi_valuetype value_type;
  nstatus = napi_typeof(env, args[8], &value_type);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  if (value_type == napi_null) {
    // TODO(kreeger): Handle this case.
    NAPI_THROW_ERROR(env, "UNIMPLEMENTED");
  } else {
    // Use byte offset
    size_t data_length;
    void *data = nullptr;
    nstatus = napi_get_typedarray_info(env, args[8], nullptr, &data_length,
                                       &data, nullptr, nullptr);
    ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

    context->eglContextWrapper_->glTexSubImage2D(
        target, level, xoffset, yoffset, width, height, format, type, data);
  }

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Uniform1i(napi_env env,
                                            napi_callback_info info) {
  LOG_CALL("Uniform1i");
  napi_status nstatus;

  size_t argc = 2;
  napi_value args[2];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 2, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);

  GLint location;
  nstatus = napi_get_value_int32(env, args[0], &location);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint v0;
  nstatus = napi_get_value_int32(env, args[1], &v0);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glUniform1i(location, v0);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Uniform1f(napi_env env,
                                            napi_callback_info info) {
  LOG_CALL("Uniform1f");
  napi_status nstatus;

  size_t argc = 2;
  napi_value args[2];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 2, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);

  GLint location;
  nstatus = napi_get_value_int32(env, args[0], &location);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  double v0;
  nstatus = napi_get_value_double(env, args[1], &v0);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glUniform1f(location, static_cast<GLfloat>(v0));

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Uniform1fv(napi_env env,
                                             napi_callback_info info) {
  LOG_CALL("Uniform1fv");
  napi_status nstatus;

  size_t argc = 2;
  napi_value args[2];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 2, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);

  GLint location;
  nstatus = napi_get_value_int32(env, args[0], &location);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  size_t size;
  void *data;
  nstatus = napi_get_typedarray_info(env, args[1], nullptr, &size, &data,
                                     nullptr, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glUniform1fv(location, size,
                                            reinterpret_cast<GLfloat *>(data));

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Uniform2f(napi_env env,
                                            napi_callback_info info) {
  LOG_CALL("Uniform2f");
  napi_status nstatus;

  size_t argc = 3;
  napi_value args[3];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 3, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);

  GLint location;
  nstatus = napi_get_value_int32(env, args[0], &location);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  double v0;
  nstatus = napi_get_value_double(env, args[1], &v0);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  double v1;
  nstatus = napi_get_value_double(env, args[2], &v1);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glUniform2f(location, static_cast<GLfloat>(v0),
                                           static_cast<GLfloat>(v1));

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Uniform2i(napi_env env,
                                            napi_callback_info info) {
  LOG_CALL("Uniform2i");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  int32_t args[3];
  nstatus = GetContextInt32Params(env, info, &context, 3, args);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glUniform2i(args[0], args[1], args[2]);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Uniform2iv(napi_env env,
                                             napi_callback_info info) {
  LOG_CALL("Uniform2iv");
  napi_status nstatus;

  size_t argc = 2;
  napi_value args[2];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 2, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);

  GLint location;
  nstatus = napi_get_value_int32(env, args[0], &location);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  size_t size;
  void *data;
  nstatus = napi_get_typedarray_info(env, args[1], nullptr, &size, &data,
                                     nullptr, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glUniform2iv(location, 1,
                                            reinterpret_cast<GLint *>(data));

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Uniform3iv(napi_env env,
                                             napi_callback_info info) {
  LOG_CALL("Uniform3iv");
  napi_status nstatus;

  size_t argc = 2;
  napi_value args[2];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 2, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);

  GLint location;
  nstatus = napi_get_value_int32(env, args[0], &location);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  size_t size;
  void *data;
  nstatus = napi_get_typedarray_info(env, args[1], nullptr, &size, &data,
                                     nullptr, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glUniform3iv(location, 1,
                                            reinterpret_cast<GLint *>(data));

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Uniform4fv(napi_env env,
                                             napi_callback_info info) {
  LOG_CALL("Uniform4fv");
  napi_status nstatus;

  size_t argc = 2;
  napi_value args[2];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 2, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);

  GLint location;
  nstatus = napi_get_value_int32(env, args[0], &location);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  size_t size;
  void *data;
  nstatus = napi_get_typedarray_info(env, args[1], nullptr, &size, &data,
                                     nullptr, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glUniform4fv(location, 1,
                                            reinterpret_cast<GLfloat *>(data));

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Uniform4i(napi_env env,
                                            napi_callback_info info) {
  LOG_CALL("Uniform4i");
  napi_status nstatus;

  WebGLRenderingContext *context = nullptr;
  int32_t args[5];
  nstatus = GetContextInt32Params(env, info, &context, 5, args);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glUniform4i(args[0], args[1], args[2], args[3],
                                           args[4]);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::UseProgram(napi_env env,
                                             napi_callback_info info) {
  LOG_CALL("UseProgram");

  WebGLRenderingContext *context = nullptr;
  GLuint program;
  napi_status nstatus =
      GetContextUint32Params(env, info, &context, 1, &program);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glUseProgram(program);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::VertexAttribPointer(napi_env env,
                                                      napi_callback_info info) {
  LOG_CALL("VertexAttribPointer");
  napi_status nstatus;

  size_t argc = 6;
  napi_value args[6];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 6, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  uint32_t index;
  nstatus = napi_get_value_uint32(env, args[0], &index);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  GLint size;
  nstatus = napi_get_value_int32(env, args[1], &size);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);
  GLenum type;
  nstatus = napi_get_value_uint32(env, args[2], &type);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  ENSURE_VALUE_IS_BOOLEAN_RETVAL(env, args[3], nullptr);
  bool normalized;
  nstatus = napi_get_value_bool(env, args[3], &normalized);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[4], nullptr);
  GLsizei stride;
  nstatus = napi_get_value_int32(env, args[4], &stride);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[5], nullptr);
  uint32_t offset;
  nstatus = napi_get_value_uint32(env, args[5], &offset);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glVertexAttribPointer(
      index, size, type, normalized, stride,
      reinterpret_cast<GLvoid *>(offset));

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

/* static */
napi_value WebGLRenderingContext::Viewport(napi_env env,
                                           napi_callback_info info) {
  LOG_CALL("Viewport");
  napi_status nstatus;

  size_t argc = 4;
  napi_value args[4];
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, &argc, args, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);
  ENSURE_ARGC_RETVAL(env, argc, 4, nullptr);

  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[0], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[1], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[2], nullptr);
  ENSURE_VALUE_IS_NUMBER_RETVAL(env, args[3], nullptr);

  GLint x;
  nstatus = napi_get_value_int32(env, args[0], &x);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLint y;
  nstatus = napi_get_value_int32(env, args[1], &y);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei width;
  nstatus = napi_get_value_int32(env, args[2], &width);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  GLsizei height;
  nstatus = napi_get_value_int32(env, args[3], &height);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLRenderingContext *context = nullptr;
  nstatus = UnwrapContext(env, js_this, &context);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  context->eglContextWrapper_->glViewport(x, y, width, height);

#if DEBUG
  context->CheckForErrors();
#endif
  return nullptr;
}

}  // namespace nodejsgl
