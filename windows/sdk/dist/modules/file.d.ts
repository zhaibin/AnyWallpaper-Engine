import type { AnyWPSDK } from '../types';
export declare namespace File {
    /**
     * Encrypt a file using XOR obfuscation (first 64 bytes)
     *
     * @param sourcePath - Path to the source file (unencrypted)
     * @param destPath - Path where the encrypted file will be saved
     * @returns Promise<boolean> - true if encryption succeeds, false otherwise
     */
    function encryptFile(this: AnyWPSDK, sourcePath: string, destPath: string): Promise<boolean>;
    /**
     * Decrypt a file (reverse of encryptFile)
     *
     * @param encryptedPath - Path to the encrypted file
     * @param destPath - Path where the decrypted file will be saved
     * @returns Promise<boolean> - true if decryption succeeds, false otherwise
     */
    function decryptFile(this: AnyWPSDK, encryptedPath: string, destPath: string): Promise<boolean>;
}
//# sourceMappingURL=file.d.ts.map