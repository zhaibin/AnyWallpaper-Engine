// Initialization module
import { Debug } from '../utils/debug';
import { Events } from '../modules/events';
import { SPA } from '../modules/spa';
import { ClickHandler } from '../modules/click';
import { Animations } from '../modules/animations';
import { initWallpaperController } from '../modules/wallpaper';
import { initLogger, logger } from '../utils/logger';
import { setupFlutterMessageListener, sendToFlutter } from '../modules/webmessage';
import type { AnyWPSDK } from '../types';

export function initializeAnyWP(anyWP: AnyWPSDK): void {
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

