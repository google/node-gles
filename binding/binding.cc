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

#include <node_api.h>

#include "utils.h"
#include "webgl_extensions.h"
#include "webgl_rendering_context.h"

namespace nodejsgl {

static napi_value CreateWebGLRenderingContext(napi_env env,
                                              napi_callback_info info) {
  napi_value instance;
  napi_status nstatus = WebGLRenderingContext::NewInstance(env, &instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  return instance;
}

static napi_value InitBinding(napi_env env, napi_value exports) {
  WebGL_EXTBlendMinmaxExtension::Register(env, exports);
  WebGL_EXTColorBufferFloat::Register(env, exports);
  WebGL_EXTColorBufferHalfFloat::Register(env, exports);
  WebGL_EXTFragDepthExtension::Register(env, exports);
  WebGL_EXTTextureFilterAnisotropic::Register(env, exports);
  WebGL_LoseContextExtension::Register(env, exports);
  WebGL_OESTextureFloatExtension::Register(env, exports);
  WebGL_OESTextureHalfFloatExtension::Register(env, exports);
  WebGLRenderingContext::Register(env, exports);

  napi_property_descriptor properties[] = {
      NAPI_DEFINE_METHOD("createWebGLRenderingContext",
                         CreateWebGLRenderingContext),
  };

  napi_status nstatus =
      napi_define_properties(env, exports, ARRAY_SIZE(properties), properties);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  return exports;
}

NAPI_MODULE(nodejs_gl_binding, InitBinding)

}  // namespace nodejsgl
