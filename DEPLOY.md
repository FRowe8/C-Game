# 🚀 Deploy to GitHub Pages

This guide will help you deploy **Quantum Idle** to GitHub Pages so you can play it live in your browser!

## Quick Start (Automatic Deployment)

Your repository is already set up for automatic deployment! Here's how to get your game live:

### Step 1: Enable GitHub Pages

1. Go to your repository on GitHub: `https://github.com/FRowe8/C-Game`
2. Click **Settings** (top menu)
3. Click **Pages** (left sidebar)
4. Under "Source", select **GitHub Actions**
5. Click **Save**

### Step 2: Trigger the Build

The game will automatically build and deploy when you:
- Push to the `main`/`master` branch, OR
- Push to your current branch: `claude/cross-platform-game-engine-013AwcfzCsrvQNSqWs4LzZ8h`

Or manually trigger it:
1. Go to **Actions** tab
2. Click **Build and Deploy to GitHub Pages**
3. Click **Run workflow**
4. Select your branch
5. Click **Run workflow**

### Step 3: Access Your Game!

After the workflow completes (takes 3-5 minutes):

**Your game will be live at:**
```
https://frowe8.github.io/C-Game/
```

## 🎮 What Gets Deployed

The deployment workflow:

1. ✅ Checks out your code
2. ✅ Installs Emscripten (WebAssembly compiler)
3. ✅ Builds the game with CMake + Emscripten
4. ✅ Creates optimized web files:
   - `index.html` - Game interface
   - `QuantumIdle.js` - Game code
   - `QuantumIdle.wasm` - Compiled C++ (runs in browser!)
   - `QuantumIdle.data` - Game assets (if any)
5. ✅ Deploys to GitHub Pages

## 📊 Monitoring Deployment

### Check Build Status

1. Go to **Actions** tab in your repository
2. Look for the latest workflow run
3. Green checkmark ✅ = Success! Game is live
4. Red X ❌ = Build failed, click to see error logs

### Troubleshooting

**If the build fails:**

1. Check the **Actions** tab for error messages
2. Common issues:
   - Missing CMakeLists.txt → Already included ✅
   - Emscripten compilation errors → Check src/ files for C++17 compliance
   - Asset loading issues → Ensure web/assets exists (optional)

**If the page shows 404:**

1. Wait 2-3 minutes after deployment (GitHub Pages can be slow)
2. Check Settings → Pages to confirm GitHub Actions is selected
3. Hard refresh your browser (Ctrl+F5 / Cmd+Shift+R)

## 🎨 Customizing the Deployment

### Change Deployed Branches

Edit `.github/workflows/deploy.yml`:

```yaml
on:
  push:
    branches:
      - main          # Add/remove branches here
      - your-branch
```

### Build Configuration

The workflow uses these CMake settings:
```cmake
-DCMAKE_BUILD_TYPE=Release    # Optimized build
-DPLATFORM_WEB=ON             # Web platform flag
```

### Update HTML Template

The deployed `index.html` comes from `web/index.html`. Edit it to:
- Change title/description
- Add Google Analytics
- Customize styling
- Add social media meta tags

## 🌐 Custom Domain (Optional)

Want a custom domain like `quantum-idle.com`?

1. Create file `dist/CNAME` containing your domain
2. Configure DNS records to point to GitHub Pages:
   ```
   185.199.108.153
   185.199.109.153
   185.199.110.153
   185.199.111.153
   ```
3. Update Settings → Pages → Custom domain

## 📱 Testing Locally

Before deploying, test the web build locally:

```bash
# Build with Emscripten (requires Emscripten SDK)
./build_web.sh

# Verify assets were preloaded into the data bundle
python3 scripts/smoke_test_web_preload.py build-web/QuantumIdle.data \
  assets/fonts/Roboto-Regular.ttf

# Serve locally
cd build
python3 -m http.server 8000

# Open: http://localhost:8000/QuantumIdle.html
```

## 🔧 Advanced: Manual Deployment

If you prefer manual deployment:

```bash
# 1. Build
emcmake cmake -B build -DCMAKE_BUILD_TYPE=Release -DPLATFORM_WEB=ON
emmake make -C build -j$(nproc)

# 2. Prepare files
mkdir dist
cp build/QuantumIdle.html dist/index.html
cp build/QuantumIdle.{js,wasm,data} dist/

# 3. Deploy to gh-pages branch
git checkout --orphan gh-pages
git rm -rf .
cp -r dist/* .
git add .
git commit -m "Deploy game"
git push -f origin gh-pages
```

## 🎯 Performance Tips

### Optimize Build Size

The Emscripten build is already optimized with:
- `-O3` (maximum optimization)
- `--closure 1` (Google Closure Compiler)
- Tree shaking (removes unused code)

Current build size: ~1-2 MB total (very reasonable!)

### Loading Speed

The game uses:
- **WebAssembly** - Near-native performance
- **Preloading** - Assets loaded during splash screen
- **Progressive enhancement** - Works on all modern browsers

### Browser Compatibility

Supports:
- ✅ Chrome/Edge (recommended)
- ✅ Firefox
- ✅ Safari (macOS/iOS)
- ✅ Mobile browsers (with touch support)

Requires:
- WebGL 2.0
- WebAssembly
- Modern JavaScript (ES6+)

## 🚀 Next Steps

Once deployed:

1. **Share your game!**
   - Post on Reddit r/incremental_games
   - Share on social media with #QuantumIdle
   - Submit to io.games, itch.io, etc.

2. **Monitor analytics**
   - Add Google Analytics to track players
   - Monitor GitHub Pages traffic in Insights → Traffic

3. **Iterate**
   - Push updates to trigger automatic redeployment
   - Each push rebuilds and redeploys automatically!

## 📞 Need Help?

- 🐛 Build issues? Check **Actions** tab logs
- 💬 Questions? Open an issue on GitHub
- 📧 Want to monetize? Consider Patreon/Ko-fi links in-game

---

**🎉 Congratulations! Your game is about to go live!**

Visit your game at: **https://frowe8.github.io/C-Game/**

(After enabling GitHub Pages and pushing changes)
