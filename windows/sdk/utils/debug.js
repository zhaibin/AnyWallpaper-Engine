// Debug utilities module
export const Debug = {
  enabled: false,
  
  // Enable debug mode
  enable() {
    this.enabled = true;
    console.log('[AnyWP] Debug mode ENABLED');
  },
  
  // Log with debug control
  log(message, forceLog = false) {
    if (this.enabled || forceLog) {
      console.log('[AnyWP] ' + message);
    }
  },
  
  // Show debug border around element
  showBorder(bounds, element, dpiScale) {
    // Remove old border if exists
    const oldBorder = element._anywpDebugBorder;
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
    
    element._anywpDebugBorder = border;
  },
  
  // Detect debug mode from URL parameter
  detectFromURL() {
    const urlParams = new URLSearchParams(window.location.search);
    if (urlParams.has('debug')) {
      this.enable();
    }
  }
};

