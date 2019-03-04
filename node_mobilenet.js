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

// End hack init!!
const fs = require('fs');
const jpeg = require('jpeg-js');
const simple_timer = require('node-simple-timer');
const fetch = require('node-fetch');

global.fetch = fetch;

const NUMBER_OF_CHANNELS = 3
const GOOGLE_CLOUD_STORAGE_DIR =
    'https://storage.googleapis.com/tfjs-models/savedmodel/';
const MODEL_FILE_URL = 'mobilenet_v1_1.0_224/tensorflowjs_model.pb';
const WEIGHT_MANIFEST_FILE_URL = 'mobilenet_v1_1.0_224/weights_manifest.json';
const INPUT_NODE_NAME = 'images';
const OUTPUT_NODE_NAME = 'module_apply_default/MobilenetV2/Logits/output';
const PREPROCESS_DIVISOR = tf.scalar(255 / 2);

// based on implementation from
// http://jamesthom.as/blog/2018/08/07/machine-learning-in-node-dot-js-with-tensorflow-dot-js/
const readImage =
    path => {
      const buf = fs.readFileSync(path);
      const pixels = jpeg.decode(buf, true);
      return pixels;
    }

const imageByteArray =
    (image, numChannels) => {
      const pixels = image.data;
      const numPixels = image.width * image.height;
      const values = new Int32Array(numPixels * numChannels);

      for (let i = 0; i < numPixels; i++) {
        for (let channel = 0; channel < numChannels; ++channel) {
          values[i * numChannels + channel] = pixels[i * 4 + channel];
        }
      }

      return values;
    }

const imageToInput =
    (image, numChannels) => {
      const values = imageByteArray(image, numChannels);
      const outShape = [1, image.height, image.width, numChannels];
      const input = tf.tensor4d(values, outShape, 'float32');
      return tf.div(tf.sub(input, PREPROCESS_DIVISOR), PREPROCESS_DIVISOR);
    }

const loadModel =
    async () => {
  return await tf.loadFrozenModel(
      GOOGLE_CLOUD_STORAGE_DIR + MODEL_FILE_URL,
      GOOGLE_CLOUD_STORAGE_DIR + WEIGHT_MANIFEST_FILE_URL);
}

const result =
    async (model, input) => {
  const r = tf.tidy(() => {
    return model.predict(input);
  });
  r.dataSync();
}

const benchmark =
    async (path) => {
  const image = readImage(path);
  const input = imageToInput(image, NUMBER_OF_CHANNELS);

  const mn_model = await loadModel();
  const timer = new simple_timer.Timer();
  timer.start();
  let predictions = await result(mn_model, input);
  timer.end();
  console.log(timer.milliseconds());

  const times = 100;
  let timers = 0;
  for (let i = 0; i < times; i++) {
    timer.start();
    predictions = await result(mn_model, input);
    timer.end();
    timers += timer.milliseconds();
  }
  console.log(timers / times);
}

if (process.argv.length !== 3) throw new Error(
    'incorrect arguments: node mobilenet_node.js <IMAGE_FILE>');

benchmark(process.argv[2]);

