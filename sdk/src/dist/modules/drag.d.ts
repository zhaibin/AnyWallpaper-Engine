import type { AnyWPSDK, DraggableOptions } from '../types';
declare global {
    interface HTMLElement {
        __anyWP_dragHandler?: {
            handleGlobalMouse: (event: Event) => void;
        };
    }
}
export declare const DragHandler: {
    makeDraggable(anyWP: AnyWPSDK, element: string | HTMLElement, options?: DraggableOptions): void;
    removeDraggable(anyWP: AnyWPSDK, element: string | HTMLElement): void;
    resetPosition(anyWP: AnyWPSDK, element: string | HTMLElement, position?: {
        left: number;
        top: number;
    }): boolean;
};
//# sourceMappingURL=drag.d.ts.map