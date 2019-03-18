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

// Contains utilities for running simple GPGPU tests.

//
// Inits GL environment for off-screen renderning and returns a vertex and index
// buffer.
//
export function initEnvGL(gl: WebGLRenderingContext):
    [WebGLBuffer, WebGLBuffer] {
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

  return [vertexBuffer, indexBuffer];
}

//
// Creates and initializes a 2D texture.
//
export function createTexture2D(
    gl: WebGLRenderingContext, internalFormat: GLenum, format: GLenum,
    type: GLenum): WebGLTexture {
  const texture = gl.createTexture();
  gl.bindTexture(gl.TEXTURE_2D, texture);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
  gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
  gl.texImage2D(gl.TEXTURE_2D, 0, internalFormat, 1, 1, 0, format, type, null);
  gl.bindTexture(gl.TEXTURE_2D, null);
  return texture;
}

//
// Ensures framebuffer has attached to a texture.
//
export function ensureFramebufferAttachment(gl: WebGLRenderingContext) {
  const status = gl.checkFramebufferStatus(gl.FRAMEBUFFER);
  if (status != gl.FRAMEBUFFER_COMPLETE) {
    if (status === gl.FRAMEBUFFER_COMPLETE) {
      console.log('FRAMEBUFFER_COMPLETE');
    } else if (status === gl.FRAMEBUFFER_INCOMPLETE_ATTACHMENT) {
      console.log('FRAMEBUFFER_INCOMPLETE_ATTACHMENT');
    } else if (status === gl.FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT) {
      console.log('FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT');
    } else if (status === gl.FRAMEBUFFER_INCOMPLETE_DIMENSIONS) {
      console.log('FRAMEBUFFER_INCOMPLETE_DIMENSIONS');
    } else if (status === gl.FRAMEBUFFER_UNSUPPORTED) {
      console.log('FRAMEBUFFER_UNSUPPORTED');
    } else {
      console.error('Unknown error');
    }
    throw new Error('Exception binding to framebuffer!');
  }
}
