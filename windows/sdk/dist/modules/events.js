// Event system module
import { Debug } from '../utils/debug';
export const Events = {
    _setupCompleted: false,
    _eventHandlers: {},
    // Setup event listeners
    setup(anyWP, clickHandler, animationsHandler) {
        // Prevent duplicate setup
        if (this._setupCompleted) {
            console.log('[AnyWP] Events already setup, skipping duplicate initialization');
            return;
        }
        // Create event handlers (store references for potential cleanup)
        this._eventHandlers.mouse = function (event) {
            const customEvent = event;
            anyWP._mouseCallbacks.forEach(function (cb) {
                cb(customEvent.detail);
            });
        };
        this._eventHandlers.keyboard = function (event) {
            const customEvent = event;
            anyWP._keyboardCallbacks.forEach(function (cb) {
                cb(customEvent.detail);
            });
        };
        this._eventHandlers.click = function (event) {
            const customEvent = event;
            clickHandler.handleClick(customEvent.detail.x, customEvent.detail.y);
        };
        this._eventHandlers.interactionMode = function (event) {
            const customEvent = event;
            anyWP.interactionEnabled = customEvent.detail.enabled;
            Debug.log('Interaction mode: ' + (anyWP.interactionEnabled ? 'ON' : 'OFF'), true);
        };
        this._eventHandlers.visibility = function (event) {
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
        };
        this._eventHandlers.resize = function () {
            Debug.log('Window resized, refreshing...');
            setTimeout(function () {
                clickHandler.refreshBounds(anyWP);
            }, 200);
        };
        // Register event listeners
        window.addEventListener('AnyWP:mouse', this._eventHandlers.mouse);
        window.addEventListener('AnyWP:keyboard', this._eventHandlers.keyboard);
        window.addEventListener('AnyWP:click', this._eventHandlers.click);
        window.addEventListener('AnyWP:interactionMode', this._eventHandlers.interactionMode);
        window.addEventListener('AnyWP:visibility', this._eventHandlers.visibility);
        window.addEventListener('resize', this._eventHandlers.resize);
        this._setupCompleted = true;
        console.log('[AnyWP] Events setup completed');
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