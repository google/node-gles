import * as gles from '.';

const gl = gles.binding.createWebGLRenderingContext();
const gl2 = gl as WebGL2RenderingContext;
gl.viewport(0, 0, 1, 1);

const samplingVs = 'attribute vec4 position;\n' +
    'varying vec2 texcoord;\n' +
    'void main()\n' +
    '{\n' +
    '    gl_Position = vec4(position.xy, 0.0, 1.0);\n' +
    '    texcoord = (position.xy * 0.5) + 0.5;\n' +
    '}\n';

const vertexShader = gl.createShader(gl.VERTEX_SHADER);
gl.shaderSource(vertexShader, samplingVs);
gl.compileShader(vertexShader);
if (!gl.getShaderParameter(vertexShader, gl.COMPILE_STATUS)) {
  console.log('shader compile failure: ' + gl.getShaderInfoLog(vertexShader));
}
const samplingFs = 'precision mediump float;\n' +
    'uniform sampler2D tex;\n' +
    'uniform vec4 subtractor;\n' +
    'varying vec2 texcoord;\n' +
    'void main()\n' +
    '{\n' +
    '    vec4 color = texture2D(tex, texcoord);\n' +
    '    if (abs(color.r - subtractor.r) +\n' +
    '        abs(color.g - subtractor.g) +\n' +
    '        abs(color.b - subtractor.b) +\n' +
    '        abs(color.a - subtractor.a) < 8.0)\n' +
    '    {\n' +
    '        gl_FragColor = vec4(0.0, 1.0, 0.0, 1.0);\n' +
    '    }\n' +
    '    else\n' +
    '    {\n' +
    '        gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n' +
    '    }\n' +
    '}\n';

const fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
gl.shaderSource(fragmentShader, samplingFs);
gl.compileShader(fragmentShader);

const program = gl.createProgram();
gl.attachShader(program, vertexShader);
// gl.deleteShader(vertexShader);
gl.attachShader(program, fragmentShader);
// gl.deleteShader(fragmentShader);
gl.linkProgram(program);
if (!gl.getProgramParameter(program, gl.LINK_STATUS)) {
  console.log('program link failure: ' + gl.getProgramInfoLog(program));
}

gl.useProgram(program);

const renderbuffer = gl.createRenderbuffer();
gl.bindRenderbuffer(gl.RENDERBUFFER, renderbuffer);
gl.renderbufferStorage(gl.RENDERBUFFER, gl2.RGBA8, 1, 1);

const oes_ext = gl.getExtension('OES_texture_half_float');

const texture = gl.createTexture();
gl.bindTexture(gl.TEXTURE_2D, texture);
const texData = new Float32Array([0, 1, 2, 3]);
gl.texImage2D(
    gl.TEXTURE_2D, 0, gl.RGBA, 1, 1, 0, gl.RGBA, oes_ext.HALF_FLOAT_OES,
    texData);

gl.uniform1i(gl.getUniformLocation(program, 'tex'), 0);

const floatData = new Float32Array([7000.0, 100.0, 33.0, -1.0]);
gl.uniform4fv(gl.getUniformLocation(program, 'subtractor'), floatData);

// const framebuffer = gl.createFramebuffer();
// gl.bindFramebuffer(gl.FRAMEBUFFER, gl.COLOR_ATTACHMENT0, gl.TEXTURE_2D,
// texture);

gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MIN_FILTER, gl.NEAREST);
gl.texParameteri(gl.TEXTURE_2D, gl.TEXTURE_MAG_FILTER, gl.NEAREST);
