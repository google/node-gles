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

#ifndef NODEJS_GL_UTILS_H_
#define NODEJS_GL_UTILS_H_

#include <node_api.h>
#include <stdio.h>
#include <sstream>

#define ARRAY_SIZE(array) (sizeof(array) / sizeof(array[0]))

#define NAPI_STRING_SIZE 512

#ifndef DEBUG
#define DEBUG 0
#endif

#define LOG_CALLS 0

#define DEBUG_LOG(message, file, lineNumber)                             \
  do {                                                                   \
    if (DEBUG)                                                           \
      fprintf(stderr, "** -%s:%zu\n-- %s\n", file, lineNumber, message); \
  } while (0)

#define LOG_CALL(message)                                \
  do {                                                   \
    if (LOG_CALLS) fprintf(stderr, "  - %s\n", message); \
  } while (0)

#define NAPI_THROW_ERROR(env, message) \
  NapiThrowError(env, message, __FILE__, __LINE__);

inline void NapiThrowError(napi_env env, const char* message, const char* file,
                           const size_t lineNumber) {
  DEBUG_LOG(message, file, lineNumber);
  napi_throw_error(env, nullptr, message);
}

#define ENSURE_NAPI_OK(env, status) \
  if (!EnsureNapiOK(env, status, __FILE__, __LINE__)) return;
#define ENSURE_NAPI_OK_RETVAL(env, status, retval) \
  if (!EnsureNapiOK(env, status, __FILE__, __LINE__)) return retval;

inline bool EnsureNapiOK(napi_env env, napi_status status, const char* file,
                         const size_t lineNumber) {
  if (status != napi_ok) {
    const napi_extended_error_info* error_info = 0;
    napi_get_last_error_info(env, &error_info);

    std::ostringstream oss;
    if (error_info->error_message) {
      oss << "Invalid napi_status: " << error_info->error_message;
    } else {
      oss << "Invalid napi_status: UNKNOWN";
    }
    NapiThrowError(env, oss.str().c_str(), file, lineNumber);
  }
  return status == napi_ok;
}

#define ENSURE_VALUE_IS_NOT_NULL(env, value) \
  if (!EnsureValueIsNotNull(env, value, __FILE__, __LINE__)) return;
#define ENSURE_VALUE_IS_NOT_NULL_RETVAL(env, value, retval) \
  if (!EnsureValueIsNotNull(env, value, __FILE__, __LINE__)) return retval;

inline bool EnsureValueIsNotNull(napi_env env, void* value, const char* file,
                                 const size_t lineNumber) {
  bool is_null = value == nullptr;
  if (is_null) {
    NapiThrowError(env, "Argument is null!", file, lineNumber);
  }
  return !is_null;
}

#define ENSURE_CONSTRUCTOR_CALL(env, info) \
  if (!EnsureConstructorCall(env, info, __FILE__, __LINE__)) return;
#define ENSURE_CONSTRUCTOR_CALL_RETVAL(env, info, retval) \
  if (!EnsureConstructorCall(env, info, __FILE__, __LINE__)) return retval;

inline bool EnsureConstructorCall(napi_env env, napi_callback_info info,
                                  const char* file, const size_t lineNumber) {
  napi_value js_target;
  napi_status nstatus = napi_get_new_target(env, info, &js_target);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, false);
  bool is_target = js_target != nullptr;
  if (!is_target) {
    NapiThrowError(env, "Function not used as a constructor!", file,
                   lineNumber);
  }
  return is_target;
}

#define ENSURE_VALUE_IS_OBJECT(env, value) \
  if (!EnsureValueIsObject(env, value, __FILE__, __LINE__)) return;
#define ENSURE_VALUE_IS_OBJECT_RETVAL(env, value, retval) \
  if (!EnsureValueIsObject(env, value, __FILE__, __LINE__)) return retval;

inline bool EnsureValueIsObject(napi_env env, napi_value value,
                                const char* file, const size_t lineNumber) {
  napi_valuetype type;
  ENSURE_NAPI_OK_RETVAL(env, napi_typeof(env, value, &type), false);
  bool is_object = type == napi_object;
  if (!is_object) {
    NapiThrowError(env, "Argument is not an object!", file, lineNumber);
  }
  return is_object;
}

#define ENSURE_VALUE_IS_STRING(env, value) \
  if (!EnsureValueIsString(env, value, __FILE__, __LINE__)) return;
#define ENSURE_VALUE_IS_STRING_RETVAL(env, value, retval) \
  if (!EnsureValueIsString(env, value, __FILE__, __LINE__)) return retval;

inline bool EnsureValueIsString(napi_env env, napi_value value,
                                const char* file, const size_t lineNumber) {
  napi_valuetype type;
  ENSURE_NAPI_OK_RETVAL(env, napi_typeof(env, value, &type), false);
  bool is_string = type == napi_string;
  if (!is_string) {
    NapiThrowError(env, "Argument is not a string!", file, lineNumber);
  }
  return is_string;
}

#define ENSURE_VALUE_IS_NUMBER(env, value) \
  if (!EnsureValueIsNumber(env, value, __FILE__, __LINE__)) return;
#define ENSURE_VALUE_IS_NUMBER_RETVAL(env, value, retval) \
  if (!EnsureValueIsNumber(env, value, __FILE__, __LINE__)) return retval;

inline bool EnsureValueIsNumber(napi_env env, napi_value value,
                                const char* file, const size_t lineNumber) {
  napi_valuetype type;
  ENSURE_NAPI_OK_RETVAL(env, napi_typeof(env, value, &type), false);
  bool is_number = type == napi_number;
  if (!is_number) {
    NapiThrowError(env, "Argument is not a number!", file, lineNumber);
  }
  return is_number;
}

#define ENSURE_VALUE_IS_BOOLEAN(env, value) \
  if (!EnsureValueIsBoolean(env, value, __FILE__, __LINE__)) return;
#define ENSURE_VALUE_IS_BOOLEAN_RETVAL(env, value, retval) \
  if (!EnsureValueIsBoolean(env, value, __FILE__, __LINE__)) return retval;

inline bool EnsureValueIsBoolean(napi_env env, napi_value value,
                                 const char* file, const size_t lineNumber) {
  napi_valuetype type;
  ENSURE_NAPI_OK_RETVAL(env, napi_typeof(env, value, &type), false);
  bool is_boolean = type == napi_boolean;
  if (!is_boolean) {
    NapiThrowError(env, "Argument is not a boolean!", file, lineNumber);
  }
  return is_boolean;
}

#define ENSURE_VALUE_IS_ARRAY_LIKE(env, value) \
  if (!EnsureValueIsArrayLike(env, value, __FILE__, __LINE__)) return;
#define ENSURE_VALUE_IS_ARRAY_LIKE_RETVAL(env, value, retval) \
  if (!EnsureValueIsArrayLike(env, value, __FILE__, __LINE__)) return retval;

inline bool EnsureValueIsArrayLike(napi_env env, napi_value value,
                                   const char* file, const size_t line_number) {
  napi_status nstatus;

  bool is_array;
  nstatus = napi_is_array(env, value, &is_array);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, false);
  if (is_array) {
    return true;
  }

  bool is_typedarray;
  nstatus = napi_is_typedarray(env, value, &is_typedarray);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, false);
  if (is_typedarray) {
    return true;
  }

  return false;
}

#define ENSURE_VALUE_IS_ARRAY(env, value) \
  if (!EnsureValueIsArray(env, value, __FILE__, __LINE__)) return;
#define ENSURE_VALUE_IS_ARRAY_RETVAL(env, value, retval) \
  if (!EnsureValueIsArray(env, value, __FILE__, __LINE__)) return retval;

inline bool EnsureValueIsArray(napi_env env, napi_value value, const char* file,
                               const size_t lineNumber) {
  bool is_array;
  ENSURE_NAPI_OK_RETVAL(env, napi_is_array(env, value, &is_array), false);
  if (!is_array) {
    NapiThrowError(env, "Argument is not an array!", file, lineNumber);
  }
  return is_array;
}

#define ENSURE_ARGC(env, argc, argc_exp) \
  if (!EnsureArgc(env, argc, argc_exp, __FILE__, __LINE__)) return;
#define ENSURE_ARGC_RETVAL(env, argc, argc_exp, retval) \
  if (!EnsureArgc(env, argc, argc_exp, __FILE__, __LINE__)) return retval;

inline bool EnsureArgc(napi_env env, size_t argc, size_t argc_exp,
                       const char* file, const size_t lineNumber) {
  bool equals = argc == argc_exp;
  if (!equals) {
    std::ostringstream oss;
    oss << "Incorrect number of arguments (" << argc << " expected " << argc_exp
        << ")";
    NapiThrowError(env, oss.str().c_str(), file, lineNumber);
  }
  return equals;
}

#define NAPI_DEFINE_METHOD(name, func) \
  { (name), nullptr, (func), nullptr, nullptr, nullptr, napi_default, nullptr }

inline napi_property_descriptor NapiDefineIntProperty(napi_env env,
                                                      int32_t value,
                                                      const char* name) {
  napi_value js_value;
  napi_status nstatus = napi_create_int32(env, value, &js_value);
  ENSURE_NAPI_OK_RETVAL(env, nstatus, {});

  return {name,    nullptr,  nullptr,      nullptr,
          nullptr, js_value, napi_default, nullptr};
}

namespace nodejsgl {

// Auto-class for alloc'ing and deleting heap allocations.
template <typename T>
class AutoBuffer {
 public:
  AutoBuffer(const size_t length) { buffer = new T[length]; }

  ~AutoBuffer() {
    if (buffer) {
      delete buffer;
    }
  }

  T* get() { return buffer; }

 private:
  T* buffer;
};

}  // namespace nodejsgl

#endif  // NODEJS_GL_UTILS_H_
