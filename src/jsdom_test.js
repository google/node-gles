
// TODO(kreeger): Convert to TS
const nodeGles = require('../dist/index');

const jsdom = require('jsdom');
const {JSDOM} = jsdom;

const stubCanvas = {
  getContext: (id, options) => {
    return nodeGles.binding.createWebGLRenderingContext();
  }
}

const options = {
  runScripts: 'dangerously',
  resources: 'usable',
  beforeParse: (window) => {
    // const fallback = window.document.getElementById;

    // window.document.getElementById = (id) => {
    //   console.log(id);
    //   const element = fallback(id);
    //   element.getContext = () => {
    //     return nodeGles.binding.createWebGLRenderingContext();
    //   };
    // if (id === 'canvas') {
    //   return stubCanvas;
    // } else {
    //   console.log('fallback');
    //   console.log(fallback);
    //   fallback.call(id);
    //   // return Function.call(fallback(id);
    // }
    // };
  }
};

const url =
    'https://www.khronos.org/registry/webgl/sdk/tests/conformance/attribs/gl-bindAttribLocation-aliasing.html?webglVersion=1&quiet=0&quick=1';
JSDOM.fromURL(url, options).then(() => console.log('hi'));

// JSDOM.fromFile('src/webgl_test.html', options)
//     .then(
//         dom => {
//             // console.log('hi');
//         });
