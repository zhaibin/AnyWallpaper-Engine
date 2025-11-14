/**
 * Animations module tests
 */
import { describe, test, expect, beforeEach, afterEach, jest } from '@jest/globals';
import { Animations } from '../modules/animations';

describe('Animations module', () => {
  let mockVideo: HTMLVideoElement;
  let mockAudio: HTMLAudioElement;
  
  beforeEach(() => {
    // Reset animation state
    delete (window as any).__anyWP_animationsPaused;
    delete (window as any).__anyWP_originalRAF;
    delete (window as any).__anyWP_originalCancelRAF;
    
    // Create mock media elements
    mockVideo = document.createElement('video');
    mockAudio = document.createElement('audio');
    document.body.appendChild(mockVideo);
    document.body.appendChild(mockAudio);
    
    // Mock play/pause
    mockVideo.play = jest.fn(() => Promise.resolve()) as any;
    mockVideo.pause = jest.fn();
    mockAudio.play = jest.fn(() => Promise.resolve()) as any;
    mockAudio.pause = jest.fn();
  });
  
  afterEach(() => {
    document.body.removeChild(mockVideo);
    document.body.removeChild(mockAudio);
    
    // Clean up style element
    const pauseStyle = document.getElementById('__anywp_pause_style');
    if (pauseStyle) {
      pauseStyle.remove();
    }
  });
  
  describe('pause', () => {
    test('should pause all videos', () => {
      // Simulate playing video
      Object.defineProperty(mockVideo, 'paused', { value: false, writable: true });
      
      Animations.pause();
      
      expect(mockVideo.pause).toHaveBeenCalled();
      expect((mockVideo as any).__anyWP_wasPlaying).toBe(true);
    });
    
    test('should not pause already paused videos', () => {
      Object.defineProperty(mockVideo, 'paused', { value: true, writable: true });
      
      Animations.pause();
      
      expect(mockVideo.pause).not.toHaveBeenCalled();
      expect((mockVideo as any).__anyWP_wasPlaying).toBeUndefined();
    });
    
    test('should pause all audio', () => {
      Object.defineProperty(mockAudio, 'paused', { value: false, writable: true });
      
      Animations.pause();
      
      expect(mockAudio.pause).toHaveBeenCalled();
      expect((mockAudio as any).__anyWP_wasPlaying).toBe(true);
    });
    
    test('should inject CSS pause style', () => {
      Animations.pause();
      
      const pauseStyle = document.getElementById('__anywp_pause_style');
      expect(pauseStyle).not.toBeNull();
      expect(pauseStyle?.textContent).toContain('animation-play-state: paused');
    });
    
    test('should intercept requestAnimationFrame', () => {
      const originalRAF = window.requestAnimationFrame;
      
      Animations.pause();
      
      expect(window.requestAnimationFrame).not.toBe(originalRAF);
      expect((window as any).__anyWP_originalRAF).toBe(originalRAF);
      
      // Test intercepted function
      const result = window.requestAnimationFrame(() => {});
      expect(result).toBe(0);
    });
    
    test('should set animation paused flag', () => {
      Animations.pause();
      
      expect((window as any).__anyWP_animationsPaused).toBe(true);
    });
    
    test('should not pause twice', () => {
      Animations.pause();
      const pauseStyle1 = document.getElementById('__anywp_pause_style');
      
      Animations.pause();
      const pauseStyle2 = document.getElementById('__anywp_pause_style');
      
      expect(pauseStyle1).toBe(pauseStyle2);
    });
  });
  
  describe('resume', () => {
    test('should remove CSS pause style', () => {
      Animations.pause();
      expect(document.getElementById('__anywp_pause_style')).not.toBeNull();
      
      Animations.resume();
      expect(document.getElementById('__anywp_pause_style')).toBeNull();
    });
    
    test('should restore requestAnimationFrame', () => {
      const originalRAF = window.requestAnimationFrame;
      
      Animations.pause();
      expect(window.requestAnimationFrame).not.toBe(originalRAF);
      
      Animations.resume();
      expect(window.requestAnimationFrame).toBe(originalRAF);
      expect((window as any).__anyWP_originalRAF).toBeUndefined();
    });
    
    test('should resume previously playing videos', async () => {
      Object.defineProperty(mockVideo, 'paused', { value: false, writable: true });
      Animations.pause();
      
      Animations.resume();
      
      // Wait for async play
      await new Promise(resolve => setTimeout(resolve, 10));
      
      expect(mockVideo.play).toHaveBeenCalled();
      expect((mockVideo as any).__anyWP_wasPlaying).toBeUndefined();
    });
    
    test('should resume previously playing audio', async () => {
      Object.defineProperty(mockAudio, 'paused', { value: false, writable: true });
      Animations.pause();
      
      Animations.resume();
      
      // Wait for async play
      await new Promise(resolve => setTimeout(resolve, 10));
      
      expect(mockAudio.play).toHaveBeenCalled();
      expect((mockAudio as any).__anyWP_wasPlaying).toBeUndefined();
    });
    
    test('should clear animation paused flag', () => {
      Animations.pause();
      expect((window as any).__anyWP_animationsPaused).toBe(true);
      
      Animations.resume();
      expect((window as any).__anyWP_animationsPaused).toBe(false);
    });
    
    test('should not resume if not paused', () => {
      const consoleLogSpy = jest.spyOn(console, 'log').mockImplementation(() => {});
      
      Animations.resume();
      
      expect(consoleLogSpy).not.toHaveBeenCalledWith(
        expect.stringContaining('Auto-resuming')
      );
      
      consoleLogSpy.mockRestore();
    });
    
    test('should handle play errors gracefully', async () => {
      mockVideo.play = jest.fn(() => Promise.reject(new Error('Play failed'))) as any;
      Object.defineProperty(mockVideo, 'paused', { value: false, writable: true });
      
      Animations.pause();
      Animations.resume();
      
      // Wait for async operation
      await new Promise(resolve => setTimeout(resolve, 10));
      
      expect(mockVideo.play).toHaveBeenCalled();
      // Should not throw
    });
  });
  
  describe('pause/resume cycle', () => {
    test('should correctly handle multiple pause/resume cycles', () => {
      // First cycle
      Animations.pause();
      expect((window as any).__anyWP_animationsPaused).toBe(true);
      
      Animations.resume();
      expect((window as any).__anyWP_animationsPaused).toBe(false);
      
      // Second cycle
      Animations.pause();
      expect((window as any).__anyWP_animationsPaused).toBe(true);
      
      Animations.resume();
      expect((window as any).__anyWP_animationsPaused).toBe(false);
    });
  });
});

