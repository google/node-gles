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

## Development
*Build instructions are under heavy development and will include an Angle binary*

This project currently requires ANGLE to be checked out and built in the same parent folder as this repo. [Checkout and build ANGLE](https://github.com/google/angle/blob/master/doc/DevSetup.md) with the `Debug` setup. After ANGLE is built, run `yarn` for this project.