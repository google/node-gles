# Headless WebGL / OpenGL ES runtime for Node.js

**This repo is under active development and is not production-ready. We are
actively developing as an open source project.**

## Details

This project aims to provide a headless runtime for WebGL and OpenGL ES shaders under Node.js. This package will use the [ANGLE](https://github.com/google/angle) engine to translate WebGL and OpenGL ES shaders to the target runtime. Please see the [ANGLE project](https://github.com/google/angle) for more details on support.

Future plans include surfacing an API for running [Compute Shaders](https://www.khronos.org/opengl/wiki/Compute_Shader) and a OpenGL ES API. Patches are welcome!

## Supported platforms

* Mac OS
* Windows
* Linux

## Creating a WebGL context

To create a new `WebGLRenderingContext` or `WebGL2RenderingContext` - simply include the package and call `createWebGLRenderingContext()`:

```js
const nodeGles = require('node-gles');

const gl = nodeGles.binding.createWebGLRenderingContext();

// Now, use `gl` for regular WebGL calls:
const tex = gl.createTexture();
gl.bindTexture(gl.TEXTURE_2D, texture);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_S, gl.CLAMP_TO_EDGE);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_WRAP_T, gl.CLAMP_TO_EDGE);
...
```

## Run demo
*Clone this repo for current demos - examples coming soon*

* Float32 texture upload and download:
```sh
$ yarn ts-node src/tests/float_texture_upload_test.ts
...

buffer:  Float32Array [ 0.5, 1.5, 2.5, 3.5 ]
```

* Float16 texture upload and download:
```sh
$ yarn ts-node src/tests/half_float_texture_upload_test.ts
...

buffer:  Float32Array [ 0.5, 1.5, 2.5, 3.5 ]
```
* Unsigned byte texture upload and download:
```sh
$ yarn ts-node src/tests/unsigned_byte_texture_upload_test.ts
...

buffer:  Uint8Array [ 1, 2, 3, 4 ]
```

## Development
*Build instructions are under heavy development and will include an Angle binary*

This project currently requires ANGLE to be checked out and built in the same parent folder as this repo. [Checkout and build ANGLE](https://github.com/google/angle/blob/master/doc/DevSetup.md) with the `Debug` setup. After ANGLE is built, run `yarn` for this project.
