// Bounds calculation utilities
import type { ElementBounds } from '../types';

export const Bounds = {
  /**
   * Calculate element bounds in physical pixels
   */
  calculate(element: HTMLElement, dpiScale: number): ElementBounds {
    const rect = element.getBoundingClientRect();
    
    return {
      left: Math.round(rect.left * dpiScale),
      top: Math.round(rect.top * dpiScale),
      right: Math.round(rect.right * dpiScale),
      bottom: Math.round(rect.bottom * dpiScale),
      width: Math.round(rect.width * dpiScale),
      height: Math.round(rect.height * dpiScale)
    };
  },
  
  /**
   * Check if point is in bounds
   */
  isInBounds(x: number, y: number, bounds: ElementBounds): boolean {
    return x >= bounds.left && x <= bounds.right &&
           y >= bounds.top && y <= bounds.bottom;
  },
  
  /**
   * Check if mouse (in physical pixels) is over element
   */
  isMouseOverElement(mouseX: number, mouseY: number, element: HTMLElement, dpiScale: number): boolean {
    const rect = element.getBoundingClientRect();
    
    const physicalLeft = Math.round(rect.left * dpiScale);
    const physicalTop = Math.round(rect.top * dpiScale);
    const physicalRight = Math.round(rect.right * dpiScale);
    const physicalBottom = Math.round(rect.bottom * dpiScale);
    
    return mouseX >= physicalLeft && mouseX <= physicalRight &&
           mouseY >= physicalTop && mouseY <= physicalBottom;
  }
};

