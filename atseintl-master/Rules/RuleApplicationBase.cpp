//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "Rules/RuleApplicationBase.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FareUsage.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/FlexFaresValidationPolicy.h"
#include "Rules/RuleUtil.h"

namespace tse
{
FALLBACK_DECL(apo36040Cat6TSI5Check);
using namespace std;

Logger
RuleApplicationBase::_logger("atseintl.Rules.RuleApplication");

namespace
{

struct SetResult
{
  SetResult(Record3ReturnTypes res) : _res(res) {}

  void operator()(Record3ReturnTypes& result) const
  {
    if (result == PASS)
      result = _res;
  }

private:
  Record3ReturnTypes _res;
};

bool
compareTravelSegs(const TravelSeg* ts1, const TravelSeg* ts2)
{

  static Logger logger("atseintl.Rules.compareTravelSeg");
  /*
    LOG4CXX_DEBUG(_logger, "In compare travel segs");
    LOG4CXX_DEBUG(_logger,"TS1 orig airport " << ts1->origAirport().c_str());
    LOG4CXX_DEBUG(_logger,"TS2 orig airport " << ts2->origAirport().c_str());
    LOG4CXX_DEBUG(_logger,"TS1 dest airport " << ts1->destAirport().c_str());
    LOG4CXX_DEBUG(_logger,"TS2 dest airport " << ts2->destAirport().c_str());
    LOG4CXX_DEBUG(_logger,"TS1 dep date " << ts1->departureDT().toSimpleString().c_str());
    LOG4CXX_DEBUG(_logger, "TS2 dep date %s",ts2->departureDT().toSimpleString().c_str());
  */

  if (ts1 == ts2)
  {
    LOG4CXX_DEBUG(logger, "MIN STAY FOUND MATCHING TRAVEL SEG");
    return true;
  }
  return false;
}

} // namespace

//-----------------------------------------------------------------------------
//   @method validateDataUnavailableTag
//
//   Description: Checks the data unavailable tag for either X or Y. If
//                the value is X then FAIL this record. If the value is Y
//                or text only then SKIP this record.
//
//   @param Indicator  - data unavailable tag , values are either 'X' or 'Y'
//
//   @return Record3ReturnTypes - possible values are:
//                                 FAIL          = 2
//                                 PASS          = 3
//                                 SKIP          = 4
//-----------------------------------------------------------------------------
Record3ReturnTypes
RuleApplicationBase::validateUnavailableDataTag(Indicator dataUnavailableTag) const
{
  if (UNLIKELY(dataUnavailableTag == dataUnavailable))
    return FAIL; // Yes, fail this fare

  if (dataUnavailableTag == textOnly)
    return SKIP; // Yes, skip this category

  return PASS;
}

//-----------------------------------------------------------------------------
//   @method removeGeoTravelSegs
//
//   Description: Checks whether any of the Geo Travel Segments returned
//                from validateGeoRuleItem are contained within the FareUsage.
//                If any of them are they are erased from the TravelSegWrapperVector.
//
//   @param RuleUtil::TravelSegWrapperVector - applTravelSegment
//   @param FareUsage                        - current fare usage
//
//   @return bool - true - yes, travel segments were erased from the TravelSegWrapperVector,
//                  else false. User needs to check size of vector after return from this
//                  method.
//-----------------------------------------------------------------------------
bool
RuleApplicationBase::removeGeoTravelSegs(RuleUtil::TravelSegWrapperVector& applTravelSegment,
                                         const FareUsage& fareUsage,
                                         const PricingUnit& pu,
                                         const int16_t& tsi,
                                         PricingTrx* trx)
{
  bool retVal = false;

  LOG4CXX_DEBUG(_logger, "Entered RuleApplicationBase::removeGeoTravelSegs");
  RuleUtil::TravelSegWrapperVectorI tsWI = applTravelSegment.begin();
  const std::vector<TravelSeg*>& fuTravelSegs = fareUsage.travelSeg();

  for (; tsWI != applTravelSegment.end(); tsWI++)
  {
    const RuleUtil::TravelSegWrapper* ts = (*tsWI);
    const TravelSeg* tvSeg = ts->travelSeg();

    std::vector<TravelSeg*>::const_iterator fuI = find_if(
        fuTravelSegs.begin(), fuTravelSegs.end(), bind2nd(ptr_fun(compareTravelSegs), tvSeg));

    if (fuI != fuTravelSegs.end())
    {
      RuleUtil::TravelSegWrapperVectorI tsWrapperI;

      if ((tsi == 5 || tsi == 17 || tsi == 34 || tsi == 35 || tsi == 55 || tsi == 56 ||
           tsi == 61) &&
          (pu.geoTravelType() == GeoTravelType::Domestic || pu.geoTravelType() == GeoTravelType::ForeignDomestic) &&
          (fareUsage.paxTypeFare()->fareMarket()->direction() == FMDirection::UNKNOWN))
      {
        continue;
      }

      if (fareUsage.isOutbound() && (pu.puType() != PricingUnit::Type::OPENJAW))
      {
        // apo-36040:the fareusage directionality  is not set correctly for circle trip pus.
        // atpco directionality is that all fcs earlier to pu turnaround are outbound and the
        // rest are inbound. Our code sets fare usage directionality by iata rules in ct pus.
        // we get the directionality by atpco rules
        if ( trx  && !fallback::apo36040Cat6TSI5Check(trx))
        {
           if ( (pu.puType() == PricingUnit::Type::CIRCLETRIP)  &&
                (pu.geoTravelType() != GeoTravelType::UnknownGeoTravelType) )
           {
              Directionality  atpcoFUDirectionality = FROM;
              for (const FareUsage* fuP : pu.fareUsage() )
              {
                 if (fuP->travelSeg().front() == pu.turnAroundPoint())
                    atpcoFUDirectionality = TO; // all fcs starting at this fu are inbound.
                 if (fuP == &fareUsage)
                    break;
              }
              if (atpcoFUDirectionality == TO) //inbound
                 continue;
           }
        }
        tsWrapperI =
            applTravelSegment.erase(remove(applTravelSegment.begin(), applTravelSegment.end(), ts),
                                    applTravelSegment.end());

        LOG4CXX_DEBUG(_logger, "Container size: " << applTravelSegment.size());
        retVal = true;

        if (tsWrapperI == applTravelSegment.end())
          break;
      }
    }
  }

  LOG4CXX_DEBUG(_logger, "Leaving RuleApplicationBase::removeGeoTravelSegs");

  return retVal;
}

//-----------------------------------------------------------------------
//   @method isDomesticUSCAOrTransborder
//
//   Description: Check if Itin is domestic US/CA or Transborder.
//
//   @param const Itin&    itin
//   @param const PricingTrx&    trx
//
//   @return bool - possible values are:
//            true     valid
//            false    invalid
//-----------------------------------------------------------------------
bool
RuleApplicationBase::isDomesticUSCAOrTransborder(const Itin& itin, PricingTrx& trx) const
{
  GeoTravelType itinTravelType;
  ShoppingTrx* shoppingTrx = dynamic_cast<ShoppingTrx*>(&trx);
  FareDisplayTrx* fareDisplayTrx = dynamic_cast<FareDisplayTrx*>(&trx);

  if (LIKELY(!fareDisplayTrx))
  {
    if (shoppingTrx != nullptr)
      itinTravelType = shoppingTrx->journeyItin()->geoTravelType();
    else
      itinTravelType = itin.geoTravelType();

    if (itinTravelType == GeoTravelType::Domestic || itinTravelType == GeoTravelType::Transborder)
      return true;
  }

  return false;
}

const DateOverrideRuleItem*
RuleApplicationBase::getDateOverrideRuleItem(PricingTrx& trx,
                                             const VendorCode& vendorCode,
                                             const uint32_t& overrideDateTblItemNo) const
{
  DataHandle dataHandle(trx.ticketingDate());
  dataHandle.setParentDataHandle(&trx.dataHandle());

  const std::vector<DateOverrideRuleItem*>& dorItemList =
      dataHandle.getDateOverrideRuleItem(vendorCode, overrideDateTblItemNo);

  return dorItemList.empty() ? nullptr : dorItemList.back();
}

bool
RuleApplicationBase::validateDateOverrideRuleItem(std::vector<Record3ReturnTypes>& r3ReturnTypes,
                                                  const std::vector<DateTime>& uniqueTvlDates,
                                                  PricingTrx& trx,
                                                  const VendorCode& vendorCode,
                                                  const uint32_t& overrideDateTblItemNo,
                                                  DiagCollector* diag,
                                                  const DiagnosticTypes& callerDiag)
{
  bool diagEnabled = false;

  if (UNLIKELY(diag && (callerDiag != DiagnosticNone) &&
               (callerDiag == trx.diagnostic().diagnosticType())))
  {
    diag->enable(callerDiag);
    diagEnabled = true;
    diag->setf(std::ios::left, std::ios::adjustfield);
    (*diag) << "  -: TABLE 994 -OVERRIDE DATE DATA :- " << vendorCode << " - "
            << overrideDateTblItemNo << endl;
  }

  const DateOverrideRuleItem* dorItem =
      getDateOverrideRuleItem(trx, vendorCode, overrideDateTblItemNo);
  if (!dorItem || dorItem->tktEffDate().date() > trx.ticketingDate().date() ||
      dorItem->tktDiscDate().date() < trx.ticketingDate().date() ||
      dorItem->resEffDate().date() > trx.ticketingDate().date() ||
      dorItem->resDiscDate().date() < trx.ticketingDate().date() ||
      dorItem->tvlEffDate().date() > uniqueTvlDates.back().date() ||
      dorItem->tvlDiscDate().date() < uniqueTvlDates.front().date())
  {
    if (UNLIKELY(diagEnabled))
    {
      (*diag) << "  TABLE 994: NOT MATCH ALL DATES" << endl;
      diag->flushMsg();
    }
    RuleApplicationBase::setResultToAllDates(r3ReturnTypes, SKIP);
    return false;
  }

  std::vector<DateTime>::const_iterator tvlDateIt = uniqueTvlDates.begin();
  std::vector<Record3ReturnTypes>::iterator r3RetIt = r3ReturnTypes.begin();

  bool foundAtLeastOneValidDate = false;
  for (; tvlDateIt != uniqueTvlDates.end() && r3RetIt != r3ReturnTypes.end();
       ++tvlDateIt, ++r3RetIt)
  {
    if (*r3RetIt == PASS)
    {
      if (dorItem->tvlEffDate().date() > tvlDateIt->date() ||
          dorItem->tvlDiscDate().date() < tvlDateIt->date())
        *r3RetIt = SKIP;
      else
        foundAtLeastOneValidDate = true;
    }
  }

  if (UNLIKELY(diagEnabled))
  {
    tvlDateIt = uniqueTvlDates.begin();
    for (; tvlDateIt != uniqueTvlDates.end(); ++tvlDateIt)
    {
      if (dorItem->tvlEffDate().date() > tvlDateIt->date() ||
          dorItem->tvlDiscDate().date() < tvlDateIt->date())
        (*diag) << "  TABLE 994 TVL DATE " << tvlDateIt->dateToSimpleString() << ": NOT MATCH"
                << endl;
      else
        (*diag) << "  TABLE 994 TVL DATE " << tvlDateIt->dateToSimpleString() << ": PASS" << endl;
    }
    diag->flushMsg();
  }
  return foundAtLeastOneValidDate;
}

void
RuleApplicationBase::setResultToAllDates(std::vector<Record3ReturnTypes>& r3ReturnTypes,
                                         Record3ReturnTypes res)
{
  std::for_each(r3ReturnTypes.begin(), r3ReturnTypes.end(), SetResult(res));
}

bool
RuleApplicationBase::isValidationNeeded(const uint16_t category, PricingTrx& trx) const
{
  return (trx.isFlexFare() && hasChancelor() && _chancelor->hasPolicy(category) &&
          _chancelor->getPolicy(category).shouldPerform(_chancelor->getContext()));
}

bool
RuleApplicationBase::shouldReturn(const uint16_t category) const
{
  return (hasChancelor() && _chancelor->getPolicy(category).shouldReturn());
}

void
RuleApplicationBase::updateStatus(const uint16_t category, const Record3ReturnTypes& result)
{
  if (!hasChancelor() || _chancelor->getContext()._paxTypeFare == nullptr)
    return;

  _chancelor->getMutableMonitor().notify(
      RuleValidationMonitor::VALIDATION_RESULT,
      _chancelor->getContext(),
      _chancelor->getContext()._paxTypeFare->getMutableFlexFaresValidationStatus(),
      category,
      result);
}

} // tse
