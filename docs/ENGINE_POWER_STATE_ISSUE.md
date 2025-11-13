# AnyWP Engine - Power State Callback Issue Report

## 🐛 Issue Summary

**Problem:** The `onPowerStateChange` callback in Dart is never triggered when the system locks/unlocks, even though the engine correctly detects the lock screen events.

**Root Cause:** The critical `InvokeMethod("onPowerStateChange")` call is commented out in the engine code.

---

## 📍 Location

**File:** `windows/anywp_engine_plugin.cpp`  
**Function:** `AnyWPEnginePlugin::NotifyPowerStateChange(PowerState newState)`  
**Line:** ~3147

```cpp
void AnyWPEnginePlugin::NotifyPowerStateChange(PowerState newState) {
  // ... state checks ...
  
  std::cout << "[AnyWP] [PowerSaving] Notifying Dart about state change" << std::endl;
  // Note: InvokeMethod may crash in some contexts, so we skip it for now
  // method_channel_->InvokeMethod("onPowerStateChange", std::move(args_value));
  //                                   ☝️ THIS LINE IS COMMENTED OUT
}
```

---

## 🔍 Impact

### What Works ✅
1. Engine detects `WTS_SESSION_LOCK` events
2. Engine updates internal state (`is_session_locked_`)
3. Engine calls `PowerManager::SetSessionLocked()`
4. Console logs show lock/unlock events

### What Doesn't Work ❌
1. Dart callback `setOnPowerStateChangeCallback()` is never called
2. Flutter app cannot respond to lock screen events
3. Features depending on lock detection don't work:
   - Pause countdown timer
   - Pause video playback
   - Save power during lock

---

## 💡 Suggested Solutions

### Solution 1: Fix InvokeMethod Crash Issue (Recommended)

The comment says "InvokeMethod may crash in some contexts". We need to:

1. **Identify the crash context:**
   - When does it crash?
   - What's the thread context?
   - Is it called from the UI thread?

2. **Use thread-safe invocation:**

```cpp
void AnyWPEnginePlugin::NotifyPowerStateChange(PowerState newState) {
  if (newState == power_state_) {
    return;  // No change
  }
  
  std::string oldStateStr = PowerStateToString(last_power_state_);
  std::string newStateStr = PowerStateToString(newState);
  
  std::cout << "[AnyWP] [PowerSaving] State changed: " << oldStateStr 
            << " -> " << newStateStr << std::endl;
  
  // Update states
  last_power_state_ = power_state_;
  power_state_ = newState;
  
  // ✅ FIX: Use thread-safe invocation
  if (method_channel_) {
    flutter::EncodableMap args;
    args[flutter::EncodableValue("oldState")] = flutter::EncodableValue(oldStateStr);
    args[flutter::EncodableValue("newState")] = flutter::EncodableValue(newStateStr);
    
    auto args_value = std::make_unique<flutter::EncodableValue>(args);
    
    // Post to UI thread to avoid crashes
    auto task = [this, args_value = std::move(args_value)]() mutable {
      try {
        method_channel_->InvokeMethod("onPowerStateChange", std::move(args_value));
        std::cout << "[AnyWP] [PowerSaving] ✅ Dart notified successfully" << std::endl;
      } catch (const std::exception& e) {
        std::cerr << "[AnyWP] [PowerSaving] ❌ Failed to notify Dart: " 
                  << e.what() << std::endl;
      }
    };
    
    // Execute on Flutter's platform task runner
    // (Need access to FlutterDesktopMessengerRef or similar)
    task();  // Or post to appropriate thread
  }
}
```

### Solution 2: Alternative Notification Method

If `InvokeMethod` is unreliable, use a polling mechanism:

```cpp
// In plugin class
std::queue<PowerStateChange> pending_state_changes_;
std::mutex state_changes_mutex_;

void AnyWPEnginePlugin::NotifyPowerStateChange(PowerState newState) {
  std::lock_guard<std::mutex> lock(state_changes_mutex_);
  pending_state_changes_.push({last_power_state_, newState});
}

// Add a new method that Dart can poll
flutter::EncodableValue AnyWPEnginePlugin::GetPendingStateChanges() {
  std::lock_guard<std::mutex> lock(state_changes_mutex_);
  flutter::EncodableList changes;
  
  while (!pending_state_changes_.empty()) {
    auto change = pending_state_changes_.front();
    pending_state_changes_.pop();
    
    flutter::EncodableMap item;
    item["oldState"] = PowerStateToString(change.old_state);
    item["newState"] = PowerStateToString(change.new_state);
    changes.push_back(item);
  }
  
  return flutter::EncodableValue(changes);
}
```

Then in Dart:
```dart
// Poll every 100ms
Timer.periodic(Duration(milliseconds: 100), (timer) async {
  final changes = await AnyWPEngine.getPendingStateChanges();
  for (var change in changes) {
    _onPowerStateChangeCallback?.call(change['oldState'], change['newState']);
  }
});
```

### Solution 3: Use Windows Message Forwarding

Forward `WM_WTSSESSION_CHANGE` to Flutter window:

```cpp
case WM_WTSSESSION_CHANGE:
  // ... existing handling ...
  
  // Forward to Flutter window
  if (flutter_window_) {
    PostMessage(flutter_window_, WM_WTSSESSION_CHANGE, wParam, lParam);
  }
  break;
```

Then handle in Flutter window proc.

---

## 🧪 Test Case

### Expected Behavior

```dart
// Dart code
AnyWPEngine.setOnPowerStateChangeCallback((oldState, newState) {
  print('Power state changed: $oldState -> $newState');
  if (newState == 'LOCKED') {
    // Pause animations, videos, timers
  } else if (newState == 'ACTIVE') {
    // Resume animations, videos, timers
  }
});

// User locks screen (Win+L)
// Expected output: "Power state changed: ACTIVE -> LOCKED"

// User unlocks
// Expected output: "Power state changed: LOCKED -> ACTIVE"
```

### Actual Behavior

```
// Nothing happens - callback is never called
```

---

## 📊 Use Cases Affected

1. **Power Saving:** Can't pause videos/animations during lock
2. **UI Updates:** Can't update countdown timer display
3. **Resource Management:** Can't reduce CPU/GPU usage during lock
4. **Multi-Monitor:** Can't handle monitor configuration changes during lock

---

## 🎯 Priority

**HIGH** - This feature is documented in the API but doesn't work.

From `lib/dart/anywp_engine.dart`:
```dart
/// Set callback for power state changes
/// 
/// States can be: ACTIVE, IDLE, SCREEN_OFF, LOCKED, FULLSCREEN_APP, PAUSED
/// 
/// Example:
/// ```dart
/// AnyWPEngine.setOnPowerStateChangeCallback((oldState, newState) {
///   print('Power state changed: $oldState -> $newState');
/// });
/// ```
static void setOnPowerStateChangeCallback(
  void Function(String oldState, String newState) callback
)
```

**The API exists and is documented, but the implementation is incomplete.**

---

## ✅ Verification

After the fix, verify with:

1. Start app with callback registered
2. Lock screen (Win+L)
3. Check console for: `Power state changed: ACTIVE -> LOCKED`
4. Unlock
5. Check console for: `Power state changed: LOCKED -> ACTIVE`

---

## 📝 Additional Notes

### Current Workaround

There's no workaround - apps cannot detect lock screen events through this engine.

### Alternative Solutions for App Developers

Until this is fixed, app developers could:
1. Use native Windows plugins to detect lock events
2. Poll system idle time
3. Use visibility API (but this doesn't cover all scenarios)

---

## 🙏 Request

Please prioritize fixing this issue as it's critical for power management and user experience. The infrastructure is already there - we just need to uncomment and stabilize the `InvokeMethod` call.

**Thank you for maintaining this excellent engine!**

---

**Reported by:** HKCW Desktop Team  
**Date:** 2025-11-13  
**Engine Version:** v2.1.0  
**Platform:** Windows 10/11

