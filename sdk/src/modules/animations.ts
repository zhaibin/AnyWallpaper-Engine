// Animation control module
import type { AnyWPSDK } from '../types';

export const Animations = {
  /**
   * Auto-pause all animations for power saving
   */
  pause(_anyWP?: AnyWPSDK): void {
    if ((window as any).__anyWP_animationsPaused) {
      return;
    }
    
    console.log('[AnyWP] Auto-pausing all animations for power saving...');
    (window as any).__anyWP_animationsPaused = true;
    
    try {
      // 1. Pause all videos
      const videos = document.querySelectorAll('video');
      console.log('[AnyWP] Found ' + videos.length + ' video(s)');
      videos.forEach((video) => {
        try {
          if (!video.paused) {
            (video as any).__anyWP_wasPlaying = true;
            video.pause();
          }
        } catch(e) {
          console.warn('[AnyWP] Failed to pause video:', e);
        }
      });
      
      // 2. Pause all audio
      const audios = document.querySelectorAll('audio');
      console.log('[AnyWP] Found ' + audios.length + ' audio(s)');
      audios.forEach((audio) => {
        try {
          if (!audio.paused) {
            (audio as any).__anyWP_wasPlaying = true;
            audio.pause();
          }
        } catch(e) {
          console.warn('[AnyWP] Failed to pause audio:', e);
        }
      });
      
      // 3. Pause CSS animations
      let pauseStyle = document.getElementById('__anywp_pause_style');
      if (!pauseStyle) {
        pauseStyle = document.createElement('style');
        pauseStyle.id = '__anywp_pause_style';
        pauseStyle.textContent = '* { animation-play-state: paused !important; } *::before, *::after { animation-play-state: paused !important; }';
        if (document.head) {
          document.head.appendChild(pauseStyle);
          console.log('[AnyWP] CSS animations paused');
        }
      }
      
      // 4. Intercept requestAnimationFrame
      if (!(window as any).__anyWP_originalRAF) {
        (window as any).__anyWP_originalRAF = window.requestAnimationFrame;
        (window as any).__anyWP_originalCancelRAF = window.cancelAnimationFrame;
        
        window.requestAnimationFrame = (_callback: FrameRequestCallback): number => 0;
        window.cancelAnimationFrame = (_id: number): void => {};
        
        console.log('[AnyWP] requestAnimationFrame disabled');
      }
      
      console.log('[AnyWP] Auto-pause complete');
    } catch(e) {
      console.error('[AnyWP] Error during auto-pause:', e);
    }
  },
  
  /**
   * Auto-resume all animations
   */
  resume(_anyWP?: AnyWPSDK): void {
    if (!(window as any).__anyWP_animationsPaused) {
      return;
    }
    
    console.log('[AnyWP] Auto-resuming all animations...');
    (window as any).__anyWP_animationsPaused = false;
    
    try {
      // 1. Remove CSS pause style
      const pauseStyle = document.getElementById('__anywp_pause_style');
      if (pauseStyle) {
        pauseStyle.remove();
        console.log('[AnyWP] CSS animations resumed');
      }
      
      // 2. Restore requestAnimationFrame
      if ((window as any).__anyWP_originalRAF) {
        window.requestAnimationFrame = (window as any).__anyWP_originalRAF;
        window.cancelAnimationFrame = (window as any).__anyWP_originalCancelRAF;
        delete (window as any).__anyWP_originalRAF;
        delete (window as any).__anyWP_originalCancelRAF;
        console.log('[AnyWP] requestAnimationFrame restored');
      }
      
      // 3. Resume videos
      const videos = document.querySelectorAll('video');
      videos.forEach((video) => {
        try {
          if ((video as any).__anyWP_wasPlaying) {
            video.play().catch((e) => {
              console.warn('[AnyWP] Failed to resume video:', e);
            });
            delete (video as any).__anyWP_wasPlaying;
          }
        } catch(e) {
          console.warn('[AnyWP] Error resuming video:', e);
        }
      });
      
      // 4. Resume audio
      const audios = document.querySelectorAll('audio');
      audios.forEach((audio) => {
        try {
          if ((audio as any).__anyWP_wasPlaying) {
            audio.play().catch((e) => {
              console.warn('[AnyWP] Failed to resume audio:', e);
            });
            delete (audio as any).__anyWP_wasPlaying;
          }
        } catch(e) {
          console.warn('[AnyWP] Error resuming audio:', e);
        }
      });
      
      console.log('[AnyWP] Auto-resume complete');
    } catch(e) {
      console.error('[AnyWP] Error during auto-resume:', e);
    }
  }
};

