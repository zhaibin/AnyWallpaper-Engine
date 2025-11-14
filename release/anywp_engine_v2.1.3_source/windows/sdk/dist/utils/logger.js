/**
 * Logger utility with configurable log levels
 *
 * Provides structured logging with different severity levels.
 * Useful for reducing console spam in production while maintaining
 * detailed logs in development.
 */
export var LogLevel;
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
export const logger = new Logger();
/**
 * Set log level from environment or URL parameter
 *
 * Examples:
 * - ?loglevel=DEBUG
 * - localStorage.setItem('anywp_loglevel', 'WARN')
 */
export function initLogger() {
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
//# sourceMappingURL=logger.js.map