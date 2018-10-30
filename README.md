# Headless OpenGL / WebGL implementation for Node.js

**This repo is under active development and is not production-ready. We are
actively developing as an open source project.**

## Details

This project aims to surface various OpenGL runtimes for Node.js. The project currently exposes a [WebGLRenderingContext](https://developer.mozilla.org/en-US/docs/Web/API/WebGLRenderingContext) for Node.js.

Future plans include surfacing an API for running [Compute Shaders](https://www.khronos.org/opengl/wiki/Compute_Shader).

## Supported platforms

* Mac OS (requires [Angle](https://github.com/google/angle))
* Windows (requires [Angle](https://github.com/google/angle))
* Linux (optional [Angle](https://github.com/google/angle))

## Installing

On platforms that require Angle - [checkout and build Angle](https://github.com/google/angle/blob/master/doc/DevSetup.md) in the same parent folder as this repo. Compile Angle with the `Debug` setup. After Angle is built, run `yarn` in this projects directory.

**Build instructions are under heavy development and will include an Angle binary**