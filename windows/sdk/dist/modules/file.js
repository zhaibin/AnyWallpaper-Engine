// File encryption/decryption module (v2.1.10+)
// Provides API for encrypting and decrypting files via Flutter MethodChannel
export var File;
(function (File) {
    /**
     * Encrypt a file using XOR obfuscation (first 64 bytes)
     *
     * @param sourcePath - Path to the source file (unencrypted)
     * @param destPath - Path where the encrypted file will be saved
     * @returns Promise<boolean> - true if encryption succeeds, false otherwise
     */
    async function encryptFile(sourcePath, destPath) {
        this._log(`[File] Encrypting: ${sourcePath} -> ${destPath}`);
        if (!window.chrome || !window.chrome.webview) {
            console.error('[AnyWP] File.encryptFile: chrome.webview not available');
            return false;
        }
        try {
            // Use sendToFlutter to call MethodChannel properly
            const success = this.sendToFlutter('encryptFile', {
                sourcePath: sourcePath,
                destPath: destPath
            });
            if (success) {
                this._log(`[File] Encryption request sent successfully`, true);
                return true;
            }
            else {
                console.error('[AnyWP] File.encryptFile: sendToFlutter failed');
                return false;
            }
        }
        catch (error) {
            console.error('[AnyWP] File.encryptFile error:', error);
            return false;
        }
    }
    File.encryptFile = encryptFile;
    /**
     * Decrypt a file (reverse of encryptFile)
     *
     * @param encryptedPath - Path to the encrypted file
     * @param destPath - Path where the decrypted file will be saved
     * @returns Promise<boolean> - true if decryption succeeds, false otherwise
     */
    async function decryptFile(encryptedPath, destPath) {
        this._log(`[File] Decrypting: ${encryptedPath} -> ${destPath}`);
        if (!window.chrome || !window.chrome.webview) {
            console.error('[AnyWP] File.decryptFile: chrome.webview not available');
            return false;
        }
        try {
            // Use sendToFlutter to call MethodChannel properly
            const success = this.sendToFlutter('decryptFile', {
                encryptedPath: encryptedPath,
                destPath: destPath
            });
            if (success) {
                this._log(`[File] Decryption request sent successfully`, true);
                return true;
            }
            else {
                console.error('[AnyWP] File.decryptFile: sendToFlutter failed');
                return false;
            }
        }
        catch (error) {
            console.error('[AnyWP] File.decryptFile error:', error);
            return false;
        }
    }
    File.decryptFile = decryptFile;
})(File || (File = {}));
//# sourceMappingURL=file.js.map