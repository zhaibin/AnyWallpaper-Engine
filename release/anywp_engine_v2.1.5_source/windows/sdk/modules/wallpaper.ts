// Wallpaper Controller Module - v2.0.1+
// Handles desktop wallpaper embedding, mouse interaction, and mouseover recovery

import type { AnyWPSDK } from '../types';

let lastMouseX = 0;
let lastMouseY = 0;
let isInteractiveMode = false;
let mouseupTimeoutId: number | null = null;

/**
 * Force trigger mouseover event at current mouse position
 * This is critical after restoring WS_EX_TRANSPARENT when mouse is stationary
 */
function forceMouseOverEvent(x: number, y: number): void {
  const AnyWP = (window as any).AnyWP as AnyWPSDK;
  
  try {
    AnyWP._log('[Wallpaper] forceMouseOverEvent(' + x + ', ' + y + ') called', true);
    
    // Get element at current position
    const element = document.elementFromPoint(x, y);
    
    if (!element) {
      AnyWP._log('[Wallpaper] ERROR: No element at position (' + x + ',' + y + ')', true);
      return;
    }
    
    const elementInfo = element.tagName + 
      (element.className ? '.' + element.className : '') +
      (element.id ? '#' + element.id : '');
    
    AnyWP._log('[Wallpaper] Target element: ' + elementInfo, true);
    
    // Create and dispatch synthetic mouseover event
    const mouseEvent = new MouseEvent('mouseover', {
      view: window,
      bubbles: true,
      cancelable: true,
      clientX: x,
      clientY: y,
      screenX: x,
      screenY: y,
      button: 0,
      buttons: 0,
      relatedTarget: null
    });
    
    const dispatched = element.dispatchEvent(mouseEvent);
    AnyWP._log('[Wallpaper] MouseEvent dispatched: ' + dispatched, true);
    
    // Also dispatch a custom event for wallpaper systems
    const customEvent = new CustomEvent('wallpaper:forceMouseUpdate', {
      detail: { x, y, element, elementInfo },
      bubbles: true
    });
    document.dispatchEvent(customEvent);
    
    AnyWP._log('[Wallpaper] [OK] Mouseover events dispatched successfully', true);
    
  } catch (e) {
    AnyWP._log('[Wallpaper] EXCEPTION in forceMouseOverEvent: ' + e, true);
  }
}


/**
 * Set interactive mode via C++ bridge
 */
function setInteractive(interactive: boolean): void {
  const AnyWP = (window as any).AnyWP as AnyWPSDK;
  
  try {
    AnyWP._log('[Wallpaper] setInteractive(' + interactive + ') called', true);
    
    if ((window as any).chrome?.webview) {
      AnyWP._log('[Wallpaper] Sending setInteractive message to C++...', true);
      
      (window as any).chrome.webview.postMessage({
        type: 'setInteractive',
        interactive: interactive
      });
      
      isInteractiveMode = interactive;
      AnyWP._log('[Wallpaper] [OK] Interactive mode: ' + (interactive ? 'ENABLED' : 'DISABLED'), true);
    } else {
      AnyWP._log('[Wallpaper] ERROR: chrome.webview not available!', true);
      AnyWP._log('[Wallpaper] window.chrome = ' + (typeof (window as any).chrome), true);
      AnyWP._log('[Wallpaper] window.chrome.webview = ' + (typeof (window as any).chrome?.webview), true);
    }
  } catch (e) {
    AnyWP._log('[Wallpaper] EXCEPTION in setInteractive: ' + e, true);
  }
}

/**
 * Wait for next animation frame
 */
function nextFrame(): Promise<void> {
  return new Promise((resolve) => {
    requestAnimationFrame(() => {
      resolve();
    });
  });
}

/**
 * Handle mousedown event
 */
function handleMouseDown(event: Event): void {
  const mouseEvent = event as MouseEvent;
  const AnyWP = (window as any).AnyWP as AnyWPSDK;
  
  lastMouseX = mouseEvent.clientX;
  lastMouseY = mouseEvent.clientY;
  
  AnyWP._log('[Wallpaper] mousedown at (' + lastMouseX + ',' + lastMouseY + ')', true);
  
  // v2.0.1+ DISABLED: Auto-toggle removed, user controls interactive mode via Flutter UI
  // Interactive mode is now fully controlled by user settings, not auto-switched by SDK
  
  // Clear any pending restore timeout
  if (mouseupTimeoutId !== null) {
    clearTimeout(mouseupTimeoutId);
    mouseupTimeoutId = null;
  }
}

/**
 * Handle mouseup event
 */
function handleMouseUp(event: Event): void {
  const mouseEvent = event as MouseEvent;
  const AnyWP = (window as any).AnyWP as AnyWPSDK;
  
  lastMouseX = mouseEvent.clientX;
  lastMouseY = mouseEvent.clientY;
  
  AnyWP._log('[Wallpaper] mouseup at (' + lastMouseX + ',' + lastMouseY + ')', true);
  
  // For all clicks: restore immediately with forced mouseover
  restoreTransparency().catch((e) => {
    AnyWP._log('[Wallpaper] Error in restoreTransparency: ' + e, true);
  });
}

/**
 * Restore transparency and force mouseover event
 */
async function restoreTransparency(): Promise<void> {
  const AnyWP = (window as any).AnyWP as AnyWPSDK;
  
  try {
    AnyWP._log('[Wallpaper] === Restoring Transparency ===', true);
    AnyWP._log('[Wallpaper] Last mouse position: (' + lastMouseX + ',' + lastMouseY + ')', true);
    
    // Wait 1 frame for event to complete
    AnyWP._log('[Wallpaper] Waiting 1 frame for event completion...', true);
    await nextFrame();
    
    // v2.0.1+ DISABLED: Auto-toggle removed, user controls interactive mode via Flutter UI
    // Restore transparent mode
    // AnyWP._log('[Wallpaper] Setting interactive mode to FALSE...', true);
    // setInteractive(false);
    
    // Wait 1 frame for style to apply
    AnyWP._log('[Wallpaper] Waiting 1 frame for style update...', true);
    await nextFrame();
    
    // Force trigger mouseover at last known position
    AnyWP._log('[Wallpaper] Forcing mouseover event...', true);
    forceMouseOverEvent(lastMouseX, lastMouseY);
    
    AnyWP._log('[Wallpaper] [OK] Transparency restore sequence complete', true);
    
  } catch (e) {
    AnyWP._log('[Wallpaper] EXCEPTION in restoreTransparency: ' + e, true);
  }
}

/**
 * Handle mousemove event (track position)
 */
function handleMouseMove(event: Event): void {
  const mouseEvent = event as MouseEvent;
  lastMouseX = mouseEvent.clientX;
  lastMouseY = mouseEvent.clientY;
}

/**
 * Initialize wallpaper controller
 */
export function initWallpaperController(sdk: AnyWPSDK): void {
  sdk._log('[Wallpaper] Initializing wallpaper controller...');
  
  try {
    // Listen to all mouse events (capture phase, passive for performance)
    document.addEventListener('mousedown', handleMouseDown, { 
      capture: true, 
      passive: true 
    });
    
    document.addEventListener('mouseup', handleMouseUp, { 
      capture: true, 
      passive: true 
    });
    
    document.addEventListener('mousemove', handleMouseMove, { 
      capture: true, 
      passive: true 
    });
    
    sdk._log('[Wallpaper] Wallpaper controller initialized successfully', true);
    
    // Expose API for manual control
    (window as any).AnyWP.Wallpaper = {
      setInteractive,
      forceMouseOverEvent,
      getState: () => ({
        isInteractive: isInteractiveMode,
        lastMouseX,
        lastMouseY
      })
    };
    
  } catch (e) {
    sdk._log('[Wallpaper] ERROR initializing controller: ' + e, true);
  }
}

