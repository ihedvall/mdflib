#include "StringUtil.h"

namespace util::string {
bool IgnoreCase::operator()(const std::string &s1, const std::string &s2) const {
  if (s1.size() != s2.size()) {
    return false;
  }
  std::string s1_lower(s1);
  std::string s2_lower(s2);
  std::transform(s1.begin(), s1.end(), s1_lower.begin(), ::tolower);
  std::transform(s2.begin(), s2.end(), s2_lower.begin(), ::tolower);
  return s1_lower == s2_lower;
}

bool IgnoreCase::operator()(const std::wstring &s1, const std::wstring &s2) const {
  if (s1.size() != s2.size()) {
    return false;
  }
  std::wstring s1_lower(s1);
  std::wstring s2_lower(s2);
  std::transform(s1.begin(), s1.end(), s1_lower.begin(), ::tolower);
  std::transform(s2.begin(), s2.end(), s2_lower.begin(), ::tolower);
  return s1_lower == s2_lower;
}
}  // namespace util::string