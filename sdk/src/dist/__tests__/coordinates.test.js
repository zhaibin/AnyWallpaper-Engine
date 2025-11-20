/**
 * Coordinates conversion tests
 */
import { describe, test, expect, beforeEach } from '@jest/globals';
import { Coordinates } from '../utils/coordinates';
describe('Coordinates utility', () => {
    beforeEach(() => {
        // Mock window properties
        Object.defineProperty(window, 'screenX', { value: 100, writable: true, configurable: true });
        Object.defineProperty(window, 'screenY', { value: 200, writable: true, configurable: true });
        // Mock document properties
        const mockDocRect = {
            left: 10,
            top: 20,
            right: 810,
            bottom: 620,
            width: 800,
            height: 600,
            x: 10,
            y: 20,
            toJSON: () => { }
        };
        document.documentElement.getBoundingClientRect = jest.fn(() => mockDocRect);
    });
    test('should convert screen to viewport coordinates', () => {
        const screenX = 500;
        const screenY = 800;
        const dpiScale = 2;
        const result = Coordinates.screenToViewport(screenX, screenY, dpiScale);
        // Screen 500 / 2 = 250 CSS pixels
        // 250 - windowLeft (100) - docLeft (10) = 140
        expect(result.x).toBe(140);
        // Screen 800 / 2 = 400 CSS pixels
        // 400 - windowTop (200) - docTop (20) = 180
        expect(result.y).toBe(180);
    });
    test('should handle 1x DPI scale', () => {
        const screenX = 250;
        const screenY = 400;
        const dpiScale = 1;
        const result = Coordinates.screenToViewport(screenX, screenY, dpiScale);
        expect(result.x).toBe(140); // 250 - 100 - 10
        expect(result.y).toBe(180); // 400 - 200 - 20
    });
    test('should handle high DPI scales', () => {
        const screenX = 750;
        const screenY = 1200;
        const dpiScale = 2.5;
        const result = Coordinates.screenToViewport(screenX, screenY, dpiScale);
        // 750 / 2.5 = 300, 300 - 100 - 10 = 190
        expect(result.x).toBe(190);
        // 1200 / 2.5 = 480, 480 - 200 - 20 = 260
        expect(result.y).toBe(260);
    });
    test('should get current offsets', () => {
        const dpiScale = 1.5;
        const offsets = Coordinates.getOffsets(dpiScale);
        expect(offsets.windowLeft).toBe(100);
        expect(offsets.windowTop).toBe(200);
        expect(offsets.docLeft).toBe(10);
        expect(offsets.docTop).toBe(20);
        expect(offsets.dpi).toBe(1.5);
    });
    test('should convert different screen positions correctly', () => {
        const dpiScale = 1.5;
        // Test position 1
        const result1 = Coordinates.screenToViewport(300, 450, dpiScale);
        // 300 / 1.5 = 200, 200 - 100 - 10 = 90
        expect(result1.x).toBe(90);
        // 450 / 1.5 = 300, 300 - 200 - 20 = 80
        expect(result1.y).toBe(80);
        // Test position 2
        const result2 = Coordinates.screenToViewport(600, 900, dpiScale);
        // 600 / 1.5 = 400, 400 - 100 - 10 = 290
        expect(result2.x).toBe(290);
        // 900 / 1.5 = 600, 600 - 200 - 20 = 380
        expect(result2.y).toBe(380);
    });
});
//# sourceMappingURL=coordinates.test.js.map