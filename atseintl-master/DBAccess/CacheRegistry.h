//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "Common/FallbackUtil.h"

#include <algorithm>
#include <map>
#include <string>

namespace tse
{
FIXEDFALLBACK_DECL(dontCopyCacheInitializer)

class CacheControl;

class CacheRegistry
{
public:
  static CacheRegistry& instance();

  void addEntry(std::string id, CacheControl* control);
  CacheControl* getCacheControl(std::string id);
  CacheControl* getCacheControlUpperCase(const std::string& id);

  template <typename T>
  void forEach(T& t)
  {
    if (fallback::fixed::dontCopyCacheInitializer())
      std::for_each(_registry.begin(), _registry.end(), t);
    else
    {
      // we don't want to use std::for_each not to copy "t" object.
      for (auto& strCacheCtrlPair : _registry)
        t(strCacheCtrlPair);
    }
  }

private:
  std::map<std::string, CacheControl*> _registry;
};

} // namespace tse

