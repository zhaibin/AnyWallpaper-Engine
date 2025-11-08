// AnyWP Engine SDK v4.2.0 - JavaScript Bridge
// Auto-injected into WebView2
// React/Vue SPA Compatible
// Built with TypeScript modular architecture (100% TS)

var AnyWPBundle = (function (exports) {
    'use strict';

    const AnyWP = {
        // Properties
        version: '4.2.0',
        dpiScale: window.devicePixelRatio || 1,
        screenWidth: screen.width * (window.devicePixelRatio || 1),
        screenHeight: screen.height * (window.devicePixelRatio || 1),
        interactionEnabled: true,
        // Internal state
        _debugMode: false,
        _clickHandlers: [],
        _mouseCallbacks: [],
        _keyboardCallbacks: [],
        _visibilityCallback: null,
        _mutationObserver: null,
        _resizeObserver: null,
        _spaMode: false,
        _autoRefreshEnabled: true,
        _draggableElements: [],
        _dragState: null,
        _persistedState: {},
        // Initialize (will be implemented in init.ts)
        _init() {
            throw new Error('_init must be implemented');
        },
        // Logging (will be implemented in index.ts)
        _log(_message, _always) {
            throw new Error('Not implemented');
        },
        log(_message) {
            throw new Error('Not implemented');
        },
        // Placeholder methods (will be implemented by modules)
        enableDebug() {
            throw new Error('Not implemented');
        },
        onClick() {
            throw new Error('Not implemented');
        },
        refreshBounds() {
            throw new Error('Not implemented');
        },
        clearHandlers() {
            throw new Error('Not implemented');
        },
        makeDraggable() {
            throw new Error('Not implemented');
        },
        removeDraggable() {
            throw new Error('Not implemented');
        },
        resetPosition() {
            throw new Error('Not implemented');
        },
        saveState() {
            throw new Error('Not implemented');
        },
        loadState() {
            throw new Error('Not implemented');
        },
        clearState() {
            throw new Error('Not implemented');
        },
        onMouse() {
            throw new Error('Not implemented');
        },
        onKeyboard() {
            throw new Error('Not implemented');
        },
        onVisibilityChange() {
            throw new Error('Not implemented');
        },
        _notifyVisibilityChange() {
            throw new Error('Not implemented');
        },
        setSPAMode() {
            throw new Error('Not implemented');
        },
        openURL(url) {
            console.log('[AnyWP] Opening URL: ' + url);
            if (window.chrome?.webview) {
                window.chrome.webview.postMessage({
                    type: 'openURL',
                    url: url
                });
            }
            else {
                console.warn('[AnyWP] Native bridge not available');
                window.open(url, '_blank');
            }
        },
        ready(name) {
            console.log('[AnyWP] Wallpaper ready: ' + name);
            if (window.chrome?.webview) {
                window.chrome.webview.postMessage({
                    type: 'ready',
                    name: name
                });
            }
        }
    };

    const Debug = {
        enabled: false,
        /**
         * Enable debug mode
         */
        enable() {
            this.enabled = true;
            console.log('[AnyWP] Debug mode ENABLED');
        },
        /**
         * Log with debug control
         */
        log(message, forceLog = false) {
            if (this.enabled || forceLog) {
                console.log('[AnyWP] ' + message);
            }
        },
        /**
         * Show debug border around element
         */
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
        /**
         * Detect debug mode from URL parameter
         */
        detectFromURL() {
            const urlParams = new URLSearchParams(window.location.search);
            if (urlParams.has('debug')) {
                this.enable();
            }
        }
    };

    // Event system module
    const Events = {
        _setupCompleted: false,
        _eventHandlers: {},
        // Setup event listeners
        setup(anyWP, clickHandler, animationsHandler) {
            // Prevent duplicate setup
            if (this._setupCompleted) {
                console.log('[AnyWP] Events already setup, skipping duplicate initialization');
                return;
            }
            // Create event handlers (store references for potential cleanup)
            this._eventHandlers.mouse = function (event) {
                const customEvent = event;
                anyWP._mouseCallbacks.forEach(function (cb) {
                    cb(customEvent.detail);
                });
            };
            this._eventHandlers.keyboard = function (event) {
                const customEvent = event;
                anyWP._keyboardCallbacks.forEach(function (cb) {
                    cb(customEvent.detail);
                });
            };
            this._eventHandlers.click = function (event) {
                const customEvent = event;
                clickHandler.handleClick(customEvent.detail.x, customEvent.detail.y);
            };
            this._eventHandlers.interactionMode = function (event) {
                const customEvent = event;
                anyWP.interactionEnabled = customEvent.detail.enabled;
                Debug.log('Interaction mode: ' + (anyWP.interactionEnabled ? 'ON' : 'OFF'), true);
            };
            this._eventHandlers.visibility = function (event) {
                const customEvent = event;
                const visible = customEvent.detail.visible;
                Debug.log('Visibility changed: ' + (visible ? 'visible' : 'hidden'), true);
                if (anyWP._visibilityCallback) {
                    anyWP._visibilityCallback(visible);
                }
                if (!visible) {
                    animationsHandler.pause(anyWP);
                }
                else {
                    animationsHandler.resume(anyWP);
                }
            };
            this._eventHandlers.resize = function () {
                Debug.log('Window resized, refreshing...');
                setTimeout(function () {
                    clickHandler.refreshBounds(anyWP);
                }, 200);
            };
            // Register event listeners
            window.addEventListener('AnyWP:mouse', this._eventHandlers.mouse);
            window.addEventListener('AnyWP:keyboard', this._eventHandlers.keyboard);
            window.addEventListener('AnyWP:click', this._eventHandlers.click);
            window.addEventListener('AnyWP:interactionMode', this._eventHandlers.interactionMode);
            window.addEventListener('AnyWP:visibility', this._eventHandlers.visibility);
            window.addEventListener('resize', this._eventHandlers.resize);
            this._setupCompleted = true;
            console.log('[AnyWP] Events setup completed');
        },
        // Register mouse callback
        onMouse(anyWP, callback) {
            anyWP._mouseCallbacks.push(callback);
            Debug.log('Mouse callback registered');
        },
        // Register keyboard callback
        onKeyboard(anyWP, callback) {
            anyWP._keyboardCallbacks.push(callback);
            Debug.log('Keyboard callback registered');
        },
        // Register visibility callback
        onVisibilityChange(anyWP, callback) {
            anyWP._visibilityCallback = callback;
            Debug.log('Visibility callback registered');
        },
        // Notify visibility change (called by C++)
        notifyVisibilityChange(anyWP, visible) {
            console.log('[AnyWP] _notifyVisibilityChange called:', visible);
            if (anyWP._visibilityCallback && typeof anyWP._visibilityCallback === 'function') {
                try {
                    anyWP._visibilityCallback(visible);
                    console.log('[AnyWP] Visibility callback executed successfully');
                }
                catch (e) {
                    console.error('[AnyWP] Error in visibility callback:', e);
                }
            }
            else {
                console.log('[AnyWP] No visibility callback registered');
            }
            try {
                const event = new CustomEvent('AnyWP:visibility', {
                    detail: { visible: visible }
                });
                window.dispatchEvent(event);
                console.log('[AnyWP] AnyWP:visibility event dispatched');
            }
            catch (e) {
                console.error('[AnyWP] Error dispatching visibility event:', e);
            }
        }
    };

    // SPA framework support module
    const SPA = {
        // Detect SPA framework
        detect(anyWP, clickHandler) {
            const self = this;
            // Immediate check for React/Vue/Angular globals
            const isReact = !!(window.React || window.ReactDOM);
            const isVue = !!(window.Vue);
            const isAngular = !!(window.angular);
            if (isReact || isVue || isAngular) {
                anyWP._spaMode = true;
                const framework = isReact ? 'React' : (isVue ? 'Vue' : 'Angular');
                console.log('[AnyWP] SPA Framework detected: ' + framework);
                self.setupMonitoring(anyWP, clickHandler);
            }
            else {
                // Delayed check for DOM elements
                setTimeout(function () {
                    const isReactDOM = !!document.querySelector('[data-reactroot], [data-reactid], #root');
                    const isVueDOM = !!document.querySelector('[data-v-]');
                    const isAngularDOM = !!document.querySelector('[ng-version]');
                    if (isReactDOM || isVueDOM || isAngularDOM) {
                        anyWP._spaMode = true;
                        const framework = isReactDOM ? 'React' : (isVueDOM ? 'Vue' : 'Angular');
                        console.log('[AnyWP] SPA Framework detected: ' + framework);
                        self.setupMonitoring(anyWP, clickHandler);
                    }
                }, 500);
            }
        },
        // Enable/Disable SPA mode manually
        setSPAMode(anyWP, clickHandler, enabled) {
            anyWP._spaMode = enabled;
            console.log('[AnyWP] SPA mode: ' + (enabled ? 'ENABLED' : 'DISABLED'));
            if (enabled) {
                this.setupMonitoring(anyWP, clickHandler);
            }
            else {
                this.teardownMonitoring(anyWP);
            }
        },
        // Setup SPA monitoring
        setupMonitoring(anyWP, clickHandler) {
            const self = this;
            // Monitor history changes
            const originalPushState = history.pushState;
            const originalReplaceState = history.replaceState;
            history.pushState = function (data, unused, url) {
                originalPushState.call(history, data, unused, url);
                self.onRouteChange(anyWP, clickHandler);
            };
            history.replaceState = function (data, unused, url) {
                originalReplaceState.call(history, data, unused, url);
                self.onRouteChange(anyWP, clickHandler);
            };
            window.addEventListener('popstate', function () {
                self.onRouteChange(anyWP, clickHandler);
            });
            // Monitor DOM mutations
            if (window.MutationObserver) {
                anyWP._mutationObserver = new MutationObserver(function (mutations) {
                    let shouldRefresh = false;
                    mutations.forEach(function (mutation) {
                        if (mutation.type === 'childList' || mutation.type === 'attributes') {
                            anyWP._clickHandlers.forEach(function (handler) {
                                if (mutation.target === handler.element ||
                                    mutation.target.contains(handler.element)) {
                                    shouldRefresh = true;
                                }
                            });
                        }
                    });
                    if (shouldRefresh) {
                        clickHandler.refreshBounds(anyWP);
                    }
                });
                anyWP._mutationObserver.observe(document.body, {
                    childList: true,
                    subtree: true,
                    attributes: true,
                    attributeFilter: ['class', 'style']
                });
            }
        },
        // Teardown SPA monitoring
        teardownMonitoring(anyWP) {
            if (anyWP._mutationObserver) {
                anyWP._mutationObserver.disconnect();
                anyWP._mutationObserver = null;
            }
        },
        // Handle SPA route change
        onRouteChange(anyWP, clickHandler) {
            Debug.log('Route changed, refreshing...');
            setTimeout(function () {
                anyWP._clickHandlers.forEach(function (handler) {
                    if (handler.selector && handler.autoRefresh) {
                        const newElement = document.querySelector(handler.selector);
                        if (newElement && newElement !== handler.element) {
                            handler.element = newElement;
                            clickHandler.refreshElementBounds(anyWP, handler);
                            if (handler.resizeObserver) {
                                handler.resizeObserver.disconnect();
                            }
                            if (window.ResizeObserver) {
                                const resizeObserver = new ResizeObserver(function () {
                                    clickHandler.refreshElementBounds(anyWP, handler);
                                });
                                resizeObserver.observe(newElement);
                                handler.resizeObserver = resizeObserver;
                            }
                        }
                    }
                });
                clickHandler.refreshBounds(anyWP);
            }, 500);
        }
    };

    const Bounds = {
        /**
         * Calculate element bounds in physical pixels
         */
        calculate(element, dpiScale) {
            // Handle string selector
            let el = element;
            if (typeof element === 'string') {
                const found = document.querySelector(element);
                if (!found) {
                    throw new Error(`[AnyWP] Element not found: ${element}`);
                }
                el = found;
            }
            // Validate element
            if (!el || typeof el.getBoundingClientRect !== 'function') {
                console.error('[AnyWP] Invalid element passed to Bounds.calculate:', element);
                throw new TypeError('[AnyWP] Element must be a valid DOM element or selector');
            }
            const rect = el.getBoundingClientRect();
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
        isInBounds(x, y, bounds) {
            return x >= bounds.left && x <= bounds.right &&
                y >= bounds.top && y <= bounds.bottom;
        },
        /**
         * Check if mouse (in physical pixels) is over element
         */
        isMouseOverElement(mouseX, mouseY, element, dpiScale) {
            // Handle string selector
            let el = element;
            if (typeof element === 'string') {
                const found = document.querySelector(element);
                if (!found) {
                    return false;
                }
                el = found;
            }
            // Validate element
            if (!el || typeof el.getBoundingClientRect !== 'function') {
                console.error('[AnyWP] Invalid element passed to isMouseOverElement:', element);
                return false;
            }
            const rect = el.getBoundingClientRect();
            const physicalLeft = Math.round(rect.left * dpiScale);
            const physicalTop = Math.round(rect.top * dpiScale);
            const physicalRight = Math.round(rect.right * dpiScale);
            const physicalBottom = Math.round(rect.bottom * dpiScale);
            return mouseX >= physicalLeft && mouseX <= physicalRight &&
                mouseY >= physicalTop && mouseY <= physicalBottom;
        }
    };

    // Click handler module
    const ClickHandler = {
        // Handle click event from native
        handleClick(x, y) {
            Debug.log('Click at: (' + x + ',' + y + ')');
            const anyWP = window.AnyWP;
            for (let i = 0; i < anyWP._clickHandlers.length; i++) {
                const handler = anyWP._clickHandlers[i];
                if (handler && Bounds.isInBounds(x, y, handler.bounds)) {
                    Debug.log('HIT: ' + (handler.element.id || handler.element.className));
                    handler.callback(x, y);
                    break;
                }
            }
        },
        // Wait for element to appear in DOM
        waitForElement(selector, callback, maxWait = 10000) {
            const startTime = Date.now();
            function check() {
                const element = document.querySelector(selector);
                if (element) {
                    callback(element);
                }
                else if (Date.now() - startTime < maxWait) {
                    setTimeout(check, 100);
                }
                else {
                    console.error('[AnyWP] Element not found: ' + selector);
                }
            }
            check();
        },
        // Register click handler with SPA support
        onClick(anyWP, element, callback, options = {}) {
            // Validate parameters
            if (typeof element === 'function') {
                console.error('[AnyWP] onClick: First parameter should be element, not callback!');
                console.error('[AnyWP] Correct usage: AnyWP.onClick(element, callback, options)');
                console.error('[AnyWP] Example: AnyWP.onClick("#my-button", (x, y) => { ... })');
                throw new TypeError('[AnyWP] onClick: Invalid parameters - element must be HTMLElement or selector string');
            }
            if (typeof callback !== 'function') {
                console.error('[AnyWP] onClick: Second parameter must be a callback function!');
                console.error('[AnyWP] Correct usage: AnyWP.onClick(element, callback, options)');
                throw new TypeError('[AnyWP] onClick: callback must be a function');
            }
            const self = this;
            const immediate = options.immediate || false;
            const waitFor = options.waitFor !== undefined ? options.waitFor : !immediate;
            const maxWait = options.maxWait || 10000;
            const autoRefresh = options.autoRefresh !== undefined ? options.autoRefresh : anyWP._autoRefreshEnabled;
            const delay = options.delay || (immediate ? 0 : 100);
            function registerElement(el) {
                if (!el) {
                    console.error('[AnyWP] Element not found:', element);
                    return;
                }
                // Check if already registered (prevent duplicate handlers)
                const existingIndex = anyWP._clickHandlers.findIndex(function (h) {
                    return h.element === el;
                });
                if (existingIndex !== -1) {
                    const existingHandler = anyWP._clickHandlers[existingIndex];
                    if (existingHandler) {
                        console.warn('[AnyWP] Element already has click handler, skipping duplicate registration:', el.id || el.className);
                        console.warn('[AnyWP] Existing handler callback:', existingHandler.callback.toString().substring(0, 100));
                        console.warn('[AnyWP] New callback (ignored):', callback.toString().substring(0, 100));
                    }
                    return; // Skip duplicate registration instead of replacing
                }
                // Calculate bounds
                const bounds = Bounds.calculate(el, anyWP.dpiScale);
                // Register handler
                const handlerData = {
                    element: el,
                    callback: callback,
                    bounds: bounds,
                    selector: typeof element === 'string' ? element : null,
                    autoRefresh: autoRefresh,
                    options: options
                };
                anyWP._clickHandlers.push(handlerData);
                // ========== Auto-Refresh: 智能位置跟踪 ==========
                if (autoRefresh) {
                    // 方案 1: ResizeObserver - 监听元素自身大小变化（快速响应）
                    if (window.ResizeObserver) {
                        const resizeObserver = new ResizeObserver(function () {
                            self.refreshElementBounds(anyWP, handlerData);
                        });
                        resizeObserver.observe(el);
                        handlerData.resizeObserver = resizeObserver;
                    }
                    // 方案 2: IntersectionObserver - 监听元素位置/可见性变化（高效）
                    if (window.IntersectionObserver) {
                        const intersectionObserver = new IntersectionObserver(function (entries) {
                            entries.forEach(function (entry) {
                                // 元素位置变化或可见性变化时触发
                                if (entry.isIntersecting || entry.intersectionRatio > 0) {
                                    self.refreshElementBounds(anyWP, handlerData);
                                }
                            });
                        }, {
                            threshold: [0, 0.1, 0.5, 0.9, 1.0] // 多个阈值，提高灵敏度
                        });
                        intersectionObserver.observe(el);
                        handlerData.intersectionObserver = intersectionObserver;
                    }
                    // 方案 3: 定期检查位置变化（兜底方案，适用于复杂布局）
                    if (!handlerData.positionCheckTimer) {
                        handlerData.lastBounds = bounds;
                        handlerData.positionCheckTimer = setInterval(function () {
                            if (!el.isConnected) {
                                clearInterval(handlerData.positionCheckTimer);
                                return;
                            }
                            // 检查位置是否变化
                            const rect = el.getBoundingClientRect();
                            const currentLeft = Math.round(rect.left * anyWP.dpiScale);
                            const currentTop = Math.round(rect.top * anyWP.dpiScale);
                            if (handlerData.lastBounds &&
                                (currentLeft !== handlerData.lastBounds.left ||
                                    currentTop !== handlerData.lastBounds.top)) {
                                // 位置变化，刷新
                                self.refreshElementBounds(anyWP, handlerData);
                                Debug.log('Position changed for: ' + (el.id || el.className), false);
                            }
                        }, 500); // 每 500ms 检查一次，性能开销低
                    }
                }
                // Debug output
                const showDebug = (options.debug !== undefined) ? options.debug : anyWP._debugMode;
                if (showDebug) {
                    console.log('[AnyWP] Click Handler Registered:', el.id || el.className);
                    console.log('  Physical: [' + bounds.left + ',' + bounds.top + '] ~ [' +
                        bounds.right + ',' + bounds.bottom + ']');
                    console.log('  Size: ' + bounds.width + 'x' + bounds.height);
                    Debug.showBorder(bounds, el, anyWP.dpiScale);
                }
            }
            // Execute registration
            if (immediate) {
                let el;
                if (typeof element === 'string') {
                    el = document.querySelector(element);
                }
                else {
                    el = element;
                }
                registerElement(el);
            }
            else if (waitFor && typeof element === 'string') {
                this.waitForElement(element, registerElement, maxWait);
            }
            else {
                setTimeout(function () {
                    let el;
                    if (typeof element === 'string') {
                        el = document.querySelector(element);
                    }
                    else {
                        el = element;
                    }
                    registerElement(el);
                }, delay);
            }
        },
        // Refresh bounds for a specific handler
        refreshElementBounds(anyWP, handler) {
            if (!handler.element || !handler.element.isConnected) {
                return;
            }
            const newBounds = Bounds.calculate(handler.element, anyWP.dpiScale);
            const changed = newBounds.left !== handler.bounds.left ||
                newBounds.top !== handler.bounds.top ||
                newBounds.width !== handler.bounds.width ||
                newBounds.height !== handler.bounds.height;
            if (changed) {
                handler.bounds = newBounds;
                handler.lastBounds = newBounds; // 更新最后已知位置
                if (handler.element._anywpDebugBorder) {
                    Debug.showBorder(newBounds, handler.element, anyWP.dpiScale);
                }
            }
        },
        // Refresh all registered click handlers' bounds
        refreshBounds(anyWP) {
            const self = this;
            let refreshed = 0;
            anyWP._clickHandlers.forEach(function (handler) {
                if (handler.element && handler.element.isConnected) {
                    self.refreshElementBounds(anyWP, handler);
                    refreshed++;
                }
            });
            Debug.log('Refreshed ' + refreshed + ' elements', true);
            return refreshed;
        },
        // Clear all registered handlers
        clearHandlers(anyWP) {
            anyWP._clickHandlers.forEach(function (handler) {
                // 清理 ResizeObserver
                if (handler.resizeObserver) {
                    handler.resizeObserver.disconnect();
                }
                // 清理 IntersectionObserver
                if (handler.intersectionObserver) {
                    handler.intersectionObserver.disconnect();
                }
                // 清理定时器
                if (handler.positionCheckTimer) {
                    clearInterval(handler.positionCheckTimer);
                }
                // 清理调试边框
                if (handler.element && handler.element._anywpDebugBorder) {
                    const border = handler.element._anywpDebugBorder;
                    if (border.parentNode) {
                        border.parentNode.removeChild(border);
                    }
                }
            });
            anyWP._clickHandlers = [];
            Debug.log('All handlers cleared', true);
        }
    };

    // Animation control module
    const Animations = {
        /**
         * Auto-pause all animations for power saving
         */
        pause(_anyWP) {
            if (window.__anyWP_animationsPaused) {
                return;
            }
            console.log('[AnyWP] Auto-pausing all animations for power saving...');
            window.__anyWP_animationsPaused = true;
            try {
                // 1. Pause all videos
                const videos = document.querySelectorAll('video');
                console.log('[AnyWP] Found ' + videos.length + ' video(s)');
                videos.forEach((video) => {
                    try {
                        if (!video.paused) {
                            video.__anyWP_wasPlaying = true;
                            video.pause();
                        }
                    }
                    catch (e) {
                        console.warn('[AnyWP] Failed to pause video:', e);
                    }
                });
                // 2. Pause all audio
                const audios = document.querySelectorAll('audio');
                console.log('[AnyWP] Found ' + audios.length + ' audio(s)');
                audios.forEach((audio) => {
                    try {
                        if (!audio.paused) {
                            audio.__anyWP_wasPlaying = true;
                            audio.pause();
                        }
                    }
                    catch (e) {
                        console.warn('[AnyWP] Failed to pause audio:', e);
                    }
                });
                // 3. Pause CSS animations
                let pauseStyle = document.getElementById('__anywp_pause_style');
                if (!pauseStyle) {
                    pauseStyle = document.createElement('style');
                    pauseStyle.id = '__anywp_pause_style';
                    pauseStyle.textContent = '* { animation-play-state: paused !important; } *::before, *::after { animation-play-state: paused !important; }';
                    if (document.head) {
                        document.head.appendChild(pauseStyle);
                        console.log('[AnyWP] CSS animations paused');
                    }
                }
                // 4. Intercept requestAnimationFrame
                if (!window.__anyWP_originalRAF) {
                    window.__anyWP_originalRAF = window.requestAnimationFrame;
                    window.__anyWP_originalCancelRAF = window.cancelAnimationFrame;
                    window.requestAnimationFrame = (_callback) => 0;
                    window.cancelAnimationFrame = (_id) => { };
                    console.log('[AnyWP] requestAnimationFrame disabled');
                }
                console.log('[AnyWP] Auto-pause complete');
            }
            catch (e) {
                console.error('[AnyWP] Error during auto-pause:', e);
            }
        },
        /**
         * Auto-resume all animations
         */
        resume(_anyWP) {
            if (!window.__anyWP_animationsPaused) {
                return;
            }
            console.log('[AnyWP] Auto-resuming all animations...');
            window.__anyWP_animationsPaused = false;
            try {
                // 1. Remove CSS pause style
                const pauseStyle = document.getElementById('__anywp_pause_style');
                if (pauseStyle) {
                    pauseStyle.remove();
                    console.log('[AnyWP] CSS animations resumed');
                }
                // 2. Restore requestAnimationFrame
                if (window.__anyWP_originalRAF) {
                    window.requestAnimationFrame = window.__anyWP_originalRAF;
                    window.cancelAnimationFrame = window.__anyWP_originalCancelRAF;
                    delete window.__anyWP_originalRAF;
                    delete window.__anyWP_originalCancelRAF;
                    console.log('[AnyWP] requestAnimationFrame restored');
                }
                // 3. Resume videos
                const videos = document.querySelectorAll('video');
                videos.forEach((video) => {
                    try {
                        if (video.__anyWP_wasPlaying) {
                            video.play().catch((e) => {
                                console.warn('[AnyWP] Failed to resume video:', e);
                            });
                            delete video.__anyWP_wasPlaying;
                        }
                    }
                    catch (e) {
                        console.warn('[AnyWP] Error resuming video:', e);
                    }
                });
                // 4. Resume audio
                const audios = document.querySelectorAll('audio');
                audios.forEach((audio) => {
                    try {
                        if (audio.__anyWP_wasPlaying) {
                            audio.play().catch((e) => {
                                console.warn('[AnyWP] Failed to resume audio:', e);
                            });
                            delete audio.__anyWP_wasPlaying;
                        }
                    }
                    catch (e) {
                        console.warn('[AnyWP] Error resuming audio:', e);
                    }
                });
                console.log('[AnyWP] Auto-resume complete');
            }
            catch (e) {
                console.error('[AnyWP] Error during auto-resume:', e);
            }
        }
    };

    // Initialization module
    function initializeAnyWP(anyWP) {
        console.log('========================================');
        console.log('AnyWP Engine v' + anyWP.version + ' (SPA Compatible)');
        console.log('========================================');
        console.log('Screen: ' + anyWP.screenWidth + 'x' + anyWP.screenHeight);
        console.log('DPI Scale: ' + anyWP.dpiScale + 'x');
        console.log('Interaction Enabled: ' + anyWP.interactionEnabled);
        console.log('========================================');
        Debug.detectFromURL();
        SPA.detect(anyWP, ClickHandler);
        Events.setup(anyWP, ClickHandler, Animations);
        // Enable debug mode automatically for testing
        anyWP._debugMode = true;
        console.log('[AnyWP] Debug mode ENABLED automatically');
    }

    // State persistence module
    const Storage = {
        /**
         * Load state from native storage
         */
        load(anyWP, key, callback) {
            console.log('[AnyWP] Loading state for key:', key);
            // Check local cache first
            if (anyWP._persistedState[key]) {
                console.log('[AnyWP] Found in cache:', anyWP._persistedState[key]);
                callback(anyWP._persistedState[key]);
                return;
            }
            // Request from native layer
            if (window.chrome?.webview) {
                console.log('[AnyWP] Requesting state from native layer...');
                const handler = (event) => {
                    if (event.detail && event.detail.type === 'stateLoaded' && event.detail.key === key) {
                        console.log('[AnyWP] Received stateLoaded event:', event.detail);
                        window.removeEventListener('AnyWP:stateLoaded', handler);
                        const value = event.detail.value ? JSON.parse(event.detail.value) : null;
                        anyWP._persistedState[key] = value;
                        console.log('[AnyWP] State loaded successfully:', value);
                        callback(value);
                    }
                };
                window.addEventListener('AnyWP:stateLoaded', handler);
                window.chrome.webview.postMessage({
                    type: 'loadState',
                    key: key
                });
                setTimeout(() => {
                    window.removeEventListener('AnyWP:stateLoaded', handler);
                    console.log('[AnyWP] loadState timeout for key:', key);
                    callback(null);
                }, 1000);
            }
            else {
                console.warn('[AnyWP] chrome.webview not available, using localStorage');
                try {
                    const stored = localStorage.getItem('AnyWP_' + key);
                    const value = stored ? JSON.parse(stored) : null;
                    anyWP._persistedState[key] = value;
                    console.log('[AnyWP] Loaded from localStorage:', value);
                    callback(value);
                }
                catch (e) {
                    console.error('[AnyWP] Failed to load state:', e);
                    callback(null);
                }
            }
        },
        /**
         * Save custom state
         */
        save(anyWP, key, value) {
            anyWP._persistedState[key] = value;
            if (window.chrome?.webview) {
                window.chrome.webview.postMessage({
                    type: 'saveState',
                    key: key,
                    value: JSON.stringify(value)
                });
                Debug.log('Saved state for ' + key);
            }
            else {
                try {
                    localStorage.setItem('AnyWP_' + key, JSON.stringify(value));
                    Debug.log('Saved state to localStorage for ' + key);
                }
                catch (e) {
                    console.warn('[AnyWP] Failed to save state:', e);
                }
            }
        },
        /**
         * Clear all saved state
         */
        clear(anyWP) {
            anyWP._persistedState = {};
            if (window.chrome?.webview) {
                window.chrome.webview.postMessage({
                    type: 'clearState'
                });
                Debug.log('Cleared all saved state');
            }
            else {
                try {
                    const keys = Object.keys(localStorage);
                    keys.forEach((key) => {
                        if (key.startsWith('AnyWP_')) {
                            localStorage.removeItem(key);
                        }
                    });
                    Debug.log('Cleared localStorage state');
                }
                catch (e) {
                    console.warn('[AnyWP] Failed to clear state:', e);
                }
            }
        },
        /**
         * Save element position
         */
        saveElementPosition(anyWP, key, x, y) {
            const position = { left: x, top: y };
            console.log('[AnyWP] Saving position for ' + key + ': ', position);
            anyWP._persistedState[key] = position;
            if (window.chrome?.webview) {
                const msg = {
                    type: 'saveState',
                    key: key,
                    value: JSON.stringify(position)
                };
                console.log('[AnyWP] Sending saveState message:', msg);
                window.chrome.webview.postMessage(msg);
                console.log('[AnyWP] Message sent successfully');
            }
            else {
                console.warn('[AnyWP] chrome.webview not available, using localStorage');
                try {
                    localStorage.setItem('AnyWP_' + key, JSON.stringify(position));
                    console.log('[AnyWP] Saved to localStorage for ' + key);
                }
                catch (e) {
                    console.error('[AnyWP] Failed to save state:', e);
                }
            }
        }
    };

    // Drag & Drop module
    const DragHandler = {
        // Make element draggable
        makeDraggable(anyWP, element, options = {}) {
            const persistKey = options.persistKey || null;
            const onDragStart = options.onDragStart || null;
            const onDrag = options.onDrag || null;
            const onDragEnd = options.onDragEnd || null;
            const bounds = options.bounds || null;
            let el;
            if (typeof element === 'string') {
                el = document.querySelector(element);
            }
            else {
                el = element;
            }
            if (!el) {
                console.error('[AnyWP] Element not found:', element);
                return;
            }
            // Load persisted position
            if (persistKey) {
                Storage.load(anyWP, persistKey, function (savedPos) {
                    if (savedPos && el) {
                        el.style.position = 'absolute';
                        el.style.left = savedPos.left + 'px';
                        el.style.top = savedPos.top + 'px';
                        console.log('[AnyWP] Restored position for ' + persistKey + ': ' + savedPos.left + ',' + savedPos.top);
                    }
                });
            }
            // Register draggable element
            const draggableData = {
                element: el,
                persistKey: persistKey,
                onDragStart: onDragStart,
                onDrag: onDrag,
                onDragEnd: onDragEnd,
                bounds: bounds
            };
            anyWP._draggableElements.push(draggableData);
            // Setup element styles
            if (window.getComputedStyle(el).position === 'static') {
                el.style.position = 'absolute';
            }
            el.style.cursor = 'move';
            el.style.userSelect = 'none';
            el.style.webkitUserSelect = 'none';
            el.style.mozUserSelect = 'none';
            el.style.msUserSelect = 'none';
            el.draggable = false;
            el.ondragstart = function (e) {
                e.preventDefault();
                return false;
            };
            el.onselectstart = function (e) {
                e.preventDefault();
                return false;
            };
            // Register global mouse handler
            function handleGlobalMouse(event) {
                if (!anyWP.interactionEnabled) {
                    Debug.log('[makeDraggable] Interaction disabled, ignoring event');
                    return;
                }
                const customEvent = event;
                const detail = customEvent.detail;
                const mouseX = detail.x;
                const mouseY = detail.y;
                const mouseType = detail.type;
                if (!el)
                    return;
                const rect = el.getBoundingClientRect();
                const dpi = anyWP.dpiScale;
                const windowLeft = (typeof window.screenX !== 'undefined') ? window.screenX : 0;
                const windowTop = (typeof window.screenY !== 'undefined') ? window.screenY : 0;
                const docRect = document.documentElement.getBoundingClientRect();
                const viewportMouseX = (mouseX / dpi) - windowLeft - docRect.left;
                const viewportMouseY = (mouseY / dpi) - windowTop - docRect.top;
                const elementLeft = rect.left;
                const elementTop = rect.top;
                const elementRight = rect.right;
                const elementBottom = rect.bottom;
                const isOver = viewportMouseX >= elementLeft && viewportMouseX <= elementRight &&
                    viewportMouseY >= elementTop && viewportMouseY <= elementBottom;
                if (mouseType === 'mousedown') {
                    const elementId = el.id || el.className || 'unknown';
                    console.log('[AnyWP] [makeDraggable] mousedown on ' + elementId + ' - Mouse (screen):', mouseX, mouseY, 'Mouse (viewport CSS):', viewportMouseX.toFixed(1), viewportMouseY.toFixed(1), 'Element (viewport CSS):', elementLeft.toFixed(1), elementTop.toFixed(1), elementRight.toFixed(1), elementBottom.toFixed(1), 'isOver:', isOver, 'dragState:', anyWP._dragState ? 'EXISTS' : 'NULL', 'Window offset:', windowLeft, windowTop, 'Doc offset:', docRect.left.toFixed(1), docRect.top.toFixed(1), 'DPI:', dpi);
                    const statusEl = document.getElementById('drag-status');
                    if (statusEl) {
                        if (isOver) {
                            statusEl.innerHTML = '📍 检测到点击在 ' + elementId + ' 上 - 准备拖拽';
                            statusEl.style.color = '#4CAF50';
                        }
                        else {
                            statusEl.innerHTML =
                                '📍 点击不在 ' + elementId + ' 上 (鼠标: ' + viewportMouseX.toFixed(0) + ',' + viewportMouseY.toFixed(0) +
                                    ' | 元素: ' + elementLeft.toFixed(0) + ',' + elementTop.toFixed(0) + '-' + elementRight.toFixed(0) + ',' + elementBottom.toFixed(0) + ')';
                            statusEl.style.color = '#ff9800';
                        }
                    }
                }
                if (mouseType !== 'mousemove') {
                    Debug.log('[makeDraggable] Mouse event: ' + mouseType + ' at screen (' + mouseX + ',' + mouseY + ') -> viewport CSS (' + viewportMouseX.toFixed(1) + ',' + viewportMouseY.toFixed(1) + ')');
                }
                if (mouseType === 'mousedown' && isOver && !anyWP._dragState) {
                    console.log('[AnyWP] [Drag] START - Mouse at viewport CSS (' + viewportMouseX.toFixed(1) + ',' + viewportMouseY.toFixed(1) + ')');
                    anyWP._dragState = {
                        element: el,
                        data: draggableData,
                        startX: viewportMouseX,
                        startY: viewportMouseY,
                        offsetX: viewportMouseX - elementLeft,
                        offsetY: viewportMouseY - elementTop,
                        initialLeft: elementLeft,
                        initialTop: elementTop,
                        windowLeft: windowLeft,
                        windowTop: windowTop,
                        docLeft: docRect.left,
                        docTop: docRect.top,
                        dpi: dpi
                    };
                    if (onDragStart) {
                        onDragStart({
                            x: elementLeft,
                            y: elementTop
                        });
                    }
                    Debug.log('Drag start at: ' + viewportMouseX.toFixed(1) + ',' + viewportMouseY.toFixed(1) + ' (element at ' + elementLeft.toFixed(1) + ',' + elementTop.toFixed(1) + ')', true);
                }
                else if (mouseType === 'mousemove' && anyWP._dragState && anyWP._dragState.element === el) {
                    const currentViewportX = (mouseX / anyWP._dragState.dpi) - anyWP._dragState.windowLeft - anyWP._dragState.docLeft;
                    const currentViewportY = (mouseY / anyWP._dragState.dpi) - anyWP._dragState.windowTop - anyWP._dragState.docTop;
                    let newLeft = currentViewportX - anyWP._dragState.offsetX;
                    let newTop = currentViewportY - anyWP._dragState.offsetY;
                    if (bounds) {
                        if (bounds.left !== undefined) {
                            newLeft = Math.max(bounds.left, newLeft);
                        }
                        if (bounds.top !== undefined) {
                            newTop = Math.max(bounds.top, newTop);
                        }
                        if (bounds.right !== undefined) {
                            newLeft = Math.min(bounds.right - el.offsetWidth, newLeft);
                        }
                        if (bounds.bottom !== undefined) {
                            newTop = Math.min(bounds.bottom - el.offsetHeight, newTop);
                        }
                    }
                    el.style.left = newLeft + 'px';
                    el.style.top = newTop + 'px';
                    if (onDrag) {
                        onDrag({
                            x: newLeft,
                            y: newTop,
                            deltaX: currentViewportX - anyWP._dragState.startX,
                            deltaY: currentViewportY - anyWP._dragState.startY
                        });
                    }
                }
                else if (mouseType === 'mouseup' && anyWP._dragState && anyWP._dragState.element === el) {
                    console.log('[AnyWP] [Drag] END');
                    const finalRect = el.getBoundingClientRect();
                    const finalPos = {
                        x: finalRect.left,
                        y: finalRect.top
                    };
                    if (persistKey) {
                        Storage.saveElementPosition(anyWP, persistKey, finalPos.x, finalPos.y);
                    }
                    if (onDragEnd) {
                        onDragEnd(finalPos);
                    }
                    Debug.log('Drag end at: ' + finalPos.x + ',' + finalPos.y, true);
                    anyWP._dragState = null;
                }
                else if (mouseType === 'mousedown' && !isOver) ;
                else if (mouseType === 'mousemove' && anyWP._dragState && anyWP._dragState.element !== el) {
                    console.log('[AnyWP] [makeDraggable] mousemove but wrong element');
                }
            }
            window.addEventListener('AnyWP:mouse', handleGlobalMouse);
            el.__anyWP_dragHandler = {
                handleGlobalMouse: handleGlobalMouse
            };
            Debug.log('Element made draggable (via mouse hook): ' + (el.id || el.className));
        },
        // Remove draggable functionality
        removeDraggable(anyWP, element) {
            let el;
            if (typeof element === 'string') {
                el = document.querySelector(element);
            }
            else {
                el = element;
            }
            if (!el) {
                console.error('[AnyWP] Element not found:', element);
                return;
            }
            if (el.__anyWP_dragHandler) {
                window.removeEventListener('AnyWP:mouse', el.__anyWP_dragHandler.handleGlobalMouse);
                delete el.__anyWP_dragHandler;
            }
            anyWP._draggableElements = anyWP._draggableElements.filter(function (d) {
                return d.element !== el;
            });
            el.style.cursor = '';
            el.style.userSelect = '';
            Debug.log('Removed draggable from element: ' + (el.id || el.className));
        },
        // Reset element position
        resetPosition(anyWP, element, position) {
            let el;
            if (typeof element === 'string') {
                el = document.querySelector(element);
            }
            else {
                el = element;
            }
            if (!el) {
                console.error('[AnyWP] Element not found:', element);
                return false;
            }
            const draggableData = anyWP._draggableElements.find(function (d) {
                return d.element === el;
            });
            if (position) {
                el.style.left = position.left + 'px';
                el.style.top = position.top + 'px';
            }
            else {
                el.style.left = '';
                el.style.top = '';
            }
            if (draggableData && draggableData.persistKey) {
                const key = draggableData.persistKey;
                delete anyWP._persistedState[key];
                if (window.chrome && window.chrome.webview) {
                    window.chrome.webview.postMessage({
                        type: 'saveState',
                        key: key,
                        value: position ? JSON.stringify(position) : ''
                    });
                }
                else {
                    try {
                        if (position) {
                            localStorage.setItem('AnyWP_' + key, JSON.stringify(position));
                        }
                        else {
                            localStorage.removeItem('AnyWP_' + key);
                        }
                    }
                    catch (e) {
                        console.warn('[AnyWP] Failed to reset position:', e);
                    }
                }
                console.log('[AnyWP] Reset position for ' + key);
            }
            return true;
        }
    };

    // AnyWP Engine SDK - Main Entry Point
    // Modular architecture with TypeScript support
    // Implement initialization
    AnyWP._init = function () {
        initializeAnyWP(this);
    };
    // Public API: Debug
    AnyWP.enableDebug = function () {
        Debug.enable();
    };
    AnyWP._log = function (message, always) {
        Debug.log(message, always);
    };
    // Public API: Debug
    AnyWP.log = function (message) {
        Debug.log(message, true);
        // Also send to native if available
        if (window.chrome && window.chrome.webview) {
            window.chrome.webview.postMessage({
                type: 'log',
                message: message
            });
        }
    };
    // Public API: Click Handler
    AnyWP.onClick = function (element, callback, options) {
        ClickHandler.onClick(this, element, callback, options);
    };
    AnyWP.refreshBounds = function () {
        return ClickHandler.refreshBounds(this);
    };
    AnyWP.clearHandlers = function () {
        ClickHandler.clearHandlers(this);
    };
    // Public API: Drag Handler
    AnyWP.makeDraggable = function (element, options) {
        DragHandler.makeDraggable(this, element, options);
    };
    AnyWP.removeDraggable = function (element) {
        DragHandler.removeDraggable(this, element);
    };
    AnyWP.resetPosition = function (element, position) {
        return DragHandler.resetPosition(this, element, position ? { left: position.x, top: position.y } : undefined);
    };
    // Public API: Storage
    AnyWP.saveState = function (key, value) {
        Storage.save(this, key, value);
    };
    AnyWP.loadState = function (key, callback) {
        Storage.load(this, key, callback);
    };
    AnyWP.clearState = function () {
        Storage.clear(this);
    };
    // Public API: Events
    AnyWP.onMouse = function (callback) {
        Events.onMouse(this, callback);
    };
    AnyWP.onKeyboard = function (callback) {
        Events.onKeyboard(this, callback);
    };
    AnyWP.onVisibilityChange = function (callback) {
        Events.onVisibilityChange(this, callback);
    };
    AnyWP._notifyVisibilityChange = function (visible) {
        Events.notifyVisibilityChange(this, visible);
    };
    // Public API: SPA
    AnyWP.setSPAMode = function (enabled) {
        SPA.setSPAMode(this, ClickHandler, enabled);
    };
    // Public API: Utility
    AnyWP.openURL = function (url) {
        Debug.log('Opening URL: ' + url);
        if (window.chrome && window.chrome.webview) {
            window.chrome.webview.postMessage({
                type: 'openURL',
                url: url
            });
        }
        else {
            console.warn('[AnyWP] Native bridge not available');
            window.open(url, '_blank');
        }
    };
    AnyWP.ready = function (name) {
        Debug.log('Wallpaper ready: ' + name, true);
        if (window.chrome && window.chrome.webview) {
            window.chrome.webview.postMessage({
                type: 'ready',
                name: name
            });
        }
    };
    // Auto-initialize when DOM is ready
    if (typeof window !== 'undefined') {
        // ========== CRITICAL: Prevent Duplicate SDK Initialization ==========
        // Check if SDK is already loaded (防止重复注入)
        if (typeof window.AnyWP !== 'undefined') {
            console.log('[AnyWP] SDK already loaded, skipping re-initialization');
            console.log('[AnyWP] This is expected when C++ plugin injects SDK multiple times');
        }
        else {
            console.log('[AnyWP] Initializing SDK for the first time');
            window.AnyWP = AnyWP;
            if (document.readyState === 'loading') {
                document.addEventListener('DOMContentLoaded', function () {
                    AnyWP._init();
                });
            }
            else {
                AnyWP._init();
            }
            console.log('[AnyWP] SDK loaded successfully');
        }
    }

    exports.AnyWP = AnyWP;

    return exports;

})({});
// Built from modular source - see windows/sdk/ for source code
