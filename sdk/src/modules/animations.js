// Animation control module
export const Animations = {
  // Auto-pause all animations for power saving
  pause() {
    if (window.__anyWP_animationsPaused) {
      return;
    }
    
    console.log('[AnyWP] Auto-pausing all animations for power saving...');
    window.__anyWP_animationsPaused = true;
    
    try {
      // 1. Pause all videos
      var videos = document.querySelectorAll('video');
      console.log('[AnyWP] Found ' + videos.length + ' video(s)');
      videos.forEach(function(video) {
        try {
          if (!video.paused) {
            video.__anyWP_wasPlaying = true;
            video.pause();
          }
        } catch(e) {
          console.warn('[AnyWP] Failed to pause video:', e);
        }
      });
      
      // 2. Pause all audio
      var audios = document.querySelectorAll('audio');
      console.log('[AnyWP] Found ' + audios.length + ' audio(s)');
      audios.forEach(function(audio) {
        try {
          if (!audio.paused) {
            audio.__anyWP_wasPlaying = true;
            audio.pause();
          }
        } catch(e) {
          console.warn('[AnyWP] Failed to pause audio:', e);
        }
      });
      
      // 3. Pause CSS animations
      var pauseStyle = document.getElementById('__anywp_pause_style');
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
      if (!window.__anyWP_originalRAF) {
        window.__anyWP_originalRAF = window.requestAnimationFrame;
        window.__anyWP_originalCancelRAF = window.cancelAnimationFrame;
        
        window.requestAnimationFrame = function(callback) { return 0; };
        window.cancelAnimationFrame = function(id) {};
        
        console.log('[AnyWP] requestAnimationFrame disabled');
      }
      
      console.log('[AnyWP] Auto-pause complete');
    } catch(e) {
      console.error('[AnyWP] Error during auto-pause:', e);
    }
  },
  
  // Auto-resume all animations
  resume() {
    if (!window.__anyWP_animationsPaused) {
      return;
    }
    
    console.log('[AnyWP] Auto-resuming all animations...');
    window.__anyWP_animationsPaused = false;
    
    try {
      // 1. Remove CSS pause style
      var pauseStyle = document.getElementById('__anywp_pause_style');
      if (pauseStyle) {
        pauseStyle.remove();
        console.log('[AnyWP] CSS animations resumed');
      }
      
      // 2. Restore requestAnimationFrame
      if (window.__anyWP_originalRAF) {
        window.requestAnimationFrame = window.__anyWP_originalRAF;
        window.cancelAnimationFrame = window.__anyWP_originalCancelRAF;
        delete window.__anyWP_originalRAF;
        delete window.__anyWP_originalCancelRAF;
        console.log('[AnyWP] requestAnimationFrame restored');
      }
      
      // 3. Resume videos
      var videos = document.querySelectorAll('video');
      videos.forEach(function(video) {
        try {
          if (video.__anyWP_wasPlaying) {
            video.play().catch(function(e) {
              console.warn('[AnyWP] Failed to resume video:', e);
            });
            delete video.__anyWP_wasPlaying;
          }
        } catch(e) {
          console.warn('[AnyWP] Error resuming video:', e);
        }
      });
      
      // 4. Resume audio
      var audios = document.querySelectorAll('audio');
      audios.forEach(function(audio) {
        try {
          if (audio.__anyWP_wasPlaying) {
            audio.play().catch(function(e) {
              console.warn('[AnyWP] Failed to resume audio:', e);
            });
            delete audio.__anyWP_wasPlaying;
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


