const path = require("path");

const suffix = `${process.platform}-${process.arch}-${process.versions.modules}`;
const baseName = `node-gles-${suffix}.node`;
const qualifiedName = path.resolve(__dirname, "..", baseName);

module.exports = { suffix, baseName, qualifiedName };
