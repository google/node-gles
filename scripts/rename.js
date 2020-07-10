const fs = require("fs");
const path = require("path");
const fileName = require("./file-name");

const source = path.resolve(__dirname, "..", "build", "Release", "nodejs_gl_binding.node");
const destination = fileName.qualifiedName;

console.log("Build artifact: " + destination);

fs.renameSync(source, destination);
