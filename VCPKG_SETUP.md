# Using vcpkg with CLion

You already have vcpkg integrated in CLion! This is actually the easiest way to get SDL2 working.

## Step 1: Install SDL2 via vcpkg

Open PowerShell or Command Prompt and run:

```cmd
cd C:\Users\Computer\.vcpkg-clion\vcpkg
.\vcpkg install sdl2:x64-windows
```

This will download and compile SDL2 specifically for your 64-bit Windows system.

## Step 2: Update CMakeLists.txt

We need to use `find_package` instead of manual paths. The CMakeLists.txt has been updated to support vcpkg.

## Step 3: Reload CMake in CLion

1. **Tools → CMake → Reset Cache and Reload Project**
2. **Build → Rebuild Project**

That's it! vcpkg will automatically provide SDL2 to CMake.

## Why This Works

- vcpkg automatically handles:
  - Correct library architecture (64-bit)
  - Proper include paths
  - Correct linking order
  - Debug vs Release builds
- No manual SDL2 installation needed
- No path conflicts

## If You Get "vcpkg not found"

The vcpkg executable is at:
```
C:\Users\Computer\.vcpkg-clion\vcpkg\vcpkg.exe
```

You can also install from within CLion:
1. File → Settings → Build, Execution, Deployment → vcpkg
2. Use the vcpkg integration to install SDL2

## Verifying Installation

After `vcpkg install sdl2:x64-windows` completes, you should see:
```
SDL2 x64-windows is installed
```

Then CMake will find it automatically using `find_package(SDL2 CONFIG REQUIRED)`.
