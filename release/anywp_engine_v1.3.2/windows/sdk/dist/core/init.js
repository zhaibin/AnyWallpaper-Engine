// Initialization module
import { Debug } from '../utils/debug';
import { Events } from '../modules/events';
import { SPA } from '../modules/spa';
import { ClickHandler } from '../modules/click';
import { Animations } from '../modules/animations';
export function initializeAnyWP(anyWP) {
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
//# sourceMappingURL=init.js.map