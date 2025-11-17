#
# To learn more about a Podspec see http://guides.cocoapods.org/syntax/podspec.html.
# Run `pod lib lint anywp_engine.podspec` to validate before publishing.
#
Pod::Spec.new do |s|
  s.name             = 'anywp_engine'
  s.version          = '2.2.0'
  s.summary          = 'AnyWP Engine - Desktop Wallpaper Plugin for Flutter (macOS)'
  s.description      = <<-DESC
AnyWP Engine plugin for macOS, providing desktop wallpaper functionality using WKWebView.
Supports multi-monitor setups, power management, bidirectional communication, and more.
                       DESC
  s.homepage         = 'https://github.com/zhaibin/AnyWallpaper-Engine'
  s.license          = { :file => '../LICENSE' }
  s.author           = { 'zhaibin' => 'zhaibin@example.com' }

  s.source           = { :path => '.' }
  s.source_files     = 'Classes/**/*.{h,m}'
  s.resources        = ['../sdk/anywp_sdk.js', 'Resources/*.html', '../examples/test_carousel_v2_flutter_controlled.html', '../examples/test_carousel.html', '../examples/test_interactive_mode.html', '../examples/test_custom_scheme.html']
  s.dependency 'FlutterMacOS'

  s.platform = :osx, '10.14'
  s.pod_target_xcconfig = { 'DEFINES_MODULE' => 'YES' }
  s.swift_version = '5.0'
end

