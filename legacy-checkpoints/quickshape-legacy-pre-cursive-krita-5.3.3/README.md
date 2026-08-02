# QuickShape legacy pre-cursive test build

This is the broad-polish behavior that was interactively described as
"significantly better" on 2026-08-02, immediately before cursive-loop
preservation was added.

It targets the official **Krita 5.3.3 Qt5 x86_64 AppImage**. It deliberately
does not preserve cursive loops reliably; use disposable documents only.

On Linux Mint:

1. Put `krita-5.3.3-x86_64.AppImage` beside this folder, or pass its full path.
2. Make the AppImage and launcher executable:

       chmod +x krita-5.3.3-x86_64.AppImage
       chmod +x run-legacy-test.sh

3. Launch with:

       ./run-legacy-test.sh ../krita-5.3.3-x86_64.AppImage

The launcher extracts the AppImage locally on first use and uses isolated
Krita configuration and data directories. It does not install the plugin into
the system or the user's normal Krita profile.

Select **QuickShape Polish**, draw, hold the endpoint until the cyan preview
appears, then lift the pen. Leave Fidelity below 100; this legacy comparison
uses its original fixed 75% polishing strength.
