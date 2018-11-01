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

const cp = require('child_process');
const os = require('os');

const BASE_URI = 'https://storage.googleapis.com/angle-builds/';

// TODO(kreeger): Roll these builds:
const LINUX_X64 = '';
const LINUX_ARM7 = '';
const LINUX_ARM8 = '';
const DARWIN = '';
const WINDOWS = '';

const arch = (os.platform() + os.arch()).toLowerCase();
console.log('platform: ' + arch);

/** Downloads ... */
async function downloadAngleLibs() {
  console.error('* Downloading ANGLE ...');
}

/** Builds application */
async function buildBindings() {
  console.error('* Building bindings ...');
  cp.execSync('node-gyp rebuild', (err) => {
    if (err) {
      throw new Error('node-gyp failed with: ' + err);
    }
  });
}

/** Main execution function */
async function run() {
  await downloadAngleLibs();
  await buildBindings();
}

run();
