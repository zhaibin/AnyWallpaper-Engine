/**
 * Storage module tests
 */
import { describe, test, expect, beforeEach, afterEach, jest } from '@jest/globals';
import { Storage } from '../modules/storage';
describe('Storage module', () => {
    let mockAnyWP;
    let mockWebview;
    beforeEach(() => {
        // Create mock AnyWP object
        mockAnyWP = {
            _persistedState: {},
            _log: jest.fn()
        };
        // Mock chrome.webview
        mockWebview = {
            postMessage: jest.fn()
        };
        window.chrome = {
            webview: mockWebview
        };
        // Clear localStorage
        localStorage.clear();
    });
    afterEach(() => {
        delete window.chrome;
    });
    describe('save', () => {
        test('should save state using WebView2', () => {
            const key = 'test_key';
            const value = { x: 100, y: 200 };
            Storage.save(mockAnyWP, key, value);
            expect(mockAnyWP._persistedState[key]).toEqual(value);
            expect(mockWebview.postMessage).toHaveBeenCalledWith({
                type: 'saveState',
                key: key,
                value: JSON.stringify(value)
            });
        });
        test('should fallback to localStorage when WebView2 not available', () => {
            delete window.chrome;
            const key = 'test_key';
            const value = { x: 100, y: 200 };
            Storage.save(mockAnyWP, key, value);
            expect(mockAnyWP._persistedState[key]).toEqual(value);
            expect(localStorage.getItem('AnyWP_' + key)).toBe(JSON.stringify(value));
        });
        test('should handle save errors gracefully', () => {
            delete window.chrome;
            // Mock localStorage.setItem to throw
            const setItemSpy = jest.spyOn(localStorage, 'setItem').mockImplementation(() => {
                throw new Error('Storage full');
            });
            const consoleWarnSpy = jest.spyOn(console, 'warn').mockImplementation(() => { });
            expect(() => {
                Storage.save(mockAnyWP, 'key', 'value');
            }).not.toThrow();
            expect(consoleWarnSpy).toHaveBeenCalled();
            setItemSpy.mockRestore();
            consoleWarnSpy.mockRestore();
        });
    });
    describe('load', () => {
        test('should load from cache if available', (done) => {
            const key = 'cached_key';
            const value = { x: 50, y: 75 };
            mockAnyWP._persistedState[key] = value;
            Storage.load(mockAnyWP, key, (loadedValue) => {
                expect(loadedValue).toEqual(value);
                expect(mockWebview.postMessage).not.toHaveBeenCalled();
                done();
            });
        });
        test('should request from WebView2 when not cached', (done) => {
            const key = 'remote_key';
            const value = { x: 150, y: 250 };
            // Simulate native response
            setTimeout(() => {
                const event = new CustomEvent('AnyWP:stateLoaded', {
                    detail: { type: 'stateLoaded', key: key, value: JSON.stringify(value) }
                });
                window.dispatchEvent(event);
            }, 50);
            Storage.load(mockAnyWP, key, (loadedValue) => {
                expect(loadedValue).toEqual(value);
                expect(mockAnyWP._persistedState[key]).toEqual(value);
                expect(mockWebview.postMessage).toHaveBeenCalledWith({
                    type: 'loadState',
                    key: key
                });
                done();
            });
        });
        test('should fallback to localStorage when WebView2 not available', (done) => {
            delete window.chrome;
            const key = 'local_key';
            const value = { x: 300, y: 400 };
            localStorage.setItem('AnyWP_' + key, JSON.stringify(value));
            Storage.load(mockAnyWP, key, (loadedValue) => {
                expect(loadedValue).toEqual(value);
                expect(mockAnyWP._persistedState[key]).toEqual(value);
                done();
            });
        });
        test('should return null when key not found', (done) => {
            delete window.chrome;
            Storage.load(mockAnyWP, 'nonexistent', (loadedValue) => {
                expect(loadedValue).toBeNull();
                done();
            });
        });
        test('should timeout if no response from native', (done) => {
            const key = 'timeout_key';
            Storage.load(mockAnyWP, key, (loadedValue) => {
                expect(loadedValue).toBeNull();
                done();
            });
        }, 1500);
    });
    describe('clear', () => {
        test('should clear all state using WebView2', () => {
            mockAnyWP._persistedState = { key1: 'value1', key2: 'value2' };
            Storage.clear(mockAnyWP);
            expect(mockAnyWP._persistedState).toEqual({});
            expect(mockWebview.postMessage).toHaveBeenCalledWith({
                type: 'clearState'
            });
        });
        test('should clear localStorage when WebView2 not available', () => {
            delete window.chrome;
            localStorage.setItem('AnyWP_key1', 'value1');
            localStorage.setItem('AnyWP_key2', 'value2');
            localStorage.setItem('other_key', 'value3');
            Storage.clear(mockAnyWP);
            expect(localStorage.getItem('AnyWP_key1')).toBeNull();
            expect(localStorage.getItem('AnyWP_key2')).toBeNull();
            expect(localStorage.getItem('other_key')).toBe('value3');
        });
    });
    describe('saveElementPosition', () => {
        test('should save element position', () => {
            const key = 'element_pos';
            const x = 100;
            const y = 200;
            Storage.saveElementPosition(mockAnyWP, key, x, y);
            expect(mockAnyWP._persistedState[key]).toEqual({ x, y });
            expect(mockWebview.postMessage).toHaveBeenCalledWith({
                type: 'saveState',
                key: key,
                value: JSON.stringify({ x, y })
            });
        });
        test('should fallback to localStorage for element position', () => {
            delete window.chrome;
            const key = 'element_pos';
            const x = 150;
            const y = 250;
            Storage.saveElementPosition(mockAnyWP, key, x, y);
            const stored = localStorage.getItem('AnyWP_' + key);
            expect(stored).toBe(JSON.stringify({ x, y }));
        });
    });
});
//# sourceMappingURL=storage.test.js.map