/**
 * Logger utility with configurable log levels
 * 
 * Provides structured logging with different severity levels.
 * Useful for reducing console spam in production while maintaining
 * detailed logs in development.
 */

export enum LogLevel {
  ERROR = 0,   // Critical errors only
  WARN = 1,    // Warnings and errors
  INFO = 2,    // Informational messages
  DEBUG = 3    // Verbose debugging (everything)
}

/**
 * Logger configuration
 */
interface LoggerConfig {
  level: LogLevel;
  prefix: string;
  timestamp: boolean;
}

/**
 * Global logger instance
 */
class Logger {
  private config: LoggerConfig = {
    level: LogLevel.INFO, // Default: INFO level
    prefix: '[AnyWP]',
    timestamp: false
  };
  
  /**
   * Set log level
   */
  setLevel(level: LogLevel): void {
    this.config.level = level;
    this.info('Log level set to:', LogLevel[level]);
  }
  
  /**
   * Get current log level
   */
  getLevel(): LogLevel {
    return this.config.level;
  }
  
  /**
   * Enable timestamp in logs
   */
  enableTimestamp(enabled: boolean = true): void {
    this.config.timestamp = enabled;
  }
  
  /**
   * Format log message with prefix and timestamp
   */
  private format(level: string, ...args: any[]): any[] {
    const parts: any[] = [];
    
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
  error(...args: any[]): void {
    if (this.config.level >= LogLevel.ERROR) {
      console.error(...this.format('ERROR', ...args));
    }
  }
  
  /**
   * Log warning message
   */
  warn(...args: any[]): void {
    if (this.config.level >= LogLevel.WARN) {
      console.warn(...this.format('WARN', ...args));
    }
  }
  
  /**
   * Log informational message
   */
  info(...args: any[]): void {
    if (this.config.level >= LogLevel.INFO) {
      console.log(...this.format('INFO', ...args));
    }
  }
  
  /**
   * Log debug message (only in DEBUG mode)
   */
  debug(...args: any[]): void {
    if (this.config.level >= LogLevel.DEBUG) {
      console.log(...this.format('DEBUG', ...args));
    }
  }
  
  /**
   * Create a scoped logger with custom prefix
   */
  scope(scopeName: string): ScopedLogger {
    return new ScopedLogger(this, scopeName);
  }
}

/**
 * Scoped logger for module-specific logging
 */
class ScopedLogger {
  constructor(
    private parent: Logger,
    private scopeName: string
  ) {}
  
  private addScope(...args: any[]): any[] {
    return [`[${this.scopeName}]`, ...args];
  }
  
  error(...args: any[]): void {
    this.parent.error(...this.addScope(...args));
  }
  
  warn(...args: any[]): void {
    this.parent.warn(...this.addScope(...args));
  }
  
  info(...args: any[]): void {
    this.parent.info(...this.addScope(...args));
  }
  
  debug(...args: any[]): void {
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
export function initLogger(): void {
  // Check URL parameter first
  if (typeof window !== 'undefined' && window.location) {
    const params = new URLSearchParams(window.location.search);
    const urlLevel = params.get('loglevel');
    
    if (urlLevel && urlLevel.toUpperCase() in LogLevel) {
      const level = LogLevel[urlLevel.toUpperCase() as keyof typeof LogLevel] as LogLevel;
      logger.setLevel(level);
      return;
    }
  }
  
  // Check localStorage
  if (typeof window !== 'undefined' && window.localStorage) {
    try {
      const storedLevel = localStorage.getItem('anywp_loglevel');
      if (storedLevel && storedLevel.toUpperCase() in LogLevel) {
        const level = LogLevel[storedLevel.toUpperCase() as keyof typeof LogLevel] as LogLevel;
        logger.setLevel(level);
        return;
      }
    } catch (e) {
      // Ignore localStorage errors
    }
  }
  
  // Default: INFO for production, DEBUG for development
  const isDevelopment = typeof window !== 'undefined' && 
                        (window.location.hostname === 'localhost' || 
                         window.location.hostname === '127.0.0.1');
  
  logger.setLevel(isDevelopment ? LogLevel.DEBUG : LogLevel.INFO);
}

