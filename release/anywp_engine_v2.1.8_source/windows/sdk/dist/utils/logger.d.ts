/**
 * Logger utility with configurable log levels
 *
 * Provides structured logging with different severity levels.
 * Useful for reducing console spam in production while maintaining
 * detailed logs in development.
 */
export declare enum LogLevel {
    ERROR = 0,// Critical errors only
    WARN = 1,// Warnings and errors
    INFO = 2,// Informational messages
    DEBUG = 3
}
/**
 * Global logger instance
 */
declare class Logger {
    private config;
    /**
     * Set log level
     */
    setLevel(level: LogLevel): void;
    /**
     * Get current log level
     */
    getLevel(): LogLevel;
    /**
     * Enable timestamp in logs
     */
    enableTimestamp(enabled?: boolean): void;
    /**
     * Format log message with prefix and timestamp
     */
    private format;
    /**
     * Log error message (always shown)
     */
    error(...args: any[]): void;
    /**
     * Log warning message
     */
    warn(...args: any[]): void;
    /**
     * Log informational message
     */
    info(...args: any[]): void;
    /**
     * Log debug message (only in DEBUG mode)
     */
    debug(...args: any[]): void;
    /**
     * Create a scoped logger with custom prefix
     */
    scope(scopeName: string): ScopedLogger;
}
/**
 * Scoped logger for module-specific logging
 */
declare class ScopedLogger {
    private parent;
    private scopeName;
    constructor(parent: Logger, scopeName: string);
    private addScope;
    error(...args: any[]): void;
    warn(...args: any[]): void;
    info(...args: any[]): void;
    debug(...args: any[]): void;
}
/**
 * Global logger instance
 */
export declare const logger: Logger;
/**
 * Set log level from environment or URL parameter
 *
 * Examples:
 * - ?loglevel=DEBUG
 * - localStorage.setItem('anywp_loglevel', 'WARN')
 */
export declare function initLogger(): void;
export {};
//# sourceMappingURL=logger.d.ts.map