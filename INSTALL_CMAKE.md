# Installing CMake on Windows

## Method 1: Official Installer (Easiest)

### Step 1: Download
Go to: https://cmake.org/download/

Look for **"Binary distributions"** section and download:
- **cmake-3.xx.x-windows-x86_64.msi** (Windows x64 Installer)

### Step 2: Install
1. Run the downloaded .msi file
2. Accept the license agreement
3. **CRITICAL**: On the "Install Options" screen, select:
   - ✅ **"Add CMake to the system PATH for all users"**
   - OR ✅ **"Add CMake to the system PATH for the current user"**

   ![This ensures cmake command works from anywhere]

4. Click Next and Install
5. Wait for installation to complete

### Step 3: Verify
1. **Close any open Command Prompt windows**
2. Open a **NEW** Command Prompt
3. Type: `cmake --version`
4. You should see something like: `cmake version 3.28.x`

### Step 4: Build the Game
```cmd
cd path\to\C-Game
build.bat
```

---

## Method 2: Chocolatey Package Manager

If you have Chocolatey installed:

```cmd
choco install cmake
```

Then restart your terminal.

---

## Method 3: Visual Studio Installer

If you're installing Visual Studio Build Tools:

1. Run the Visual Studio Installer
2. Select "Desktop development with C++"
3. On the right panel, under "Optional", check:
   - ✅ **C++ CMake tools for Windows**
4. Install

This installs CMake but might not add it to PATH automatically.

---

## Troubleshooting

### "cmake is not recognized" even after installing

**Cause**: CMake not in PATH or terminal not restarted

**Fix**:
1. Close ALL Command Prompt/PowerShell windows
2. Open a NEW terminal
3. Try `cmake --version` again

If still doesn't work:

#### Manually Add to PATH
1. Press `Windows Key`, search for "Environment Variables"
2. Click "Edit the system environment variables"
3. Click "Environment Variables" button
4. Under "System variables" (or "User variables"), find "Path"
5. Click "Edit"
6. Click "New"
7. Add: `C:\Program Files\CMake\bin`
8. Click OK on all windows
9. Restart terminal

### Wrong version installed

CMake 3.15+ is required. Check with:
```cmd
cmake --version
```

If too old, uninstall and reinstall the latest version.

---

## Quick Test

After installing, test that CMake works:

```cmd
cmake --version
```

Expected output:
```
cmake version 3.28.x
CMake suite maintained and supported by Kitware (kitware.com/cmake).
```

---

## Next Steps

Once CMake is installed:

1. Install Visual Studio Build Tools (if not done yet)
2. Run `build.bat` in the C-Game folder
3. The script will handle SDL2 automatically

---

## Alternative: Use Pre-built Binaries

If you can't install CMake system-wide, download the ZIP version:
1. Download "cmake-3.xx.x-windows-x86_64.zip"
2. Extract to `C:\cmake`
3. Use full path when building:
   ```cmd
   C:\cmake\bin\cmake --version
   ```

But installing properly is easier long-term!
