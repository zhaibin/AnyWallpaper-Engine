/**
 * Click handler module tests
 */
import { describe, test, expect, beforeEach, afterEach, jest } from '@jest/globals';
import { ClickHandler } from '../modules/click';
import { Bounds } from '../utils/bounds';
describe('Click handler module', () => {
    let mockAnyWP;
    let mockElement;
    beforeEach(() => {
        // Create mock AnyWP object
        mockAnyWP = {
            _clickHandlers: [],
            dpiScale: 1,
            _debugMode: false,
            _autoRefreshEnabled: true,
            _log: jest.fn()
        };
        // Set global AnyWP
        window.AnyWP = mockAnyWP;
        // Create mock element
        mockElement = document.createElement('div');
        mockElement.id = 'test-element';
        document.body.appendChild(mockElement);
        // Mock getBoundingClientRect
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
        document.body.removeChild(mockElement);
        delete window.AnyWP;
    });
    describe('handleClick', () => {
        test('should trigger callback when click is in bounds', () => {
            const callback = jest.fn();
            const bounds = Bounds.calculate(mockElement, 1);
            mockAnyWP._clickHandlers.push({
                element: mockElement,
                callback: callback,
                bounds: bounds,
                selector: null,
                autoRefresh: true,
                options: {}
            });
            ClickHandler.handleClick(150, 250);
            expect(callback).toHaveBeenCalledWith(150, 250);
        });
        test('should not trigger callback when click is out of bounds', () => {
            const callback = jest.fn();
            const bounds = Bounds.calculate(mockElement, 1);
            mockAnyWP._clickHandlers.push({
                element: mockElement,
                callback: callback,
                bounds: bounds,
                selector: null,
                autoRefresh: true,
                options: {}
            });
            ClickHandler.handleClick(50, 50);
            expect(callback).not.toHaveBeenCalled();
        });
        test('should only trigger first matching handler', () => {
            const callback1 = jest.fn();
            const callback2 = jest.fn();
            const bounds = Bounds.calculate(mockElement, 1);
            mockAnyWP._clickHandlers.push({
                element: mockElement,
                callback: callback1,
                bounds: bounds,
                selector: null,
                autoRefresh: true,
                options: {}
            }, {
                element: mockElement,
                callback: callback2,
                bounds: bounds,
                selector: null,
                autoRefresh: true,
                options: {}
            });
            ClickHandler.handleClick(150, 250);
            expect(callback1).toHaveBeenCalled();
            expect(callback2).not.toHaveBeenCalled();
        });
    });
    describe('waitForElement', () => {
        test('should call callback when element exists', (done) => {
            const element = document.createElement('div');
            element.id = 'wait-element';
            document.body.appendChild(element);
            ClickHandler.waitForElement('#wait-element', (el) => {
                expect(el).toBe(element);
                document.body.removeChild(element);
                done();
            });
        });
        test('should wait for element to appear', (done) => {
            ClickHandler.waitForElement('#delayed-element', (el) => {
                expect(el).not.toBeNull();
                expect(el.id).toBe('delayed-element');
                document.body.removeChild(el);
                done();
            });
            // Add element after 100ms
            setTimeout(() => {
                const element = document.createElement('div');
                element.id = 'delayed-element';
                document.body.appendChild(element);
            }, 100);
        });
        test('should timeout if element not found', (done) => {
            const consoleErrorSpy = jest.spyOn(console, 'error').mockImplementation(() => { });
            ClickHandler.waitForElement('#nonexistent', () => {
                // Should not be called
                expect(true).toBe(false);
            }, 200);
            setTimeout(() => {
                expect(consoleErrorSpy).toHaveBeenCalledWith(expect.stringContaining('Element not found'));
                consoleErrorSpy.mockRestore();
                done();
            }, 300);
        }, 500);
    });
    describe('onClick', () => {
        test('should register click handler immediately', () => {
            const callback = jest.fn();
            ClickHandler.onClick(mockAnyWP, mockElement, callback, { immediate: true });
            expect(mockAnyWP._clickHandlers.length).toBe(1);
            expect(mockAnyWP._clickHandlers[0]?.element).toBe(mockElement);
            expect(mockAnyWP._clickHandlers[0]?.callback).toBe(callback);
        });
        test('should register handler with delay', (done) => {
            const callback = jest.fn();
            ClickHandler.onClick(mockAnyWP, mockElement, callback, { delay: 50 });
            expect(mockAnyWP._clickHandlers.length).toBe(0);
            setTimeout(() => {
                expect(mockAnyWP._clickHandlers.length).toBe(1);
                done();
            }, 100);
        });
        test('should register handler by selector', () => {
            const callback = jest.fn();
            ClickHandler.onClick(mockAnyWP, '#test-element', callback, { immediate: true });
            expect(mockAnyWP._clickHandlers.length).toBe(1);
            expect(mockAnyWP._clickHandlers[0]?.element).toBe(mockElement);
            expect(mockAnyWP._clickHandlers[0]?.selector).toBe('#test-element');
        });
        test('should ignore duplicate registration for same element', () => {
            const callback1 = jest.fn();
            const callback2 = jest.fn();
            const warnSpy = jest.spyOn(console, 'warn').mockImplementation(() => { });
            ClickHandler.onClick(mockAnyWP, mockElement, callback1, { immediate: true });
            expect(mockAnyWP._clickHandlers.length).toBe(1);
            ClickHandler.onClick(mockAnyWP, mockElement, callback2, { immediate: true });
            expect(mockAnyWP._clickHandlers.length).toBe(1);
            expect(mockAnyWP._clickHandlers[0]?.callback).toBe(callback1);
            expect(warnSpy).toHaveBeenCalled();
            warnSpy.mockRestore();
        });
        test('should setup ResizeObserver when autoRefresh enabled', () => {
            // Skip if ResizeObserver not available in test environment
            if (typeof ResizeObserver === 'undefined') {
                expect(true).toBe(true);
                return;
            }
            const callback = jest.fn();
            ClickHandler.onClick(mockAnyWP, mockElement, callback, {
                immediate: true,
                autoRefresh: true
            });
            expect(mockAnyWP._clickHandlers[0]?.resizeObserver).toBeDefined();
        });
        test('should handle element not found', () => {
            const consoleErrorSpy = jest.spyOn(console, 'error').mockImplementation(() => { });
            const callback = jest.fn();
            ClickHandler.onClick(mockAnyWP, '#nonexistent', callback, { immediate: true });
            expect(consoleErrorSpy).toHaveBeenCalled();
            expect(mockAnyWP._clickHandlers.length).toBe(0);
            consoleErrorSpy.mockRestore();
        });
    });
    describe('refreshElementBounds', () => {
        test('should update handler bounds when element moves', () => {
            const callback = jest.fn();
            const oldBounds = Bounds.calculate(mockElement, 1);
            const handler = {
                element: mockElement,
                callback: callback,
                bounds: oldBounds,
                selector: null,
                autoRefresh: true,
                options: {}
            };
            // Change element position
            mockElement.getBoundingClientRect = jest.fn(() => ({
                left: 150,
                top: 250,
                right: 350,
                bottom: 450,
                width: 200,
                height: 200,
                x: 150,
                y: 250,
                toJSON: () => { }
            }));
            ClickHandler.refreshElementBounds(mockAnyWP, handler);
            expect(handler.bounds.left).toBe(150);
            expect(handler.bounds.top).toBe(250);
        });
        test('should not update if element disconnected', () => {
            const callback = jest.fn();
            const oldBounds = Bounds.calculate(mockElement, 1);
            const disconnectedElement = document.createElement('div');
            const handler = {
                element: disconnectedElement,
                callback: callback,
                bounds: oldBounds,
                selector: null,
                autoRefresh: true,
                options: {}
            };
            ClickHandler.refreshElementBounds(mockAnyWP, handler);
            expect(handler.bounds).toBe(oldBounds);
        });
    });
    describe('refreshBounds', () => {
        test('should refresh all registered handlers', () => {
            const callback1 = jest.fn();
            const callback2 = jest.fn();
            ClickHandler.onClick(mockAnyWP, mockElement, callback1, { immediate: true });
            const element2 = document.createElement('div');
            document.body.appendChild(element2);
            element2.getBoundingClientRect = jest.fn(() => ({
                left: 200,
                top: 300,
                right: 400,
                bottom: 500,
                width: 200,
                height: 200,
                x: 200,
                y: 300,
                toJSON: () => { }
            }));
            ClickHandler.onClick(mockAnyWP, element2, callback2, { immediate: true });
            const refreshed = ClickHandler.refreshBounds(mockAnyWP);
            expect(refreshed).toBe(2);
            document.body.removeChild(element2);
        });
    });
    describe('clearHandlers', () => {
        test('should clear all registered handlers', () => {
            const callback = jest.fn();
            ClickHandler.onClick(mockAnyWP, mockElement, callback, { immediate: true });
            expect(mockAnyWP._clickHandlers.length).toBe(1);
            ClickHandler.clearHandlers(mockAnyWP);
            expect(mockAnyWP._clickHandlers.length).toBe(0);
        });
        test('should disconnect ResizeObservers', () => {
            // Skip if ResizeObserver not available in test environment
            if (typeof ResizeObserver === 'undefined') {
                expect(true).toBe(true);
                return;
            }
            const callback = jest.fn();
            ClickHandler.onClick(mockAnyWP, mockElement, callback, {
                immediate: true,
                autoRefresh: true
            });
            const observer = mockAnyWP._clickHandlers[0]?.resizeObserver;
            if (observer) {
                const disconnectSpy = jest.spyOn(observer, 'disconnect');
                ClickHandler.clearHandlers(mockAnyWP);
                expect(disconnectSpy).toHaveBeenCalled();
            }
            else {
                expect(true).toBe(true);
            }
        });
    });
});
//# sourceMappingURL=click.test.js.map