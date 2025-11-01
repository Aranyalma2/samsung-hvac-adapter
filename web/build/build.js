// build.js

const fs = require('fs').promises;
const path = require('path');
const { minify: minifyHtml } = require('html-minifier-terser');
const terser = require('terser');
const CleanCSS = require('clean-css');
const { execFile } = require('child_process');
const { promisify } = require('util');
const injectables = require('./injectables.json');

const execFileAsync = promisify(execFile);

const htmlOptions = {
  collapseWhitespace: true,
  removeComments: true,
  minifyCSS: true,
  minifyJS: true,
};

async function ensureDir(dir) {
  await fs.mkdir(dir, { recursive: true });
}

async function copyAndMinify(src, dest) {
  try {
    await fs.access(src);
  } catch (err) {
    console.warn(`⚠️  Source ${src} does not exist, skipping...`);
    return;
  }
  const stat = await fs.stat(src);
  if (stat.isDirectory()) {
    await ensureDir(dest);
    const entries = await fs.readdir(src);
    for (const entry of entries) {
      await copyAndMinify(path.join(src, entry), path.join(dest, entry));
    }
  } else {
    const ext = path.extname(src).toLowerCase();
    let content = await fs.readFile(src, 'utf8');
    let minified;

    try {
      switch (ext) {
        case '.html':
          minified = await minifyHtml(content, htmlOptions);
          minified = await injectPatterns(minified, injectables);
          break;
        case '.js':
          minified = (await terser.minify(content)).code;
          minified = await injectPatterns(minified, injectables);
          break;
        case '.css':
          minified = new CleanCSS().minify(content).styles;
          break;
        default:
          minified = null;
      }

      await ensureDir(path.dirname(dest));
      if (minified != null) {
        await fs.writeFile(dest, minified, 'utf8');
      } else {
        await fs.copyFile(src, dest);
      }
    } catch (err) {
      console.error(`Error processing ${src}:`, err);
      await ensureDir(path.dirname(dest));
      await fs.copyFile(src, dest); // fallback
    }
  }
}

async function runFinishScript(scripts) {
  await runPreScript(scripts);
}

async function runPreScript(scripts) {
  for (const script of scripts) {
    const scriptPath = path.resolve(script);
    try {
      console.log(`▶️  Running ${scriptPath}...`);
      await execFileAsync(scriptPath, { shell: true });
      console.log(`✅ ${script} executed`);
    } catch (err) {
      console.error(`❌ Failed to run ${script}`);
      throw err;
    }
  }
}

async function injectPatterns(template, injections) {
  if (!template || !Array.isArray(injections)) return template;
  
  let result = template;
  injections.forEach(({pattern, replacement, type = 'string'}) => {
    if (pattern && replacement !== undefined) {
      let value = replacement;
      if (type === 'script') {
        try {
          value = eval(replacement);
        } catch (e) {
          value = replacement; // fallback to original if eval fails
        }
      }
      result = result.replace(
        new RegExp(`__%${pattern.replace(/[.*+?^${}()|[\]\\]/g, '\\$&')}%__`, 'g'), 
        String(value)
      );
    }
  });
  
  return result;
}

async function main() {

  const dist = path.resolve('../firmware/data/public');
  await fs.rm(dist, { recursive: true, force: true });
  await ensureDir(dist);

  const preScripts = [];

  const finishScripts = [];

  const public = [
    './public',
  ];

  await runPreScript(preScripts);

  for (const file of public) {
    const src = path.resolve(file);
    await copyAndMinify(src, dist);
  }

  await runFinishScript(finishScripts);

  console.log('✅ Minification complete. Output: /firmware/data');
}

main().catch(err => {
  console.error(err);
  process.exit(1);
});
