const nodeGles = require('./dist/index');
const tf = require('@tensorflow/tfjs');

const tfWebGLUtil = require('@tensorflow/tfjs-core/dist/kernels/webgl/webgl_util');
const tfWebGL = require('@tensorflow/tfjs-core/dist/webgl');

const gl = nodeGles.binding.createWebGLRenderingContext();

tf.ENV.set('WEBGL_VERSION', 1);
tf.ENV.set('WEBGL_RENDER_FLOAT32_ENABLED', false);
tf.ENV.set('WEBGL_DOWNLOAD_FLOAT_ENABLED', true);
tf.ENV.set('WEBGL_FENCE_API_ENABLED', true);
tf.ENV.set('WEBGL_MAX_TEXTURE_SIZE', 10000);

tfWebGLUtil.enableDebugWebGLErrorChecking(false);

tf.ENV.registerBackend(
  'headless-webgl',
  () => new tfWebGL.MathBackendWebGL(new tfWebGL.GPGPUContext(gl), true), 3);

if (tf.ENV.findBackend('headless-webgl') !== null) {
  tf.setBackend('headless-webgl');
}

console.log(gl.getParameter(gl.VERSION));
console.log(gl.getParameter(gl.RENDERER));

// End hack
let a = tf.tensor2d([1, 2, 3, 4], [2, 2], 'float32');
let b = tf.tensor2d([5, 6, 7, 8], [2, 2], 'float32');
let c = a.matMul(b);
console.log('c (int32): ', c.dataSync());

