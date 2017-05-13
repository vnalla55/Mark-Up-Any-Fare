#include "DBAccess/CacheRegistry.h"

#include <cctype>

namespace tse
{
static void
toUpper(std::string& s)
{
  std::transform(s.begin(), s.end(), s.begin(), (int (*)(int))std::toupper);
}

CacheRegistry&
CacheRegistry::instance()
{
  static CacheRegistry* const instance = new CacheRegistry();
  return *instance;
}

void
CacheRegistry::addEntry(std::string id, CacheControl* control)
{
  toUpper(id);
  _registry[id] = control;
}

CacheControl*
CacheRegistry::getCacheControl(std::string id)
{
  toUpper(id);
  return getCacheControlUpperCase(id);
}

CacheControl*
CacheRegistry::getCacheControlUpperCase(const std::string& id)
{
  auto it = _registry.find(id);
  return (it != _registry.end()) ? it->second : nullptr;
}
}
