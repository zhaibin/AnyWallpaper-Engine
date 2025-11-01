# Build Notes

## Header File Naming Convention

### Background

The AnyWP Engine project uses a unified naming convention where "AnyWP" or "AnyWallpaper" is written without underscores between "any" and "wallpaper". However, Flutter's plugin system has specific expectations for header file naming.

### Current Solution

We maintain two versions of header files in `windows/include/anywallpaper_engine/`:

**Primary Headers** (for development):
- `anywallpaper_engine_plugin.h`
- `anywallpaper_engine_plugin_c_api.h`

**Compatibility Headers** (for Flutter):
- `any_wallpaper_engine_plugin.h` (copy)
- `any_wallpaper_engine_plugin_c_api.h` (copy)

### Build Process

1. When you modify plugin code, edit the **primary headers** only
2. After making changes, update the compatibility copies:

```powershell
cd windows\include\anywallpaper_engine
Copy-Item anywallpaper_engine_plugin.h -Destination any_wallpaper_engine_plugin.h -Force
Copy-Item anywallpaper_engine_plugin_c_api.h -Destination any_wallpaper_engine_plugin_c_api.h -Force
```

3. Or use the provided script (if available)

### Why This Approach?

Flutter generates `example/windows/flutter/generated_plugin_registrant.cc` which includes:
```cpp
#include <anywallpaper_engine/any_wallpaper_engine_plugin.h>
```

This file is auto-generated and will be overwritten on `flutter clean` and `flutter pub get`. The naming pattern it uses is based on Flutter's plugin naming conventions.

### Future Improvements

Options to consider:
1. **Configure Flutter** - Find if there's a way to configure Flutter's code generator
2. **Post-build script** - Automatically copy headers after generation
3. **Accept Flutter's convention** - Rename all files to match Flutter's expectations

For now, the dual-header approach provides the best balance between our naming standard and Flutter compatibility.

## Troubleshooting

### Error: Cannot find header file

If you see:
```
error C1083: Cannot find include file: "anywallpaper_engine/any_wallpaper_engine_plugin.h"
```

**Solution**:
```bash
cd windows\include\anywallpaper_engine
Copy-Item anywallpaper_engine_plugin.h -Destination any_wallpaper_engine_plugin.h -Force
Copy-Item anywallpaper_engine_plugin_c_api.h -Destination any_wallpaper_engine_plugin_c_api.h -Force
```

Then rebuild:
```bash
cd example
flutter clean
flutter build windows --debug
```

---

**Last Updated**: 2025-11-02  
**Version**: 1.0.0


