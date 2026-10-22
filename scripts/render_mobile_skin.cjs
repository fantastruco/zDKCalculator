const fs = require('node:fs/promises');
const path = require('node:path');
const sharp = require('sharp');
const root = path.resolve(__dirname, '..', 'asset', 'ui', 'mobile');
async function main() {
  const records = JSON.parse(await fs.readFile(path.join(root, 'catalog.json'), 'utf8'));
  for (const asset of records) {
    const destination = path.join(root, asset.png);
    await fs.mkdir(path.dirname(destination), {recursive: true});
    const svgStat = await fs.stat(path.join(root, asset.svg));
    const pngStat = await fs.stat(destination).catch(() => null);
    if (pngStat && pngStat.mtimeMs >= svgStat.mtimeMs) continue;
    await sharp(path.join(root, asset.svg), {density: 72 * asset.scale})
      .resize(asset.width * asset.scale, asset.height * asset.scale)
      .png({compressionLevel: 8}).toFile(destination);
  }
  console.log(`Rendered ${records.length} mobile skin textures.`);
}
main().catch(error => { console.error(error); process.exitCode = 1; });
