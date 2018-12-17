/**
 * @license
 * Copyright 2018 Google Inc. All Rights Reserved.
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

const https = require('https');
const HttpsProxyAgent = require('https-proxy-agent');
const url = require('url');
const fs = require('fs');
let path = require('path');
const rimraf = require('rimraf');
const tar = require('tar');
const util = require('util');
const zip = require('adm-zip');
const cp = require('child_process');
const os = require('os');
const ProgressBar = require('progress');

const BASE_URI = 'https://storage.googleapis.com/angle-builds/';

// TODO(kreeger): Roll these builds:
// const LINUX_X64 = '';
// const LINUX_ARM7 = '';
// const LINUX_ARM8 = '';
const WINDOWS = '';
const DARWIN = 'angle-darwin-x64.tar.gz';

const DEPS_PATH = path.join(__dirname, '..', 'deps');
// const DEPS_LIB_PATH = path.join(DEPS_LIB_PATH, 'lib', libName);

const platform = os.platform().toLowerCase();
const arch = `${platform}-${os.arch().toLowerCase()}`;

const depsPath = path.join(__dirname, '..', 'deps');
const depsLibPath = path.join(depsPath, 'out', 'Release');

const mkdir = util.promisify(fs.mkdir);
const exists = util.promisify(fs.exists);

/** Ensures a directory exists, creates as needed. */
async function ensureDir(dirPath) {
  if (!await exists(dirPath)) {
    await mkdir(dirPath);
  }
}

/** Downloads Angle tarball and unpacks in the deps directory */
async function downloadAngleLibs(callback) {
  console.error('* Downloading ANGLE');

  const targetUri = BASE_URI + DARWIN;  // TODO(kreeger): Fix this.

  await ensureDir(DEPS_PATH);

  // If HTTPS_PROXY, https_proxy, HTTP_PROXY, or http_proxy is set
  const proxy = process.env['HTTPS_PROXY'] || process.env['https_proxy'] ||
      process.env['HTTP_PROXY'] || process.env['http_proxy'] || '';

  // Using object destructuring to construct the options object for the
  // http request.  the '...url.parse(targetUri)' part fills in the host,
  // path, protocol, etc from the targetUri and then we set the agent to the
  // default agent which is overridden a few lines down if there is a proxy
  const options = {...url.parse(targetUri), agent: https.globalAgent};

  if (proxy !== '') {
    options.agent = new HttpsProxyAgent(proxy);
  }

  const request = https.get(options, response => {
    const bar = new ProgressBar('[:bar] :rate/bps :percent :etas', {
      complete: '=',
      incomplete: ' ',
      width: 30,
      total: parseInt(response.headers['content-length'], 10)
    });

    if (platform === 'win32') {
      //
      // TODO(kreeger): write me.
      //
    } else {
      // All other platforms use a tarball:
      response
          .on('data',
              (chunk) => {
                bar.tick(chunk.length);
              })
          .pipe(tar.x({C: depsPath, strict: true}))
          .on('close', () => {
            if (callback !== undefined) {
              callback();
            }
          });
    }
  });

  request.end();
}

/** Builds application */
async function buildBindings() {
  console.error('* Building ANGLE bindings')
  cp.execSync('node-gyp rebuild', (err) => {
    if (err) {
      throw new Error('node-gyp failed with: ' + err);
    }
  });
}

/** Main execution function */
async function run() {
  await downloadAngleLibs(buildBindings);
  // await buildBindings();
}

run();
