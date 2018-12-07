##
# @license
# Copyright 2018 Google Inc. All Rights Reserved.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# =============================================================================

# Node.js TensorFlow Binding config:
{
  'variables' : {
    'angle_lib_dir': '<(module_root_dir)/angle/out/Release'
  },
  'targets' : [{
    'target_name' : 'nodejs_gl_binding',
    'sources' : [
      'binding/binding.cc',
      'binding/egl_context_wrapper.cc',
      'binding/webgl_extensions.cc',
      'binding/webgl_rendering_context.cc'
    ],
    'include_dirs' : [
      '..',
      '<(module_root_dir)/angle/include'
    ],
    'conditions' : [
      [
        'OS=="linux"', {
          'libraries' : [
            '-Wl,-rpath,<@(angle_lib_dir)',
            '-lGLESv2',
            '-lEGL',
          ],
          'library_dirs' : ['<(angle_lib_dir)'],
        }
      ],
      [
        'OS=="mac"', {
          'libraries' : [
            '-Wl,-rpath,<@(angle_lib_dir)',
            '-lGLESv2',
            '-lEGL',
          ],
          'library_dirs' : ['<(angle_lib_dir)'],
        }
      ]
    ]
  }]
}
