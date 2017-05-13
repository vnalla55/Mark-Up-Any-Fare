//----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//
//----------------------------------------------------------------------------

#include "Common/RtwUtil.h"

#include "DataModel/AncRequest.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"


#include <algorithm>

namespace tse
{
namespace RtwUtil
{
ScopedRtwDisabler::ScopedRtwDisabler(PricingTrx& trx)
  : _prOpt(trx.getOptions()), _oldValue(_prOpt && _prOpt->PricingOptions::isRtw())
{
  if (_prOpt)
    _prOpt->setRtw(false);
}

ScopedRtwDisabler::~ScopedRtwDisabler()
{
  if (_prOpt)
    _prOpt->setRtw(_oldValue);
}

bool
isRtwArunk(const PricingTrx& trx, const TravelSeg* ts)
{
  if (!RtwUtil::isRtw(trx))
    return false;

  if (!ts)
    return false;

  const ArunkSeg* arunk = ts->toArunkSeg();
  return arunk && arunk->isRtwDynamicSupplementalArunk();
}

bool
isRtwFareMarket(const Itin& itin, const FareMarket* fm)
{
  const std::vector<TravelSeg*>& fmSegs = fm->travelSeg();

  if (fmSegs.empty() || itin.travelSeg().size() != fmSegs.size())
    return false;

  return fmSegs.front()->origAirport() == fmSegs.back()->destAirport();
}

FareMarket*
getFirstRtwFareMarket(const Itin& itin)
{
  for (FareMarket* fm : itin.fareMarket())
  {
    if (isRtwFareMarket(itin, fm))
      return fm;
  }
  return nullptr;
}

bool
isRtwAncillaryRequest(const AncRequest& req)
{
  typedef AncRequest::FareBreakAssocPerItinMap FbaMap;
  typedef AncRequest::AncFareBreakAssociation Fba;

  for (const FbaMap::value_type& itinFba : req.fareBreakAssociationPerItin())
  {
    const std::vector<Fba*>& fbaVec = itinFba.second;

    if (fbaVec.empty())
      continue;

    return std::all_of(fbaVec.cbegin(),
                       fbaVec.cend(),
                       [](const Fba* fba)
                       { return fba->fareComponentID() == 1; });
  }

  return false;
}

} // ns RtwUtil
} // ns tse


