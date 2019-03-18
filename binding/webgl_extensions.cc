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

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include "utils.h"

namespace nodejsgl {

// Throws error if extension is not supported:
#ifndef ENSURE_EXTENSION_IS_SUPPORTED
#define ENSURE_EXTENSION_IS_SUPPORTED               \
  if (!IsSupported(egl_context_wrapper)) {          \
    NAPI_THROW_ERROR(env, "Unsupported extension"); \
    return napi_invalid_arg;                        \
  }
#endif

// Returns true if extension is requestable or enabled:
#ifndef IS_EXTENSION_NAME_AVAILABLE
#define IS_EXTENSION_NAME_AVAILABLE(ext_name)                             \
  return egl_context_wrapper->angle_requestable_extensions->HasExtension( \
             ext_name) ||                                                 \
         egl_context_wrapper->gl_extensions->HasExtension(ext_name);
#endif

//==============================================================================
// WebGLExtensionBase

/* static */
napi_value WebGLExtensionBase::InitStubClass(napi_env env,
                                             napi_callback_info info) {
  ENSURE_CONSTRUCTOR_CALL_RETVAL(env, info, nullptr);

  napi_status nstatus;
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, 0, nullptr, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  return js_this;
}

/* static */
napi_status WebGLExtensionBase::NewInstanceBase(napi_env env,
                                                napi_ref constructor_ref,
                                                napi_value* instance) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_get_reference_value(env, constructor_ref, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_new_instance(env, ctor_value, 0, nullptr, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

//==============================================================================
// WebGL_EXTBlendMinmaxExtension

napi_ref WebGL_EXTBlendMinmaxExtension::constructor_ref_;

WebGL_EXTBlendMinmaxExtension::WebGL_EXTBlendMinmaxExtension(napi_env env)
    : WebGLExtensionBase(env) {}

/* static */
bool WebGL_EXTBlendMinmaxExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_blend_minmax");
}

/* static */
napi_status WebGL_EXTBlendMinmaxExtension::Register(napi_env env,
                                                    napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NapiDefineIntProperty(env, GL_MAX_EXT, "MIN_EXT"),
      NapiDefineIntProperty(env, GL_MIN_EXT, "MAX_EXT")};

  napi_value ctor_value;
  nstatus = napi_define_class(env, "EXT_blend_minmax", NAPI_AUTO_LENGTH,
                              WebGLExtensionBase::InitStubClass, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGL_EXTBlendMinmaxExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_EXT_blend_minmax");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// WebGL_EXTFragDepthExtension

napi_ref WebGL_EXTFragDepthExtension::constructor_ref_;

WebGL_EXTFragDepthExtension::WebGL_EXTFragDepthExtension(napi_env env)
    : WebGLExtensionBase(env) {}

/* static */
bool WebGL_EXTFragDepthExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_frag_depth");
}

/* static */
napi_status WebGL_EXTFragDepthExtension::Register(napi_env env,
                                                  napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "EXT_frag_depth", NAPI_AUTO_LENGTH,
                              WebGLExtensionBase::InitStubClass, nullptr, 0,
                              nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGL_EXTFragDepthExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_EXT_frag_depth");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// WebGL_EXTShaderTextureLod

napi_ref WebGL_EXTShaderTextureLod::constructor_ref_;

WebGL_EXTShaderTextureLod::WebGL_EXTShaderTextureLod(napi_env env)
    : WebGLExtensionBase(env) {}

/* static */
bool WebGL_EXTShaderTextureLod::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_shader_texture_lod");
}

/* static */
napi_status WebGL_EXTShaderTextureLod::Register(napi_env env,
                                                napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "EXT_shader_texture_lod", NAPI_AUTO_LENGTH,
                              WebGLExtensionBase::InitStubClass, nullptr, 0,
                              nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGL_EXTShaderTextureLod::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_EXT_shader_texture_lod");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// WebGL_EXTSRGB

napi_ref WebGL_EXTSRGB::constructor_ref_;

WebGL_EXTSRGB::WebGL_EXTSRGB(napi_env env) : WebGLExtensionBase(env) {}

/* static */
bool WebGL_EXTSRGB::IsSupported(EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_sRGB");
}

/* static */
napi_status WebGL_EXTSRGB::Register(napi_env env, napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NapiDefineIntProperty(env, GL_SRGB_EXT, "SRGB_EXT"),
      NapiDefineIntProperty(env, GL_SRGB_ALPHA_EXT, "SRGB_ALPHA_EXT"),
      NapiDefineIntProperty(env, GL_SRGB8_ALPHA8_EXT, "SRGB8_ALPHA8_EXT"),
      NapiDefineIntProperty(env, GL_FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT,
                            "FRAMEBUFFER_ATTACHMENT_COLOR_ENCODING_EXT"),
  };

  napi_value ctor_value;
  nstatus = napi_define_class(env, "EXT_sRGB", NAPI_AUTO_LENGTH,
                              WebGLExtensionBase::InitStubClass, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGL_EXTSRGB::NewInstance(napi_env env, napi_value* instance,
                                       EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_EXT_sRGB");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// WebGL_EXTTextureFilterAnisotropic

napi_ref WebGL_EXTTextureFilterAnisotropic::constructor_ref_;

WebGL_EXTTextureFilterAnisotropic::WebGL_EXTTextureFilterAnisotropic(
    napi_env env)
    : WebGLExtensionBase(env) {}

/* static */
bool WebGL_EXTTextureFilterAnisotropic::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_texture_filter_anisotropic");
}

/* static */
napi_status WebGL_EXTTextureFilterAnisotropic::Register(napi_env env,
                                                        napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NapiDefineIntProperty(env, GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT,
                            "MAX_TEXTURE_MAX_ANISOTROPY_EXT"),
      NapiDefineIntProperty(env, GL_TEXTURE_MAX_ANISOTROPY_EXT,
                            "TEXTURE_MAX_ANISOTROPY_EXT"),
  };

  napi_value ctor_value;
  nstatus =
      napi_define_class(env, "EXT_texture_filter_anisotropic", NAPI_AUTO_LENGTH,
                        WebGLExtensionBase::InitStubClass, nullptr,
                        ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 2, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGL_EXTTextureFilterAnisotropic::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE(
      "GL_EXT_texture_filter_anisotropic");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// WebGL_OESTextureFloatExtension

napi_ref WebGL_OESTextureFloatExtension::constructor_ref_;

WebGL_OESTextureFloatExtension::WebGL_OESTextureFloatExtension(napi_env env)
    : WebGLExtensionBase(env) {}

/* static */
bool WebGL_OESTextureFloatExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_OES_texture_float");
}

/* static */
napi_status WebGL_OESTextureFloatExtension::Register(napi_env env,
                                                     napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "OES_texture_float", NAPI_AUTO_LENGTH,
                              WebGLExtensionBase::InitStubClass, nullptr, 0,
                              nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGL_OESTextureFloatExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_OES_texture_float");
  egl_context_wrapper->glRequestExtensionANGLE(
      "GL_CHROMIUM_color_buffer_float_rgba");
  egl_context_wrapper->glRequestExtensionANGLE(
      "GL_CHROMIUM_color_buffer_float_rgb");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// WebGL_OESTextureHalfFloatExtension

napi_ref WebGL_OESTextureHalfFloatExtension::constructor_ref_;

WebGL_OESTextureHalfFloatExtension::WebGL_OESTextureHalfFloatExtension(
    napi_env env)
    : WebGLExtensionBase(env) {}

/* static */
bool WebGL_OESTextureHalfFloatExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_OES_texture_half_float");
}

/* static */
napi_status WebGL_OESTextureHalfFloatExtension::Register(napi_env env,
                                                         napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NapiDefineIntProperty(env, GL_HALF_FLOAT_OES, "HALF_FLOAT_OES")};

  napi_value ctor_value;
  nstatus = napi_define_class(env, "OES_texture_half_float", NAPI_AUTO_LENGTH,
                              WebGLExtensionBase::InitStubClass, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGL_OESTextureHalfFloatExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_OES_texture_half_float");

  if (egl_context_wrapper->angle_requestable_extensions->HasExtension(
          "GL_EXT_color_buffer_half_float")) {
    egl_context_wrapper->glRequestExtensionANGLE(
        "GL_EXT_color_buffer_half_float");
  }

  egl_context_wrapper->RefreshGLExtensions();
  return napi_ok;
}

//==============================================================================
// WebGL_EXTColorBufferFloat

napi_ref WebGL_EXTColorBufferFloat::constructor_ref_;

WebGL_EXTColorBufferFloat::WebGL_EXTColorBufferFloat(napi_env env)
    : WebGLExtensionBase(env) {}

/* static */
bool WebGL_EXTColorBufferFloat::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_color_buffer_float");
}

/* static */
napi_status WebGL_EXTColorBufferFloat::Register(napi_env env,
                                                napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "EXT_color_buffer_float", NAPI_AUTO_LENGTH,
                              WebGLExtensionBase::InitStubClass, nullptr, 0,
                              nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGL_EXTColorBufferFloat::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_EXT_color_buffer_float");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// WebGL_EXTColorBufferHalfFloat

napi_ref WebGL_EXTColorBufferHalfFloat::constructor_ref_;

WebGL_EXTColorBufferHalfFloat::WebGL_EXTColorBufferHalfFloat(napi_env env)
    : WebGLExtensionBase(env) {}

/* static */
bool WebGL_EXTColorBufferHalfFloat::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_color_buffer_half_float");
}

/* static */
napi_status WebGL_EXTColorBufferHalfFloat::Register(napi_env env,
                                                    napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(
      env, "EXT_color_buffer_half_float", NAPI_AUTO_LENGTH,
      WebGLExtensionBase::InitStubClass, nullptr, 0, nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGL_EXTColorBufferHalfFloat::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE(
      "GL_EXT_color_buffer_half_float");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// WebGL_LoseContextExtension

napi_ref WebGL_LoseContextExtension::constructor_ref_;

/* static */
bool WebGL_LoseContextExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  // This extension is purely a stub in Node - return true.
  return true;
}

/* static */
napi_status WebGL_LoseContextExtension::Register(napi_env env,
                                                 napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NAPI_DEFINE_METHOD("loseContext", LoseContext),
      NAPI_DEFINE_METHOD("restoreContext", RestoreContext),
  };

  napi_value ctor_value;
  nstatus = napi_define_class(env, "WEBGL_lose_context", NAPI_AUTO_LENGTH,
                              WebGL_LoseContextExtension::InitInternal, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static  */
napi_status WebGL_LoseContextExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  return NewInstanceBase(env, constructor_ref_, instance);
}

/* static */
napi_value WebGL_LoseContextExtension::InitInternal(napi_env env,
                                                    napi_callback_info info) {
  napi_status nstatus;

  ENSURE_CONSTRUCTOR_CALL_RETVAL(env, info, nullptr);

  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, 0, nullptr, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGL_LoseContextExtension* context = new WebGL_LoseContextExtension(env);
  ENSURE_VALUE_IS_NOT_NULL_RETVAL(env, context, nullptr);

  nstatus = napi_wrap(env, js_this, context, Cleanup, nullptr, &context->ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  return js_this;
}

/* static */
void WebGL_LoseContextExtension::Cleanup(napi_env env, void* native,
                                         void* hint) {
  WebGL_LoseContextExtension* extension =
      static_cast<WebGL_LoseContextExtension*>(native);
  delete extension;
}

/* static */
napi_value WebGL_LoseContextExtension::LoseContext(napi_env env,
                                                   napi_callback_info info) {
  // Pure stub extension - no-op.
  return nullptr;
}

/* static */
napi_value WebGL_LoseContextExtension::RestoreContext(napi_env env,
                                                      napi_callback_info info) {
  // Pure stub extension - no-op.
  return nullptr;
}

}  // namespace nodejsgl
