// Rollup configuration for building AnyWP SDK with TypeScript support
import resolve from '@rollup/plugin-node-resolve';
import typescript from '@rollup/plugin-typescript';
import terser from '@rollup/plugin-terser';

const isProduction = process.env.NODE_ENV === 'production';

export default [
  // Standard build (unminified)
  {
    input: 'index.ts',  // TypeScript entry point (100% TypeScript)
    output: {
      file: '../anywp_sdk.js',
      format: 'iife',
      name: 'AnyWPBundle',
      banner: '// AnyWP Engine SDK v4.2.0 - JavaScript Bridge\n// Auto-injected into WebView2\n// React/Vue SPA Compatible\n// Built with TypeScript modular architecture (100% TS)\n',
      footer: '// Built from modular source - see windows/sdk/ for source code'
    },
    plugins: [
      resolve(),
      // TypeScript plugin for .ts files
      typescript({
        tsconfig: './tsconfig.json',
        compilerOptions: {
          declaration: false,  // Disable .d.ts generation for main build
          declarationMap: false,
          sourceMap: false,
          outDir: undefined  // Let Rollup handle output
        }
      })
    ]
  },
  
  // Minified build (production)
  ...(isProduction ? [{
    input: 'index.ts',
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
        compilerOptions: {
          declaration: false,
          declarationMap: false,
          sourceMap: true,
          outDir: undefined  // Let Rollup handle output
        }
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
