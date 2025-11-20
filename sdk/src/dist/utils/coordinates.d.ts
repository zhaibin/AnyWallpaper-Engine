import type { Position, CoordinateOffsets } from '../types';
export declare const Coordinates: {
    /**
     * Convert screen coordinates (physical pixels) to viewport CSS coordinates
     */
    screenToViewport(mouseX: number, mouseY: number, dpiScale: number): Position;
    /**
     * Get window and document offsets
     */
    getOffsets(dpiScale: number): CoordinateOffsets;
};
//# sourceMappingURL=coordinates.d.ts.map