// Debug utilities module
import type { ElementBounds } from '../types';

export const Debug = {
  enabled: false,
  
  /**
   * Enable debug mode
   */
  enable(): void {
    this.enabled = true;
    console.log('[AnyWP] Debug mode ENABLED');
  },
  
  /**
   * Log with debug control
   */
  log(message: string, forceLog: boolean = false): void {
    if (this.enabled || forceLog) {
      console.log('[AnyWP] ' + message);
    }
  },
  
  /**
   * Show debug border around element
   */
  showBorder(bounds: ElementBounds, element: HTMLElement, dpiScale: number): void {
    // Remove old border if exists
    const oldBorder = (element as any)._anywpDebugBorder as HTMLElement | undefined;
    if (oldBorder && oldBorder.parentNode) {
      oldBorder.parentNode.removeChild(oldBorder);
    }
    
    const border = document.createElement('div');
    border.className = 'AnyWP-debug-border';
    border.style.cssText = 
      'position: fixed;' +
      'left: ' + (bounds.left / dpiScale) + 'px;' +
      'top: ' + (bounds.top / dpiScale) + 'px;' +
      'width: ' + (bounds.width / dpiScale) + 'px;' +
      'height: ' + (bounds.height / dpiScale) + 'px;' +
      'border: 2px solid red;' +
      'box-shadow: 0 0 10px red;' +
      'pointer-events: none;' +
      'z-index: 999999;';
    document.body.appendChild(border);
    
    (element as any)._anywpDebugBorder = border;
  },
  
  /**
   * Detect debug mode from URL parameter
   */
  detectFromURL(): void {
    const urlParams = new URLSearchParams(window.location.search);
    if (urlParams.has('debug')) {
      this.enable();
    }
  }
};

