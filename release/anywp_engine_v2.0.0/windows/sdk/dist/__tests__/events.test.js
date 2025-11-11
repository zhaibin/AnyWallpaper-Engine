/**
 * Events module tests
 */
import { describe, test, expect, beforeEach, afterEach, jest } from '@jest/globals';
import { Events } from '../modules/events';
const removeRegisteredHandlers = () => {
    const handlers = Events._eventHandlers || {};
    if (handlers.mouse) {
        window.removeEventListener('AnyWP:mouse', handlers.mouse);
    }
    if (handlers.keyboard) {
        window.removeEventListener('AnyWP:keyboard', handlers.keyboard);
    }
    if (handlers.click) {
        window.removeEventListener('AnyWP:click', handlers.click);
    }
    if (handlers.interactionMode) {
        window.removeEventListener('AnyWP:interactionMode', handlers.interactionMode);
    }
    if (handlers.visibility) {
        window.removeEventListener('AnyWP:visibility', handlers.visibility);
    }
    if (handlers.resize) {
        window.removeEventListener('resize', handlers.resize);
    }
};
describe('Events module', () => {
    let mockAnyWP;
    let mockClickHandler;
    let mockAnimationsHandler;
    beforeEach(() => {
        removeRegisteredHandlers();
        Events._setupCompleted = false;
        Events._eventHandlers = {};
        // Create mock AnyWP object
        mockAnyWP = {
            _mouseCallbacks: [],
            _keyboardCallbacks: [],
            _visibilityCallback: null
        };
        // Create mock handlers
        mockClickHandler = {
            handleClick: jest.fn(),
            refreshBounds: jest.fn()
        };
        mockAnimationsHandler = {
            pause: jest.fn(),
            resume: jest.fn()
        };
    });
    afterEach(() => {
        removeRegisteredHandlers();
        Events._setupCompleted = false;
        Events._eventHandlers = {};
    });
    describe('setup', () => {
        test('should setup mouse event listener', () => {
            Events.setup(mockAnyWP, mockClickHandler, mockAnimationsHandler);
            const mouseCallback = jest.fn();
            mockAnyWP._mouseCallbacks.push(mouseCallback);
            const event = new CustomEvent('AnyWP:mouse', {
                detail: { type: 'mousedown', x: 100, y: 200 }
            });
            window.dispatchEvent(event);
            expect(mouseCallback).toHaveBeenCalledWith({
                type: 'mousedown',
                x: 100,
                y: 200
            });
        });
        test('should setup keyboard event listener', () => {
            Events.setup(mockAnyWP, mockClickHandler, mockAnimationsHandler);
            const keyboardCallback = jest.fn();
            mockAnyWP._keyboardCallbacks.push(keyboardCallback);
            const event = new CustomEvent('AnyWP:keyboard', {
                detail: { type: 'keydown', key: 'A', code: 'KeyA' }
            });
            window.dispatchEvent(event);
            expect(keyboardCallback).toHaveBeenCalledWith({
                type: 'keydown',
                key: 'A',
                code: 'KeyA'
            });
        });
        test('should setup click event listener', () => {
            Events.setup(mockAnyWP, mockClickHandler, mockAnimationsHandler);
            const event = new CustomEvent('AnyWP:click', {
                detail: { x: 150, y: 250 }
            });
            window.dispatchEvent(event);
            expect(mockClickHandler.handleClick).toHaveBeenCalledWith(150, 250);
        });
        test('should setup visibility listener and pause animations when hidden', () => {
            Events.setup(mockAnyWP, mockClickHandler, mockAnimationsHandler);
            const visibilityCallback = jest.fn();
            mockAnyWP._visibilityCallback = visibilityCallback;
            const event = new CustomEvent('AnyWP:visibility', {
                detail: { visible: false }
            });
            window.dispatchEvent(event);
            expect(visibilityCallback).toHaveBeenCalledWith(false);
            expect(mockAnimationsHandler.pause).toHaveBeenCalledWith(mockAnyWP);
        });
        test('should resume animations when visible', () => {
            Events.setup(mockAnyWP, mockClickHandler, mockAnimationsHandler);
            const event = new CustomEvent('AnyWP:visibility', {
                detail: { visible: true }
            });
            window.dispatchEvent(event);
            expect(mockAnimationsHandler.resume).toHaveBeenCalledWith(mockAnyWP);
        });
        test('should setup resize listener', (done) => {
            Events.setup(mockAnyWP, mockClickHandler, mockAnimationsHandler);
            window.dispatchEvent(new Event('resize'));
            // RefreshBounds is called after 200ms delay
            setTimeout(() => {
                expect(mockClickHandler.refreshBounds).toHaveBeenCalledWith(mockAnyWP);
                done();
            }, 250);
        });
    });
    describe('onMouse', () => {
        test('should register mouse callback', () => {
            const callback = jest.fn();
            Events.onMouse(mockAnyWP, callback);
            expect(mockAnyWP._mouseCallbacks).toContain(callback);
            expect(mockAnyWP._mouseCallbacks.length).toBe(1);
        });
        test('should register multiple mouse callbacks', () => {
            const callback1 = jest.fn();
            const callback2 = jest.fn();
            Events.onMouse(mockAnyWP, callback1);
            Events.onMouse(mockAnyWP, callback2);
            expect(mockAnyWP._mouseCallbacks.length).toBe(2);
        });
    });
    describe('onKeyboard', () => {
        test('should register keyboard callback', () => {
            const callback = jest.fn();
            Events.onKeyboard(mockAnyWP, callback);
            expect(mockAnyWP._keyboardCallbacks).toContain(callback);
            expect(mockAnyWP._keyboardCallbacks.length).toBe(1);
        });
    });
    describe('onVisibilityChange', () => {
        test('should register visibility callback', () => {
            const callback = jest.fn();
            Events.onVisibilityChange(mockAnyWP, callback);
            expect(mockAnyWP._visibilityCallback).toBe(callback);
        });
        test('should replace existing visibility callback', () => {
            const callback1 = jest.fn();
            const callback2 = jest.fn();
            Events.onVisibilityChange(mockAnyWP, callback1);
            Events.onVisibilityChange(mockAnyWP, callback2);
            expect(mockAnyWP._visibilityCallback).toBe(callback2);
        });
    });
    describe('notifyVisibilityChange', () => {
        test('should call visibility callback', () => {
            const callback = jest.fn();
            mockAnyWP._visibilityCallback = callback;
            Events.notifyVisibilityChange(mockAnyWP, true);
            expect(callback).toHaveBeenCalledWith(true);
        });
        test('should dispatch visibility event', () => {
            const eventListener = jest.fn();
            window.addEventListener('AnyWP:visibility', eventListener);
            Events.notifyVisibilityChange(mockAnyWP, false);
            expect(eventListener).toHaveBeenCalled();
            window.removeEventListener('AnyWP:visibility', eventListener);
        });
        test('should handle missing callback gracefully', () => {
            mockAnyWP._visibilityCallback = null;
            expect(() => {
                Events.notifyVisibilityChange(mockAnyWP, true);
            }).not.toThrow();
        });
        test('should handle callback errors', () => {
            const consoleErrorSpy = jest.spyOn(console, 'error').mockImplementation(() => { });
            const errorCallback = jest.fn().mockImplementation(() => {
                throw new Error('Callback error');
            });
            mockAnyWP._visibilityCallback = errorCallback;
            Events.notifyVisibilityChange(mockAnyWP, true);
            expect(consoleErrorSpy).toHaveBeenCalled();
            consoleErrorSpy.mockRestore();
        });
    });
});
//# sourceMappingURL=events.test.js.map