# Web Build Guide - Quantum Idle

This guide explains how to build and deploy the web version of Quantum Idle using Emscripten.

## Why Web Version?

✅ **Advantages:**
- **No installation required** - plays in any browser
- **Cross-platform** - works on Windows, Mac, Linux, phones, tablets
- **Easy sharing** - just send a link
- **No DLL/library issues** - everything bundled together
- **Instant updates** - refresh page for latest version
- **Responsive UI** - adapts to any screen size

## Prerequisites

### 1. Install Emscripten SDK

#### Windows:
```cmd
# Clone the Emscripten SDK
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Install and activate latest version
emsdk install latest
emsdk activate latest

# Set up environment (run this in every new terminal)
emsdk_env.bat
```

#### Linux/macOS:
```bash
# Clone the Emscripten SDK
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk

# Install and activate latest version
./emsdk install latest
./emsdk activate latest

# Set up environment (add to ~/.bashrc or ~/.zshrc)
source ./emsdk_env.sh
```

### 2. Verify Installation

```bash
emcc --version
```

You should see output like: `emcc (Emscripten gcc/clang-like replacement) 3.1.x`

## Building for Web

### Quick Build

```bash
# Linux/macOS
./build_web.sh

# Windows
build_web.bat
```

### Manual Build

```bash
# Create build directory
mkdir build-web
cd build-web

# Configure with Emscripten
emcmake cmake .. -DCMAKE_BUILD_TYPE=Release -DPLATFORM_WEB=ON

# Build
emmake make -j4

# Output files will be in build-web/:
#   - QuantumIdle.html  (main page)
#   - QuantumIdle.js    (game code)
#   - QuantumIdle.wasm  (WebAssembly binary)
#   - QuantumIdle.data  (asset bundle, if you have assets)
```

## Testing Locally

You need to run a local web server (browsers block file:// access for security).

### Option 1: Python (easiest)
```bash
cd web/dist
python3 -m http.server 8080
```

Then open: http://localhost:8080

### Option 2: Node.js
```bash
npm install -g http-server
cd web/dist
http-server -p 8080
```

### Option 3: PHP
```bash
cd web/dist
php -S localhost:8080
```

## Deploying to Production

### Option 1: Static Hosting (Easiest)

Upload the contents of `web/dist/` to any static web host:

#### Free Options:
- **GitHub Pages**:
  1. Create repo, push code
  2. Settings → Pages → Enable
  3. Access at: `https://yourusername.github.io/repo-name/`

- **Netlify**:
  1. Drop `web/dist` folder on netlify.com
  2. Get instant deployment

- **Vercel**:
  ```bash
  npm i -g vercel
  cd web/dist
  vercel
  ```

- **Surge**:
  ```bash
  npm i -g surge
  cd web/dist
  surge
  ```

#### Paid Options (Better Performance):
- **Cloudflare Pages** - Fast CDN, free tier
- **AWS S3 + CloudFront** - Enterprise-grade
- **DigitalOcean Spaces** - Simple object storage

### Option 2: Own Server

Upload via FTP/SFTP to your web server:

```bash
# Example with scp
scp -r web/dist/* user@yourserver.com:/var/www/html/game/
```

### Option 3: Docker

```dockerfile
FROM nginx:alpine
COPY web/dist /usr/share/nginx/html
EXPOSE 80
```

```bash
docker build -t quantum-idle-web .
docker run -p 8080:80 quantum-idle-web
```

## Build Configuration

### File Sizes

Typical build sizes:
- **QuantumIdle.wasm**: ~500KB - 2MB (game code)
- **QuantumIdle.js**: ~100KB - 300KB (loader)
- **QuantumIdle.data**: Varies (your assets)
- **index.html**: ~10KB

### Optimization

For smaller file sizes, edit `CMakeLists.txt`:

```cmake
# Add these flags for maximum compression
"-O3"
"-s AGGRESSIVE_VARIABLE_ELIMINATION=1"
"--closure 1"
"-flto"  # Link-time optimization
```

For faster load times:
- Use **gzip compression** on your server
- Enable **brotli compression** if available
- Add **cache headers** (cache .wasm for 1 year)

Example nginx config:
```nginx
location ~* \.(wasm|js|data)$ {
    expires 1y;
    add_header Cache-Control "public, immutable";
    gzip on;
    gzip_types application/wasm application/javascript;
}
```

## Responsive Design

The game automatically adapts to:
- **Desktop**: Full 1280x720 display
- **Tablets**: Scaled to fit screen
- **Phones**: Portrait/landscape support
- **Touch**: Full touch input support

### Mobile Controls

SDL2 automatically translates:
- **Touch → Mouse clicks**
- **Pinch → Zoom** (if enabled)
- **Swipe → Drag**

## Troubleshooting

### Build Errors

**Error: `emcc: command not found`**
- Solution: Run `emsdk_env.bat` (Windows) or `source ./emsdk_env.sh` (Linux/macOS)

**Error: `SDL2 not found`**
- Solution: Emscripten includes SDL2, this shouldn't happen. Try `emcc --clear-cache`

**Error: `Cannot read properties of undefined`**
- Solution: Browser compatibility. Requires modern browser (Chrome 90+, Firefox 88+, Safari 15+)

### Runtime Errors

**Error: `Failed to load .wasm file`**
- Solution: Must use web server, not file://
- Run: `python3 -m http.server`

**Error: `Out of memory`**
- Solution: Increase memory limits in CMakeLists.txt:
  ```cmake
  "-s INITIAL_MEMORY=134217728"  # 128MB
  "-s MAXIMUM_MEMORY=536870912"  # 512MB
  ```

**Error: `GL context lost`**
- Solution: Browser tab was suspended. Reload page.

### Performance Issues

If game runs slowly:
1. Build in Release mode (`-DCMAKE_BUILD_TYPE=Release`)
2. Enable optimization flags (`-O3`)
3. Use Chrome/Edge (fastest WebAssembly support)
4. Close other tabs (frees memory)

## Browser Compatibility

| Browser | Version | Status |
|---------|---------|--------|
| Chrome | 90+ | ✅ Excellent |
| Edge | 90+ | ✅ Excellent |
| Firefox | 88+ | ✅ Good |
| Safari | 15+ | ✅ Good |
| Mobile Chrome | Latest | ✅ Good |
| Mobile Safari | iOS 15+ | ✅ Good |
| IE11 | Any | ❌ Not supported |

## Features

✅ **Working:**
- SDL2 input (mouse, keyboard, touch)
- OpenGL ES 2.0/3.0 rendering
- Full game logic
- Save/load (uses LocalStorage)
- Offline play (cached after first load)
- Responsive UI
- Fullscreen mode

⏳ **Limitations:**
- No native file dialogs (use web storage)
- Slightly lower performance than native
- Requires modern browser
- Initial load time (~2-5 seconds)

## Next Steps

1. **Test on multiple devices** - phone, tablet, desktop
2. **Add PWA support** - make it installable
3. **Optimize assets** - compress images
4. **Add analytics** - track usage
5. **Set up CI/CD** - auto-deploy on push

## Need Help?

- **Emscripten Docs**: https://emscripten.org/docs/
- **SDL2 Web Guide**: https://wiki.libsdl.org/SDL2/README/emscripten
- **WebGL Issues**: Check browser console (F12)

---

**Enjoy your cross-platform quantum idle game!** 🎮✨
