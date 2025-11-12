/**
 * WebMessage Handler Module
 *
 * Handles all communication with the C++ native layer via chrome.webview.postMessage
 * This is a critical module that must be initialized EARLY to catch all messages.
 *
 * Key responsibilities:
 * - Set up WebMessage listener for C++ → JavaScript communication
 * - Convert screen coordinates to viewport coordinates (DPI scaling)
 * - Dispatch real MouseEvents to DOM for proper event handling
 * - Find interactive elements at click coordinates
 * - Handle CustomEvent dispatching for backward compatibility
 */
import { Coordinates } from '../utils/coordinates';
import { throttle } from '../utils/throttle';
import { logger } from '../utils/logger';
import { isMouseEventData } from '../types/webmessage';
// Create scoped logger for WebMessage module
const log = logger.scope('WebMessage');
/**
 * Element cache for performance optimization
 * WeakMap allows automatic garbage collection when elements are removed from DOM
 */
const elementBoundsCache = new WeakMap();
const CACHE_TTL = 100; // Cache Time-To-Live: 100ms
/**
 * Interactive elements cache (recomputed when DOM changes)
 */
let cachedInteractiveElements = [];
let lastDOMUpdate = Date.now();
const DOM_UPDATE_THROTTLE = 1000; // Check DOM updates every 1 second
/**
 * Initialize WebMessage listener (must be called IMMEDIATELY after script load)
 */
export function setupWebMessageListener() {
    if (!window.chrome || !window.chrome.webview) {
        log.info('chrome.webview not available');
        return;
    }
    const globalAny = window;
    // Prevent duplicate listener registration
    if (globalAny._anywpEarlyMessageListenerRegistered) {
        log.warn('WebMessage listener already registered (EARLY), skipping duplicate');
        return;
    }
    log.info('Setting up WebMessage listener (EARLY)');
    globalAny._anywpEarlyMessageListenerRegistered = true;
    window.chrome.webview.addEventListener('message', handleWebMessage);
    log.info('WebMessage listener setup complete (EARLY)');
}
/**
 * Main WebMessage event handler
 */
function handleWebMessage(event) {
    const data = event.data;
    if (!data) {
        log.warn('Received empty WebMessage');
        return;
    }
    try {
        // Log selective messages to avoid spam
        logMessage(data);
        // Handle different message types using type guards
        if (isMouseEventData(data)) {
            handleMouseEvent(data);
        }
        // Add other message type handlers here as needed
    }
    catch (error) {
        log.error('Error handling WebMessage:', error);
    }
}
/**
 * Selective logging for WebMessage events
 */
function logMessage(data) {
    if (data.type === 'mouseEvent' && data.eventType === 'mousemove') {
        // Log every 100th mousemove to avoid spam
        if (!window._anywp_msg_count) {
            window._anywp_msg_count = 0;
        }
        window._anywp_msg_count++;
        if (window._anywp_msg_count % 100 === 0) {
            log.debug('WebMessage received #' + window._anywp_msg_count + ': ' + data.eventType);
        }
    }
    else if (data.type === 'mouseEvent') {
        log.debug('WebMessage: ' + data.eventType + ' at (' + data.x + ',' + data.y + ')');
    }
}
/**
 * Handle mouseEvent messages from C++
 */
function handleMouseEvent(data) {
    try {
        // v2.0.5+ Convert screen coordinates to viewport coordinates
        // C++ sends physical screen pixels, but we need CSS viewport coordinates
        const AnyWP = window.AnyWP;
        const viewportPos = Coordinates.screenToViewport(data.x, data.y, AnyWP.dpiScale);
        log.debug('[CoordinateConversion] Screen (' + data.x + ',' + data.y + ') -> Viewport (' +
            Math.round(viewportPos.x) + ',' + Math.round(viewportPos.y) + ') DPI:' + AnyWP.dpiScale);
        // v2.0.4+ Dispatch REAL MouseEvent to document for proper DOM interaction
        const eventInit = {
            view: window,
            bubbles: true,
            cancelable: true,
            clientX: viewportPos.x,
            clientY: viewportPos.y,
            screenX: data.x,
            screenY: data.y,
            button: data.button || 0,
            buttons: data.button === 0 ? 1 : 0
        };
        // v2.0.6+ Simplified handling for mousemove (performance optimization)
        if (data.eventType === 'mousemove') {
            handleMouseMove(eventInit, data);
            return; // Skip click-specific logic
        }
        // Handle click events (mousedown, mouseup, click)
        handleClickEvent(data, eventInit, viewportPos);
    }
    catch (e) {
        log.error('Error handling mouse event:', e);
    }
}
/**
 * Handle mousemove events (optimized for performance with throttling)
 */
const handleMouseMove = throttle((eventInit, data) => {
    // For mousemove, just dispatch to document without element search
    // This avoids expensive DOM queries for every mouse movement
    const mousemoveEvent = new MouseEvent('mousemove', eventInit);
    document.dispatchEvent(mousemoveEvent);
    // Log every 50th mousemove for debugging
    window._mousemove_count = (window._mousemove_count || 0) + 1;
    if (window._mousemove_count % 50 === 0) {
        log.debug('[DOMDispatch] mousemove #' + window._mousemove_count +
            ' at viewport (' + Math.round(eventInit.clientX) + ',' + Math.round(eventInit.clientY) + ')');
    }
    // Still dispatch CustomEvent for backward compatibility
    const customEvent = new CustomEvent('AnyWP:mouse', {
        detail: {
            type: 'mousemove',
            x: data.x,
            y: data.y,
            button: data.button || 0
        }
    });
    window.dispatchEvent(customEvent);
}, 16); // ~60 FPS throttle (16ms)
/**
 * Get cached or fresh interactive elements
 */
function getInteractiveElements() {
    const now = Date.now();
    // Recompute cache if TTL expired
    if (now - lastDOMUpdate > DOM_UPDATE_THROTTLE || cachedInteractiveElements.length === 0) {
        const interactiveSelectors = [
            'button', 'a', 'input', 'textarea', 'select',
            '[onclick]', '.clickable', '[role="button"]',
            '.hover-box', '[onmouseenter]', '[onmouseleave]'
        ];
        const candidates = [];
        for (const selector of interactiveSelectors) {
            const elements = document.querySelectorAll(selector);
            elements.forEach((elem) => {
                if (document.body.contains(elem)) { // Only include elements still in DOM
                    candidates.push(elem);
                }
            });
        }
        cachedInteractiveElements = candidates;
        lastDOMUpdate = now;
        log.debug('[ElementCache] Updated interactive elements cache:', candidates.length, 'elements');
    }
    return cachedInteractiveElements;
}
/**
 * Get cached or fresh element bounds
 */
function getElementBounds(elem) {
    const now = Date.now();
    const cached = elementBoundsCache.get(elem);
    // Return cached bounds if still valid
    if (cached && (now - cached.timestamp) < CACHE_TTL) {
        return cached.bounds;
    }
    // Compute fresh bounds
    const bounds = elem.getBoundingClientRect();
    elementBoundsCache.set(elem, { bounds, timestamp: now });
    return bounds;
}
/**
 * Handle click events (mousedown, mouseup, click)
 */
function handleClickEvent(data, eventInit, viewportPos) {
    // v2.0.5+ Manual element detection using getBoundingClientRect
    // elementFromPoint() doesn't work reliably when wallpaper is below desktop icons
    let targetElement = null;
    const findElementAtPoint = (x, y) => {
        log.debug('[ElementFinder] Searching at viewport coordinates:', x, y);
        log.debug('[ElementFinder] Window size:', window.innerWidth, 'x', window.innerHeight);
        // Use cached interactive elements
        const candidates = getInteractiveElements();
        log.debug('[ElementFinder] Found', candidates.length, 'interactive elements (cached)');
        // Find the topmost element at the given coordinates
        let found = null;
        let maxZIndex = -Infinity;
        for (const elem of candidates) {
            // Use cached bounds
            const rect = getElementBounds(elem);
            const isInside = x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
            if (isInside) {
                log.debug('[ElementFinder] HIT:', elem.tagName, elem.id || elem.className, 'rect:', Math.round(rect.left), Math.round(rect.top), Math.round(rect.right), Math.round(rect.bottom));
                const style = window.getComputedStyle(elem);
                const zIndex = style.zIndex === 'auto' ? 0 : parseInt(style.zIndex);
                log.debug('[ElementFinder]   zIndex:', zIndex);
                if (zIndex >= maxZIndex) {
                    maxZIndex = zIndex;
                    found = elem;
                }
            }
        }
        if (found) {
            log.debug('[ElementFinder] Selected element:', found.tagName, found.id || found.className, 'zIndex:', maxZIndex);
        }
        else {
            log.debug('[ElementFinder] No interactive element found at coordinates');
        }
        return found;
    };
    targetElement = findElementAtPoint(viewportPos.x, viewportPos.y);
    // Create the MouseEvent
    const mouseEvent = new MouseEvent(data.eventType, eventInit);
    // Dispatch to the target element or document
    if (targetElement) {
        log.debug('[DOMDispatch] Dispatching', data.eventType, 'to element:', targetElement.tagName, targetElement.id || targetElement.className);
        targetElement.dispatchEvent(mouseEvent);
    }
    else {
        log.debug('[DOMDispatch] Dispatching', data.eventType, 'to document (no element found)');
        document.dispatchEvent(mouseEvent);
    }
    // Log event details
    if (data.eventType === 'mousedown') {
        log.info('[DOMDispatch] mousedown dispatched to', targetElement ? (targetElement.tagName + '#' + (targetElement.id || '?')) : 'document');
    }
    else if (data.eventType === 'mouseup') {
        log.info('[DOMDispatch] mouseup dispatched to', targetElement ? (targetElement.tagName + '#' + (targetElement.id || '?')) : 'document');
    }
    else if (data.eventType === 'click') {
        log.info('[DOMDispatch] click dispatched to', targetElement ? (targetElement.tagName + '#' + (targetElement.id || '?')) : 'document');
    }
    // Also dispatch CustomEvent for backward compatibility
    const customEvent = new CustomEvent('AnyWP:mouse', {
        detail: {
            type: data.eventType,
            x: data.x,
            y: data.y,
            button: data.button || 0
        }
    });
    window.dispatchEvent(customEvent);
    // Also dispatch legacy click event
    if (data.eventType === 'mouseup') {
        const legacyClickEvent = new CustomEvent('AnyWP:click', {
            detail: {
                x: data.x,
                y: data.y
            }
        });
        window.dispatchEvent(legacyClickEvent);
    }
}
/**
 * Send message to Flutter
 *
 * Sends a structured message to the Flutter application via chrome.webview.postMessage
 *
 * @param type - Message type (e.g., 'carouselStateChanged', 'wallpaperReady', 'error')
 * @param data - Message data payload
 * @returns true if message was sent, false if bridge not available
 *
 * @example
 * ```typescript
 * // Send carousel state change
 * sendToFlutter('carouselStateChanged', {
 *   currentIndex: 2,
 *   totalImages: 10,
 *   isPlaying: true
 * });
 *
 * // Send error
 * sendToFlutter('error', {
 *   code: 'IMAGE_LOAD_FAILED',
 *   message: 'Failed to load image',
 *   details: { url: 'https://example.com/image.jpg' }
 * });
 * ```
 */
export function sendToFlutter(type, data = {}) {
    if (!window.chrome?.webview) {
        log.warn('chrome.webview not available, cannot send message to Flutter');
        return false;
    }
    const message = {
        type: type,
        timestamp: Date.now(),
        data: data
    };
    log.info('[SendToFlutter] Sending message:', type);
    log.debug('[SendToFlutter] Message data:', message);
    try {
        window.chrome.webview.postMessage(message);
        return true;
    }
    catch (error) {
        log.error('[SendToFlutter] Error sending message:', error);
        return false;
    }
}
/**
 * Setup listener for messages from Flutter
 *
 * Listens for CustomEvent 'AnyWP:message' dispatched by Flutter
 * and allows user code to handle these messages
 */
export function setupFlutterMessageListener() {
    log.info('Setting up Flutter message listener');
    window.addEventListener('AnyWP:message', (event) => {
        const customEvent = event;
        const message = customEvent.detail;
        log.info('[FlutterMessage] Received message from Flutter:', message?.type || 'unknown');
        log.debug('[FlutterMessage] Message details:', message);
        // Dispatch to user-registered handler if available
        const AnyWP = window.AnyWP;
        if (AnyWP && AnyWP._onFlutterMessage) {
            try {
                AnyWP._onFlutterMessage(message);
            }
            catch (error) {
                log.error('[FlutterMessage] Error in user message handler:', error);
            }
        }
    });
    log.info('Flutter message listener setup complete');
}
/**
 * WebMessage Module
 */
export const WebMessage = {
    setup: setupWebMessageListener,
    setupFlutterListener: setupFlutterMessageListener,
    sendToFlutter: sendToFlutter
};
//# sourceMappingURL=webmessage.js.map