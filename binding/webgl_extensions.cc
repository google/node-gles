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

#include "angle/include/GLES2/gl2.h"
#include "angle/include/GLES2/gl2ext.h"

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
// GLExtensionBase

/* static */
napi_value GLExtensionBase::InitStubClass(napi_env env,
                                          napi_callback_info info) {
  ENSURE_CONSTRUCTOR_CALL_RETVAL(env, info, nullptr);

  napi_status nstatus;
  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, 0, nullptr, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  return js_this;
}

/* static */
napi_status GLExtensionBase::NewInstanceBase(napi_env env,
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
// ANGLEInstancedArraysExtension

napi_ref ANGLEInstancedArraysExtension::constructor_ref_;

ANGLEInstancedArraysExtension::ANGLEInstancedArraysExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool ANGLEInstancedArraysExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_ANGLE_instanced_arrays");
}

/* static */
napi_status ANGLEInstancedArraysExtension::Register(napi_env env,
                                                    napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "GL_ANGLE_instanced_arrays",
                              NAPI_AUTO_LENGTH, GLExtensionBase::InitStubClass,
                              nullptr, 0, nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status ANGLEInstancedArraysExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_ANGLE_instanced_arrays");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// EXTBlendMinmaxExtension

napi_ref EXTBlendMinmaxExtension::constructor_ref_;

EXTBlendMinmaxExtension::EXTBlendMinmaxExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool EXTBlendMinmaxExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_blend_minmax");
}

/* static */
napi_status EXTBlendMinmaxExtension::Register(napi_env env,
                                              napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NapiDefineIntProperty(env, GL_MAX_EXT, "MAX_EXT"),
      NapiDefineIntProperty(env, GL_MIN_EXT, "MIN_EXT")};

  napi_value ctor_value;
  nstatus = napi_define_class(env, "EXT_blend_minmax", NAPI_AUTO_LENGTH,
                              GLExtensionBase::InitStubClass, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status EXTBlendMinmaxExtension::NewInstance(
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
// EXTColorBufferFloatExtension

napi_ref EXTColorBufferFloatExtension::constructor_ref_;

EXTColorBufferFloatExtension::EXTColorBufferFloatExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool EXTColorBufferFloatExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_color_buffer_float");
}

/* static */
napi_status EXTColorBufferFloatExtension::Register(napi_env env,
                                                   napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "EXT_color_buffer_float", NAPI_AUTO_LENGTH,
                              GLExtensionBase::InitStubClass, nullptr, 0,
                              nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status EXTColorBufferFloatExtension::NewInstance(
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
// EXTColorBufferHalfFloatExtension

napi_ref EXTColorBufferHalfFloatExtension::constructor_ref_;

EXTColorBufferHalfFloatExtension::EXTColorBufferHalfFloatExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool EXTColorBufferHalfFloatExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_color_buffer_half_float");
}

/* static */
napi_status EXTColorBufferHalfFloatExtension::Register(napi_env env,
                                                       napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "EXT_color_buffer_half_float",
                              NAPI_AUTO_LENGTH, GLExtensionBase::InitStubClass,
                              nullptr, 0, nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status EXTColorBufferHalfFloatExtension::NewInstance(
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
// EXTFragDepthExtension

napi_ref EXTFragDepthExtension::constructor_ref_;

EXTFragDepthExtension::EXTFragDepthExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool EXTFragDepthExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_frag_depth");
}

/* static */
napi_status EXTFragDepthExtension::Register(napi_env env, napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "EXT_frag_depth", NAPI_AUTO_LENGTH,
                              GLExtensionBase::InitStubClass, nullptr, 0,
                              nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status EXTFragDepthExtension::NewInstance(
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
// EXTShaderTextureLodExtension

napi_ref EXTShaderTextureLodExtension::constructor_ref_;

EXTShaderTextureLodExtension::EXTShaderTextureLodExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool EXTShaderTextureLodExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_shader_texture_lod");
}

/* static */
napi_status EXTShaderTextureLodExtension::Register(napi_env env,
                                                   napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "EXT_shader_texture_lod", NAPI_AUTO_LENGTH,
                              GLExtensionBase::InitStubClass, nullptr, 0,
                              nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status EXTShaderTextureLodExtension::NewInstance(
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
// EXTSRGBExtension

napi_ref EXTSRGBExtension::constructor_ref_;

EXTSRGBExtension::EXTSRGBExtension(napi_env env) : GLExtensionBase(env) {}

/* static */
bool EXTSRGBExtension::IsSupported(EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_sRGB");
}

/* static */
napi_status EXTSRGBExtension::Register(napi_env env, napi_value exports) {
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
                              GLExtensionBase::InitStubClass, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status EXTSRGBExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_EXT_sRGB");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// EXTTextureFilterAnisotropicExtension

napi_ref EXTTextureFilterAnisotropicExtension::constructor_ref_;

EXTTextureFilterAnisotropicExtension::EXTTextureFilterAnisotropicExtension(
    napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool EXTTextureFilterAnisotropicExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_EXT_texture_filter_anisotropic");
}

/* static */
napi_status EXTTextureFilterAnisotropicExtension::Register(napi_env env,
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
                        GLExtensionBase::InitStubClass, nullptr,
                        ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 2, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status EXTTextureFilterAnisotropicExtension::NewInstance(
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
// OESElementIndexUintExtension

napi_ref OESElementIndexUintExtension::constructor_ref_;

OESElementIndexUintExtension::OESElementIndexUintExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool OESElementIndexUintExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_OES_element_index_uint");
}

/* static */
napi_status OESElementIndexUintExtension::Register(napi_env env,
                                                   napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "OES_element_index_uint", NAPI_AUTO_LENGTH,
                              GLExtensionBase::InitStubClass, nullptr, 0,
                              nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 2, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status OESElementIndexUintExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_OES_element_index_uint");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// OESStandardDerivativesExtension

napi_ref OESStandardDerivativesExtension::constructor_ref_;

OESStandardDerivativesExtension::OESStandardDerivativesExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool OESStandardDerivativesExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_OES_standard_derivatives");
}

/* static */
napi_status OESStandardDerivativesExtension::Register(napi_env env,
                                                      napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NapiDefineIntProperty(env, GL_FRAGMENT_SHADER_DERIVATIVE_HINT_OES,
                            "FRAGMENT_SHADER_DERIVATIVE_HINT_OES")};

  napi_value ctor_value;
  nstatus = napi_define_class(env, "OES_standard_derivatives", NAPI_AUTO_LENGTH,
                              GLExtensionBase::InitStubClass, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 2, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status OESStandardDerivativesExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_OES_standard_derivatives");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// OESTextureFloatExtension

napi_ref OESTextureFloatExtension::constructor_ref_;

OESTextureFloatExtension::OESTextureFloatExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool OESTextureFloatExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_OES_texture_float");
}

/* static */
napi_status OESTextureFloatExtension::Register(napi_env env,
                                               napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "OES_texture_float", NAPI_AUTO_LENGTH,
                              GLExtensionBase::InitStubClass, nullptr, 0,
                              nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status OESTextureFloatExtension::NewInstance(
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
// OESTextureFloatLinearExtension

napi_ref OESTextureFloatLinearExtension::constructor_ref_;

OESTextureFloatLinearExtension::OESTextureFloatLinearExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool OESTextureFloatLinearExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_OES_texture_float_linear");
}

/* static */
napi_status OESTextureFloatLinearExtension::Register(napi_env env,
                                                     napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "OES_texture_float_linear", NAPI_AUTO_LENGTH,
                              GLExtensionBase::InitStubClass, nullptr, 0,
                              nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status OESTextureFloatLinearExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_OES_texture_float_linear");
  egl_context_wrapper->RefreshGLExtensions();
  return napi_ok;
}

//==============================================================================
// OESTextureHalfFloatExtension

napi_ref OESTextureHalfFloatExtension::constructor_ref_;

OESTextureHalfFloatExtension::OESTextureHalfFloatExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool OESTextureHalfFloatExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_OES_texture_half_float");
}

/* static */
napi_status OESTextureHalfFloatExtension::Register(napi_env env,
                                                   napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NapiDefineIntProperty(env, GL_HALF_FLOAT_OES, "HALF_FLOAT_OES")};

  napi_value ctor_value;
  nstatus = napi_define_class(env, "OES_texture_half_float", NAPI_AUTO_LENGTH,
                              GLExtensionBase::InitStubClass, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status OESTextureHalfFloatExtension::NewInstance(
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
// OESTextureHalfFloatLinearExtension

napi_ref OESTextureHalfFloatLinearExtension::constructor_ref_;

OESTextureHalfFloatLinearExtension::OESTextureHalfFloatLinearExtension(
    napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool OESTextureHalfFloatLinearExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_OES_texture_half_float_linear");
}

/* static */
napi_status OESTextureHalfFloatLinearExtension::Register(napi_env env,
                                                         napi_value exports) {
  napi_status nstatus;

  napi_value ctor_value;
  nstatus = napi_define_class(env, "OES_texture_half_float_linear",
                              NAPI_AUTO_LENGTH, GLExtensionBase::InitStubClass,
                              nullptr, 0, nullptr, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status OESTextureHalfFloatLinearExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE(
      "GL_OES_texture_half_float_linear");
  egl_context_wrapper->RefreshGLExtensions();
  return napi_ok;
}

//==============================================================================
// WebGLDebugRendererInfoExtension

napi_ref WebGLDebugRendererInfoExtension::constructor_ref_;

WebGLDebugRendererInfoExtension::WebGLDebugRendererInfoExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool WebGLDebugRendererInfoExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  return true;
}

/* static */
napi_status WebGLDebugRendererInfoExtension::Register(napi_env env,
                                                      napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NapiDefineIntProperty(env, GL_VENDOR, "UNMASKED_VENDOR_WEBGL"),
      NapiDefineIntProperty(env, GL_RENDERER, "UNMASKED_RENDERER_WEBGL")};

  napi_value ctor_value;
  nstatus =
      napi_define_class(env, "WEBGL_debug_renderer_info", NAPI_AUTO_LENGTH,
                        GLExtensionBase::InitStubClass, nullptr,
                        ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGLDebugRendererInfoExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

//==============================================================================
// WebGLDepthTextureExtension

napi_ref WebGLDepthTextureExtension::constructor_ref_;

WebGLDepthTextureExtension::WebGLDepthTextureExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool WebGLDepthTextureExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  IS_EXTENSION_NAME_AVAILABLE("GL_OES_packed_depth_stencil");
  IS_EXTENSION_NAME_AVAILABLE("GL_ANGLE_depth_texture");
}

/* static */
napi_status WebGLDepthTextureExtension::Register(napi_env env,
                                                 napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {NapiDefineIntProperty(
      env, GL_UNSIGNED_INT_24_8_OES, "UNSIGNED_INT_24_8_WEBGL")};

  napi_value ctor_value;
  nstatus = napi_define_class(env, "WEBGL_depth_texture", NAPI_AUTO_LENGTH,
                              GLExtensionBase::InitStubClass, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static */
napi_status WebGLDepthTextureExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  ENSURE_EXTENSION_IS_SUPPORTED

  napi_status nstatus = NewInstanceBase(env, constructor_ref_, instance);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  egl_context_wrapper->glRequestExtensionANGLE("GL_ANGLE_depth_texture");
  egl_context_wrapper->RefreshGLExtensions();

  return napi_ok;
}

//==============================================================================
// WebGLLoseContextExtension

napi_ref WebGLLoseContextExtension::constructor_ref_;

WebGLLoseContextExtension::WebGLLoseContextExtension(napi_env env)
    : GLExtensionBase(env) {}

/* static */
bool WebGLLoseContextExtension::IsSupported(
    EGLContextWrapper* egl_context_wrapper) {
  // This extension is purely a stub in Node - return true.
  return true;
}

/* static */
napi_status WebGLLoseContextExtension::Register(napi_env env,
                                                napi_value exports) {
  napi_status nstatus;

  napi_property_descriptor properties[] = {
      NAPI_DEFINE_METHOD("loseContext", LoseContext),
      NAPI_DEFINE_METHOD("restoreContext", RestoreContext),
  };

  napi_value ctor_value;
  nstatus = napi_define_class(env, "WEBGL_lose_context", NAPI_AUTO_LENGTH,
                              WebGLLoseContextExtension::InitInternal, nullptr,
                              ARRAY_SIZE(properties), properties, &ctor_value);

  nstatus = napi_create_reference(env, ctor_value, 1, &constructor_ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nstatus);

  return napi_ok;
}

/* static  */
napi_status WebGLLoseContextExtension::NewInstance(
    napi_env env, napi_value* instance,
    EGLContextWrapper* egl_context_wrapper) {
  return NewInstanceBase(env, constructor_ref_, instance);
}

/* static */
napi_value WebGLLoseContextExtension::InitInternal(napi_env env,
                                                   napi_callback_info info) {
  napi_status nstatus;

  ENSURE_CONSTRUCTOR_CALL_RETVAL(env, info, nullptr);

  napi_value js_this;
  nstatus = napi_get_cb_info(env, info, 0, nullptr, &js_this, nullptr);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  WebGLLoseContextExtension* context = new WebGLLoseContextExtension(env);
  ENSURE_VALUE_IS_NOT_NULL_RETVAL(env, context, nullptr);

  nstatus = napi_wrap(env, js_this, context, Cleanup, nullptr, &context->ref_);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, nullptr);

  return js_this;
}

/* static */
void WebGLLoseContextExtension::Cleanup(napi_env env, void* native,
                                        void* hint) {
  WebGLLoseContextExtension* extension =
      static_cast<WebGLLoseContextExtension*>(native);
  delete extension;
}

/* static */
napi_value WebGLLoseContextExtension::LoseContext(napi_env env,
                                                  napi_callback_info info) {
  // Pure stub extension - no-op.
  return nullptr;
}

/* static */
napi_value WebGLLoseContextExtension::RestoreContext(napi_env env,
                                                     napi_callback_info info) {
  // Pure stub extension - no-op.
  return nullptr;
}

}  // namespace nodejsgl
