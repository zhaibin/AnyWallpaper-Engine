// Event system module
import { Debug } from '../utils/debug';
export const Events = {
    // Setup event listeners
    setup(anyWP, clickHandler, animationsHandler) {
        window.addEventListener('AnyWP:mouse', function (event) {
            const customEvent = event;
            anyWP._mouseCallbacks.forEach(function (cb) {
                cb(customEvent.detail);
            });
        });
        window.addEventListener('AnyWP:keyboard', function (event) {
            const customEvent = event;
            anyWP._keyboardCallbacks.forEach(function (cb) {
                cb(customEvent.detail);
            });
        });
        window.addEventListener('AnyWP:click', function (event) {
            const customEvent = event;
            clickHandler.handleClick(customEvent.detail.x, customEvent.detail.y);
        });
        window.addEventListener('AnyWP:interactionMode', function (event) {
            const customEvent = event;
            anyWP.interactionEnabled = customEvent.detail.enabled;
            Debug.log('Interaction mode: ' + (anyWP.interactionEnabled ? 'ON' : 'OFF'), true);
        });
        window.addEventListener('AnyWP:visibility', function (event) {
            const customEvent = event;
            const visible = customEvent.detail.visible;
            Debug.log('Visibility changed: ' + (visible ? 'visible' : 'hidden'), true);
            if (anyWP._visibilityCallback) {
                anyWP._visibilityCallback(visible);
            }
            if (!visible) {
                animationsHandler.pause(anyWP);
            }
            else {
                animationsHandler.resume(anyWP);
            }
        });
        window.addEventListener('resize', function () {
            Debug.log('Window resized, refreshing...');
            setTimeout(function () {
                clickHandler.refreshBounds(anyWP);
            }, 200);
        });
    },
    // Register mouse callback
    onMouse(anyWP, callback) {
        anyWP._mouseCallbacks.push(callback);
        Debug.log('Mouse callback registered');
    },
    // Register keyboard callback
    onKeyboard(anyWP, callback) {
        anyWP._keyboardCallbacks.push(callback);
        Debug.log('Keyboard callback registered');
    },
    // Register visibility callback
    onVisibilityChange(anyWP, callback) {
        anyWP._visibilityCallback = callback;
        Debug.log('Visibility callback registered');
    },
    // Notify visibility change (called by C++)
    notifyVisibilityChange(anyWP, visible) {
        console.log('[AnyWP] _notifyVisibilityChange called:', visible);
        if (anyWP._visibilityCallback && typeof anyWP._visibilityCallback === 'function') {
            try {
                anyWP._visibilityCallback(visible);
                console.log('[AnyWP] Visibility callback executed successfully');
            }
            catch (e) {
                console.error('[AnyWP] Error in visibility callback:', e);
            }
        }
        else {
            console.log('[AnyWP] No visibility callback registered');
        }
        try {
            const event = new CustomEvent('AnyWP:visibility', {
                detail: { visible: visible }
            });
            window.dispatchEvent(event);
            console.log('[AnyWP] AnyWP:visibility event dispatched');
        }
        catch (e) {
            console.error('[AnyWP] Error dispatching visibility event:', e);
        }
    }
};
//# sourceMappingURL=events.js.map