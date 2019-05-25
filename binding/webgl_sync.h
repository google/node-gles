/**
 * @license
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

#ifndef NODEJS_GL_WEBGL_SYNC_H_
#define NODEJS_GL_WEBGL_SYNC_H_

#include <node_api.h>

#include "angle/include/GLES2/gl2.h"

#include "egl_context_wrapper.h"

namespace nodejsgl {

// Creates and wraps a JS object with a GLsync instance.
napi_status WrapGLsync(napi_env env, GLsync &sync,
                       EGLContextWrapper *egl_context_wrapper,
                       napi_value *wrapped_value);

}  // namespace nodejsgl

#endif  // NODEJS_GL_WEBGL_SYNC_H_
