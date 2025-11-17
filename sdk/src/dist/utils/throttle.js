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
export function throttle(func, delay) {
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
 * Creates a debounced version of the provided function
 *
 * Debounce delays function execution until after a specified time has passed
 * since the last invocation. Useful for search inputs, resize handlers.
 *
 * @param func - Function to debounce
 * @param delay - Time to wait before calling function (ms)
 * @returns Debounced function
 */
export function debounce(func, delay) {
    let timeoutId = null;
    return function (...args) {
        if (timeoutId !== null) {
            clearTimeout(timeoutId);
        }
        timeoutId = window.setTimeout(() => {
            func.apply(this, args);
            timeoutId = null;
        }, delay);
    };
}
/**
 * Throttle utility exports
 */
export const Throttle = {
    throttle,
    debounce
};
//# sourceMappingURL=throttle.js.map