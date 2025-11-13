// Bounds utility tests - Comprehensive coverage
import { describe, it, expect, beforeEach, afterEach } from '@jest/globals';
import { Bounds } from '../utils/bounds';

describe('Bounds utility', () => {
  let mockElement: HTMLElement;
  
  beforeEach(() => {
    document.body.innerHTML = '';
    mockElement = document.createElement('div');
    document.body.appendChild(mockElement);
  });
  
  afterEach(() => {
    document.body.innerHTML = '';
  });
  
  describe('calculate - Basic functionality', () => {
    it('should calculate bounds with 1x DPI scaling', () => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 100,
        top: 200,
        right: 300,
        bottom: 400,
        width: 200,
        height: 200,
        x: 100,
        y: 200,
        toJSON: () => {}
      }));
      
      const bounds = Bounds.calculate(mockElement, 1);
      expect(bounds.left).toBe(100);
      expect(bounds.top).toBe(200);
      expect(bounds.right).toBe(300);
      expect(bounds.bottom).toBe(400);
      expect(bounds.width).toBe(200);
      expect(bounds.height).toBe(200);
    });
    
    it('should calculate bounds with selector string', () => {
      mockElement.id = 'test-element';
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 50,
        top: 100,
        right: 150,
        bottom: 200,
        width: 100,
        height: 100,
        x: 50,
        y: 100,
        toJSON: () => {}
      }));
      
      const bounds = Bounds.calculate('#test-element', 1);
      expect(bounds.width).toBe(100);
      expect(bounds.height).toBe(100);
    });
    
    it('should throw error for invalid selector', () => {
      expect(() => {
        Bounds.calculate('#nonexistent', 1);
      }).toThrow('[AnyWP] Element not found: #nonexistent');
    });
    
    it('should throw error for null element', () => {
      expect(() => {
        Bounds.calculate(null as any, 1);
      }).toThrow('[AnyWP] Element must be a valid DOM element or selector');
    });
    
    it('should throw error for element without getBoundingClientRect', () => {
      const invalidElement = {} as HTMLElement;
      expect(() => {
        Bounds.calculate(invalidElement, 1);
      }).toThrow('[AnyWP] Element must be a valid DOM element or selector');
    });
  });
  
  describe('calculate - DPI scaling', () => {
    beforeEach(() => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 100,
        top: 200,
        right: 300,
        bottom: 400,
        width: 200,
        height: 200,
        x: 100,
        y: 200,
        toJSON: () => {}
      }));
    });
    
    it('should calculate bounds with 1.25x DPI scaling', () => {
      const bounds = Bounds.calculate(mockElement, 1.25);
      expect(bounds.left).toBe(125);
      expect(bounds.top).toBe(250);
      expect(bounds.right).toBe(375);
      expect(bounds.bottom).toBe(500);
      expect(bounds.width).toBe(250);
      expect(bounds.height).toBe(250);
    });
    
    it('should calculate bounds with 1.5x DPI scaling', () => {
      const bounds = Bounds.calculate(mockElement, 1.5);
      expect(bounds.left).toBe(150);
      expect(bounds.top).toBe(300);
      expect(bounds.right).toBe(450);
      expect(bounds.bottom).toBe(600);
      expect(bounds.width).toBe(300);
      expect(bounds.height).toBe(300);
    });
    
    it('should calculate bounds with 2x DPI scaling', () => {
      const bounds = Bounds.calculate(mockElement, 2);
      expect(bounds.left).toBe(200);
      expect(bounds.top).toBe(400);
      expect(bounds.right).toBe(600);
      expect(bounds.bottom).toBe(800);
      expect(bounds.width).toBe(400);
      expect(bounds.height).toBe(400);
    });
    
    it('should calculate bounds with 3x DPI scaling', () => {
      const bounds = Bounds.calculate(mockElement, 3);
      expect(bounds.left).toBe(300);
      expect(bounds.top).toBe(600);
      expect(bounds.right).toBe(900);
      expect(bounds.bottom).toBe(1200);
      expect(bounds.width).toBe(600);
      expect(bounds.height).toBe(600);
    });
    
    it('should round fractional DPI results', () => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 99.7,
        top: 199.3,
        right: 299.8,
        bottom: 399.2,
        width: 200.1,
        height: 199.9,
        x: 99.7,
        y: 199.3,
        toJSON: () => {}
      }));
      
      const bounds = Bounds.calculate(mockElement, 1.5);
      // Math.round(99.7 * 1.5) = Math.round(149.55) = 150
      expect(bounds.left).toBe(150);
      expect(bounds.top).toBe(299);
      expect(bounds.right).toBe(450);
      expect(bounds.bottom).toBe(599);
    });
  });
  
  describe('calculate - Edge cases', () => {
    it('should handle zero-size element', () => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 100,
        top: 200,
        right: 100,
        bottom: 200,
        width: 0,
        height: 0,
        x: 100,
        y: 200,
        toJSON: () => {}
      }));
      
      const bounds = Bounds.calculate(mockElement, 1);
      expect(bounds.width).toBe(0);
      expect(bounds.height).toBe(0);
      expect(bounds.left).toBe(100);
      expect(bounds.right).toBe(100);
    });
    
    it('should handle very small element (1px)', () => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 100,
        top: 200,
        right: 101,
        bottom: 201,
        width: 1,
        height: 1,
        x: 100,
        y: 200,
        toJSON: () => {}
      }));
      
      const bounds = Bounds.calculate(mockElement, 1);
      expect(bounds.width).toBe(1);
      expect(bounds.height).toBe(1);
    });
    
    it('should handle very large element', () => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 0,
        top: 0,
        right: 10000,
        bottom: 5000,
        width: 10000,
        height: 5000,
        x: 0,
        y: 0,
        toJSON: () => {}
      }));
      
      const bounds = Bounds.calculate(mockElement, 1);
      expect(bounds.width).toBe(10000);
      expect(bounds.height).toBe(5000);
    });
    
    it('should handle negative position (off-screen left/top)', () => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: -50,
        top: -30,
        right: 50,
        bottom: 70,
        width: 100,
        height: 100,
        x: -50,
        y: -30,
        toJSON: () => {}
      }));
      
      const bounds = Bounds.calculate(mockElement, 1);
      expect(bounds.left).toBe(-50);
      expect(bounds.top).toBe(-30);
      expect(bounds.right).toBe(50);
      expect(bounds.bottom).toBe(70);
      expect(bounds.width).toBe(100);
      expect(bounds.height).toBe(100);
    });
    
    it('should handle element with decimal positions', () => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 100.5,
        top: 200.7,
        right: 300.3,
        bottom: 400.9,
        width: 199.8,
        height: 200.2,
        x: 100.5,
        y: 200.7,
        toJSON: () => {}
      }));
      
      const bounds = Bounds.calculate(mockElement, 1);
      expect(bounds.left).toBe(101); // rounded
      expect(bounds.top).toBe(201);
      expect(bounds.right).toBe(300);
      expect(bounds.bottom).toBe(401);
    });
  });
  
  describe('isInBounds - Basic functionality', () => {
    const bounds = {
      left: 10,
      top: 20,
      right: 110,
      bottom: 70,
      width: 100,
      height: 50
    };
    
    it('should return true for point in center', () => {
      expect(Bounds.isInBounds(50, 40, bounds)).toBe(true);
    });
    
    it('should return true for point at top-left corner', () => {
      expect(Bounds.isInBounds(10, 20, bounds)).toBe(true);
    });
    
    it('should return true for point at top-right corner', () => {
      expect(Bounds.isInBounds(110, 20, bounds)).toBe(true);
    });
    
    it('should return true for point at bottom-left corner', () => {
      expect(Bounds.isInBounds(10, 70, bounds)).toBe(true);
    });
    
    it('should return true for point at bottom-right corner', () => {
      expect(Bounds.isInBounds(110, 70, bounds)).toBe(true);
    });
    
    it('should return false for point outside left edge', () => {
      expect(Bounds.isInBounds(9, 40, bounds)).toBe(false);
    });
    
    it('should return false for point outside right edge', () => {
      expect(Bounds.isInBounds(111, 40, bounds)).toBe(false);
    });
    
    it('should return false for point outside top edge', () => {
      expect(Bounds.isInBounds(50, 19, bounds)).toBe(false);
    });
    
    it('should return false for point outside bottom edge', () => {
      expect(Bounds.isInBounds(50, 71, bounds)).toBe(false);
    });
    
    it('should return false for point far outside', () => {
      expect(Bounds.isInBounds(200, 200, bounds)).toBe(false);
      expect(Bounds.isInBounds(-100, -100, bounds)).toBe(false);
    });
  });
  
  describe('isInBounds - Edge cases', () => {
    it('should handle negative coordinates', () => {
      const bounds = {
        left: -50,
        top: -30,
        right: 50,
        bottom: 20,
        width: 100,
        height: 50
      };
      
      expect(Bounds.isInBounds(0, 0, bounds)).toBe(true);
      expect(Bounds.isInBounds(-10, 0, bounds)).toBe(true);
      expect(Bounds.isInBounds(-51, 0, bounds)).toBe(false);
      expect(Bounds.isInBounds(51, 0, bounds)).toBe(false);
    });
    
    it('should handle zero-size bounds', () => {
      const bounds = {
        left: 100,
        top: 200,
        right: 100,
        bottom: 200,
        width: 0,
        height: 0
      };
      
      expect(Bounds.isInBounds(100, 200, bounds)).toBe(true);
      expect(Bounds.isInBounds(99, 200, bounds)).toBe(false);
      expect(Bounds.isInBounds(101, 200, bounds)).toBe(false);
      expect(Bounds.isInBounds(100, 199, bounds)).toBe(false);
      expect(Bounds.isInBounds(100, 201, bounds)).toBe(false);
    });
    
    it('should handle very large coordinates', () => {
      const bounds = {
        left: 10000,
        top: 20000,
        right: 11000,
        bottom: 21000,
        width: 1000,
        height: 1000
      };
      
      expect(Bounds.isInBounds(10500, 20500, bounds)).toBe(true);
      expect(Bounds.isInBounds(9999, 20500, bounds)).toBe(false);
      expect(Bounds.isInBounds(11001, 20500, bounds)).toBe(false);
    });
    
    it('should handle fractional coordinates', () => {
      const bounds = {
        left: 10.5,
        top: 20.7,
        right: 110.5,
        bottom: 70.7,
        width: 100,
        height: 50
      };
      
      expect(Bounds.isInBounds(50.3, 40.2, bounds)).toBe(true);
      expect(Bounds.isInBounds(10.4, 40.2, bounds)).toBe(false);
      expect(Bounds.isInBounds(110.6, 40.2, bounds)).toBe(false);
    });
  });
  
  describe('isMouseOverElement - Basic functionality', () => {
    beforeEach(() => {
      mockElement.id = 'test-element';
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 100,
        top: 200,
        right: 300,
        bottom: 400,
        width: 200,
        height: 200,
        x: 100,
        y: 200,
        toJSON: () => {}
      }));
    });
    
    it('should return true when mouse is over element', () => {
      const result = Bounds.isMouseOverElement(200, 300, mockElement, 1);
      expect(result).toBe(true);
    });
    
    it('should return false when mouse is outside element', () => {
      const result = Bounds.isMouseOverElement(500, 600, mockElement, 1);
      expect(result).toBe(false);
    });
    
    it('should work with element selector', () => {
      const result = Bounds.isMouseOverElement(200, 300, '#test-element', 1);
      expect(result).toBe(true);
    });
    
    it('should return false for nonexistent selector', () => {
      const result = Bounds.isMouseOverElement(200, 300, '#nonexistent', 1);
      expect(result).toBe(false);
    });
    
    it('should return false for null element', () => {
      const result = Bounds.isMouseOverElement(200, 300, null as any, 1);
      expect(result).toBe(false);
    });
    
    it('should return false for invalid element', () => {
      const invalidElement = {} as HTMLElement;
      const result = Bounds.isMouseOverElement(200, 300, invalidElement, 1);
      expect(result).toBe(false);
    });
  });
  
  describe('isMouseOverElement - DPI scaling', () => {
    beforeEach(() => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 100,
        top: 200,
        right: 300,
        bottom: 400,
        width: 200,
        height: 200,
        x: 100,
        y: 200,
        toJSON: () => {}
      }));
    });
    
    it('should handle 1.5x DPI scaling', () => {
      // CSS: 100-300, Physical: 150-450
      expect(Bounds.isMouseOverElement(200, 350, mockElement, 1.5)).toBe(true);
      expect(Bounds.isMouseOverElement(149, 350, mockElement, 1.5)).toBe(false);
      expect(Bounds.isMouseOverElement(451, 350, mockElement, 1.5)).toBe(false);
    });
    
    it('should handle 2x DPI scaling', () => {
      // CSS: 100-300, Physical: 200-600
      expect(Bounds.isMouseOverElement(400, 500, mockElement, 2)).toBe(true);
      expect(Bounds.isMouseOverElement(199, 500, mockElement, 2)).toBe(false);
      expect(Bounds.isMouseOverElement(601, 500, mockElement, 2)).toBe(false);
    });
    
    it('should handle edge coordinates with DPI scaling', () => {
      // CSS: 100-300, Physical with 1.5x: 150-450
      expect(Bounds.isMouseOverElement(150, 300, mockElement, 1.5)).toBe(true);
      expect(Bounds.isMouseOverElement(450, 600, mockElement, 1.5)).toBe(true);
    });
  });
  
  describe('isMouseOverElement - Edge cases', () => {
    it('should handle zero-size element', () => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: 100,
        top: 200,
        right: 100,
        bottom: 200,
        width: 0,
        height: 0,
        x: 100,
        y: 200,
        toJSON: () => {}
      }));
      
      expect(Bounds.isMouseOverElement(100, 200, mockElement, 1)).toBe(true);
      expect(Bounds.isMouseOverElement(101, 200, mockElement, 1)).toBe(false);
    });
    
    it('should handle negative position element', () => {
      mockElement.getBoundingClientRect = jest.fn(() => ({
        left: -50,
        top: -30,
        right: 50,
        bottom: 70,
        width: 100,
        height: 100,
        x: -50,
        y: -30,
        toJSON: () => {}
      }));
      
      expect(Bounds.isMouseOverElement(0, 20, mockElement, 1)).toBe(true);
      expect(Bounds.isMouseOverElement(-51, 20, mockElement, 1)).toBe(false);
    });
  });
});
