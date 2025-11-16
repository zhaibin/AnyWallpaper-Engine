#include "conflict_detector.h"
#include "logger.h"
#include <algorithm>

namespace anywp_engine {

ConflictDetector::ConflictDetector() {
  // Initialize with default list
  known_wallpapers_ = GetDefaultWallpaperList();
}

ConflictDetector::~ConflictDetector() {
}

std::vector<WallpaperConflictInfo> ConflictDetector::GetDefaultWallpaperList() {
  return {
    {"Wallpaper Engine", L"wallpaper32.exe", false, 0},
    {"Wallpaper Engine", L"wallpaper64.exe", false, 0},
    {"Lively Wallpaper", L"lively.exe", false, 0},
    {"Lively Wallpaper", L"livelywpf.exe", false, 0},
    {"DeskScapes", L"DeskScapes.exe", false, 0},
    {"RainWallpaper", L"RainWallpaper.exe", false, 0},
    {"Push Video Wallpaper", L"PushVideoWallpaper.exe", false, 0}
  };
}

void ConflictDetector::AddWallpaperSoftware(const std::string& name, const std::wstring& process_name) {
  WallpaperConflictInfo info;
  info.name = name;
  info.process_name = process_name;
  info.detected = false;
  info.process_id = 0;

  known_wallpapers_.push_back(info);
  
  Logger::Instance().Info("ConflictDetector", 
    "Added wallpaper software to detection list: " + name);
}

void ConflictDetector::ClearWallpaperList() {
  known_wallpapers_.clear();
  Logger::Instance().Info("ConflictDetector", "Wallpaper detection list cleared");
}

bool ConflictDetector::EnumerateProcesses(std::vector<DWORD>& process_ids) {
  DWORD processes[1024];
  DWORD bytes_needed;

  if (!EnumProcesses(processes, sizeof(processes), &bytes_needed)) {
    Logger::Instance().Error("ConflictDetector", "Failed to enumerate processes");
    return false;
  }

  DWORD process_count = bytes_needed / sizeof(DWORD);
  process_ids.assign(processes, processes + process_count);

  Logger::Instance().Debug("ConflictDetector", 
    "Enumerated " + std::to_string(process_count) + " processes");
  
  return true;
}

std::wstring ConflictDetector::GetProcessName(DWORD process_id) {
  if (process_id == 0) {
    return L"";
  }

  HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, process_id);
  if (!process) {
    return L"";
  }

  wchar_t process_name[MAX_PATH] = {0};
  if (GetModuleBaseNameW(process, nullptr, process_name, MAX_PATH)) {
    CloseHandle(process);
    return process_name;
  }

  CloseHandle(process);
  return L"";
}

DWORD ConflictDetector::IsProcessRunning(const std::wstring& process_name) {
  DWORD processes[1024];
  DWORD bytes_needed;

  if (!EnumProcesses(processes, sizeof(processes), &bytes_needed)) {
    return 0;
  }

  DWORD process_count = bytes_needed / sizeof(DWORD);

  for (DWORD i = 0; i < process_count; i++) {
    DWORD pid = processes[i];
    if (pid == 0) continue;

    HANDLE process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pid);
    if (process) {
      wchar_t name[MAX_PATH] = {0};
      if (GetModuleBaseNameW(process, nullptr, name, MAX_PATH)) {
        // Case-insensitive comparison
        if (_wcsicmp(name, process_name.c_str()) == 0) {
          CloseHandle(process);
          return pid;
        }
      }
      CloseHandle(process);
    }
  }

  return 0;
}

bool ConflictDetector::DetectConflicts(std::vector<WallpaperConflictInfo>& conflicts) {
  conflicts.clear();

  Logger::Instance().Info("ConflictDetector", "Starting conflict detection...");

  // Get all process IDs
  std::vector<DWORD> process_ids;
  if (!EnumerateProcesses(process_ids)) {
    return false;
  }

  // Check each known wallpaper software
  for (auto& wp : known_wallpapers_) {
    wp.detected = false;
    wp.process_id = 0;

    // Search for matching process
    for (DWORD pid : process_ids) {
      if (pid == 0) continue;

      std::wstring process_name = GetProcessName(pid);
      if (process_name.empty()) continue;

      // Case-insensitive comparison
      if (_wcsicmp(process_name.c_str(), wp.process_name.c_str()) == 0) {
        wp.detected = true;
        wp.process_id = pid;
        conflicts.push_back(wp);

        // Convert wstring to string for logging (safe conversion)
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, wp.process_name.c_str(), 
                                              (int)wp.process_name.length(), 
                                              nullptr, 0, nullptr, nullptr);
        std::string narrow_name(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, wp.process_name.c_str(), 
                           (int)wp.process_name.length(), 
                           &narrow_name[0], size_needed, nullptr, nullptr);
        
        Logger::Instance().Warning("ConflictDetector", 
          "Detected conflicting wallpaper software: " + wp.name + 
          " (" + narrow_name + ", PID: " + std::to_string(pid) + ")");
        
        break;
      }
    }
  }

  if (conflicts.empty()) {
    Logger::Instance().Info("ConflictDetector", "No conflicts detected");
    return false;
  }

  Logger::Instance().Warning("ConflictDetector", 
    "Detected " + std::to_string(conflicts.size()) + " conflicting wallpaper software(s)");
  
  return true;
}

}  // namespace anywp_engine

