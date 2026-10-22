// Render geometry at target resolution; never upscale the source sprite sheet.
const fs = require('node:fs/promises');
const path = require('node:path');
const sharp = require('sharp');
const root = path.resolve(__dirname, '..', 'asset', 'ui');
const xmlEscape = text => text.replace(/&/g, '&amp;').replace(/</g, '&lt;').replace(/"/g, '&quot;');

async function main() {
  const catalog = JSON.parse(await fs.readFile(path.join(root, 'catalog.json'), 'utf8'));
  let count = 0;
  for (const asset of catalog.assets) {
    const source = path.join(root, asset.svg);
    const destination = path.join(root, asset.png_4x);
    await fs.mkdir(path.dirname(destination), { recursive: true });
    await sharp(source, { density: 288, limitInputPixels: 150000000 })
      .resize(asset.width * 4, asset.height * 4)
      .png({ compressionLevel: 9 }).toFile(destination);
    count++;
  }
  const columns = 4, cellW = 420, cellH = 208, top = 104;
  const width = columns * cellW + 32;
  const height = Math.ceil(catalog.assets.length / columns) * cellH + top + 20;
  let page = `<svg xmlns="http://www.w3.org/2000/svg" width="${width}" height="${height}" viewBox="0 0 ${width} ${height}">
  <defs><pattern id="checker" width="16" height="16" patternUnits="userSpaceOnUse"><rect width="16" height="16" fill="#100d14"/><path d="M0,0 H8 V8 H0 Z M8,8 H16 V16 H8 Z" fill="#151019"/></pattern></defs>
  <rect width="100%" height="100%" fill="#09070c"/>
  <text x="28" y="42" fill="#f5edf4" font-family="Segoe UI,sans-serif" font-size="26" font-weight="600">UI INTERFACE / VECTOR ASSET LIBRARY</text>
  <text x="28" y="72" fill="#b7a5b2" font-family="Segoe UI,sans-serif" font-size="15">38 individual assets · transparent exterior · editable geometry · 4× PNG exports</text>`;
  const composites = [];
  for (let i = 0; i < catalog.assets.length; i++) {
    const a = catalog.assets[i], x = 16 + (i % columns) * cellW, y = top + Math.floor(i / columns) * cellH;
    page += `<rect x="${x+4}" y="${y}" width="${cellW-12}" height="${cellH-12}" rx="8" fill="#110c13" stroke="#432035"/>
    <rect x="${x+12}" y="${y+38}" width="${cellW-28}" height="${cellH-62}" rx="3" fill="url(#checker)"/>
    <text x="${x+16}" y="${y+25}" fill="#ff66aa" font-family="Segoe UI,sans-serif" font-size="14">${String(i+1).padStart(2,'0')} / ${xmlEscape(a.id)}</text>`;
    const maxW = cellW - 44, maxH = cellH - 78;
    const factor = Math.min(maxW / a.width, maxH / a.height, 1.7);
    const w = Math.round(a.width * factor), h = Math.round(a.height * factor);
    const image = await sharp(path.join(root, a.svg), { density: 288 })
      .resize(w, h).png().toBuffer();
    composites.push({input: image, left: Math.round(x+cellW/2-w/2-2), top: Math.round(y+45+(maxH-h)/2)});
  }
  page += '</svg>';
  await fs.writeFile(path.join(root, 'contact-sheet.svg'), page);
  await sharp(Buffer.from(page)).composite(composites).png().toFile(path.join(root, 'contact-sheet.png'));

  // The standalone catalog SVG includes vector geometry, without embedded PNGs.
  let vectorPage = page.replace('</svg>', '');
  for (let i = 0; i < catalog.assets.length; i++) {
    const a = catalog.assets[i], x = 16+(i%columns)*cellW, y=top+Math.floor(i/columns)*cellH;
    const factor = Math.min((cellW-44)/a.width,(cellH-78)/a.height,1.7);
    let svg = await fs.readFile(path.join(root,a.svg),'utf8');
    svg=svg.replace(/<\?xml[^>]*\?>\s*/,'').replace(/\bid="([^"]+)"/g,(_,id)=>`id="a${i}-${id}"`).replace(/url\(#([^)]*)\)/g,(_,id)=>`url(#a${i}-${id})`).replace(/aria-labelledby="title desc"/g,`aria-labelledby="a${i}-title a${i}-desc"`);
    const left=x+cellW/2-a.width*factor/2-2, upper=y+45+((cellH-78)-a.height*factor)/2;
    vectorPage+=`<g transform="translate(${left} ${upper}) scale(${factor})">${svg}</g>`;
  }
  vectorPage+='</svg>';
  await fs.writeFile(path.join(root,'contact-sheet.svg'),vectorPage);
  console.log(`Rendered ${count} PNG assets at 4× and the catalog preview.`);
}
main().catch(error => { console.error(error); process.exitCode = 1; });
