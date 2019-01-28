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
  static napi_status Register(napi_env env, napi_value exports);          \
  static napi_status NewInstance(napi_env env, napi_value* instance,      \
                                 EGLContextWrapper* egl_context_wrapper); \
                                                                          \
 private:                                                                 \
  static napi_ref constructor_ref_;
#endif

namespace nodejsgl {

// Base class for all WebGL Extensions
class WebGLExtensionBase {
 public:
  WebGLExtensionBase(napi_env env) : env_(env), ref_(nullptr) {}
  virtual ~WebGLExtensionBase() { napi_delete_reference(env_, ref_); }

 protected:
  napi_env env_;
  napi_ref ref_;
};

// Provides 'OES_texture_float':
class WebGL_OESTextureFloatExtension : public WebGLExtensionBase {
 public:
  WebGL_OESTextureFloatExtension(napi_env env);
  virtual ~WebGL_OESTextureFloatExtension();

  NAPI_BOOTSTRAP_METHODS

 private:
  static napi_value InitInternal(napi_env env, napi_callback_info info);
  static void Cleanup(napi_env env, void* native, void* hint);
};

// Provides the 'WEBGL_lose_context' extension:
// https://www.khronos.org/registry/webgl/extensions/WEBGL_lose_context/
class WebGL_LoseContextExtension {
 public:
  static napi_status Register(napi_env env, napi_value exports);
  static napi_status NewInstance(napi_env env, napi_value* instance);

 private:
  WebGL_LoseContextExtension(napi_env env);
  ~WebGL_LoseContextExtension();

  static napi_value InitInternal(napi_env env, napi_callback_info info);
  static void Cleanup(napi_env env, void* native, void* hint);

  // User facing methods:
  static napi_value LoseContext(napi_env env, napi_callback_info info);
  static napi_value RestoreContext(napi_env env, napi_callback_info info);

  static napi_ref constructor_ref_;

  napi_env env_;
  napi_ref ref_;
};

// Provides 'OES_texture_half_float' extension:
// https://www.khronos.org/registry/webgl/extensions/OES_texture_half_float/
class WebGL_OESTextureHalfFloatExtension {
 public:
  static napi_status Register(napi_env env, napi_value exports);
  static napi_status NewInstance(napi_env env, napi_value* instance);

 private:
  WebGL_OESTextureHalfFloatExtension(napi_env env);
  ~WebGL_OESTextureHalfFloatExtension();

  static napi_value InitInternal(napi_env env, napi_callback_info info);
  static void Cleanup(napi_env env, void* native, void* hint);

  static napi_ref constructor_ref_;

  napi_env env_;
  napi_ref ref_;
};

}  // namespace nodejsgl

#endif  // NODEJS_GL_WEBGL_EXTENSIONS_H_
