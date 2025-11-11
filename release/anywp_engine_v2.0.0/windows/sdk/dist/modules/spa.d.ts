import type { AnyWPSDK } from '../types';
import type { ClickHandler as ClickHandlerType } from './click';
declare global {
    interface Window {
        React?: unknown;
        ReactDOM?: unknown;
        Vue?: unknown;
        angular?: unknown;
    }
}
export declare const SPA: {
    detect(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType): void;
    setSPAMode(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType, enabled: boolean): void;
    setupMonitoring(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType): void;
    teardownMonitoring(anyWP: AnyWPSDK): void;
    onRouteChange(anyWP: AnyWPSDK, clickHandler: typeof ClickHandlerType): void;
};
//# sourceMappingURL=spa.d.ts.map