import * as gles from '../.';

declare interface WebGLParameterDefinition {
  name: string;
  type: string;
  subType?: string;
  length?: number;
  nullable?: boolean;
  extension?: string;
}
class VerifyResult {
  passed: boolean = false;
  name: string = '';
  message: string = '';
  object: any;

  constructor() {
  }

  setPassed(passed: boolean) {
    this.passed = passed;
    return this;
  }

  setName(name: string) {
    this.name = name;
    return this;
  }

  setMessage(message: string) {
    this.message = message;
    return this;
  }

  setObject(...args: any[]) {
    this.object = args;
    return this;
  }

  print() {
    const { passed, name, message, object } = this;
    const args = [passed ? '√' : 'x', name, passed ? 'passed.' : 'failed.'];
    if (message) args.push(message);
    if (object) {
      if (Array.isArray(object)) object.forEach(o => args.push(o));
      else args.push(object);
    }
    passed ? console.log.apply(console, args) : console.warn.apply(console, args);
  }
}

// https://developer.mozilla.org/en-US/docs/Web/API/WebGLRenderingContext/getParameter
const webGL1ParameterDefinitions: WebGLParameterDefinition[] = [
  { "name": "ACTIVE_TEXTURE", "type": "GLenum" },
  { "name": "ALIASED_LINE_WIDTH_RANGE", "type": "Float32Array", "length": 2 },
  { "name": "ALIASED_POINT_SIZE_RANGE", "type": "Float32Array", "length": 2 },
  { "name": "ALPHA_BITS", "type": "GLint" },
  { "name": "ARRAY_BUFFER_BINDING", "type": "WebGLBuffer" },
  { "name": "BLEND", "type": "GLboolean" },
  { "name": "BLEND_COLOR", "type": "Float32Array", "length": 4 },
  { "name": "BLEND_DST_ALPHA", "type": "GLenum" },
  { "name": "BLEND_DST_RGB", "type": "GLenum" },
  { "name": "BLEND_EQUATION", "type": "GLenum" },
  { "name": "BLEND_EQUATION_ALPHA", "type": "GLenum" },
  { "name": "BLEND_EQUATION_RGB", "type": "GLenum" },
  { "name": "BLEND_SRC_ALPHA", "type": "GLenum" },
  { "name": "BLEND_SRC_RGB", "type": "GLenum" },
  { "name": "BLUE_BITS", "type": "GLint" },
  { "name": "COLOR_CLEAR_VALUE", "type": "Float32Array", "length": 4 },
  { "name": "COLOR_WRITEMASK", "type": "Array", "subType": "GLboolean", "length": 4 },
  // { "name": "COMPRESSED_TEXTURE_FORMATS", "type": "Uint32Array" }, // TODO
  { "name": "CULL_FACE", "type": "GLboolean" },
  { "name": "CULL_FACE_MODE", "type": "GLenum" },
  { "name": "CURRENT_PROGRAM", "type": "WebGLProgram", "nullable": true },
  { "name": "DEPTH_BITS", "type": "GLint" },
  { "name": "DEPTH_CLEAR_VALUE", "type": "GLfloat" },
  { "name": "DEPTH_FUNC", "type": "GLenum" },
  { "name": "DEPTH_RANGE", "type": "Float32Array", "length": 2 },
  { "name": "DEPTH_TEST", "type": "GLboolean" },
  { "name": "DEPTH_WRITEMASK", "type": "GLboolean" },
  { "name": "DITHER", "type": "GLboolean" },
  { "name": "ELEMENT_ARRAY_BUFFER_BINDING", "type": "WebGLBuffer" },
  { "name": "FRAMEBUFFER_BINDING", "type": "WebGLFramebuffer", "nullable": true },
  { "name": "FRONT_FACE", "type": "GLenum" },
  { "name": "GENERATE_MIPMAP_HINT", "type": "GLenum" },
  { "name": "GREEN_BITS", "type": "GLint" },
  { "name": "IMPLEMENTATION_COLOR_READ_FORMAT", "type": "GLenum" },
  { "name": "IMPLEMENTATION_COLOR_READ_TYPE", "type": "GLenum" },
  { "name": "LINE_WIDTH", "type": "GLfloat" },
  { "name": "MAX_COMBINED_TEXTURE_IMAGE_UNITS", "type": "GLint" },
  { "name": "MAX_CUBE_MAP_TEXTURE_SIZE", "type": "GLint" },
  { "name": "MAX_FRAGMENT_UNIFORM_VECTORS", "type": "GLint" },
  { "name": "MAX_RENDERBUFFER_SIZE", "type": "GLint" },
  { "name": "MAX_TEXTURE_IMAGE_UNITS", "type": "GLint" },
  { "name": "MAX_TEXTURE_SIZE", "type": "GLint" },
  { "name": "MAX_VARYING_VECTORS", "type": "GLint" },
  { "name": "MAX_VERTEX_ATTRIBS", "type": "GLint" },
  { "name": "MAX_VERTEX_TEXTURE_IMAGE_UNITS", "type": "GLint" },
  { "name": "MAX_VERTEX_UNIFORM_VECTORS", "type": "GLint" },
  { "name": "MAX_VIEWPORT_DIMS", "type": "Int32Array", "length": 2 },
  { "name": "PACK_ALIGNMENT", "type": "GLint" },
  { "name": "POLYGON_OFFSET_FACTOR", "type": "GLfloat" },
  { "name": "POLYGON_OFFSET_FILL", "type": "GLboolean" },
  { "name": "POLYGON_OFFSET_UNITS", "type": "GLfloat" },
  { "name": "RED_BITS", "type": "GLint" },
  { "name": "RENDERBUFFER_BINDING", "type": "WebGLRenderbuffer", "nullable": true },
  { "name": "RENDERER", "type": "DOMString" },
  { "name": "SAMPLE_BUFFERS", "type": "GLint" },
  { "name": "SAMPLE_COVERAGE_INVERT", "type": "GLboolean" },
  { "name": "SAMPLE_COVERAGE_VALUE", "type": "GLfloat" },
  { "name": "SAMPLES", "type": "GLint" },
  { "name": "SCISSOR_BOX", "type": "Int32Array", "length": 4 },
  { "name": "SCISSOR_TEST", "type": "GLboolean" },
  { "name": "SHADING_LANGUAGE_VERSION", "type": "DOMString" },
  { "name": "STENCIL_BACK_FAIL", "type": "GLenum" },
  { "name": "STENCIL_BACK_FUNC", "type": "GLenum" },
  { "name": "STENCIL_BACK_PASS_DEPTH_FAIL", "type": "GLenum" },
  { "name": "STENCIL_BACK_PASS_DEPTH_PASS", "type": "GLenum" },
  { "name": "STENCIL_BACK_REF", "type": "GLint" },
  { "name": "STENCIL_BACK_VALUE_MASK", "type": "GLuint" },
  { "name": "STENCIL_BACK_WRITEMASK", "type": "GLuint" },
  { "name": "STENCIL_BITS", "type": "GLint" },
  { "name": "STENCIL_CLEAR_VALUE", "type": "GLint" },
  { "name": "STENCIL_FAIL", "type": "GLenum" },
  { "name": "STENCIL_FUNC", "type": "GLenum" },
  { "name": "STENCIL_PASS_DEPTH_FAIL", "type": "GLenum" },
  { "name": "STENCIL_PASS_DEPTH_PASS", "type": "GLenum" },
  { "name": "STENCIL_REF", "type": "GLint" },
  { "name": "STENCIL_TEST", "type": "GLboolean" },
  { "name": "STENCIL_VALUE_MASK", "type": "GLuint" },
  { "name": "STENCIL_WRITEMASK", "type": "GLuint" },
  { "name": "SUBPIXEL_BITS", "type": "GLint" },
  { "name": "TEXTURE_BINDING_2D", "type": "WebGLTexture", "nullable": true },
  { "name": "TEXTURE_BINDING_CUBE_MAP", "type": "WebGLTexture", "nullable": true },
  { "name": "UNPACK_ALIGNMENT", "type": "GLint" },
  { "name": "UNPACK_COLORSPACE_CONVERSION_WEBGL", "type": "GLenum" },
  { "name": "UNPACK_FLIP_Y_WEBGL", "type": "GLboolean" },
  { "name": "UNPACK_PREMULTIPLY_ALPHA_WEBGL", "type": "GLboolean" },
  { "name": "VENDOR", "type": "DOMString" },
  { "name": "VERSION", "type": "DOMString" },
  { "name": "VIEWPORT", "type": "Int32Array", "length": 4 },
];
const webGL2ParameterDefinitions: WebGLParameterDefinition[] = [
  { "name": "COPY_READ_BUFFER_BINDING", "type": "WebGLBuffer", "nullable": true },
  { "name": "COPY_WRITE_BUFFER_BINDING", "type": "WebGLBuffer", "nullable": true },
  // DRAW_BUFFERi, i from 0 to gl.getParameter(ext.MAX_DRAW_BUFFERS)-1
  { "name": "DRAW_BUFFER0", "type": "GLenum" },
  { "name": "DRAW_FRAMEBUFFER_BINDING", "type": "WebGLFramebuffer", "nullable": true },
  { "name": "FRAGMENT_SHADER_DERIVATIVE_HINT", "type": "GLenum" },
  { "name": "MAX_3D_TEXTURE_SIZE", "type": "GLint" },
  { "name": "MAX_ARRAY_TEXTURE_LAYERS", "type": "GLint" },
  { "name": "MAX_CLIENT_WAIT_TIMEOUT_WEBGL", "type": "GLint64" },
  { "name": "MAX_COLOR_ATTACHMENTS", "type": "GLint" },
  { "name": "MAX_COMBINED_FRAGMENT_UNIFORM_COMPONENTS", "type": "GLint64" },
  { "name": "MAX_COMBINED_UNIFORM_BLOCKS", "type": "GLint" },
  { "name": "MAX_COMBINED_VERTEX_UNIFORM_COMPONENTS", "type": "GLint64" },
  { "name": "MAX_DRAW_BUFFERS", "type": "GLint" },
  { "name": "MAX_ELEMENT_INDEX", "type": "GLint64" },
  { "name": "MAX_ELEMENTS_INDICES", "type": "GLint" },
  { "name": "MAX_ELEMENTS_VERTICES", "type": "GLint" },
  { "name": "MAX_FRAGMENT_INPUT_COMPONENTS", "type": "GLint" },
  { "name": "MAX_FRAGMENT_UNIFORM_BLOCKS", "type": "GLint" },
  { "name": "MAX_FRAGMENT_UNIFORM_COMPONENTS", "type": "GLint" },
  { "name": "MAX_PROGRAM_TEXEL_OFFSET", "type": "GLint" },
  { "name": "MAX_SAMPLES", "type": "GLint" },
  { "name": "MAX_SERVER_WAIT_TIMEOUT", "type": "GLint64" },
  { "name": "MAX_TEXTURE_LOD_BIAS", "type": "GLfloat" },
  { "name": "MAX_TRANSFORM_FEEDBACK_INTERLEAVED_COMPONENTS", "type": "GLint" },
  { "name": "MAX_TRANSFORM_FEEDBACK_SEPARATE_ATTRIBS", "type": "GLint" },
  { "name": "MAX_TRANSFORM_FEEDBACK_SEPARATE_COMPONENTS", "type": "GLint" },
  { "name": "MAX_UNIFORM_BLOCK_SIZE", "type": "GLint64" },
  { "name": "MAX_UNIFORM_BUFFER_BINDINGS", "type": "GLint" },
  { "name": "MAX_VARYING_COMPONENTS", "type": "GLint" },
  { "name": "MAX_VERTEX_OUTPUT_COMPONENTS", "type": "GLint" },
  { "name": "MAX_VERTEX_UNIFORM_BLOCKS", "type": "GLint" },
  { "name": "MAX_VERTEX_UNIFORM_COMPONENTS", "type": "GLint" },
  { "name": "MIN_PROGRAM_TEXEL_OFFSET", "type": "GLint" },
  { "name": "PACK_ROW_LENGTH", "type": "GLint" },
  { "name": "PACK_SKIP_PIXELS", "type": "GLint" },
  { "name": "PACK_SKIP_ROWS", "type": "GLint" },
  { "name": "PIXEL_PACK_BUFFER_BINDING", "type": "WebGLBuffer", "nullable": true },
  { "name": "PIXEL_UNPACK_BUFFER_BINDING", "type": "WebGLBuffer", "nullable": true },
  { "name": "RASTERIZER_DISCARD", "type": "GLboolean" },
  { "name": "READ_BUFFER", "type": "GLenum" },
  { "name": "READ_FRAMEBUFFER_BINDING", "type": "WebGLFramebuffer", "nullable": true },
  { "name": "SAMPLE_ALPHA_TO_COVERAGE", "type": "GLboolean" },
  { "name": "SAMPLE_COVERAGE", "type": "GLboolean" },
  { "name": "SAMPLER_BINDING", "type": "WebGLSampler", "nullable": true },
  { "name": "TEXTURE_BINDING_2D_ARRAY", "type": "WebGLTexture", "nullable": true },
  { "name": "TEXTURE_BINDING_3D", "type": "WebGLTexture", "nullable": true },
  { "name": "TRANSFORM_FEEDBACK_ACTIVE", "type": "GLboolean" },
  { "name": "TRANSFORM_FEEDBACK_BINDING", "type": "WebGLTransformFeedback", "nullable": true },
  { "name": "TRANSFORM_FEEDBACK_BUFFER_BINDING", "type": "WebGLBuffer", "nullable": true },
  { "name": "TRANSFORM_FEEDBACK_PAUSED", "type": "GLboolean" },
  { "name": "UNIFORM_BUFFER_BINDING", "type": "WebGLBuffer", "nullable": true },
  { "name": "UNIFORM_BUFFER_OFFSET_ALIGNMENT", "type": "GLint" },
  { "name": "UNPACK_IMAGE_HEIGHT", "type": "GLint" },
  { "name": "UNPACK_ROW_LENGTH", "type": "GLint" },
  { "name": "UNPACK_SKIP_IMAGES", "type": "GLint" },
  { "name": "UNPACK_SKIP_PIXELS", "type": "GLint" },
  { "name": "UNPACK_SKIP_ROWS", "type": "GLint" },
  { "name": "VERTEX_ARRAY_BINDING", "type": "WebGLVertexArrayObject", "nullable": true },
];
const webGLExtensionsParameterDefinitions: WebGLParameterDefinition[] = [
  { "name": "MAX_TEXTURE_MAX_ANISOTROPY_EXT", "type": "GLfloat", "extension": "EXT_texture_filter_anisotropic" },
  { "name": "FRAGMENT_SHADER_DERIVATIVE_HINT_OES", "type": "GLenum", "extension": "OES_standard_derivatives" },
  { "name": "MAX_COLOR_ATTACHMENTS_WEBGL", "type": "GLint", "extension": "WEBGL_draw_buffers" },
  { "name": "MAX_DRAW_BUFFERS_WEBGL", "type": "GLint", "extension": "WEBGL_draw_buffers" },
  // DRAW_BUFFERi_WEBGL, i from 0 to gl.getParameter(ext.MAX_DRAW_BUFFERS_WEBGL)-1
  { "name": "DRAW_BUFFER0_WEBGL", "type": "GLenum", "extension": "WEBGL_draw_buffers" },
  { "name": "VERTEX_ARRAY_BINDING_OES", "type": "WebGLVertexArrayObjectOES", "extension": "OES_vertex_array_object" },
  { "name": "TIMESTAMP_EXT", "type": "GLuint64EXT", "extension": "EXT_disjoint_timer_query" },
  { "name": "GPU_DISJOINT_EXT", "type": "GLboolean", "extension": "EXT_disjoint_timer_query" },
  { "name": "MAX_VIEWS_OVR", "type": "GLint", "extension": "OVR_multiview2" },
  // https://developer.mozilla.org/en-US/docs/Web/API/WEBGL_debug_renderer_info
  { "name": "UNMASKED_VENDOR_WEBGL", "type": "DOMString", "extension": "WEBGL_debug_renderer_info" },
  { "name": "UNMASKED_RENDERER_WEBGL", "type": "DOMString", "extension": "WEBGL_debug_renderer_info" },
];

let GLenumNameMap: any;
const getGLenumName = function (gl: WebGLRenderingContext, value: GLenum) {
  if (!GLenumNameMap) {
    GLenumNameMap = {};
    Object.getOwnPropertyNames(Object.getPrototypeOf(gl)).forEach(name => {
      if (/^[A-Z]/.test(name)) GLenumNameMap[(gl as any)[name]] = name;
    });
  }
  return GLenumNameMap[value];
}

const verifyParameterDefinition = function (
  gl: WebGLRenderingContext,
  parameterDefinition: WebGLParameterDefinition
): VerifyResult {
  const { name, type, subType, length, /*nullable,*/ extension } = parameterDefinition;
  let ptarget: any = gl, pname: GLenum;
  const result = new VerifyResult();
  // if (name === 'DRAW_BUFFER?_WEBGL') return verifyDrawBuffersWebgl(gl, parameterDefinition);
  if (extension) {
    const ext = gl.getExtension(extension);
    if (!ext) return result
      .setName(`gl.getParameter(ext.${name})`)
      .setMessage(`Can NOT get extension ${extension}.`);
    ptarget = ext;
  }
  pname = ptarget[name] as GLenum;
  const info0 = `${ptarget === gl ? 'gl' : 'ext'}.${name}`;
  const info1 = `gl.getParameter(${info0})`;
  result.setName(info1);
  const infoType = `${type}${subType ? `<${subType}>` : ''}${length ? `[${length}]` : ''}`;
  if (typeof pname !== 'number') return result.setMessage(`${info0} is NOT a GLenum.`);
  let value;
  try {
    value = gl.getParameter(pname);
  }
  catch (error) {
    // return result.setObject(error);
    return result.setMessage(error.toString());
  }
  let infoValue = `${value}`;
  result.setMessage(`Got ${infoValue} does NOT satisfy type ${infoType}.`);
  switch (type) {
    case 'GLenum': {
      if (typeof value !== 'number') return result;
      const s = getGLenumName(gl, value);
      if (!s) return result;
      infoValue = `gl.${s}(0x${value.toString(16)})`;
      break;
    }
    case 'GLfloat':
    case 'GLint64':
    case 'GLuint':
    case 'GLint': {
      if (typeof value !== 'number') return result;
      break;
    }
    case 'Int32Array': {
      if (value.constructor !== Int32Array) return result;
      if (length && value.length !== length) return result;
      infoValue = `[${value}]`;
      break;
    }
    case 'Float32Array': {
      if (value.constructor !== Float32Array) return result;
      if (length && value.length !== length) return result;
      infoValue = `[${value}]`;
      break;
    }
    case 'Array': {
      if (!Array.isArray(value)) return result;
      if (length && value.length !== length) return result;
      if (subType) {
        for (let i = 0; i < length; i++) {
          const v = value[i];
          if (subType === 'GLboolean' && typeof v !== 'boolean') return result;
        }
      }
      infoValue = `[${value}]`;
      break;
    }
    case 'DOMString': {
      if (typeof value !== 'string') return result;
      infoValue = `"${value}"`;
      break;
    }
  }
  return result.setPassed(true).setMessage(`Got ${infoValue} satisfies type ${infoType}.`);
};

const verifyParameterDefinitions = function (title: string, gl: WebGLRenderingContext, parameterDefinitions: WebGLParameterDefinition[]) {
  const total = parameterDefinitions.length;
  console.log(`# Verify ${total} parameter definitions for ${title}.`);
  let numPassed = 0;
  parameterDefinitions.forEach((parameterDefinition, index) => {
    const result = verifyParameterDefinition(gl, parameterDefinition);
    result.passed ? process.stdout.write(`\u001b[92m`) : process.stdout.write(`\u001b[91m`);
    process.stdout.write(`  ${index + 1}/${total} `);
    if (result.passed) numPassed++;
    result.print();
    process.stdout.write(`\u001b[39m`);
  })
  process.stdout.write(`\u001b[0m`);
  console.log(`RESULT: ${numPassed} parameter definition${numPassed > 1 ? 's' : ''} passed, ${(numPassed / total * 100).toFixed(2)}%`);
}

const gl = gles.createWebGLRenderingContext({ width: 512, height: 512 });
verifyParameterDefinitions('WebGL 1', gl, webGL1ParameterDefinitions);
verifyParameterDefinitions('WebGL 2', gl, webGL2ParameterDefinitions);
verifyParameterDefinitions('WebGL Extensions', gl, webGLExtensionsParameterDefinitions);
