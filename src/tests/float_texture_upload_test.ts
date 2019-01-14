import * as gles from '../.';

import {createTexture2D, ensureFramebufferAttachment, initEnvGL} from './test_utils';

const gl = gles.binding.createWebGLRenderingContext();

// TODO - cleanup these extension names:
console.log('OES_texture_float: ', gl.getExtension('OES_texture_float'));
// TODO - this one throws an exception:
console.log(
    'EXT_color_buffer_float: ', gl.getExtension('EXT_color_buffer_float'));

initEnvGL(gl);  // Don't worry about buffers in this demo

const texture = createTexture2D(gl, gl.RGBA, gl.RGBA, gl.FLOAT);
gl.bindTexture(gl.TEXTURE_2D, texture);

const values = new Float32Array([0.5, 1.5, 2.5, 3.5]);
gl.texSubImage2D(gl.TEXTURE_2D, 0, 0, 0, 1, 1, gl.RGBA, gl.FLOAT, values);

const framebuffer = gl.createFramebuffer();
gl.bindFramebuffer(gl.FRAMEBUFFER, framebuffer);
gl.framebufferTexture2D(
    gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D, texture, 0);
ensureFramebufferAttachment(gl);

// TODO(kreeger: Use this when attaching a shader.
// gl.viewport(0, 0, 1, 1);
// gl.scissor(0, 0, 1, 1);
// gl.drawElements(gl.TRIANGLES, 6, gl.UNSIGNED_SHORT, 0);

const buffer = new Float32Array(4);
gl.readPixels(0, 0, 1, 1, gl.RGBA, gl.FLOAT, buffer);
console.log('buffer: ', buffer);
