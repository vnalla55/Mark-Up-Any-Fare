//-------------------------------------------------------------------
//
//  Created:     April 5, 2005
//  Authors:     Abu
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/StrategyBuilder.h"

#include "BrandedFares/BrandedFaresSelector.h"
#include "BrandedFares/BrandingService.h"
#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/ConfigList.h"
#include "Common/FallbackUtil.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "DBAccess/Customer.h"
#include "FareDisplay/BrandDataRetriever.h"
#include "FareDisplay/CabinGroup.h"
#include "FareDisplay/DefaultGroupingStrategy.h"
#include "FareDisplay/Group.h"
#include "FareDisplay/GroupingDataRetriever.h"
#include "FareDisplay/OverrideGroupingStrategy.h"
#include "FareDisplay/PreferredGroupingStrategy.h"
#include "FareDisplay/S8Brand.h"

using namespace std;
namespace tse
{
FALLBACK_DECL(fallbackBrandedFaresFareDisplay);
FALLBACK_DECL(fallbackFareDisplayByCabinActivation);

static Logger
logger("atseintl.FareDisplay.StrategyBuilder");

namespace
{
ConfigurableValue<ConfigSet<uint16_t>>
groupOverrideCfg("FAREDISPLAY_SVC", "GROUP_OVERRIDE");
}

//-------------------------------------------------------------------
// constructor
//-------------------------------------------------------------------
StrategyBuilder::StrategyBuilder()
{
}

//-------------------------------------------------------------------
// SortByCarrier() : functor to make sure YY fares goes at the end.
//-------------------------------------------------------------------

struct SortByCarrier : public std::binary_function<PaxTypeFare, PaxTypeFare, bool>
{
public:
  bool operator()(const PaxTypeFare* l, const PaxTypeFare* r) const
  {
    if (l == nullptr)
      return true;
    if (r == nullptr)
      return false;

    if (l->carrier() == tse::INDUSTRY_CARRIER)
      return false;

    if (l->carrier() < r->carrier())
    {
      return true;
    }
    else if ((l->carrier() > r->carrier()) && (r->carrier() == tse::INDUSTRY_CARRIER))
    {
      return true;
    }
    return false;
  }

  // lint -e{1509}
} lessByIndustryCxr; // lint !e1502

//-------------------------------------------------------------------
// FirstIndCxr() : functor to find the boundary between YY and non-YY fares
//-------------------------------------------------------------------

struct FirstIndCxr : public std::unary_function<PaxTypeFare, bool>
{
public:
  bool operator()(const PaxTypeFare* p) const
  {
    if (p->carrier() == INDUSTRY_CARRIER)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  // lint -e{1509}
} firstIndCxr; // lint !e1502

//------------------------------------------------------
// StrategyBuilder::buildStrategy()
//------------------------------------------------------

GroupingStrategy*
StrategyBuilder::buildStrategy(FareDisplayTrx& trx) const
{
  LOG4CXX_INFO(logger, " Entered StrategyBuilder::buildStrategy()");

  std::vector<PaxTypeFare*>& fares = trx.allPaxTypeFare();
  GroupingStrategy* strategy(nullptr);
  selectStrategy(trx, strategy);

  strategy->_isShopperRequest = trx.isShopperRequest();
  strategy->_isInternationalFare =
      ((trx.itin().front())->geoTravelType() == GeoTravelType::International ||
       trx.isSameCityPairRqst());

  // lint --e{413}

  if (strategy->_isShopperRequest || !strategy->_isInternationalFare)
  {
    std::copy(fares.begin(), fares.end(), inserter(strategy->fares(), strategy->fares().begin()));
  }
  else
  {
    std::vector<PaxTypeFare*>::iterator i(fares.end());
    std::sort(fares.begin(), fares.end(), lessByIndustryCxr);
    i = find_if(fares.begin(), fares.end(), firstIndCxr);

    if (i != fares.end())
    {
      std::copy(fares.begin(), i, inserter(strategy->fares(), strategy->fares().begin()));

      std::copy(i, fares.end(), inserter(strategy->yy_fares(), strategy->yy_fares().begin()));
    }
    else
    {
      std::copy(fares.begin(), fares.end(), inserter(strategy->fares(), strategy->fares().begin()));
    }
  }
  LOG4CXX_INFO(logger, " Leaving StrategyBuilder::buildStrategy()");
  return strategy;
}

//------------------------------------------------------
// get  Grouping Data
//------------------------------------------------------

bool
StrategyBuilder::getGroupingData(FareDisplayTrx& trx, std::vector<Group*>& groups) const
{
  GroupingDataRetriever retriever(trx);
  retriever.getGroupAndSortPref(groups);
  return groups.empty() == false;
}

//------------------------------------------------------
// get Brand Data
//------------------------------------------------------

bool
StrategyBuilder::getBrandData(FareDisplayTrx& trx, std::vector<Group*>& groups) const
{
  if (TrxUtil::isFqS8BrandedServiceActivated(trx) &&
      !fallback::fallbackBrandedFaresFareDisplay(&trx) && trx.isS8ServiceCalled())
  {
    if (trx.bfErrorCode() != PricingTrx::NO_ERROR || trx.brandedMarketMap().empty())
      return false;

    S8Brand s8;
    s8.initializeS8BrandGroup(trx, groups);
    // call T189 right here to validate all paxtypefares
    if (trx.getRequest()->diagnosticNumber() == Diagnostic889)
    {
      trx.diagnostic().diagnosticType() =
          static_cast<DiagnosticTypes>(trx.getRequest()->diagnosticNumber());
    }

    BrandedFareValidatorFactory brandSelectorFactory(trx);
    BrandedFaresSelector brandedFaresSelector(trx, brandSelectorFactory);
    brandedFaresSelector.validate(trx.allPaxTypeFare());

    return groups.empty() == false;
  }
  else
  {
    BrandDataRetriever retriever(trx);
    retriever.getBrandData(groups);
    return groups.empty() == false;
  }
}

//------------------------------------------------------
// StrategyBuilder::selectStrategy()
//------------------------------------------------------

bool
StrategyBuilder::selectStrategy(FareDisplayTrx& trx, GroupingStrategy*& strategy) const
{
  std::vector<Group*> groups;

  if (!fallback::fallbackFareDisplayByCabinActivation(&trx) &&
      trx.getRequest()->inclusionNumber(trx.getRequest()->requestedInclusionCode()) != 0)
    invokeCabinGroup(trx, groups);

  bool isSortOverride = applySortOverride(trx);
  bool isGroupOverride = trx.getRequest()->isPaxTypeRequested();

  if (isGroupOverride && isSortOverride)
  {
    OverrideGroupingStrategy* overrideStrategy(nullptr);
    trx.dataHandle().get(overrideStrategy);
    overrideStrategy->groups() = groups;
    overrideStrategy->_trx = &trx;
    strategy = overrideStrategy;
  }
  else if (isSortOverride)
  {
    // lint --e{413}
    DefaultGroupingStrategy* defaultStrategy(nullptr);
    trx.dataHandle().get(defaultStrategy);
    defaultStrategy->_trx = &trx;
    strategy = defaultStrategy;
  }
  else if (FareDisplayUtil::isBrandGrouping(trx) && getBrandData(trx, groups)) // CBAS or S8 call
  {
    getGroupingData(trx, groups);
    PreferredGroupingStrategy* prefrdStrategy(nullptr);
    trx.dataHandle().get(prefrdStrategy);
    prefrdStrategy->groups() = groups;
    prefrdStrategy->_trx = &trx;
    strategy = prefrdStrategy;
  }
  else if (getGroupingData(trx, groups))
  {
    // lint --e{413}
    PreferredGroupingStrategy* prefrdStrategy(nullptr);
    trx.dataHandle().get(prefrdStrategy);
    prefrdStrategy->groups() = groups;
    prefrdStrategy->_trx = &trx;
    strategy = prefrdStrategy;
  }
  else if (isGroupOverride)
  {
    // lint --e{413}
    PreferredGroupingStrategy* prefrdStrategy(nullptr);
    trx.dataHandle().get(prefrdStrategy);
    prefrdStrategy->groups() = groups;
    prefrdStrategy->_trx = &trx;
    strategy = prefrdStrategy;
  }
  else
  {
    LOG4CXX_INFO(logger,
                 " Selecting Default Grouping Strategy as the user doesnt meet any "
                 "grouping criteria in StrategyBuilder::getStrategy()");
    // lint --e{413}
    DefaultGroupingStrategy* defaultStrategy(nullptr);
    trx.dataHandle().get(defaultStrategy);
    defaultStrategy->_trx = &trx;
    strategy = defaultStrategy;
  }
  return (strategy != nullptr);
}

//------------------------------------------------------
// StrategyBuilder::applySortOverride()
//------------------------------------------------------
bool
StrategyBuilder::applySortOverride(FareDisplayTrx& trx) const
{
  if ((trx.getOptions()->sortAscending() || trx.getOptions()->sortDescending() ||
       trx.getRequest()->numberOfFareLevels() > 0))
  {
    return true;
  }

  Customer* customer = trx.getRequest()->ticketingAgent()->agentTJR();
  if (!customer)
    return false;
  return groupOverrideCfg.getValue().has(customer->ssgGroupNo());
}

void
StrategyBuilder::invokeCabinGroup(FareDisplayTrx& trx, std::vector<Group*>& groups) const
{
  CabinGroup cabin;
  cabin.initializeCabinGroup(trx, groups);
}
}
