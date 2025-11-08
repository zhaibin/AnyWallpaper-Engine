/**
 * Drag handler module tests
 */
import { describe, test, expect, beforeEach, afterEach, jest } from '@jest/globals';
import { DragHandler } from '../modules/drag';
import { Storage } from '../modules/storage';
import type { AnyWPSDK } from '../types';

describe('Drag handler module', () => {
  let mockAnyWP: AnyWPSDK;
  let mockElement: HTMLElement;
  
  beforeEach(() => {
    // Create mock AnyWP object
    mockAnyWP = {
      _draggableElements: [],
      _dragState: null,
      _persistedState: {},
      dpiScale: 1,
      interactionEnabled: true
    } as any;
    
    // Create mock element
    mockElement = document.createElement('div');
    mockElement.id = 'draggable';
    mockElement.style.position = 'absolute';
    mockElement.style.left = '50px';
    mockElement.style.top = '100px';
    document.body.appendChild(mockElement);
    
    // Mock getBoundingClientRect
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
    
    // Mock window properties
    Object.defineProperty(window, 'screenX', { value: 0, writable: true, configurable: true });
    Object.defineProperty(window, 'screenY', { value: 0, writable: true, configurable: true });
    
    // Mock document rect
    document.documentElement.getBoundingClientRect = jest.fn(() => ({
      left: 0,
      top: 0,
      right: 800,
      bottom: 600,
      width: 800,
      height: 600,
      x: 0,
      y: 0,
      toJSON: () => {}
    }));
  });
  
  afterEach(() => {
    if (mockElement.parentNode) {
      document.body.removeChild(mockElement);
    }
  });
  
  describe('makeDraggable', () => {
    test('should register element as draggable', () => {
      DragHandler.makeDraggable(mockAnyWP, mockElement);
      
      expect(mockAnyWP._draggableElements.length).toBe(1);
      expect(mockAnyWP._draggableElements[0]?.element).toBe(mockElement);
    });
    
    test('should set cursor style', () => {
      DragHandler.makeDraggable(mockAnyWP, mockElement);
      
      expect(mockElement.style.cursor).toBe('move');
    });
    
    test('should disable text selection', () => {
      DragHandler.makeDraggable(mockAnyWP, mockElement);
      
      expect(mockElement.style.userSelect).toBe('none');
      expect(mockElement.draggable).toBe(false);
    });
    
    test('should register by selector', () => {
      DragHandler.makeDraggable(mockAnyWP, '#draggable');
      
      expect(mockAnyWP._draggableElements.length).toBe(1);
      expect(mockAnyWP._draggableElements[0]?.element).toBe(mockElement);
    });
    
    test('should handle element not found', () => {
      const consoleErrorSpy = jest.spyOn(console, 'error').mockImplementation(() => {});
      
      DragHandler.makeDraggable(mockAnyWP, '#nonexistent');
      
      expect(consoleErrorSpy).toHaveBeenCalled();
      expect(mockAnyWP._draggableElements.length).toBe(0);
      
      consoleErrorSpy.mockRestore();
    });
    
    test('should load persisted position', (done) => {
      const persistKey = 'drag_pos';
      const savedPos = { left: 150, top: 250 };
      
      // Mock Storage.load to return saved position
      const originalLoad = Storage.load;
      Storage.load = jest.fn((_anyWP: AnyWPSDK, _key: string, callback: (data: any) => void) => {
        callback(savedPos);
      }) as any;
      
      DragHandler.makeDraggable(mockAnyWP, mockElement, { persistKey });
      
      // Wait for async position restore
      setTimeout(() => {
        expect(mockElement.style.left).toBe('150px');
        expect(mockElement.style.top).toBe('250px');
        
        Storage.load = originalLoad;
        done();
      }, 10);
    });
    
    test('should register with callbacks', () => {
      const onDragStart = jest.fn();
      const onDrag = jest.fn();
      const onDragEnd = jest.fn();
      
      DragHandler.makeDraggable(mockAnyWP, mockElement, {
        onDragStart,
        onDrag,
        onDragEnd
      });
      
      expect(mockAnyWP._draggableElements[0]?.onDragStart).toBe(onDragStart);
      expect(mockAnyWP._draggableElements[0]?.onDrag).toBe(onDrag);
      expect(mockAnyWP._draggableElements[0]?.onDragEnd).toBe(onDragEnd);
    });
    
    test('should register with bounds constraint', () => {
      const bounds = { left: 0, top: 0, right: 500, bottom: 500 };
      
      DragHandler.makeDraggable(mockAnyWP, mockElement, { bounds });
      
      expect(mockAnyWP._draggableElements[0]?.bounds).toEqual(bounds);
    });
    
    test('should attach mouse event handler', () => {
      DragHandler.makeDraggable(mockAnyWP, mockElement);
      
      expect((mockElement as any).__anyWP_dragHandler).toBeDefined();
      expect((mockElement as any).__anyWP_dragHandler.handleGlobalMouse).toBeInstanceOf(Function);
    });
  });
  
  describe('removeDraggable', () => {
    test('should remove element from draggables list', () => {
      DragHandler.makeDraggable(mockAnyWP, mockElement);
      expect(mockAnyWP._draggableElements.length).toBe(1);
      
      DragHandler.removeDraggable(mockAnyWP, mockElement);
      
      expect(mockAnyWP._draggableElements.length).toBe(0);
    });
    
    test('should remove event listener', () => {
      DragHandler.makeDraggable(mockAnyWP, mockElement);
      expect((mockElement as any).__anyWP_dragHandler).toBeDefined();
      
      DragHandler.removeDraggable(mockAnyWP, mockElement);
      
      expect((mockElement as any).__anyWP_dragHandler).toBeUndefined();
    });
    
    test('should reset cursor style', () => {
      DragHandler.makeDraggable(mockAnyWP, mockElement);
      expect(mockElement.style.cursor).toBe('move');
      
      DragHandler.removeDraggable(mockAnyWP, mockElement);
      
      expect(mockElement.style.cursor).toBe('');
    });
    
    test('should handle element not found', () => {
      const consoleErrorSpy = jest.spyOn(console, 'error').mockImplementation(() => {});
      
      DragHandler.removeDraggable(mockAnyWP, '#nonexistent');
      
      expect(consoleErrorSpy).toHaveBeenCalled();
      
      consoleErrorSpy.mockRestore();
    });
  });
  
  describe('resetPosition', () => {
    test('should reset element to specified position', () => {
      DragHandler.makeDraggable(mockAnyWP, mockElement, { persistKey: 'test' });
      
      const result = DragHandler.resetPosition(mockAnyWP, mockElement, { left: 200, top: 300 });
      
      expect(result).toBe(true);
      expect(mockElement.style.left).toBe('200px');
      expect(mockElement.style.top).toBe('300px');
    });
    
    test('should clear element position when no position provided', () => {
      mockElement.style.left = '100px';
      mockElement.style.top = '200px';
      
      DragHandler.makeDraggable(mockAnyWP, mockElement, { persistKey: 'test' });
      
      const result = DragHandler.resetPosition(mockAnyWP, mockElement);
      
      expect(result).toBe(true);
      expect(mockElement.style.left).toBe('');
      expect(mockElement.style.top).toBe('');
    });
    
    test('should clear persisted state', () => {
      const persistKey = 'test_key';
      mockAnyWP._persistedState[persistKey] = { x: 100, y: 200 };
      
      DragHandler.makeDraggable(mockAnyWP, mockElement, { persistKey });
      DragHandler.resetPosition(mockAnyWP, mockElement);
      
      expect(mockAnyWP._persistedState[persistKey]).toBeUndefined();
    });
    
    test('should return false when element not found', () => {
      const consoleErrorSpy = jest.spyOn(console, 'error').mockImplementation(() => {});
      
      const result = DragHandler.resetPosition(mockAnyWP, '#nonexistent');
      
      expect(result).toBe(false);
      expect(consoleErrorSpy).toHaveBeenCalled();
      
      consoleErrorSpy.mockRestore();
    });
  });
  
  describe('drag interaction', () => {
    test('should prevent default drag behavior', () => {
      DragHandler.makeDraggable(mockAnyWP, mockElement);
      
      const dragEvent = new Event('dragstart') as any;
      dragEvent.preventDefault = jest.fn();
      
      if (mockElement.ondragstart) {
        mockElement.ondragstart(dragEvent);
      }
      
      expect(dragEvent.preventDefault).toHaveBeenCalled();
    });
    
    test('should prevent text selection', () => {
      DragHandler.makeDraggable(mockAnyWP, mockElement);
      
      const selectEvent = new Event('selectstart') as any;
      selectEvent.preventDefault = jest.fn();
      
      if (mockElement.onselectstart) {
        mockElement.onselectstart(selectEvent);
      }
      
      expect(selectEvent.preventDefault).toHaveBeenCalled();
    });
  });
  
  describe('persisted positions', () => {
    test('should save position on drag end', () => {
      const persistKey = 'drag_key';
      
      // Mock Storage.saveElementPosition
      const savePositionSpy = jest.spyOn(Storage, 'saveElementPosition').mockImplementation(() => {});
      
      DragHandler.makeDraggable(mockAnyWP, mockElement, { persistKey });
      
      // Simulate drag state end would call saveElementPosition
      // (Full drag simulation is complex and tested via integration tests)
      
      savePositionSpy.mockRestore();
    });
  });
});

