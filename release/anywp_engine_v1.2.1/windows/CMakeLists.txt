cmake_minimum_required(VERSION 3.14)
project(anywp_engine LANGUAGES CXX)

set(PLUGIN_NAME " anywp_engine_plugin\)
set(PRECOMPILED_DIR \/..\)
set(PRECOMPILED_DLL \/bin/anywp_engine_plugin.dll\)
set(PRECOMPILED_LIB \/lib/anywp_engine_plugin.lib\)
set(SOURCE_FILE \/src/anywp_engine_plugin.cpp\)

if(EXISTS AND EXISTS )
 message(STATUS \AnyWP Engine: 鉁?浣跨敤棰勭紪璇?DLL  \)
 add_library( SHARED IMPORTED GLOBAL)
 set_target_properties( PROPERTIES
 IMPORTED_LOCATION \\
 IMPORTED_IMPLIB \\
 )
 target_include_directories( INTERFACE
 \/include\
 )
else()
 message(STATUS \AnyWP Engine: 馃敡 鏈壘鍒伴缂栬瘧 DLL锛岃浆涓烘簮鐮佹瀯寤篭)
 if(NOT EXISTS )
 message(FATAL_ERROR \AnyWP Engine: 婧愮爜鏂囦欢涓嶅瓨鍦? \)
 endif()
 add_library( SHARED
 \\
 )
 apply_standard_settings()
 set_target_properties( PROPERTIES
 CXX_STANDARD 17
 CXX_STANDARD_REQUIRED ON
 CXX_VISIBILITY_PRESET hidden
 )
 target_compile_definitions( PRIVATE FLUTTER_PLUGIN_IMPL)
 target_include_directories( PRIVATE
 \/include\
 )
 target_include_directories( INTERFACE
 \/include\
 )
 target_link_libraries( PRIVATE flutter flutter_wrapper_plugin)
 set(WEBVIEW2_PACKAGE_DIR \/packages/Microsoft.Web.WebView2.1.0.2592.51\)
 if(EXISTS )
 target_include_directories( PRIVATE \/build/native/include\)
 if(CMAKE_SIZEOF_VOID_P EQUAL 8)
 target_link_libraries( PRIVATE \/build/native/x64/WebView2LoaderStatic.lib\)
 else()
 target_link_libraries( PRIVATE \/build/native/x86/WebView2LoaderStatic.lib\)
 endif()
 else()
 message(WARNING \AnyWP Engine: 鏈壘鍒?WebView2 NuGet 鍖咃紝璇峰厛鎵ц nuget restore\)
 endif()
endif()

set(anywp_engine_bundled_libraries
 \/bin/anywp_engine_plugin.dll\
 \/bin/WebView2Loader.dll\
 PARENT_SCOPE
)
