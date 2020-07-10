const path = require("path");
const fs = require("fs");
const request = require("request");
const fileName = require("./file-name");

if (fs.existsSync(fileName.qualifiedName)) { return; }
if (process.env["NODE_GLES_SKIP_BINARY_DOWNLOAD_FOR_CI"] || process.env["npm_config_node_gles_skip_binary_download_for_ci"]) { return; }

// Get the version of the library;
const pkg = require(path.resolve(__dirname, "..", "package.json"));
const packageVersion = pkg.version;
const baseUrl = process.env["NODE_GLES_BINARY_URL"] || process.env["npm_config_node_gles_binary_url"] || "https://github.com/Prior99/node-gles/releases/download";
const url = `${baseUrl}/v${packageVersion}/${fileName.baseName}`;

console.info(`Downloading node-gles prebuilt binary from "${url}".`);

const destination = fs.createWriteStream(fileName.qualifiedName);

request.get(url)
  .on("error", err => { throw err; })
  .on("response", response => {
    if (response.statusCode >= 200 && response.statusCode < 300) {
      response.pipe(destination);
      destination.on("finish", () => {
        console.info("Successfully downloaded binaries for node-gles.");
      });
      return;
    }
    if (response.statusCode === 404) {
      throw new Error(`No supported node-gles ${packageVersion} build found for node ${process.version} on ${process.platform} (${process.arch}).`);
    } else {
      throw new Error(`Error downloading binaries for node-gles ${packageVersion}. Received status code ${response.statusCode}`)
    }
    destination.close();
    fs.unlink(fileName.qualifiedName);
  });
