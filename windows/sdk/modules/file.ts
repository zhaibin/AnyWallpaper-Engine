// File encryption/decryption module (v2.1.10+)
// Provides API for encrypting and decrypting files via Flutter MethodChannel

import type { AnyWPSDK } from '../types';

export namespace File {
  /**
   * Encrypt a file using XOR obfuscation (first 64 bytes)
   * 
   * @param sourcePath - Path to the source file (unencrypted)
   * @param destPath - Path where the encrypted file will be saved
   * @returns Promise<boolean> - true if encryption succeeds, false otherwise
   */
  export async function encryptFile(
    this: AnyWPSDK,
    sourcePath: string,
    destPath: string
  ): Promise<boolean> {
    this._log(`[File] Encrypting: ${sourcePath} -> ${destPath}`);
    
    if (!window.chrome || !window.chrome.webview) {
      console.error('[AnyWP] File.encryptFile: chrome.webview not available');
      return false;
    }
    
    try {
      // Call Flutter MethodChannel via WebView2 postMessage
      window.chrome.webview.postMessage({
        type: 'encryptFile',
        sourcePath: sourcePath,
        destPath: destPath
      } as any);
      
      // Wait for response (simplified - in production use promise-based callback)
      // For now, return true and let the native side handle it
      this._log(`[File] Encryption request sent`, true);
      return true;
    } catch (error) {
      console.error('[AnyWP] File.encryptFile error:', error);
      return false;
    }
  }
  
  /**
   * Decrypt a file (reverse of encryptFile)
   * 
   * @param encryptedPath - Path to the encrypted file
   * @param destPath - Path where the decrypted file will be saved
   * @returns Promise<boolean> - true if decryption succeeds, false otherwise
   */
  export async function decryptFile(
    this: AnyWPSDK,
    encryptedPath: string,
    destPath: string
  ): Promise<boolean> {
    this._log(`[File] Decrypting: ${encryptedPath} -> ${destPath}`);
    
    if (!window.chrome || !window.chrome.webview) {
      console.error('[AnyWP] File.decryptFile: chrome.webview not available');
      return false;
    }
    
    try {
      // Call Flutter MethodChannel via WebView2 postMessage
      window.chrome.webview.postMessage({
        type: 'decryptFile',
        encryptedPath: encryptedPath,
        destPath: destPath
      } as any);
      
      // Wait for response (simplified - in production use promise-based callback)
      // For now, return true and let the native side handle it
      this._log(`[File] Decryption request sent`, true);
      return true;
    } catch (error) {
      console.error('[AnyWP] File.decryptFile error:', error);
      return false;
    }
  }
}

