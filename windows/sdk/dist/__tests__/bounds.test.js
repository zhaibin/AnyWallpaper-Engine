/**
 * Bounds calculation tests
 */
import { describe, test, expect, beforeEach } from '@jest/globals';
import { Bounds } from '../utils/bounds';
describe('Bounds utility', () => {
    let mockElement;
    beforeEach(() => {
        // Create a mock element
        mockElement = document.createElement('div');
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
        // Mock window properties
        Object.defineProperty(window, 'screenX', { value: 50, writable: true });
        Object.defineProperty(window, 'screenY', { value: 100, writable: true });
    });
    afterEach(() => {
        document.body.removeChild(mockElement);
    });
    test('should calculate bounds with DPI scaling', () => {
        const dpiScale = 1.5;
        const bounds = Bounds.calculate(mockElement, dpiScale);
        // CSS pixel * DPI scale = Physical pixel
        expect(bounds.left).toBe(150); // 100 * 1.5
        expect(bounds.top).toBe(300); // 200 * 1.5
        expect(bounds.right).toBe(450); // 300 * 1.5
        expect(bounds.bottom).toBe(600); // 400 * 1.5
        expect(bounds.width).toBe(300); // 200 * 1.5
        expect(bounds.height).toBe(300); // 200 * 1.5
    });
    test('should calculate bounds without DPI scaling', () => {
        const dpiScale = 1;
        const bounds = Bounds.calculate(mockElement, dpiScale);
        expect(bounds.left).toBe(100);
        expect(bounds.top).toBe(200);
        expect(bounds.right).toBe(300);
        expect(bounds.bottom).toBe(400);
        expect(bounds.width).toBe(200);
        expect(bounds.height).toBe(200);
    });
    test('should check if point is in bounds', () => {
        const bounds = {
            left: 100,
            top: 200,
            right: 300,
            bottom: 400,
            width: 200,
            height: 200
        };
        // Inside
        expect(Bounds.isInBounds(150, 250, bounds)).toBe(true);
        expect(Bounds.isInBounds(200, 300, bounds)).toBe(true);
        // On edge (inclusive)
        expect(Bounds.isInBounds(100, 200, bounds)).toBe(true);
        expect(Bounds.isInBounds(300, 400, bounds)).toBe(true);
        // Outside
        expect(Bounds.isInBounds(50, 150, bounds)).toBe(false);
        expect(Bounds.isInBounds(350, 450, bounds)).toBe(false);
        expect(Bounds.isInBounds(150, 500, bounds)).toBe(false);
    });
    test('should handle edge cases for point checking', () => {
        const bounds = {
            left: 0,
            top: 0,
            right: 100,
            bottom: 100,
            width: 100,
            height: 100
        };
        expect(Bounds.isInBounds(0, 0, bounds)).toBe(true);
        expect(Bounds.isInBounds(100, 100, bounds)).toBe(true);
        expect(Bounds.isInBounds(-1, 50, bounds)).toBe(false);
        expect(Bounds.isInBounds(50, -1, bounds)).toBe(false);
        expect(Bounds.isInBounds(101, 50, bounds)).toBe(false);
        expect(Bounds.isInBounds(50, 101, bounds)).toBe(false);
    });
});
//# sourceMappingURL=bounds.test.js.map