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

#ifndef NODEJS_GL_WEBGL_EXTENSIONS_H_
#define NODEJS_GL_WEBGL_EXTENSIONS_H_

#include <node_api.h>

#include "egl_context_wrapper.h"

#ifndef NAPI_BOOTSTRAP_METHODS
#define NAPI_BOOTSTRAP_METHODS                                            \
 public:                                                                  \
  static bool IsSupported(EGLContextWrapper* egl_context_wrapper);        \
  static napi_status Register(napi_env env, napi_value exports);          \
  static napi_status NewInstance(napi_env env, napi_value* instance,      \
                                 EGLContextWrapper* egl_context_wrapper); \
                                                                          \
 private:                                                                 \
  static napi_ref constructor_ref_;
#endif

namespace nodejsgl {

// Base class for all GL Extensions
class GLExtensionBase {
  // TODO(kreeger): Need a 'supported' method here!

 protected:
  GLExtensionBase(napi_env env) : env_(env), ref_(nullptr) {}
  virtual ~GLExtensionBase() { napi_delete_reference(env_, ref_); }

  // Returns a default JS object without wrapping a C++ object. Subclasses that
  // don't need to expose any methods should use this.
  static napi_value InitStubClass(napi_env env, napi_callback_info info);

  // Creates a new instance from a constructor ref.
  static napi_status NewInstanceBase(napi_env env, napi_ref constructor_ref,
                                     napi_value* instance);

  napi_env env_;
  napi_ref ref_;
};

// TODO(kreeger): WebGL extensions enable a variety of core ANGLE extensions.
// This directory contains the extensions Chrome enables through WebGL
// extensions:
// https://cs.chromium.org/chromium/src/third_party/blink/renderer/modules/webgl/

// Provides 'GL_ANGLE_instanced_array':
// https://www.khronos.org/registry/OpenGL/extensions/ANGLE/ANGLE_instanced_arrays.txt
class ANGLEInstancedArraysExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  ANGLEInstancedArraysExtension(napi_env env);
  virtual ~ANGLEInstancedArraysExtension() {}
};

// Provides 'EXT_blend_minmax':
// https://www.khronos.org/registry/webgl/extensions/EXT_blend_minmax/
class EXTBlendMinmaxExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  EXTBlendMinmaxExtension(napi_env env);
  virtual ~EXTBlendMinmaxExtension() {}
};

// Provides 'EXT_color_buffer_float':
// https://www.khronos.org/registry/webgl/extensions/EXT_color_buffer_float/
class EXTColorBufferFloatExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  EXTColorBufferFloatExtension(napi_env env);
  virtual ~EXTColorBufferFloatExtension() {}
};

// Provides 'EXT_color_buffer_half_float':
// https://www.khronos.org/registry/webgl/extensions/EXT_color_buffer_half_float/
class EXTColorBufferHalfFloatExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  EXTColorBufferHalfFloatExtension(napi_env env);
  virtual ~EXTColorBufferHalfFloatExtension() {}
};

// Provides 'EXT_frag_depth':
// https://www.khronos.org/registry/webgl/extensions/EXT_frag_depth/
class EXTFragDepthExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  EXTFragDepthExtension(napi_env env);
  virtual ~EXTFragDepthExtension() {}
};

// Provides 'EXT_shader_texture_lod':
// https://www.khronos.org/registry/webgl/extensions/EXT_shader_texture_lod/
class EXTShaderTextureLodExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  EXTShaderTextureLodExtension(napi_env env);
  virtual ~EXTShaderTextureLodExtension() {}
};

// Provides 'EXT_sRGB':
// https://www.khronos.org/registry/webgl/extensions/EXT_sRGB/
class EXTSRGBExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  EXTSRGBExtension(napi_env env);
  virtual ~EXTSRGBExtension() {}
};

// Provides 'EXT_texture_filter_anisotropic'
// https://www.khronos.org/registry/webgl/extensions/EXT_texture_filter_anisotropic/
class EXTTextureFilterAnisotropicExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  EXTTextureFilterAnisotropicExtension(napi_env env);
  virtual ~EXTTextureFilterAnisotropicExtension() {}
};

// Provides 'OES_element_index_uint':
// https://www.khronos.org/registry/webgl/extensions/OES_element_index_uint/
class OESElementIndexUintExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  OESElementIndexUintExtension(napi_env env);
  virtual ~OESElementIndexUintExtension() {}
};

// Provides 'OES_standard_derivatives':
// https://www.khronos.org/registry/webgl/extensions/OES_standard_derivatives/
class OESStandardDerivativesExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  OESStandardDerivativesExtension(napi_env env);
  virtual ~OESStandardDerivativesExtension() {}
};

// Provides 'OES_texture_float':
// https://www.khronos.org/registry/webgl/extensions/OES_texture_float/
class OESTextureFloatExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  OESTextureFloatExtension(napi_env env);
  virtual ~OESTextureFloatExtension() {}
};

// Provides 'OES_texture_float_linear':
// https://www.khronos.org/registry/webgl/extensions/OES_texture_float_linear/
class OESTextureFloatLinearExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  OESTextureFloatLinearExtension(napi_env env);
  virtual ~OESTextureFloatLinearExtension() {}
};

// Provides 'OES_texture_half_float':
// https://www.khronos.org/registry/webgl/extensions/OES_texture_half_float/
class OESTextureHalfFloatExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  OESTextureHalfFloatExtension(napi_env env);
  virtual ~OESTextureHalfFloatExtension() {}
};

// Provides 'OES_texture_half_float_linear':
// https://www.khronos.org/registry/webgl/extensions/OES_texture_half_float_linear/
class OESTextureHalfFloatLinearExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  OESTextureHalfFloatLinearExtension(napi_env env);
  virtual ~OESTextureHalfFloatLinearExtension() {}
};

// Provides 'WEBGL_debug_renderer_info':
// https://www.khronos.org/registry/webgl/extensions/WEBGL_debug_renderer_info/
class WebGLDebugRendererInfoExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  WebGLDebugRendererInfoExtension(napi_env env);
  virtual ~WebGLDebugRendererInfoExtension() {}
};

// Provides 'WEBGL_depth_texture':
// https://www.khronos.org/registry/webgl/extensions/WEBGL_depth_texture/
class WebGLDepthTextureExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  WebGLDepthTextureExtension(napi_env env);
  virtual ~WebGLDepthTextureExtension() {}
};

// Provides the 'WEBGL_lose_context' extension:
// https://www.khronos.org/registry/webgl/extensions/WEBGL_lose_context/
class WebGLLoseContextExtension : public GLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  WebGLLoseContextExtension(napi_env env);
  virtual ~WebGLLoseContextExtension() {}

  // User facing methods:
  static napi_value LoseContext(napi_env env, napi_callback_info info);
  static napi_value RestoreContext(napi_env env, napi_callback_info info);

 private:
  static napi_value InitInternal(napi_env env, napi_callback_info info);
  static void Cleanup(napi_env env, void* native, void* hint);
};

}  // namespace nodejsgl

#endif  // NODEJS_GL_WEBGL_EXTENSIONS_H_
