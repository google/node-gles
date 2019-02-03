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

// TODO(kreeger): WebGL extensions enable a variety of core ANGLE extensions.
// This directory contains the extensions Chrome enables through WebGL
// extensions:
// https://cs.chromium.org/chromium/src/third_party/blink/renderer/modules/webgl/

// Base class for all WebGL Extensions
class WebGLExtensionBase {
  // TODO(kreeger): Need a 'supported' method here!

 protected:
  WebGLExtensionBase(napi_env env) : env_(env), ref_(nullptr) {}
  virtual ~WebGLExtensionBase() { napi_delete_reference(env_, ref_); }

  // Returns a default JS object without wrapping a C++ object. Subclasses that
  // don't need to expose any methods should use this.
  static napi_value InitStubClass(napi_env env, napi_callback_info info);

  // Creates a new instance from a constructor ref.
  static napi_status NewInstanceBase(napi_env env, napi_ref constructor_ref,
                                     napi_value* instance);

  napi_env env_;
  napi_ref ref_;
};

// Provides 'OES_texture_float':
// https://www.khronos.org/registry/webgl/extensions/OES_texture_float/
class WebGL_OESTextureFloatExtension : public WebGLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  WebGL_OESTextureFloatExtension(napi_env env);
  virtual ~WebGL_OESTextureFloatExtension() {}

 private:
  static void Cleanup(napi_env env, void* native,
                      void* hint);  // TODO(kreeger): Remove?
};

// Provides 'OES_texture_half_float':
// https://www.khronos.org/registry/webgl/extensions/OES_texture_half_float/
class WebGL_OESTextureHalfFloatExtension : public WebGLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  WebGL_OESTextureHalfFloatExtension(napi_env env);
  virtual ~WebGL_OESTextureHalfFloatExtension() {}
};

// Provides 'EXT_color_buffer_float':
// https://www.khronos.org/registry/webgl/extensions/EXT_color_buffer_float/
class WebGL_EXTColorBufferFloat : public WebGLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  WebGL_EXTColorBufferFloat(napi_env env);
  virtual ~WebGL_EXTColorBufferFloat() {}
};

// Provides the 'WEBGL_lose_context' extension:
// https://www.khronos.org/registry/webgl/extensions/WEBGL_lose_context/
class WebGL_LoseContextExtension : public WebGLExtensionBase {
  NAPI_BOOTSTRAP_METHODS

 protected:
  WebGL_LoseContextExtension(napi_env env);
  virtual ~WebGL_LoseContextExtension() {}

  // User facing methods:
  static napi_value LoseContext(napi_env env, napi_callback_info info);
  static napi_value RestoreContext(napi_env env, napi_callback_info info);

 private:
  static napi_value InitInternal(napi_env env, napi_callback_info info);
  static void Cleanup(napi_env env, void* native, void* hint);
};

}  // namespace nodejsgl

#endif  // NODEJS_GL_WEBGL_EXTENSIONS_H_
