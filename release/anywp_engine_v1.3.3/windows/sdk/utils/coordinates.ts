// Coordinate conversion utilities
import type { Position, CoordinateOffsets } from '../types';

export const Coordinates = {
  /**
   * Convert screen coordinates (physical pixels) to viewport CSS coordinates
   */
  screenToViewport(mouseX: number, mouseY: number, dpiScale: number): Position {
    // Get window position (in CSS pixels)
    const windowLeft = (typeof window.screenX !== 'undefined') ? window.screenX : 0;
    const windowTop = (typeof window.screenY !== 'undefined') ? window.screenY : 0;
    
    // Get document element position
    const docRect = document.documentElement.getBoundingClientRect();
    
    // Convert: physical pixels -> CSS pixels -> viewport coordinates
    const viewportX = (mouseX / dpiScale) - windowLeft - docRect.left;
    const viewportY = (mouseY / dpiScale) - windowTop - docRect.top;
    
    return { x: viewportX, y: viewportY };
  },
  
  /**
   * Get window and document offsets for drag calculations
   */
  getOffsets(dpiScale: number): CoordinateOffsets {
    const windowLeft = (typeof window.screenX !== 'undefined') ? window.screenX : 0;
    const windowTop = (typeof window.screenY !== 'undefined') ? window.screenY : 0;
    const docRect = document.documentElement.getBoundingClientRect();
    
    return {
      windowLeft,
      windowTop,
      docLeft: docRect.left,
      docTop: docRect.top,
      dpi: dpiScale
    };
  }
};

