import type { AnyWPSDK, MouseCallback, KeyboardCallback } from '../types';
import type { ClickHandler as ClickHandlerType } from './click';
import type { Animations as AnimationsType } from './animations';
export declare const Events: {
    setup(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType, animationsHandler: typeof AnimationsType): void;
    onMouse(anyWP: AnyWPSDK, callback: MouseCallback): void;
    onKeyboard(anyWP: AnyWPSDK, callback: KeyboardCallback): void;
    onVisibilityChange(anyWP: AnyWPSDK, callback: (visible: boolean) => void): void;
    notifyVisibilityChange(anyWP: AnyWPSDK, visible: boolean): void;
};
//# sourceMappingURL=events.d.ts.map