import * as gles from '.';

const gl = gles.binding.createWebGLRenderingContext();
//
// TODO(kreeger): Port this to an actual Jasmine test.
// - Ideally, the unit tests are running the WebGL Khronos test suite.
//

const oes_ext = gl.getExtension('OES_texture_half_float');

// Sample script: Simply bind some values to a texture and print them out.
const texture = gl.createTexture();
gl.bindTexture(gl.TEXTURE_2D, texture);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
gl.texImage2D(
    gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, oes_ext.HALF_FLOAT_OES, null);

const values = new Float32Array([1.5, 0, 0, 0]);
gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, 1, 1, gl.RGBA, gl.FLOAT, values);

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
