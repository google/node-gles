/** * @license
 * Copyright 2019 Google Inc. All Rights Reserved.
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

#include "webgl_sync.h"

#include "utils.h"

namespace nodejsgl {

void Cleanup(napi_env env, void* native, void* hint) {
  GLsync sync = static_cast<GLsync>(native);
  EGLContextWrapper* egl_context_wrapper =
      static_cast<EGLContextWrapper*>(hint);

  egl_context_wrapper->glDeleteSync(sync);
#if DEBUG
  // TODO(kreeger): Fix this.
  /* context->CheckForErrors(); */
#endif
}

napi_status WrapGLsync(napi_env env, GLsync& sync,
                       EGLContextWrapper* egl_context_wrapper,
                       napi_value* wrapped_value) {
  napi_status nstatus;

  nstatus = napi_create_object(env, wrapped_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_wrap(env, *wrapped_value, sync, Cleanup, egl_context_wrapper,
                      nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

}  // namespace nodejsgl
