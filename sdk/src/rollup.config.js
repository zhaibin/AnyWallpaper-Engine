// Rollup configuration for building AnyWP SDK with TypeScript support
import resolve from '@rollup/plugin-node-resolve';
import typescript from '@rollup/plugin-typescript';
import terser from '@rollup/plugin-terser';
import replace from '@rollup/plugin-replace';
import { readFileSync } from 'fs';

// Read version from package.json (single source of truth)
const pkg = JSON.parse(readFileSync('./package.json', 'utf-8'));
const version = pkg.version;

const isProduction = process.env.NODE_ENV === 'production';

export default [
  // Standard build (unminified)
  {
    input: 'index.ts',  // TypeScript entry point (100% TypeScript)
    output: {
      file: '../../sdk/dist/anywp_sdk.js',
      format: 'iife',
      name: 'AnyWPBundle',
      banner: `// AnyWP Engine SDK v${version} - JavaScript Bridge\n// Auto-injected into WebView2\n// React/Vue SPA Compatible\n// Built with TypeScript modular architecture (100% TS)\n`,
      footer: '// Built from modular source - see sdk/src/ for source code'
    },
    plugins: [
      replace({
        preventAssignment: true,
        values: {
          '__SDK_VERSION__': version
        }
      }),
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
      file: '../../sdk/dist/anywp_sdk.min.js',
      format: 'iife',
      name: 'AnyWPBundle',
      banner: `// AnyWP Engine SDK v${version} - Minified (TypeScript)\n`,
      sourcemap: true
    },
    plugins: [
      replace({
        preventAssignment: true,
        values: {
          '__SDK_VERSION__': version
        }
      }),
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
