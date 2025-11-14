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
export declare function throttle<T extends (...args: any[]) => void>(func: T, delay: number): (...args: Parameters<T>) => void;
/**
 * Creates a debounced version of the provided function
 *
 * Debounce delays function execution until after a specified time has passed
 * since the last invocation. Useful for search inputs, resize handlers.
 *
 * @param func - Function to debounce
 * @param delay - Time to wait before calling function (ms)
 * @returns Debounced function
 */
export declare function debounce<T extends (...args: any[]) => void>(func: T, delay: number): (...args: Parameters<T>) => void;
/**
 * Throttle utility exports
 */
export declare const Throttle: {
    throttle: typeof throttle;
    debounce: typeof debounce;
};
//# sourceMappingURL=throttle.d.ts.map