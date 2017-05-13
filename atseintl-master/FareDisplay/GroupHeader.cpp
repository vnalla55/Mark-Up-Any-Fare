//-------------------------------------------------------------------
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/GroupHeader.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/FDConsts.h"
#include "FareDisplay/Group.h"

namespace tse
{
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

static Logger
logger("atseintl.FareDiplay.GroupHeader");

namespace
{
ConfigurableValue<bool>
groupMultitransportFares("FAREDISPLAY_SVC", "GROUP_MULTITRANSPORT_FARES", false);
}

GroupHeader::GroupHeader(FareDisplayTrx& trx) : _trx(trx)
{
  if (!trx.itin().empty())
  {
    Itin* itin = trx.itin().front();
    _isInternational = (itin->geoTravelType() == GeoTravelType::International);
    _isSameCityPair = trx.isSameCityPairRqst();
  }
}

void
GroupHeader::setGroupHeaderInfo(std::vector<Group*>& groups)
{
  setCxrHeader();

  if (_trx.getRequest()->isPaxTypeRequested() && isMultipleGlobal())
  {
    if (!fallback::fallbackFareDisplayByCabinActivation(&_trx) &&
        _trx.getRequest()->multiInclusionCodes())
      setCabinHeader(groups, _trx.fdResponse()->groupHeaders());
    setPaxHeader(groups, _trx.fdResponse()->groupHeaders());
    setGlobalHeader(groups, _trx.fdResponse()->groupHeaders());
  }
  else if (!fallback::fallbackFareDisplayByCabinActivation(&_trx) &&
           _trx.getRequest()->multiInclusionCodes() && isMultipleGlobal())
  {
    setCabinHeader(groups, _trx.fdResponse()->groupHeaders());
    setGlobalHeader(groups, _trx.fdResponse()->groupHeaders());
    setPaxHeader(groups, _trx.fdResponse()->groupHeaders());
  }
  else
  {
    setGlobalHeader(groups, _trx.fdResponse()->groupHeaders());
    if (!fallback::fallbackFareDisplayByCabinActivation(&_trx) &&
        _trx.getRequest()->multiInclusionCodes())
      setCabinHeader(groups, _trx.fdResponse()->groupHeaders());
    setPaxHeader(groups, _trx.fdResponse()->groupHeaders());
  }
}

void
GroupHeader::setCxrHeader()
{
  if (_isInternational && !_trx.isShopperRequest())
  {
    LOG4CXX_DEBUG(logger, "Setting YY Cxr Header");
    _trx.fdResponse()->groupHeaders().push_back(Group::GROUP_BY_CARRIER);
  }
}

void
GroupHeader::setPaxHeader(std::vector<Group*>& groups, std::vector<Group::GroupType>& groupHeader)
{
  bool isPaxHeaderRequired = _trx.getRequest()->isPaxTypeRequested();
  if (isPaxHeaderRequired)
  {
    LOG4CXX_DEBUG(logger, "Setting PAX Type Header for inclusion code / Pax Type Request");
    groupHeader.push_back(Group::GROUP_BY_PSG_TYPE);
  }
  else
  {
    LOG4CXX_DEBUG(logger, "Setting PAX Type Header for Pax Type Group");
    std::vector<Group*>::iterator i(groups.end());
    i = find_if(groups.begin(), groups.end(), EqualByGrpType(Group::GROUP_BY_PSG_TYPE));
    if (i != groups.end())
      groupHeader.push_back(Group::GROUP_BY_PSG_TYPE);
  }
}

void
GroupHeader::setGlobalHeader(std::vector<Group*>& groups,
                             std::vector<Group::GroupType>& groupHeader)
{
  if (_isInternational || _isSameCityPair || FareDisplayUtil::isAxessUser(_trx))
  {
    LOG4CXX_DEBUG(logger, "Setting Global Header");
    std::vector<Group*>::iterator i(groups.end());
    i = find_if(groups.begin(), groups.end(), EqualByGrpType(Group::GROUP_BY_GLOBAL_DIR));
    if (i != groups.end())
      groupHeader.push_back(Group::GROUP_BY_GLOBAL_DIR);
  }
}

struct GetUniqueGlobals : public std::unary_function<PaxTypeFare, GlobalDirection>

{
  GlobalDirection operator()(const PaxTypeFare* p) const { return p->globalDirection(); }

  // lint -e{1509}
};

bool
GroupHeader::isMultipleGlobal()
{
  LOG4CXX_DEBUG(logger, "CHECKING FOR MULTIPLE GLOBALS ");
  if (!_isInternational && !_isSameCityPair)
  {
    return false;
  }
  else
  {
    std::set<GlobalDirection> uniqueGlobals;
    std::transform(_trx.allPaxTypeFare().begin(),
                   _trx.allPaxTypeFare().end(),
                   inserter(uniqueGlobals, uniqueGlobals.end()),
                   GetUniqueGlobals());
    LOG4CXX_DEBUG(logger, "TOTAL GLOBALS FOUND" << uniqueGlobals.size());
    return uniqueGlobals.size() > GroupHeader::MORE_THAN_ONE;
  }
}

void
GroupHeader::setMultiTransportHeader(std::vector<Group*>& groups,
                                     std::vector<Group::GroupType>& groupHeader)
{
  LOG4CXX_DEBUG(logger, "Setting PAX Type Header for MultiTransport  Group");
  if (_trx.getOptions()->sortAscending() || _trx.getOptions()->sortDescending())
    return;
  else if (_trx.getRequest()->inclusionCode() == ALL_INCLUSION_CODE)
    return;

  std::vector<Group*>::iterator i(groups.end());
  i = find_if(groups.begin(), groups.end(), EqualByGrpType(Group::GROUP_BY_MULTITRANSPORT));

  if (i != groups.end())
    groupHeader.push_back(Group::GROUP_BY_MULTITRANSPORT);

  else if (groupMultitransportFares.getValue())
  {
    groupHeader.push_back(Group::GROUP_BY_MULTITRANSPORT);
  }
}

void
GroupHeader::setBrandHeader()
{
  LOG4CXX_DEBUG(logger, "Setting Brand Header");
  _trx.fdResponse()->groupHeaders().push_back(Group::GROUP_BY_BRAND);
}

void
GroupHeader::setS8BrandHeader()
{
  LOG4CXX_DEBUG(logger, "Setting S8Brand Header");
  _trx.fdResponse()->groupHeaders().push_back(Group::GROUP_BY_S8BRAND);
}

void
GroupHeader::setCabinHeader(std::vector<Group*>& groups, std::vector<Group::GroupType>& groupHeader)
{
  LOG4CXX_DEBUG(logger, "Setting Cabin Header");
  groupHeader.push_back(Group::GROUP_BY_CABIN);
}

void
GroupHeader::setCabinHeader()
{
  LOG4CXX_DEBUG(logger, "Setting Cabin Header");
  _trx.fdResponse()->groupHeaders().push_back(Group::GROUP_BY_CABIN);
}
}
