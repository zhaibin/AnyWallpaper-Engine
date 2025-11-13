// AnyWP Engine SDK v2.0.0 - JavaScript Bridge
// Auto-injected into WebView2
// React/Vue SPA Compatible
// Built with TypeScript modular architecture (100% TS)

var AnyWPBundle = (function (exports) {
    'use strict';

    const AnyWP = {
        // Properties
        version: '2.1.1',
        dpiScale: window.devicePixelRatio || 1,
        screenWidth: screen.width * (window.devicePixelRatio || 1),
        screenHeight: screen.height * (window.devicePixelRatio || 1),
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
        _persistedState: {},
        _onFlutterMessage: null,
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
        },
        // ========================================
        // Bidirectional Communication APIs
        // ========================================
        /**
         * Send message to Flutter application
         * (Will be implemented in webmessage module)
         */
        sendToFlutter(_type, _data) {
            throw new Error('Not implemented - will be injected by webmessage module');
        },
        /**
         * Register callback to receive messages from Flutter
         *
         * @param callback - Function to handle messages from Flutter
         *
         * @example
         * ```typescript
         * window.AnyWP.onMessage((message) => {
         *   console.log('Received from Flutter:', message.type);
         *
         *   if (message.type === 'updateCarousel') {
         *     // Update carousel with message.data
         *   }
         * });
         * ```
         */
        onMessage(callback) {
            console.log('[AnyWP] Registering Flutter message handler');
            this._onFlutterMessage = callback;
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
                    // 方案 3: 高频位置检查（兜底方案，适用于快速移动/动画）
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
                                // 位置变化，立即刷新
                                self.refreshElementBounds(anyWP, handlerData);
                                Debug.log('Position changed for: ' + (el.id || el.className), false);
                            }
                        }, 100); // 每 100ms 检查一次，快速响应
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

    // Wallpaper Controller Module - v2.0.1+
    // Handles desktop wallpaper embedding, mouse interaction, and mouseover recovery
    let lastMouseX = 0;
    let lastMouseY = 0;
    let isInteractiveMode = false;
    /**
     * Force trigger mouseover event at current mouse position
     * This is critical after restoring WS_EX_TRANSPARENT when mouse is stationary
     */
    function forceMouseOverEvent(x, y) {
        const AnyWP = window.AnyWP;
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
        }
        catch (e) {
            AnyWP._log('[Wallpaper] EXCEPTION in forceMouseOverEvent: ' + e, true);
        }
    }
    /**
     * Set interactive mode via C++ bridge
     */
    function setInteractive(interactive) {
        const AnyWP = window.AnyWP;
        try {
            AnyWP._log('[Wallpaper] setInteractive(' + interactive + ') called', true);
            if (window.chrome?.webview) {
                AnyWP._log('[Wallpaper] Sending setInteractive message to C++...', true);
                window.chrome.webview.postMessage({
                    type: 'setInteractive',
                    interactive: interactive
                });
                isInteractiveMode = interactive;
                AnyWP._log('[Wallpaper] [OK] Interactive mode: ' + (interactive ? 'ENABLED' : 'DISABLED'), true);
            }
            else {
                AnyWP._log('[Wallpaper] ERROR: chrome.webview not available!', true);
                AnyWP._log('[Wallpaper] window.chrome = ' + (typeof window.chrome), true);
                AnyWP._log('[Wallpaper] window.chrome.webview = ' + (typeof window.chrome?.webview), true);
            }
        }
        catch (e) {
            AnyWP._log('[Wallpaper] EXCEPTION in setInteractive: ' + e, true);
        }
    }
    /**
     * Wait for next animation frame
     */
    function nextFrame() {
        return new Promise((resolve) => {
            requestAnimationFrame(() => {
                resolve();
            });
        });
    }
    /**
     * Handle mousedown event
     */
    function handleMouseDown(event) {
        const mouseEvent = event;
        const AnyWP = window.AnyWP;
        lastMouseX = mouseEvent.clientX;
        lastMouseY = mouseEvent.clientY;
        AnyWP._log('[Wallpaper] mousedown at (' + lastMouseX + ',' + lastMouseY + ')', true);
    }
    /**
     * Handle mouseup event
     */
    function handleMouseUp(event) {
        const mouseEvent = event;
        const AnyWP = window.AnyWP;
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
    async function restoreTransparency() {
        const AnyWP = window.AnyWP;
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
        }
        catch (e) {
            AnyWP._log('[Wallpaper] EXCEPTION in restoreTransparency: ' + e, true);
        }
    }
    /**
     * Handle mousemove event (track position)
     */
    function handleMouseMove$1(event) {
        const mouseEvent = event;
        lastMouseX = mouseEvent.clientX;
        lastMouseY = mouseEvent.clientY;
    }
    /**
     * Initialize wallpaper controller
     */
    function initWallpaperController(sdk) {
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
            document.addEventListener('mousemove', handleMouseMove$1, {
                capture: true,
                passive: true
            });
            sdk._log('[Wallpaper] Wallpaper controller initialized successfully', true);
            // Expose API for manual control
            window.AnyWP.Wallpaper = {
                setInteractive,
                forceMouseOverEvent,
                getState: () => ({
                    isInteractive: isInteractiveMode,
                    lastMouseX,
                    lastMouseY
                })
            };
        }
        catch (e) {
            sdk._log('[Wallpaper] ERROR initializing controller: ' + e, true);
        }
    }

    /**
     * Logger utility with configurable log levels
     *
     * Provides structured logging with different severity levels.
     * Useful for reducing console spam in production while maintaining
     * detailed logs in development.
     */
    var LogLevel;
    (function (LogLevel) {
        LogLevel[LogLevel["ERROR"] = 0] = "ERROR";
        LogLevel[LogLevel["WARN"] = 1] = "WARN";
        LogLevel[LogLevel["INFO"] = 2] = "INFO";
        LogLevel[LogLevel["DEBUG"] = 3] = "DEBUG"; // Verbose debugging (everything)
    })(LogLevel || (LogLevel = {}));
    /**
     * Global logger instance
     */
    class Logger {
        constructor() {
            this.config = {
                level: LogLevel.INFO, // Default: INFO level
                prefix: '[AnyWP]',
                timestamp: false
            };
        }
        /**
         * Set log level
         */
        setLevel(level) {
            this.config.level = level;
            this.info('Log level set to:', LogLevel[level]);
        }
        /**
         * Get current log level
         */
        getLevel() {
            return this.config.level;
        }
        /**
         * Enable timestamp in logs
         */
        enableTimestamp(enabled = true) {
            this.config.timestamp = enabled;
        }
        /**
         * Format log message with prefix and timestamp
         */
        format(level, ...args) {
            const parts = [];
            // Add prefix
            parts.push(this.config.prefix);
            // Add timestamp if enabled
            if (this.config.timestamp) {
                const now = new Date();
                const timestamp = now.toISOString().substring(11, 23); // HH:MM:SS.mmm
                parts.push(`[${timestamp}]`);
            }
            // Add level
            parts.push(`[${level}]`);
            // Add original message
            parts.push(...args);
            return parts;
        }
        /**
         * Log error message (always shown)
         */
        error(...args) {
            if (this.config.level >= LogLevel.ERROR) {
                console.error(...this.format('ERROR', ...args));
            }
        }
        /**
         * Log warning message
         */
        warn(...args) {
            if (this.config.level >= LogLevel.WARN) {
                console.warn(...this.format('WARN', ...args));
            }
        }
        /**
         * Log informational message
         */
        info(...args) {
            if (this.config.level >= LogLevel.INFO) {
                console.log(...this.format('INFO', ...args));
            }
        }
        /**
         * Log debug message (only in DEBUG mode)
         */
        debug(...args) {
            if (this.config.level >= LogLevel.DEBUG) {
                console.log(...this.format('DEBUG', ...args));
            }
        }
        /**
         * Create a scoped logger with custom prefix
         */
        scope(scopeName) {
            return new ScopedLogger(this, scopeName);
        }
    }
    /**
     * Scoped logger for module-specific logging
     */
    class ScopedLogger {
        constructor(parent, scopeName) {
            this.parent = parent;
            this.scopeName = scopeName;
        }
        addScope(...args) {
            return [`[${this.scopeName}]`, ...args];
        }
        error(...args) {
            this.parent.error(...this.addScope(...args));
        }
        warn(...args) {
            this.parent.warn(...this.addScope(...args));
        }
        info(...args) {
            this.parent.info(...this.addScope(...args));
        }
        debug(...args) {
            this.parent.debug(...this.addScope(...args));
        }
    }
    /**
     * Global logger instance
     */
    const logger = new Logger();
    /**
     * Set log level from environment or URL parameter
     *
     * Examples:
     * - ?loglevel=DEBUG
     * - localStorage.setItem('anywp_loglevel', 'WARN')
     */
    function initLogger() {
        // Check URL parameter first
        if (typeof window !== 'undefined' && window.location) {
            const params = new URLSearchParams(window.location.search);
            const urlLevel = params.get('loglevel');
            if (urlLevel && urlLevel.toUpperCase() in LogLevel) {
                const level = LogLevel[urlLevel.toUpperCase()];
                logger.setLevel(level);
                return;
            }
        }
        // Check localStorage
        if (typeof window !== 'undefined' && window.localStorage) {
            try {
                const storedLevel = localStorage.getItem('anywp_loglevel');
                if (storedLevel && storedLevel.toUpperCase() in LogLevel) {
                    const level = LogLevel[storedLevel.toUpperCase()];
                    logger.setLevel(level);
                    return;
                }
            }
            catch (e) {
                // Ignore localStorage errors
            }
        }
        // Default: INFO for production, DEBUG for development
        const isDevelopment = typeof window !== 'undefined' &&
            (window.location.hostname === 'localhost' ||
                window.location.hostname === '127.0.0.1');
        logger.setLevel(isDevelopment ? LogLevel.DEBUG : LogLevel.INFO);
    }

    const Coordinates = {
        /**
         * Convert screen coordinates (physical pixels) to viewport CSS coordinates
         */
        screenToViewport(mouseX, mouseY, dpiScale) {
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
         * Get window and document offsets
         */
        getOffsets(dpiScale) {
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

    /**
     * Throttle utility for performance optimization
     *
     * Throttle ensures a function is called at most once per specified time interval.
     * This is useful for high-frequency events like mousemove, scroll, resize.
     */
    /**
     * Creates a throttled version of the provided function
     *
     * @param func - Function to throttle
     * @param delay - Minimum time between function calls (ms)
     * @returns Throttled function
     */
    function throttle(func, delay) {
        let lastCall = 0;
        let timeoutId = null;
        return function (...args) {
            const now = Date.now();
            const timeSinceLastCall = now - lastCall;
            // If enough time has passed, call immediately
            if (timeSinceLastCall >= delay) {
                lastCall = now;
                func.apply(this, args);
            }
            else {
                // Otherwise, schedule for later (trailing edge)
                if (timeoutId !== null) {
                    clearTimeout(timeoutId);
                }
                timeoutId = window.setTimeout(() => {
                    lastCall = Date.now();
                    func.apply(this, args);
                    timeoutId = null;
                }, delay - timeSinceLastCall);
            }
        };
    }

    /**
     * WebMessage Types
     *
     * Type definitions for C++ <-> JavaScript communication via chrome.webview
     */
    /**
     * Type guard to check if data is MouseEventData
     */
    function isMouseEventData(data) {
        return data.type === 'mouseEvent';
    }

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
    // Create scoped logger for WebMessage module
    const log$1 = logger.scope('WebMessage');
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
    function setupWebMessageListener() {
        if (!window.chrome || !window.chrome.webview) {
            log$1.info('chrome.webview not available');
            return;
        }
        const globalAny = window;
        // Prevent duplicate listener registration
        if (globalAny._anywpEarlyMessageListenerRegistered) {
            log$1.warn('WebMessage listener already registered (EARLY), skipping duplicate');
            return;
        }
        log$1.info('Setting up WebMessage listener (EARLY)');
        globalAny._anywpEarlyMessageListenerRegistered = true;
        window.chrome.webview.addEventListener('message', handleWebMessage);
        log$1.info('WebMessage listener setup complete (EARLY)');
    }
    /**
     * Main WebMessage event handler
     */
    function handleWebMessage(event) {
        const data = event.data;
        if (!data) {
            log$1.warn('Received empty WebMessage');
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
            log$1.error('Error handling WebMessage:', error);
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
                log$1.debug('WebMessage received #' + window._anywp_msg_count + ': ' + data.eventType);
            }
        }
        else if (data.type === 'mouseEvent') {
            log$1.debug('WebMessage: ' + data.eventType + ' at (' + data.x + ',' + data.y + ')');
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
            log$1.debug('[CoordinateConversion] Screen (' + data.x + ',' + data.y + ') -> Viewport (' +
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
            log$1.error('Error handling mouse event:', e);
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
            log$1.debug('[DOMDispatch] mousemove #' + window._mousemove_count +
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
            log$1.debug('[ElementCache] Updated interactive elements cache:', candidates.length, 'elements');
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
            log$1.debug('[ElementFinder] Searching at viewport coordinates:', x, y);
            log$1.debug('[ElementFinder] Window size:', window.innerWidth, 'x', window.innerHeight);
            // Use cached interactive elements
            const candidates = getInteractiveElements();
            log$1.debug('[ElementFinder] Found', candidates.length, 'interactive elements (cached)');
            // Find the topmost element at the given coordinates
            let found = null;
            let maxZIndex = -Infinity;
            for (const elem of candidates) {
                // Use cached bounds
                const rect = getElementBounds(elem);
                const isInside = x >= rect.left && x <= rect.right && y >= rect.top && y <= rect.bottom;
                if (isInside) {
                    log$1.debug('[ElementFinder] HIT:', elem.tagName, elem.id || elem.className, 'rect:', Math.round(rect.left), Math.round(rect.top), Math.round(rect.right), Math.round(rect.bottom));
                    const style = window.getComputedStyle(elem);
                    const zIndex = style.zIndex === 'auto' ? 0 : parseInt(style.zIndex);
                    log$1.debug('[ElementFinder]   zIndex:', zIndex);
                    if (zIndex >= maxZIndex) {
                        maxZIndex = zIndex;
                        found = elem;
                    }
                }
            }
            if (found) {
                log$1.debug('[ElementFinder] Selected element:', found.tagName, found.id || found.className, 'zIndex:', maxZIndex);
            }
            else {
                log$1.debug('[ElementFinder] No interactive element found at coordinates');
            }
            return found;
        };
        targetElement = findElementAtPoint(viewportPos.x, viewportPos.y);
        // Create the MouseEvent
        const mouseEvent = new MouseEvent(data.eventType, eventInit);
        // Dispatch to the target element or document
        if (targetElement) {
            log$1.debug('[DOMDispatch] Dispatching', data.eventType, 'to element:', targetElement.tagName, targetElement.id || targetElement.className);
            targetElement.dispatchEvent(mouseEvent);
        }
        else {
            log$1.debug('[DOMDispatch] Dispatching', data.eventType, 'to document (no element found)');
            document.dispatchEvent(mouseEvent);
        }
        // Log event details
        if (data.eventType === 'mousedown') {
            log$1.info('[DOMDispatch] mousedown dispatched to', targetElement ? (targetElement.tagName + '#' + (targetElement.id || '?')) : 'document');
        }
        else if (data.eventType === 'mouseup') {
            log$1.info('[DOMDispatch] mouseup dispatched to', targetElement ? (targetElement.tagName + '#' + (targetElement.id || '?')) : 'document');
        }
        else if (data.eventType === 'click') {
            log$1.info('[DOMDispatch] click dispatched to', targetElement ? (targetElement.tagName + '#' + (targetElement.id || '?')) : 'document');
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
    function sendToFlutter(type, data = {}) {
        if (!window.chrome?.webview) {
            log$1.warn('chrome.webview not available, cannot send message to Flutter');
            return false;
        }
        const message = {
            type: type,
            timestamp: Date.now(),
            data: data
        };
        log$1.info('[SendToFlutter] Sending message:', type);
        log$1.debug('[SendToFlutter] Message data:', message);
        try {
            window.chrome.webview.postMessage(message);
            return true;
        }
        catch (error) {
            log$1.error('[SendToFlutter] Error sending message:', error);
            return false;
        }
    }
    /**
     * Setup listener for messages from Flutter
     *
     * Listens for CustomEvent 'AnyWP:message' dispatched by Flutter
     * and allows user code to handle these messages
     */
    function setupFlutterMessageListener() {
        log$1.info('Setting up Flutter message listener');
        window.addEventListener('AnyWP:message', (event) => {
            const customEvent = event;
            const message = customEvent.detail;
            log$1.info('[FlutterMessage] Received message from Flutter:', message?.type || 'unknown');
            log$1.debug('[FlutterMessage] Message details:', message);
            // Dispatch to user-registered handler if available
            const AnyWP = window.AnyWP;
            if (AnyWP && AnyWP._onFlutterMessage) {
                try {
                    AnyWP._onFlutterMessage(message);
                }
                catch (error) {
                    log$1.error('[FlutterMessage] Error in user message handler:', error);
                }
            }
        });
        log$1.info('Flutter message listener setup complete');
    }
    /**
     * WebMessage Module
     */
    const WebMessage = {
        setup: setupWebMessageListener,
        setupFlutterListener: setupFlutterMessageListener,
        sendToFlutter: sendToFlutter
    };

    // Initialization module
    function initializeAnyWP(anyWP) {
        // Initialize logger first
        initLogger();
        logger.info('========================================');
        logger.info('AnyWP Engine v' + anyWP.version + ' (Simple Mode)');
        logger.info('========================================');
        logger.info('Screen: ' + anyWP.screenWidth + 'x' + anyWP.screenHeight);
        logger.info('DPI Scale: ' + anyWP.dpiScale + 'x');
        logger.info('========================================');
        Debug.detectFromURL();
        SPA.detect(anyWP, ClickHandler);
        Events.setup(anyWP, ClickHandler, Animations);
        // Initialize wallpaper controller (v2.0.1+)
        initWallpaperController(anyWP);
        // Note: WebMessage listener is now setup in index.ts (EARLY) before any initialization
        // This ensures we catch all messages from C++ immediately when SDK is loaded
        // v2.1.0+ Setup Flutter message listener for bidirectional communication
        setupFlutterMessageListener();
        // v2.1.0+ Inject sendToFlutter method to AnyWP object
        anyWP.sendToFlutter = sendToFlutter;
        logger.info('Bidirectional communication enabled (Flutter ↔ JavaScript)');
        // Enable debug mode automatically for testing
        anyWP._debugMode = true;
        logger.info('Debug mode ENABLED automatically');
    }

    // State persistence module
    const log = logger.scope('Storage');
    const Storage = {
        /**
         * Load state from native storage
         */
        load(anyWP, key, callback) {
            log.debug('Loading state for key:', key);
            // Check local cache first
            if (anyWP._persistedState[key]) {
                log.debug('Found in cache:', anyWP._persistedState[key]);
                callback(anyWP._persistedState[key]);
                return;
            }
            // Request from native layer
            if (window.chrome?.webview) {
                log.debug('Requesting state from native layer...');
                let resolved = false;
                let timeoutId;
                const handler = (event) => {
                    if (event.detail && event.detail.type === 'stateLoaded' && event.detail.key === key) {
                        log.debug('Received stateLoaded event:', event.detail);
                        window.removeEventListener('AnyWP:stateLoaded', handler);
                        if (timeoutId !== undefined) {
                            clearTimeout(timeoutId);
                        }
                        resolved = true;
                        const value = event.detail.value ? JSON.parse(event.detail.value) : null;
                        anyWP._persistedState[key] = value;
                        log.info('State loaded successfully:', value);
                        callback(value);
                    }
                };
                window.addEventListener('AnyWP:stateLoaded', handler);
                window.chrome.webview.postMessage({
                    type: 'loadState',
                    key: key
                });
                timeoutId = window.setTimeout(() => {
                    if (!resolved) {
                        window.removeEventListener('AnyWP:stateLoaded', handler);
                        log.warn('loadState timeout for key:', key);
                        callback(null);
                    }
                }, 1000);
            }
            else {
                log.warn('chrome.webview not available, using localStorage');
                try {
                    const stored = localStorage.getItem('AnyWP_' + key);
                    const value = stored ? JSON.parse(stored) : null;
                    anyWP._persistedState[key] = value;
                    log.info('Loaded from localStorage:', value);
                    callback(value);
                }
                catch (e) {
                    log.error('Failed to load state:', e);
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
            log.debug('Saving position for ' + key + ': ', position);
            anyWP._persistedState[key] = position;
            if (window.chrome?.webview) {
                const msg = {
                    type: 'saveState',
                    key: key,
                    value: JSON.stringify(position)
                };
                log.debug('Sending saveState message:', msg);
                window.chrome.webview.postMessage(msg);
                log.debug('Message sent successfully');
            }
            else {
                log.warn('chrome.webview not available, using localStorage');
                try {
                    localStorage.setItem('AnyWP_' + key, JSON.stringify(position));
                    log.info('Saved to localStorage for ' + key);
                }
                catch (e) {
                    log.error('Failed to save state:', e);
                }
            }
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
    // Auto-initialize when DOM is ready
    if (typeof window !== 'undefined') {
        // ========== CRITICAL: Setup WebMessage listener IMMEDIATELY ==========
        // This must be done BEFORE any other initialization to catch all messages from C++
        WebMessage.setup();
        // ========== CRITICAL: Prevent Duplicate SDK Initialization ==========
        // Check if SDK is already loaded (防止重复注入)
        if (typeof window.AnyWP !== 'undefined') {
            console.info('[AnyWP] SDK already loaded, skipping re-initialization');
            console.info('[AnyWP] This is expected when C++ plugin injects SDK multiple times');
        }
        else {
            console.info('[AnyWP] Initializing SDK for the first time');
            window.AnyWP = AnyWP;
            if (document.readyState === 'loading') {
                document.addEventListener('DOMContentLoaded', function () {
                    AnyWP._init();
                });
            }
            else {
                AnyWP._init();
            }
            console.info('[AnyWP] SDK loaded successfully');
        }
    }

    exports.AnyWP = AnyWP;

    return exports;

})({});
// Built from modular source - see windows/sdk/ for source code
