import type { ElementBounds } from '../types';
export declare const Debug: {
    enabled: boolean;
    /**
     * Enable debug mode
     */
    enable(): void;
    /**
     * Log with debug control
     */
    log(message: string, forceLog?: boolean): void;
    /**
     * Show debug border around element
     */
    showBorder(bounds: ElementBounds, element: HTMLElement, dpiScale: number): void;
    /**
     * Detect debug mode from URL parameter
     */
    detectFromURL(): void;
};
//# sourceMappingURL=debug.d.ts.map