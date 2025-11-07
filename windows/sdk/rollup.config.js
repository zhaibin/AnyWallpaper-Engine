// Rollup configuration for building AnyWP SDK
import resolve from '@rollup/plugin-node-resolve';
import terser from '@rollup/plugin-terser';

const isProduction = process.env.NODE_ENV === 'production';

export default [
  // Standard build (unminified)
  {
    input: 'index.js',
    output: {
      file: '../anywp_sdk.js',
      format: 'iife',
      name: 'AnyWPBundle',
      banner: '// AnyWP Engine SDK v4.2.0 - JavaScript Bridge\n// Auto-injected into WebView2\n// React/Vue SPA Compatible\n',
      footer: '// Built from modular source - see windows/sdk/ for source code'
    },
    plugins: [
      resolve()
    ]
  },
  
  // Minified build (production)
  ...(isProduction ? [{
    input: 'index.js',
    output: {
      file: '../anywp_sdk.min.js',
      format: 'iife',
      name: 'AnyWPBundle',
      banner: '// AnyWP Engine SDK v4.2.0 - Minified\n',
      sourcemap: true
    },
    plugins: [
      resolve(),
      terser({
        compress: {
          drop_console: false, // Keep console logs for debugging
          pure_funcs: []
        },
        mangle: {
          keep_fnames: true // Keep function names for debugging
        }
      })
    ]
  }] : [])
];

