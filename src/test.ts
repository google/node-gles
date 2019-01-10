import * as gles from '.';

const gl = gles.binding.createWebGLRenderingContext();
const gl2 = gl as WebGL2RenderingContext;

//
// TODO(kreeger): Port this to an actual Jasmine test.
// - Ideally, the unit tests are running the WebGL Khronos test suite.
//

// const oes_ext = gl.getExtension('OES_texture_half_float');

gl.disable(gl.DEPTH_TEST);
gl.disable(gl.STENCIL_TEST);
gl.disable(gl.BLEND);
gl.disable(gl.DITHER);
gl.disable(gl.POLYGON_OFFSET_FILL);
gl.disable(gl.SAMPLE_COVERAGE);
gl.enable(gl.SCISSOR_TEST);
gl.enable(gl.CULL_FACE);
gl.cullFace(gl.BACK);

const vertexBuffer = gl.createBuffer();
gl.bindBuffer(gl.ARRAY_BUFFER, vertexBuffer);
const coords = new Float32Array([
  -1.0, 1.0, 0.0, 0.0, 1.0, -1.0, -1.0, 0.0, 0.0, 0.0,
  1.0,  1.0, 0.0, 1.0, 1.0, 1.0,  -1.0, 0.0, 1.0, 0.0
]);
gl.bufferData(gl.ARRAY_BUFFER, coords, gl.STATIC_DRAW);

const indexBuffer = gl.createBuffer();
gl.bindBuffer(gl.ELEMENT_ARRAY_BUFFER, indexBuffer);
const indices = new Uint16Array([0, 1, 2, 2, 1, 3]);
gl.bufferData(gl.ELEMENT_ARRAY_BUFFER, indices, gl.STATIC_DRAW);

// Sample script: Simply bind some values to a texture and print them out.
const texture = gl.createTexture();
gl.bindTexture(gl.TEXTURE_2D, texture);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
gl.texImage2D(
    gl.TEXTURE_2D, 0, gl2.R16F, 1, 1, 0, gl2.RED, gl2.HALF_FLOAT, null);

const values = new Float32Array([1.5, 0, 0, 0]);
gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, 1, 1, gl2.RED, gl2.HALF_FLOAT, values);

const framebuffer = gl.createFramebuffer();
gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
gl.framebufferTexture2D(
    gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
const status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
if (status != gl.FRAMEBUFFER_COMPLETE) {
  console.error('Exception binding to framebuffer!');
}

gl.viewport(0, 0, 1, 1);
gl.scissor(0, 0, 1, 1);
gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);

const buffer = new Float32Array(4);
gl.readPixels(0, 0, 1, 1, gl.RGBA, gl.FLOAT, buffer);
console.log('buffer', buffer);
