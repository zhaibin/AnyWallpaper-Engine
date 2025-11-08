// Rollup configuration for building AnyWP SDK with TypeScript support
import resolve from '@rollup/plugin-node-resolve';
import typescript from '@rollup/plugin-typescript';
import terser from '@rollup/plugin-terser';

const isProduction = process.env.NODE_ENV === 'production';

export default [
  // Standard build (unminified)
  {
    input: 'index.js',  // Keep using JS entry for now (will migrate incrementally)
    output: {
      file: '../anywp_sdk.js',
      format: 'iife',
      name: 'AnyWPBundle',
      banner: '// AnyWP Engine SDK v4.2.0 - JavaScript Bridge\n// Auto-injected into WebView2\n// React/Vue SPA Compatible\n// Built with TypeScript modular architecture\n',
      footer: '// Built from modular source - see windows/sdk/ for source code'
    },
    plugins: [
      resolve(),
      // TypeScript plugin for .ts files (if any are imported)
      typescript({
        tsconfig: './tsconfig.json',
        declaration: false,  // Don't emit declarations in main build
        sourceMap: false
      })
    ]
  },
  
  // Minified build (production)
  ...(isProduction ? [{
    input: 'index.js',
    output: {
      file: '../anywp_sdk.min.js',
      format: 'iife',
      name: 'AnyWPBundle',
      banner: '// AnyWP Engine SDK v4.2.0 - Minified (TypeScript)\n',
      sourcemap: true
    },
    plugins: [
      resolve(),
      typescript({
        tsconfig: './tsconfig.json',
        declaration: false,
        sourceMap: true
      }),
      terser({
        compress: {
          drop_console: false,
          pure_funcs: []
        },
        mangle: {
          keep_fnames: true
        }
      })
    ]
  }] : [])
];
