# AnyWP Engine v1.2.1 - 棰勭紪璇戠増鏈?
## 馃摝 鍖呭惈鍐呭

- in/ - 杩愯鏃?DLL锛坅nywp_engine_plugin.dll锛変笌 WebView2Loader
- lib/ - Dart 婧愮爜涓庨摼鎺ュ簱锛坅nywp_engine_plugin.lib锛?- include/ - C++ 鍏紑澶存枃浠?- windows/anywp_sdk.js - JavaScript SDK
- windows/src/ - C++ 瀹炵幇婧愮爜锛堝彲閫夋嫨鑷缂栬瘧锛?- windows/CMakeLists.txt - 鑷姩妫€娴嬮缂栬瘧/婧愮爜妯″紡

## 馃殌 蹇€熼泦鎴?
### 1. 鎺ㄨ崘锛氳繍琛屽畨瑁呰剼鏈?`powershell
.\setup_precompiled.bat
`

### 2. 鎴栨墜鍔ㄥ湪 pubspec.yaml 涓坊鍔狅細
`yaml
dependencies:
  anywp_engine:
    path: ./packages/anywp_engine_v1.2.1
`

### 3. 瀹夎鍚庤繍琛?`ash
flutter pub get
flutter build windows
`

### 4. 鍚姩绀轰緥
`ash
cd example_minimal
flutter pub get
flutter run -d windows
`

## 馃摎 瀹屾暣鏂囨。

- README.md / CHANGELOG_CN.md
- setup_precompiled.bat锛堣嚜鍔ㄥ畨瑁咃級
- verify_precompiled.bat锛堝揩閫熼獙璇侊級
- generate_pubspec_snippet.bat锛坧ubspec 鐗囨锛?
鏇村淇℃伅璇疯闂細https://github.com/zhaibin/AnyWallpaper-Engine
