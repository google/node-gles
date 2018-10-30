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

#include "webgl_extensions.h"

#include "utils.h"

namespace nodejsgl {

napi_ref WebGLLoseContext::constructor_ref_;

WebGLLoseContext::WebGLLoseContext(napi_env env) : env_(env), ref_(nullptr) {}

WebGLLoseContext::~WebGLLoseContext() {
  // TODO(kreeger): Auto-clean this up with inheritance?
  napi_delete_reference(env_, ref_);
}

/* static */
napi_status WebGLLoseContext::Register(napi_env env, napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NAPI_DEFINE_METHOD("loseContext", LoseContext),
      NAPI_DEFINE_METHOD("restoreContext", RestoreContext),
  };

  napi_value ctor_value;
  nstatus = napi_define_class(env, "WEBGL_lose_context", NAPI_AUTO_LENGTH,
                              WebGLLoseContext::InitInternal, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}  // namespace nodejsgl

/* static  */
napi_status WebGLLoseContext::NewInstance(napi_env env, napi_value* instance) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_get_reference_value(env, constructor_ref_, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_new_instance(env, ctor_value, 0, nullptr, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_value WebGLLoseContext::InitInternal(napi_env env,
                                          napi_callback_info info) {
  napi_status nstatus;

  ENSURE_CONSTRUCTOR_CALL_RETVAL(env, info, nullptr);

  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, 0, nullptr, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  // TODO(kreeger): If this is a stub class - can probably just define a JS
  // object with no-op methods.
  WebGLLoseContext* context = new WebGLLoseContext(env);
  ENSURE_VALUE_IS_NOT_NULL_RETVAL(env, context, nullptr);

  nstatus = napi_wrap(env, js_this, context, Cleanup, nullptr, &context->ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  return js_this;
}

/* static */
void WebGLLoseContext::Cleanup(napi_env env, void* native, void* hint) {
  WebGLLoseContext* context = static_cast<WebGLLoseContext*>(native);
  delete context;
}

/* static */
napi_value WebGLLoseContext::LoseContext(napi_env env,
                                         napi_callback_info info) {
  // TODO(kreeger): No-op OK?
  return nullptr;
}

/* static */
napi_value WebGLLoseContext::RestoreContext(napi_env env,
                                            napi_callback_info info) {
  // TODO(kreeger): No-op OK?
  return nullptr;
}

}  // namespace nodejsgl