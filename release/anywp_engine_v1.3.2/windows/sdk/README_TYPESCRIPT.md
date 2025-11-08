# AnyWP SDK - TypeScript Migration Guide

**Status**: 🚧 In Progress (Phase 1 Complete)  
**Version**: v4.2.0  
**Started**: 2025-11-07

---

## 📋 Migration Phases

### ✅ Phase 1: Infrastructure Setup (COMPLETED)

- ✅ Added TypeScript configuration (`tsconfig.json`)
- ✅ Updated `package.json` with TypeScript dependencies
- ✅ Created comprehensive type definitions (`types.ts`)
- ✅ Updated Rollup configuration for TypeScript support
- ✅ Migrated utility modules to TypeScript:
  - ✅ `utils/debug.ts`
  - ✅ `utils/bounds.ts`
  - ✅ `utils/coordinates.ts`

### 🚧 Phase 2: Core & Modules Migration (IN PROGRESS)

**Planned**:
- [ ] Migrate core modules (`core/AnyWP.ts`, `core/init.ts`)
- [ ] Migrate function modules:
  - [ ] `modules/events.ts`
  - [ ] `modules/click.ts`
  - [ ] `modules/drag.ts`
  - [ ] `modules/storage.ts`
  - [ ] `modules/spa.ts`
  - [ ] `modules/animations.ts`
- [ ] Create TypeScript entry point (`index.ts`)

### 🎯 Phase 3: Build & Test (PENDING)

- [ ] Generate `.d.ts` declaration files
- [ ] Test TypeScript build
- [ ] Update documentation
- [ ] Publish v5.0.0 (TypeScript version)

---

## 🛠️ Current Setup

### TypeScript Configuration

**Target**: ES2020  
**Module**: ESNext  
**Strict Mode**: Enabled  
**Output**: `dist/` directory

**Key Features**:
- ✅ Full type checking
- ✅ Declaration file generation
- ✅ Source maps
- ✅ Strict null checks
- ✅ No unused locals/parameters
- ✅ No implicit returns

### Build Scripts

```bash
# TypeScript compilation
npm run compile

# Type checking (no emit)
npm run typecheck

# Full build (compile + bundle)
npm run build

# Production build (minified)
npm run build:production

# Clean dist directory
npm run clean
```

---

## 📦 Dependencies

### New TypeScript Dependencies

```json
{
  "typescript": "^5.3.3",
  "@rollup/plugin-typescript": "^11.1.6",
  "@types/node": "^20.11.5",
  "tslib": "^2.6.2",
  "rimraf": "^5.0.5"
}
```

### Installation

```bash
cd windows/sdk
npm install
```

---

## 🔧 Type Definitions

### Core Types

Located in `types.ts`:

```typescript
// Element bounds
interface ElementBounds {
  left: number;
  top: number;
  right: number;
  bottom: number;
  width: number;
  height: number;
}

// Position coordinates
interface Position {
  x: number;
  y: number;
}

// Drag options
interface DraggableOptions {
  persistKey?: string;
  onDragStart?: (pos: Position) => void;
  onDrag?: (data: DragEventData) => void;
  onDragEnd?: (pos: Position) => void;
  bounds?: BoundsConstraint;
}

// Click handler options
interface ClickHandlerOptions {
  immediate?: boolean;
  waitFor?: boolean;
  maxWait?: number;
  autoRefresh?: boolean;
  delay?: number;
  debug?: boolean;
}

// Main SDK interface
interface AnyWPSDK {
  version: string;
  dpiScale: number;
  screenWidth: number;
  screenHeight: number;
  interactionEnabled: boolean;
  
  onClick(element: string | HTMLElement, callback: (x: number, y: number) => void, options?: ClickHandlerOptions): void;
  makeDraggable(element: string | HTMLElement, options?: DraggableOptions): void;
  // ... more methods
}
```

---

## 🎯 Usage Examples

### For TypeScript Projects

```typescript
import { AnyWP } from 'anywp-sdk';

// Type-safe API calls
AnyWP.onClick('#myButton', (x: number, y: number) => {
  console.log(`Clicked at ${x}, ${y}`);
}, {
  debug: true,
  immediate: true
});

AnyWP.makeDraggable('#myElement', {
  persistKey: 'my-element-position',
  onDragEnd: (pos) => {
    console.log(`Final position: ${pos.x}, ${pos.y}`);
  }
});
```

### For JavaScript Projects

Still fully compatible:

```javascript
AnyWP.onClick('#myButton', (x, y) => {
  console.log(`Clicked at ${x}, ${y}`);
});

AnyWP.makeDraggable('#myElement', {
  persistKey: 'my-element-position'
});
```

---

## 📊 Benefits of TypeScript

### 1. Type Safety ✅

```typescript
// ❌ Error: Type 'number' is not assignable to type 'string'
AnyWP.onClick(123, () => {});

// ✅ Correct
AnyWP.onClick('#button', () => {});
```

### 2. Better IDE Support ✅

- IntelliSense autocomplete
- Parameter hints
- Jump to definition
- Refactoring tools

### 3. Compile-Time Error Detection ✅

```typescript
// ❌ Error: Property 'invalidOption' does not exist
AnyWP.makeDraggable('#element', {
  invalidOption: true
});

// ✅ Correct
AnyWP.makeDraggable('#element', {
  persistKey: 'my-key'
});
```

### 4. Self-Documenting Code ✅

```typescript
// Types serve as inline documentation
function makeDraggable(
  element: string | HTMLElement,
  options?: DraggableOptions
): void {
  // Implementation
}
```

---

## 🔄 Migration Strategy

### Incremental Approach

We're using a **hybrid approach** to minimize disruption:

1. ✅ **Phase 1**: Infrastructure + Utility modules
2. 🚧 **Phase 2**: Core + Function modules (can be done incrementally)
3. 🎯 **Phase 3**: Full TypeScript build + type declarations

### Backward Compatibility

- ✅ JavaScript projects continue to work
- ✅ No breaking API changes
- ✅ Optional TypeScript types
- ✅ Gradual migration path

---

## 🧪 Testing TypeScript Code

### Type Checking

```bash
# Check types without building
npm run typecheck
```

### Build Test

```bash
# Compile TypeScript to JavaScript
npm run compile

# Full build with bundling
npm run build
```

### Output Verification

```bash
# Check generated declarations
ls -la dist/*.d.ts

# View compiled JavaScript
cat dist/utils/debug.js
```

---

## 📚 Migration TODO

### High Priority

- [ ] Migrate `core/AnyWP.ts` (main SDK object)
- [ ] Migrate `modules/drag.ts` (most complex module)
- [ ] Migrate `modules/click.ts`
- [ ] Create `index.ts` entry point

### Medium Priority

- [ ] Migrate `modules/storage.ts`
- [ ] Migrate `modules/events.ts`
- [ ] Migrate `modules/spa.ts`
- [ ] Migrate `modules/animations.ts`

### Low Priority

- [ ] Update examples with TypeScript
- [ ] Add JSDoc comments with TypeScript types
- [ ] Create TypeScript usage guide

---

## 🐛 Known Issues

### Current Limitations

1. **Hybrid Build**: Currently uses JavaScript entry point with TypeScript utilities
   - **Solution**: Will migrate to TypeScript entry point in Phase 2

2. **No Type Declarations Yet**: `.d.ts` files not generated for main SDK
   - **Solution**: Will be generated in Phase 3

3. **Some `any` Types**: Temporary use of `any` in some places
   - **Solution**: Will be refined during full migration

---

## 📖 Resources

### TypeScript Documentation

- [TypeScript Handbook](https://www.typescriptlang.org/docs/handbook/intro.html)
- [TypeScript Deep Dive](https://basarat.gitbook.io/typescript/)
- [Rollup TypeScript Plugin](https://github.com/rollup/plugins/tree/master/packages/typescript)

### Project Documentation

- [SDK Refactoring Report](../../docs/SDK_REFACTORING_REPORT.md)
- [Web Developer Guide](../../docs/WEB_DEVELOPER_GUIDE_CN.md)
- [API Usage Examples](../../docs/API_USAGE_EXAMPLES.md)

---

## 🎯 Next Steps

1. **Install Dependencies**: `npm install` (adds TypeScript packages)
2. **Test Compilation**: `npm run compile`
3. **Continue Migration**: Migrate remaining modules to TypeScript
4. **Test Build**: `npm run build`
5. **Update Documentation**: Document TypeScript usage

---

**Status**: Phase 1 完成！基础设施已就绪，可以继续迁移核心模块。

**Contact**: 如有问题请参考文档或联系开发团队

