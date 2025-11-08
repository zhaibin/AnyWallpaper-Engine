import type { ElementBounds } from '../types';
export declare const Bounds: {
    /**
     * Calculate element bounds in physical pixels
     */
    calculate(element: HTMLElement | string, dpiScale: number): ElementBounds;
    /**
     * Check if point is in bounds
     */
    isInBounds(x: number, y: number, bounds: ElementBounds): boolean;
    /**
     * Check if mouse (in physical pixels) is over element
     */
    isMouseOverElement(mouseX: number, mouseY: number, element: HTMLElement | string, dpiScale: number): boolean;
};
//# sourceMappingURL=bounds.d.ts.map