# Include Files

This directory contains the public header files for the AnyWP Engine plugin.

## Files

### Primary Headers (Recommended)
- `anywallpaper_engine_plugin.h` - Main plugin header
- `anywallpaper_engine_plugin_c_api.h` - C API header

### Compatibility Headers
- `any_wallpaper_engine_plugin.h` - **Auto-generated copy for Flutter compatibility**
- `any_wallpaper_engine_plugin_c_api.h` - **Auto-generated copy for Flutter compatibility**

## Why Two Versions?

Flutter's plugin system generates code that expects header files to follow a specific naming pattern. When we renamed the headers to follow our unified naming convention (`anywallpaper` without underscore between "any" and "wallpaper"), Flutter's generated code still references the old naming pattern.

To maintain compatibility with Flutter's auto-generated code while following our naming convention:

1. **Primary headers** use our naming standard: `anywallpaper_engine_plugin.h`
2. **Compatibility headers** are copies that match Flutter's expected pattern: `any_wallpaper_engine_plugin.h`

## Maintenance

When updating the plugin code:
1. Edit the **primary headers** only
2. The **compatibility headers** are automatically generated during build
3. Do not manually edit the compatibility headers

---

**Note**: This is a temporary solution until we can configure Flutter's code generator to use our preferred naming convention.


