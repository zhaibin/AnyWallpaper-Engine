#include "url_validator.h"

#include <algorithm>
#include <cctype>
#include <iostream>

namespace anywp_engine {

URLValidator::URLValidator() {
}

URLValidator::~URLValidator() {
}

// ========== Validation ==========

bool URLValidator::IsAllowed(const std::string& url) {
  // Empty whitelist = allow all (except blacklist)
  bool allowed = whitelist_.empty();
  
  // Check whitelist
  if (!whitelist_.empty()) {
    for (const auto& pattern : whitelist_) {
      if (MatchesPattern(url, pattern)) {
        allowed = true;
        break;
      }
    }
  }
  
  // Check blacklist (overrides whitelist)
  for (const auto& pattern : blacklist_) {
    if (MatchesPattern(url, pattern)) {
      std::cout << "[AnyWP] [Security] URL blocked by blacklist: " << url << std::endl;
      return false;
    }
  }
  
  if (!allowed && !whitelist_.empty()) {
    std::cout << "[AnyWP] [Security] URL not in whitelist: " << url << std::endl;
  }
  
  return allowed;
}

// ========== Whitelist Management ==========

void URLValidator::AddWhitelist(const std::string& pattern) {
  whitelist_.push_back(pattern);
  std::cout << "[AnyWP] [Security] Added to whitelist: " << pattern << std::endl;
}

void URLValidator::ClearWhitelist() {
  whitelist_.clear();
  std::cout << "[AnyWP] [Security] Whitelist cleared" << std::endl;
}

const std::vector<std::string>& URLValidator::GetWhitelist() const {
  return whitelist_;
}

// ========== Blacklist Management ==========

void URLValidator::AddBlacklist(const std::string& pattern) {
  blacklist_.push_back(pattern);
  std::cout << "[AnyWP] [Security] Added to blacklist: " << pattern << std::endl;
}

void URLValidator::ClearBlacklist() {
  blacklist_.clear();
  std::cout << "[AnyWP] [Security] Blacklist cleared" << std::endl;
}

const std::vector<std::string>& URLValidator::GetBlacklist() const {
  return blacklist_;
}

// ========== Pattern Matching ==========

bool URLValidator::MatchesPattern(const std::string& url, const std::string& pattern) {
  // Simple wildcard matching (* = any characters)
  std::string lower_url = url;
  std::string lower_pattern = pattern;
  
  // Convert to lowercase for case-insensitive matching
  std::transform(lower_url.begin(), lower_url.end(), lower_url.begin(), 
    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  std::transform(lower_pattern.begin(), lower_pattern.end(), lower_pattern.begin(),
    [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
  
  // Handle wildcard patterns
  if (lower_pattern.find('*') != std::string::npos) {
    // Split pattern by * and check each part
    size_t pos = 0;
    size_t url_pos = 0;
    
    while (pos < lower_pattern.size()) {
      size_t star_pos = lower_pattern.find('*', pos);
      
      if (star_pos == std::string::npos) {
        // Last segment - must match at the end
        std::string segment = lower_pattern.substr(pos);
        if (lower_url.size() < segment.size()) {
          return false;
        }
        return lower_url.substr(lower_url.size() - segment.size()) == segment;
      }
      
      if (star_pos > pos) {
        // Non-wildcard segment before the star
        std::string segment = lower_pattern.substr(pos, star_pos - pos);
        size_t found = lower_url.find(segment, url_pos);
        
        if (found == std::string::npos) {
          return false;
        }
        
        // First segment must match at the beginning
        if (pos == 0 && found != 0) {
          return false;
        }
        
        url_pos = found + segment.size();
      }
      
      pos = star_pos + 1;
    }
    
    return true;
  }
  
  // No wildcards - simple substring match
  return lower_url.find(lower_pattern) != std::string::npos;
}

}  // namespace anywp_engine

