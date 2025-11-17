/**
 * AnyWP SDK for macOS
 * Version: 2.2.0
 * 
 * Provides JavaScript API for interacting with AnyWP Engine on macOS
 * Uses WKWebView's webkit.messageHandlers for communication
 */

(function() {
    'use strict';

    // Check if already initialized
    if (window.AnyWP) {
        console.warn('[AnyWP SDK] Already initialized');
        return;
    }

    /**
     * Main AnyWP namespace
     */
    const AnyWP = {
        version: '2.2.0',
        platform: 'macOS',

        /**
         * Send message to Flutter application
         * @param {Object|string} message - Message to send (will be JSON-encoded if object)
         */
        sendMessage: function(message) {
            if (!window.webkit || !window.webkit.messageHandlers || !window.webkit.messageHandlers.anywpMessage) {
                console.error('[AnyWP SDK] Message handlers not available');
                return false;
            }

            try {
                const messageStr = typeof message === 'string' ? message : JSON.stringify(message);
                window.webkit.messageHandlers.anywpMessage.postMessage(messageStr);
                return true;
            } catch (e) {
                console.error('[AnyWP SDK] Failed to send message:', e);
                return false;
            }
        },

        /**
         * Send structured message with metadata
         * @param {string} type - Message type
         * @param {Object} data - Message data
         */
        sendStructuredMessage: function(type, data) {
            const message = {
                id: this._generateId(),
                type: type,
                timestamp: Date.now(),
                data: data || {}
            };
            return this.sendMessage(message);
        },

        /**
         * Get monitor information
         * @returns {Object} Monitor info (width, height, etc.)
         */
        getMonitorInfo: function() {
            return {
                width: screen.width,
                height: screen.height,
                availWidth: screen.availWidth,
                availHeight: screen.availHeight,
                colorDepth: screen.colorDepth,
                pixelDepth: screen.pixelDepth,
                orientation: screen.orientation ? screen.orientation.type : 'unknown'
            };
        },

        /**
         * Log message (will be sent to Flutter)
         * @param {string} level - Log level (info, warn, error, debug)
         * @param {string} message - Log message
         */
        log: function(level, message) {
            console.log(`[AnyWP ${level.toUpperCase()}]`, message);
            this.sendStructuredMessage('log', {
                level: level,
                message: message
            });
        },

        /**
         * Notify wallpaper ready
         */
        ready: function() {
            this.sendStructuredMessage('ready', {
                sdk_version: this.version,
                platform: this.platform
            });
        },

        /**
         * Save state to persistent storage
         * @param {string} key - State key
         * @param {string|Object} value - State value
         */
        saveState: function(key, value) {
            const valueStr = typeof value === 'string' ? value : JSON.stringify(value);
            this.sendStructuredMessage('saveState', {
                key: key,
                value: valueStr
            });
        },

        /**
         * Load state from persistent storage
         * Note: This is async and result comes via message callback
         * @param {string} key - State key
         */
        loadState: function(key) {
            this.sendStructuredMessage('loadState', {
                key: key
            });
        },

        /**
         * Clear all saved state
         */
        clearState: function() {
            this.sendStructuredMessage('clearState', {});
        },

        /**
         * Generate unique message ID
         * @private
         */
        _generateId: function() {
            return 'msg_' + Date.now() + '_' + Math.random().toString(36).substr(2, 9);
        },

        /**
         * Utilities
         */
        utils: {
            /**
             * Detect if running in AnyWP Engine
             */
            isAnyWP: function() {
                return window.webkit && 
                       window.webkit.messageHandlers && 
                       window.webkit.messageHandlers.anywpMessage !== undefined;
            },

            /**
             * Get device pixel ratio
             */
            getPixelRatio: function() {
                return window.devicePixelRatio || 1;
            },

            /**
             * Throttle function execution
             */
            throttle: function(func, delay) {
                let lastCall = 0;
                return function(...args) {
                    const now = Date.now();
                    if (now - lastCall >= delay) {
                        lastCall = now;
                        return func(...args);
                    }
                };
            },

            /**
             * Debounce function execution
             */
            debounce: function(func, delay) {
                let timeoutId;
                return function(...args) {
                    clearTimeout(timeoutId);
                    timeoutId = setTimeout(() => func(...args), delay);
                };
            }
        }
    };

    // Expose AnyWP globally
    window.AnyWP = AnyWP;

    // Auto-notify ready when DOM is loaded
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', function() {
            AnyWP.ready();
        });
    } else {
        // DOM is already ready
        setTimeout(() => AnyWP.ready(), 0);
    }

    console.log('[AnyWP SDK] Initialized successfully (v' + AnyWP.version + ' - ' + AnyWP.platform + ')');
})();

