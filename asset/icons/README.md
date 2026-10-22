# Dokkan Defense Calculator icon

The user's original artwork remains at ../icon.png (144 x 144 RGBA).
The previous ICO and original are backed up in build/before-icon-update/.

zDKIconMaster.png is a 1254 x 1254 RGBA restoration generated with the built-in image generation tool. It is a faithful generative reconstruction, rather than recovery of the original missing pixels. The original artwork supplies renditions through 144 px to keep small icons faithful; the restored master supplies the larger sizes.

Windows ICO: ../icon.ico, with 27 PNG-compressed 32-bit alpha frames:
16, 20, 24, 28, 30, 32, 36, 40, 42, 48, 56, 60, 64, 72, 80, 84, 96, 112, 120, 128, 144, 160, 168, 192, 224, 240, 256 px.

Individual PNG exports include those sizes plus 512 and 1024 px (29 sizes).
The ICO uses the Windows 256 px maximum; larger PNGs are reusable standalone assets. Windows icon size depends on DPI and shell view, rather than the monitor's total pixel resolution.

Regenerate the encoded resources without calling an image model:

~~~powershell
.\tools\zDKBuildIcons.ps1
~~~

The executable embeds asset/icon.ico. It loads small and large window icons at GetSystemMetricsForDpi sizes and reloads on WM_DPICHANGED. Owned icon handles are released when replaced or the window closes. The in-app header remains without an icon.

The UI suite checks actual small/large icon bitmap dimensions at 96, 120, 144, 168, 192, 240, 288, 336, 384, 480, 576, 672 and 768 DPI (100–800%).

References: [Microsoft icon guidance](https://learn.microsoft.com/en-us/windows/win32/menurc/about-icons), [LoadImageW](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-loadimagew), [GetSystemMetricsForDpi](https://learn.microsoft.com/en-us/windows/win32/api/winuser/nf-winuser-getsystemmetricsfordpi).
