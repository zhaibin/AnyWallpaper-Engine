export const Bounds = {
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
//# sourceMappingURL=bounds.js.map