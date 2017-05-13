//-------------------------------------------------------------------
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Rules/CommissionsProgramValidator.h"

#include "DataModel/Agent.h"
#include "DataModel/FareUsage.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "Diagnostic/Diag867Collector.h"
#include "Rules/RuleUtil.h"
#include "Pricing/FarePathUtils.h"
#include "Util/BranchPrediction.h"

namespace tse
{
bool
CommissionsProgramValidator::isCommissionProgramMatched(const CommissionProgramInfo& cpi,
                                                        const CarrierCode& valCxr)
{
  CommissionValidationStatus rc = validateCommissionProgram(cpi, valCxr);
  if (_diag867)
    _diag867->printCommissionProgramProcess(cpi, rc);

  return (rc == PASS_CR);
}

CommissionValidationStatus
CommissionsProgramValidator::validateCommissionProgram(const CommissionProgramInfo& cpi,
                                                       const CarrierCode& valCxr)
{
  CommissionValidationStatus rc = PASS_CR;

  if (notValidToValidate())
  {
    return FAIL_CP_NOT_VALID;
  }
  if (!matchTicketingDates(cpi))
  {
    return FAIL_CP_TICKET_DATE;
  }
  if (!matchTravelDates(cpi))
  {
    return FAIL_CP_TRAVEL_DATE;
  }
  if (!matchPointOfSale(cpi, valCxr))
  {
    return FAIL_CP_POINT_OF_SALE;
  }
  if (!matchPointOfOrigin(cpi, valCxr))
  {
    return FAIL_CP_POINT_OF_ORIGIN;
  }
  if (!matchMarket(cpi, valCxr))
  {
    return FAIL_CP_MARKET;
  }
  return rc;
}

bool
CommissionsProgramValidator::matchTicketingDates(const CommissionProgramInfo& cpi) const
{
  if(!cpi.startTktDate().isValid() && !cpi.endTktDate().isValid())
    return true;

  DateTime& date = _trx.ticketingDate();

  return checkIsDateBetween(cpi.startTktDate(), cpi.endTktDate(), date);
}

bool
CommissionsProgramValidator::matchTravelDates(const CommissionProgramInfo& cpi) const
{
  // journey tvl commence
  if(cpi.travelDates().empty())
    return true;

  for(const CommissionTravelDatesSegInfo* travelDt : cpi.travelDates())
  {
    if(!travelDt)
      continue;

    DateTime& date = _fp.itin()->travelSeg().front()->departureDT();

    if(checkIsDateBetween(travelDt->firstTravelDate(), travelDt->endTravelDate(), date))
      return true;
  }
  return false;
}

bool
CommissionsProgramValidator::matchPointOfSale(const CommissionProgramInfo& cpi,
                                              const CarrierCode& valCxr)
{
  if(cpi.pointOfSale().empty())
    return true;

  for (const CommissionLocSegInfo* pos : cpi.pointOfSale())
  {
    if(!pos)
      continue;

    if(validateLocation(cpi,
                        _fu.paxTypeFare()->vendor(),
                        pos->loc(),
                        *(_trx.getRequest()->ticketingAgent()->agentLocation()),
                        valCxr))
    {
      return (pos->inclExclInd() == 'I');
    }
  }
  return false;
}

bool
CommissionsProgramValidator::matchPointOfOrigin(const CommissionProgramInfo& cpi,
                                                const CarrierCode& valCxr)
{
  if(cpi.pointOfOrigin().empty())
    return true;

  // So point of origin of the journey - we believe this will be the case- pending (waiting for Milorad)
  for (const CommissionLocSegInfo* orig : cpi.pointOfOrigin())
  {
    if(!orig)
      continue;

    if(validateLocation(cpi,
                        _fu.paxTypeFare()->vendor(),
                        orig->loc(),
                        *(_fp.itin()->travelSeg().front()->origin()),
                        valCxr))
    {
      return (orig->inclExclInd() == 'I');
    }
  }
  return false;
}

bool
CommissionsProgramValidator::matchMarket(const CommissionProgramInfo& cpi,
                                         const CarrierCode& valCxr)
{
  if(cpi.markets().empty())
    return true;

  for (const CommissionMarketSegInfo* market : cpi.markets())
  {
    if(!market)
      continue;

    if(market->origin().loc().empty() && market->destination().loc().empty())
      return true;

    if(!market->origin().loc().empty() && !market->destination().loc().empty())
    {
      if( (validateLocation(cpi,
                            _fu.paxTypeFare()->vendor(),
                            market->origin(),
                            *(_fu.travelSeg().front()->origin()),
                            valCxr)
           &&
           validateLocation(cpi, _fu.paxTypeFare()->vendor(),
                            market->destination(),
                            *(_fu.travelSeg().back()->destination()),
                            valCxr))
        ||
          (market->bidirectional() == 'Y' &&
           validateLocation(cpi, _fu.paxTypeFare()->vendor(),
                            market->destination(),
                            *(_fu.travelSeg().front()->origin()),
                            valCxr)
           &&
           validateLocation(cpi, _fu.paxTypeFare()->vendor(),
                            market->origin(),
                            *(_fu.travelSeg().back()->destination()),
                            valCxr)))
      {
        return (market->inclExclInd() == 'I');
      }
    }
    else
    if(!market->origin().loc().empty() && market->destination().loc().empty())
    {
      if( validateLocation(cpi, _fu.paxTypeFare()->vendor(),
                           market->origin(),
                           *(_fu.travelSeg().front()->origin()),
                           valCxr)
        ||
          (market->bidirectional() == 'Y' &&
           validateLocation(cpi, _fu.paxTypeFare()->vendor(),
                            market->origin(),
                            *(_fu.travelSeg().back()->destination()),
                            valCxr)))
      {
        return (market->inclExclInd() == 'I');
      }
    }
    else
    if(market->origin().loc().empty() && !market->destination().loc().empty())
    {
      if( validateLocation(cpi, _fu.paxTypeFare()->vendor(),
                           market->destination(),
                           *(_fu.travelSeg().back()->destination()),
                           valCxr)
        ||
          (market->bidirectional() == 'Y' &&
           validateLocation(cpi, _fu.paxTypeFare()->vendor(),
                            market->destination(),
                            *(_fu.travelSeg().front()->origin()),
                            valCxr)))
      {
        return (market->inclExclInd() == 'I');
      }
    }
  }
  return false;
}

bool
CommissionsProgramValidator::validateLocation(const CommissionProgramInfo& cpi,
                                              const VendorCode& vendor,
                                              const LocKey& locKey,
                                              const Loc& loc,
                                              const CarrierCode& carrier)
{
  if (locKey.isNull())
    return false;

  if (locKey.locType() == LOCTYPE_USER)
    return isInZone(cpi.vendor(), locKey.loc(), loc, carrier);

  return isInLoc(vendor, locKey, loc, carrier);
}

bool
CommissionsProgramValidator::isInZone(const VendorCode& vendor,
                                      const LocCode& zone,
                                      const Loc& loc,
                                      const CarrierCode& carrier)
{
  return LocUtil::isInZone(loc,
                           vendor,
                           zone,
                           USER_DEFINED,
                           LocUtil::OTHER,
                           _fu.paxTypeFare()->fareMarket()->geoTravelType(),
                           carrier,
                           _trx.ticketingDate());
}

bool
CommissionsProgramValidator::isInLoc(const VendorCode& vendor,
                                     const LocKey& locKey,
                                     const Loc& loc,
                                     const CarrierCode& carrier)
{
  return LocUtil::isInLoc(loc,
                          locKey.locType(),
                          locKey.loc(),
                          vendor,
                          RESERVED,
                          LocUtil::OTHER,
                          _fu.paxTypeFare()->fareMarket()->geoTravelType(),
                          carrier,
                          _trx.ticketingDate());
}

bool
CommissionsProgramValidator::checkIsDateBetween(DateTime startDate,
                                                DateTime endDate,
                                                DateTime& betweenDate) const
{
  DateTime& start = startDate.isInfinity() ? betweenDate : startDate;
  DateTime& end = endDate.isInfinity() ? betweenDate : endDate;

  return betweenDate >= start && betweenDate <= end;
}

bool
CommissionsProgramValidator::notValidToValidate()
{
  return (!_fp.itin() || _fp.itin()->travelSeg().empty() || !_fp.itin()->travelSeg().front() ||
          !_fp.itin()->travelSeg().front()->origin() ||
          !_trx.getRequest()->ticketingAgent()->agentLocation() || !_fu.paxTypeFare() ||
          !_fu.paxTypeFare()->fareMarket() || _fu.travelSeg().empty() || !_fu.travelSeg().front() ||
          !_fu.travelSeg().front()->origin() || !_fu.travelSeg().back() ||
          !_fu.travelSeg().back()->destination());
}

} //tse
