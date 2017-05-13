//----------------------------------------------------------------------------
//
//  Description: Global TSE Resources
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/Global.h"

#include "Common/Config/ConfigMan.h"
#include "Common/TseEnums.h"

#include <string>
#include <vector>

namespace tse
{

//----------------------------------------------------------------------------
// static initializers
//----------------------------------------------------------------------------
tse::ConfigMan* Global::_configMan = nullptr;
tse::MetricsMan* Global::_metricsMan = nullptr;

Global::configPtr Global::_dynamicCfg;
Global::configPtr Global::_newDynamicCfg;
std::atomic<bool> Global::_configUpdateInProgress(false);
#ifdef CONFIG_HIERARCHY_REFACTOR
std::atomic<ConfigBundle*> Global::_configBundle{nullptr};
#endif

bool Global::_allowHistorical = false;
size_t Global::_unlimitedCacheSize = 0;
DateTime Global::_startTime;
HasherMethod Global::_hasherMethod = HasherMethod::METHOD_0;

std::map<std::string, LoadableModule<Adapter> >* Global::_adapters = nullptr;
std::map<std::string, LoadableModule<Manager> >* Global::_managers = nullptr;
std::map<std::string, LoadableModule<Service> >* Global::_services = nullptr;
std::map<std::string, LoadableModule<Xform> >* Global::_xforms = nullptr;

void
Global::setHasherMethod(uint16_t hasherMethodNumber)
{
  switch (hasherMethodNumber)
  {
  case 0:
  {
    _hasherMethod = HasherMethod::METHOD_0;
    break;
  }
  case 1:
  {
    _hasherMethod = HasherMethod::METHOD_1;
    break;
  }
  case 2:
  {
    _hasherMethod = HasherMethod::METHOD_2;
    break;
  }
  case 3:
  {
    _hasherMethod = HasherMethod::METHOD_3;
    break;
  }
  case 4:
  {
    _hasherMethod = HasherMethod::METHOD_4;
    break;
  }
  case 5:
  {
    _hasherMethod = HasherMethod::METHOD_5;
    break;
  }
  case 6:
  {
    _hasherMethod = HasherMethod::METHOD_6;
    break;
  }
  case 7:
  {
    _hasherMethod = HasherMethod::METHOD_7;
    break;
  }
  default:
    _hasherMethod = HasherMethod::METHOD_0;
  }
}

} // tse
