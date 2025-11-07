#ifndef ANYWP_ENGINE_URL_VALIDATOR_H_
#define ANYWP_ENGINE_URL_VALIDATOR_H_

#include <string>
#include <vector>

namespace anywp_engine {

/**
 * URLValidator - Security validation for URLs
 * 
 * Features:
 * - Whitelist/Blacklist pattern matching
 * - Wildcard support (*)
 * - Protocol validation
 * - Domain validation
 * 
 * Usage:
 *   URLValidator validator;
 *   validator.AddWhitelist("https://*.example.com/*");
 *   validator.AddBlacklist("*://malicious.com/*");
 *   if (validator.IsAllowed(url)) { ... }
 */
class URLValidator {
public:
  URLValidator();
  ~URLValidator();

  // Validation
  bool IsAllowed(const std::string& url);

  // Whitelist management (if empty, all URLs allowed by default)
  void AddWhitelist(const std::string& pattern);
  void ClearWhitelist();
  const std::vector<std::string>& GetWhitelist() const;

  // Blacklist management
  void AddBlacklist(const std::string& pattern);
  void ClearBlacklist();
  const std::vector<std::string>& GetBlacklist() const;

private:
  bool MatchesPattern(const std::string& url, const std::string& pattern);

  std::vector<std::string> whitelist_;
  std::vector<std::string> blacklist_;
};

}  // namespace anywp_engine

#endif  // ANYWP_ENGINE_URL_VALIDATOR_H_

