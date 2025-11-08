import type { AnyWPSDK } from '../types';
import type { ClickHandler as ClickHandlerType } from './click';
declare global {
    interface Window {
        React?: any;
        ReactDOM?: any;
        Vue?: any;
        angular?: any;
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