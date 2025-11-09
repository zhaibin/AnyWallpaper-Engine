import type { AnyWPSDK, StateLoadCallback } from '../types';
export declare const Storage: {
    /**
     * Load state from native storage
     */
    load(anyWP: AnyWPSDK, key: string, callback: StateLoadCallback): void;
    /**
     * Save custom state
     */
    save(anyWP: AnyWPSDK, key: string, value: any): void;
    /**
     * Clear all saved state
     */
    clear(anyWP: AnyWPSDK): void;
    /**
     * Save element position
     */
    saveElementPosition(anyWP: AnyWPSDK, key: string, x: number, y: number): void;
};
//# sourceMappingURL=storage.d.ts.map