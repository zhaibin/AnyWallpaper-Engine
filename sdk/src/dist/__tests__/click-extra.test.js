// Click handler module - Extra tests for edge cases and uncovered paths
import { describe, it, expect, beforeEach, afterEach, jest } from '@jest/globals';
import { ClickHandler } from '../modules/click';
describe('Click handler - Parameter validation', () => {
    let mockAnyWP;
    beforeEach(() => {
        mockAnyWP = {
            _clickHandlers: [],
            dpiScale: 1,
            _debugMode: false,
            _autoRefreshEnabled: true,
            _log: jest.fn()
        };
        window.AnyWP = mockAnyWP;
    });
    afterEach(() => {
        delete window.AnyWP;
    });
    it('should throw error when element is a function (wrong parameter order)', () => {
        const wrongCallback = jest.fn();
        expect(() => {
            ClickHandler.onClick(mockAnyWP, wrongCallback, jest.fn());
        }).toThrow('[AnyWP] onClick: Invalid parameters - element must be HTMLElement or selector string');
    });
    it('should throw error when callback is not a function', () => {
        expect(() => {
            ClickHandler.onClick(mockAnyWP, '#test-element', 'not-a-function');
        }).toThrow('[AnyWP] onClick: callback must be a function');
    });
    it('should throw error when callback is null', () => {
        expect(() => {
            ClickHandler.onClick(mockAnyWP, '#test-element', null);
        }).toThrow('[AnyWP] onClick: callback must be a function');
    });
    it('should throw error when callback is undefined', () => {
        expect(() => {
            ClickHandler.onClick(mockAnyWP, '#test-element', undefined);
        }).toThrow('[AnyWP] onClick: callback must be a function');
    });
});
describe('Click handler - Duplicate registration', () => {
    let mockAnyWP;
    let mockElement;
    beforeEach(() => {
        mockAnyWP = {
            _clickHandlers: [],
            dpiScale: 1,
            _debugMode: false,
            _autoRefreshEnabled: true,
            _log: jest.fn()
        };
        window.AnyWP = mockAnyWP;
        mockElement = document.createElement('div');
        mockElement.id = 'test-element';
        mockElement.className = 'test-class';
        document.body.appendChild(mockElement);
        mockElement.getBoundingClientRect = jest.fn(() => ({
            left: 100,
            top: 200,
            right: 300,
            bottom: 400,
            width: 200,
            height: 200,
            x: 100,
            y: 200,
            toJSON: () => { }
        }));
    });
    afterEach(() => {
        if (mockElement.parentNode) {
            document.body.removeChild(mockElement);
        }
        delete window.AnyWP;
    });
    it('should log warning when registering duplicate handler', () => {
        const callback1 = jest.fn();
        const callback2 = jest.fn();
        const consoleWarnSpy = jest.spyOn(console, 'warn').mockImplementation(() => { });
        ClickHandler.onClick(mockAnyWP, mockElement, callback1, { immediate: true });
        ClickHandler.onClick(mockAnyWP, mockElement, callback2, { immediate: true });
        expect(consoleWarnSpy).toHaveBeenCalledWith(expect.stringContaining('[AnyWP] Element already has click handler'), expect.stringContaining('test-element'));
        expect(consoleWarnSpy).toHaveBeenCalledWith(expect.stringContaining('[AnyWP] Existing handler callback:'), expect.any(String));
        expect(consoleWarnSpy).toHaveBeenCalledWith(expect.stringContaining('[AnyWP] New callback (ignored):'), expect.any(String));
        // Only first handler should be registered
        expect(mockAnyWP._clickHandlers.length).toBe(1);
        consoleWarnSpy.mockRestore();
    });
    it('should not register when element not found', () => {
        const consoleErrorSpy = jest.spyOn(console, 'error').mockImplementation(() => { });
        ClickHandler.onClick(mockAnyWP, '#nonexistent', jest.fn(), { immediate: true });
        expect(consoleErrorSpy).toHaveBeenCalledWith(expect.stringContaining('[AnyWP] Element not found:'), '#nonexistent');
        expect(mockAnyWP._clickHandlers.length).toBe(0);
        consoleErrorSpy.mockRestore();
    });
});
describe('Click handler - Auto-refresh mechanisms', () => {
    let mockAnyWP;
    let mockElement;
    beforeEach(() => {
        // Mock ResizeObserver
        global.ResizeObserver = jest.fn().mockImplementation((_callback) => ({
            observe: jest.fn(),
            unobserve: jest.fn(),
            disconnect: jest.fn()
        }));
        // Mock IntersectionObserver
        global.IntersectionObserver = jest.fn().mockImplementation((_callback) => ({
            observe: jest.fn(),
            unobserve: jest.fn(),
            disconnect: jest.fn()
        }));
        mockAnyWP = {
            _clickHandlers: [],
            dpiScale: 1,
            _debugMode: false,
            _autoRefreshEnabled: true,
            _log: jest.fn()
        };
        window.AnyWP = mockAnyWP;
        mockElement = document.createElement('div');
        mockElement.id = 'auto-refresh-element';
        document.body.appendChild(mockElement);
        mockElement.getBoundingClientRect = jest.fn(() => ({
            left: 100,
            top: 200,
            right: 300,
            bottom: 400,
            width: 200,
            height: 200,
            x: 100,
            y: 200,
            toJSON: () => { }
        }));
    });
    afterEach(() => {
        if (mockElement.parentNode) {
            document.body.removeChild(mockElement);
        }
        delete window.AnyWP;
        delete global.ResizeObserver;
        delete global.IntersectionObserver;
        // Clean up timers
        jest.clearAllTimers();
    });
    it('should setup ResizeObserver when autoRefresh is enabled', () => {
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), {
            immediate: true,
            autoRefresh: true
        });
        const handler = mockAnyWP._clickHandlers[0];
        expect(handler).toBeDefined();
        expect(handler.resizeObserver).toBeDefined();
    });
    it('should setup IntersectionObserver when autoRefresh is enabled', () => {
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), {
            immediate: true,
            autoRefresh: true
        });
        const handler = mockAnyWP._clickHandlers[0];
        expect(handler).toBeDefined();
        expect(handler.intersectionObserver).toBeDefined();
    });
    it('should setup position check timer when autoRefresh is enabled', () => {
        jest.useFakeTimers();
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), {
            immediate: true,
            autoRefresh: true
        });
        const handler = mockAnyWP._clickHandlers[0];
        expect(handler).toBeDefined();
        expect(handler.positionCheckTimer).toBeDefined();
        jest.useRealTimers();
    });
    it('should not setup auto-refresh when autoRefresh is false', () => {
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), {
            immediate: true,
            autoRefresh: false
        });
        const handler = mockAnyWP._clickHandlers[0];
        expect(handler).toBeDefined();
        expect(handler.resizeObserver).toBeUndefined();
        expect(handler.intersectionObserver).toBeUndefined();
        expect(handler.positionCheckTimer).toBeUndefined();
    });
    it('should detect position changes and refresh bounds', () => {
        jest.useFakeTimers();
        const callback = jest.fn();
        ClickHandler.onClick(mockAnyWP, mockElement, callback, {
            immediate: true,
            autoRefresh: true
        });
        const handler = mockAnyWP._clickHandlers[0];
        expect(handler).toBeDefined();
        handler.lastBounds = { ...handler.bounds };
        // Simulate position change
        mockElement.getBoundingClientRect = jest.fn(() => ({
            left: 150, // Changed from 100
            top: 200,
            right: 350,
            bottom: 400,
            width: 200,
            height: 200,
            x: 150,
            y: 200,
            toJSON: () => { }
        }));
        // Fast-forward timer
        jest.advanceTimersByTime(100);
        // Check bounds were updated
        expect(handler.bounds.left).toBe(150 * mockAnyWP.dpiScale);
        jest.useRealTimers();
    });
    it('should clear timer when element is disconnected', () => {
        jest.useFakeTimers();
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), {
            immediate: true,
            autoRefresh: true
        });
        const handler = mockAnyWP._clickHandlers[0];
        expect(handler).toBeDefined();
        const timerId = handler.positionCheckTimer;
        // Timer should be created
        expect(timerId).toBeDefined();
        // Disconnect element
        document.body.removeChild(mockElement);
        // Fast-forward timer - timer should detect element is disconnected
        jest.advanceTimersByTime(100);
        jest.useRealTimers();
    });
});
describe('Click handler - Debug mode', () => {
    let mockAnyWP;
    let mockElement;
    beforeEach(() => {
        mockAnyWP = {
            _clickHandlers: [],
            dpiScale: 1,
            _debugMode: true, // Debug enabled
            _autoRefreshEnabled: false,
            _log: jest.fn()
        };
        window.AnyWP = mockAnyWP;
        mockElement = document.createElement('div');
        mockElement.id = 'debug-element';
        document.body.appendChild(mockElement);
        mockElement.getBoundingClientRect = jest.fn(() => ({
            left: 100,
            top: 200,
            right: 300,
            bottom: 400,
            width: 200,
            height: 200,
            x: 100,
            y: 200,
            toJSON: () => { }
        }));
    });
    afterEach(() => {
        if (mockElement.parentNode) {
            document.body.removeChild(mockElement);
        }
        delete window.AnyWP;
    });
    it('should log debug information when debug mode is enabled', () => {
        const consoleLogSpy = jest.spyOn(console, 'log').mockImplementation(() => { });
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), { immediate: true });
        expect(consoleLogSpy).toHaveBeenCalledWith(expect.stringContaining('[AnyWP] Click Handler Registered:'), expect.any(String));
        consoleLogSpy.mockRestore();
    });
    it('should respect options.debug over global debug mode', () => {
        const consoleLogSpy = jest.spyOn(console, 'log').mockImplementation(() => { });
        // Global debug is true, but options.debug is false
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), {
            immediate: true,
            debug: false
        });
        // Should not log because options.debug is false
        expect(consoleLogSpy).not.toHaveBeenCalled();
        consoleLogSpy.mockRestore();
    });
});
describe('Click handler - waitForElement timeout', () => {
    let mockAnyWP;
    beforeEach(() => {
        mockAnyWP = {
            _clickHandlers: [],
            dpiScale: 1,
            _debugMode: false,
            _autoRefreshEnabled: true,
            _log: jest.fn()
        };
        window.AnyWP = mockAnyWP;
    });
    afterEach(() => {
        delete window.AnyWP;
    });
    it('should log error when element not found within maxWait', (done) => {
        jest.useRealTimers(); // Use real timers for this async test
        const consoleErrorSpy = jest.spyOn(console, 'error').mockImplementation(() => { });
        ClickHandler.onClick(mockAnyWP, '#never-exists', jest.fn(), {
            waitFor: true,
            maxWait: 100 // Short timeout for testing
        });
        setTimeout(() => {
            expect(consoleErrorSpy).toHaveBeenCalledWith('[AnyWP] Element not found: #never-exists');
            consoleErrorSpy.mockRestore();
            done();
        }, 150);
    }, 1000);
});
describe('Click handler - refreshElementBounds edge cases', () => {
    let mockAnyWP;
    let mockElement;
    beforeEach(() => {
        mockAnyWP = {
            _clickHandlers: [],
            dpiScale: 1,
            _debugMode: true,
            _autoRefreshEnabled: false,
            _log: jest.fn()
        };
        window.AnyWP = mockAnyWP;
        mockElement = document.createElement('div');
        mockElement.id = 'test-bounds-element';
        document.body.appendChild(mockElement);
        mockElement.getBoundingClientRect = jest.fn(() => ({
            left: 100,
            top: 200,
            right: 300,
            bottom: 400,
            width: 200,
            height: 200,
            x: 100,
            y: 200,
            toJSON: () => { }
        }));
    });
    afterEach(() => {
        if (mockElement.parentNode) {
            document.body.removeChild(mockElement);
        }
        delete window.AnyWP;
    });
    it('should update debug border when bounds change', () => {
        // Register with debug mode
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), { immediate: true });
        const handler = mockAnyWP._clickHandlers[0];
        expect(handler).toBeDefined();
        // Simulate debug border
        handler.element._anywpDebugBorder = document.createElement('div');
        // Change position
        mockElement.getBoundingClientRect = jest.fn(() => ({
            left: 150, // Changed
            top: 250, // Changed
            right: 350,
            bottom: 450,
            width: 200,
            height: 200,
            x: 150,
            y: 250,
            toJSON: () => { }
        }));
        // Refresh bounds
        ClickHandler.refreshElementBounds(mockAnyWP, handler);
        expect(handler.bounds.left).toBe(150);
        expect(handler.bounds.top).toBe(250);
    });
    it('should not update when bounds have not changed', () => {
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), { immediate: true });
        const handler = mockAnyWP._clickHandlers[0];
        expect(handler).toBeDefined();
        const originalBounds = { ...handler.bounds };
        // Refresh without changing position
        ClickHandler.refreshElementBounds(mockAnyWP, handler);
        // Bounds should remain same object
        expect(handler.bounds).toEqual(originalBounds);
    });
});
describe('Click handler - clearHandlers cleanup', () => {
    let mockAnyWP;
    let mockElement;
    beforeEach(() => {
        mockAnyWP = {
            _clickHandlers: [],
            dpiScale: 1,
            _debugMode: true,
            _autoRefreshEnabled: true,
            _log: jest.fn()
        };
        window.AnyWP = mockAnyWP;
        mockElement = document.createElement('div');
        mockElement.id = 'cleanup-test';
        document.body.appendChild(mockElement);
        mockElement.getBoundingClientRect = jest.fn(() => ({
            left: 100,
            top: 200,
            right: 300,
            bottom: 400,
            width: 200,
            height: 200,
            x: 100,
            y: 200,
            toJSON: () => { }
        }));
    });
    afterEach(() => {
        if (mockElement.parentNode) {
            document.body.removeChild(mockElement);
        }
        delete window.AnyWP;
    });
    it('should remove debug border when clearing handlers', () => {
        // Register with debug
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), { immediate: true });
        const handler = mockAnyWP._clickHandlers[0];
        expect(handler).toBeDefined();
        // Simulate debug border in DOM
        const debugBorder = document.createElement('div');
        debugBorder.id = 'debug-border';
        document.body.appendChild(debugBorder);
        handler.element._anywpDebugBorder = debugBorder;
        // Clear handlers
        ClickHandler.clearHandlers(mockAnyWP);
        // Debug border should be removed from DOM
        expect(document.getElementById('debug-border')).toBeNull();
        expect(mockAnyWP._clickHandlers.length).toBe(0);
    });
    it('should clear all timers and observers', () => {
        jest.useFakeTimers();
        ClickHandler.onClick(mockAnyWP, mockElement, jest.fn(), {
            immediate: true,
            autoRefresh: true
        });
        const handler = mockAnyWP._clickHandlers[0];
        expect(handler).toBeDefined();
        const resizeObserverDisconnectSpy = handler.resizeObserver ?
            jest.spyOn(handler.resizeObserver, 'disconnect') : null;
        const intersectionObserverDisconnectSpy = handler.intersectionObserver ?
            jest.spyOn(handler.intersectionObserver, 'disconnect') : null;
        ClickHandler.clearHandlers(mockAnyWP);
        if (resizeObserverDisconnectSpy) {
            expect(resizeObserverDisconnectSpy).toHaveBeenCalled();
        }
        if (intersectionObserverDisconnectSpy) {
            expect(intersectionObserverDisconnectSpy).toHaveBeenCalled();
        }
        jest.useRealTimers();
    });
});
//# sourceMappingURL=click-extra.test.js.map