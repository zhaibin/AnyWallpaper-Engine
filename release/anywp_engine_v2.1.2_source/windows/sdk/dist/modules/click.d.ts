import type { AnyWPSDK, ClickCallback, ClickHandlerOptions, ClickHandlerData } from '../types';
declare global {
    interface HTMLElement {
        _anywpDebugBorder?: HTMLElement;
    }
}
export declare const ClickHandler: {
    handleClick(x: number, y: number): void;
    waitForElement(selector: string, callback: (el: HTMLElement) => void, maxWait?: number): void;
    onClick(anyWP: AnyWPSDK, element: string | HTMLElement, callback: ClickCallback, options?: ClickHandlerOptions): void;
    refreshElementBounds(anyWP: AnyWPSDK, handler: ClickHandlerData): void;
    refreshBounds(anyWP: AnyWPSDK): number;
    clearHandlers(anyWP: AnyWPSDK): void;
};
//# sourceMappingURL=click.d.ts.map