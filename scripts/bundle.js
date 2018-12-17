const os = require('os');

console.log('-----------');
const arch = `${os.platform()}-${os.arch().toLowerCase()}`;
console.log('platform: ' + arch);
console.log('-----------');

// > tar -czvf angle-darwin-x64.tar.gz angle/out/Release/*.dylib angle/include/