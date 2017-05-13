#pragma once

#include "DBAccess/DataManager.h"

namespace tse
{

class MockDataManager : public virtual DataManager
{
public:
  MockDataManager();

protected:
  static tse::ConfigMan _mainConfig;
};

} // namespace tse
