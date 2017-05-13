//----------------------------------------------------------------------------
//  Copyright Sabre 2016
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

#include "Shopping/Cat31RestrictionMerger.h"

#include "Common/ExchShopCalendarUtils.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FareMarket.h"
#include "Rules/VoluntaryChanges.h"
#include "Common/ShoppingRexUtil.h"
#include "Common/FallbackUtil.h"

#include <boost/range/adaptors.hpp>

#include <algorithm>
#include <functional>

namespace tse {

FALLBACK_DECL(changeIndOpt)
FALLBACK_DECL(exsCalendar)
FALLBACK_DECL(exscCat31MergeCalendarRange)
FALLBACK_DECL(exscSortOADResponse)

void
Cat31RestrictionMerger::operator_impl_OLD(const RexShoppingTrx::OADDataPair& in)
{
  const FareMarket* fareMarket = in.first;
  const R3SeqsConstraintMap& oadData = in.second;

  auto& oadMergedResponse = _oadResponseData[fareMarket];
  oadMergedResponse.resize(1);

  auto rec3Constraint = [&]
    {
      R3SeqsConstraintVec result;
      for (const auto& pair : oadData)
        for (const auto& constraint : pair.second)
          result.push_back(&constraint);
      return result;
    }();

  mergePortion(rec3Constraint, oadMergedResponse.front());

  mergeForcedConnection(rec3Constraint, oadMergedResponse.front());

  mergeFirstBreakRestr(rec3Constraint, oadMergedResponse.front());

  mergeFlightNumber(rec3Constraint, oadMergedResponse.front());

  mergeFareByteCxrAppl(rec3Constraint, oadMergedResponse.front());

  mergeOutboundPortion(rec3Constraint, oadMergedResponse.front());

  if (!fallback::changeIndOpt(&_trx))
  {
    mergeChangeInd(rec3Constraint, oadMergedResponse.front(), *fareMarket);
  }
}

std::vector<std::vector<const RexShoppingTrx::R3SeqsConstraint*>>
Cat31RestrictionMerger::divideConstraintsToCorrectRanges(
    const RexShoppingTrx::R3SeqsConstraintMap& oadData)
{
  std::vector<std::vector<const RexShoppingTrx::R3SeqsConstraint*>> result;
  result.resize(ExchShopCalendar::DATE_APPLICATION_MAX_SIZE);

  for (const auto& pair : oadData)
    for (const auto& constraint : pair.second)
      result[constraint.calendarAppl].push_back(&constraint);

  auto& wholePeriod = result[ExchShopCalendar::WHOLE_PERIOD];
  auto& sameDep = result[ExchShopCalendar::SAME_DEPARTURE_DATE];
  auto& laterDep = result[ExchShopCalendar::LATER_DEPARTURE_DATE];

  for (const auto* r3SeqConstraint : wholePeriod)
  {
    sameDep.push_back(r3SeqConstraint);
    laterDep.push_back(r3SeqConstraint);
  }

  auto newEnd = std::remove_if(result.begin(), result.end(), [](auto& v)
                               {
                                 return v.empty();
                               });

  newEnd = std::unique(result.begin(), newEnd, [](auto& v1, auto& v2)
                       {
                         return v1.front()->calendarAppl == v2.front()->calendarAppl;
                       });

  result.erase(newEnd, result.end());

  return result;
}

void
Cat31RestrictionMerger::operator_impl(const RexShoppingTrx::OADDataPair& in)
{
  const FareMarket* fareMarket = in.first;
  const RexShoppingTrx::R3SeqsConstraintMap& oadData = in.second;

  std::vector<RexShoppingTrx::OADResponseData>& oadMergedResponseVec = _oadResponseData[fareMarket];

  auto rec3Constraints = divideConstraintsToCorrectRanges(oadData);
  size_t constraintSize = rec3Constraints.size();
  if (!constraintSize)
    rec3Constraints.resize(++constraintSize);

  oadMergedResponseVec.resize(constraintSize);
  for (std::size_t i = 0; i < constraintSize; ++i)
  {
    mergePortion(rec3Constraints[i], oadMergedResponseVec[i]);
    mergeForcedConnection(rec3Constraints[i], oadMergedResponseVec[i]);
    mergeFirstBreakRestr(rec3Constraints[i], oadMergedResponseVec[i]);
    mergeFlightNumber(rec3Constraints[i], oadMergedResponseVec[i]);
    mergeFareByteCxrAppl(rec3Constraints[i], oadMergedResponseVec[i]);
    mergeOutboundPortion(rec3Constraints[i], oadMergedResponseVec[i]);
    if (!fallback::changeIndOpt(&_trx))
    {
      mergeChangeInd(rec3Constraints[i], oadMergedResponseVec[i], *fareMarket);
    }
  }

  mergeCalendarRange(rec3Constraints, oadMergedResponseVec);

  if (!fallback::exscSortOADResponse(&_trx))
  {
    std::sort(oadMergedResponseVec.begin(), oadMergedResponseVec.end(),
              [](const OADResponseData& lhs, const OADResponseData& rhs)
              {
                return lhs.calendarRange < rhs.calendarRange;
              });
  }
}

void
Cat31RestrictionMerger::operator()(const RexShoppingTrx::OADDataPair& in)
{
  /* on fallback removal:
   * - move operator_impl body to Cat31RestrictionMerger::operator()
   */
  if (!fallback::exsCalendar(&_trx))
    operator_impl(in);
  else
    operator_impl_OLD(in);
}

void
Cat31RestrictionMerger::mergeForcedConnection(const R3SeqsConstraintVec& rec3Constraint,
                           RexShoppingTrx::OADResponseData& oadMergedResponse)
{
  std::set<LocCode>& resultCnxSet = oadMergedResponse.forcedConnections;

  for (const auto* r3SeqConstraint : rec3Constraint)
  {
    const std::set<LocCode>& recConnections = r3SeqConstraint->forcedConnection;
    if (recConnections.empty())
    {
      resultCnxSet.clear();
      break;
    }
    else if (resultCnxSet.empty())
    {
      resultCnxSet = recConnections;
    }
  }
}

void
Cat31RestrictionMerger::mergeFirstBreakRestr(const R3SeqsConstraintVec& rec3Constraint,
                          RexShoppingTrx::OADResponseData& oadMergedResponse)

{

  bool isFirstBreakRest = false;
  for (const auto* r3SeqConstraint : rec3Constraint)
  {

    if (r3SeqConstraint->firstBreakStatus)
    {
      isFirstBreakRest = true;
      break;
    }
  }

  oadMergedResponse.firstBreakRest = isFirstBreakRest;
}

void
Cat31RestrictionMerger::mergePortion(const R3SeqsConstraintVec& rec3Constraint,
                  RexShoppingTrx::OADResponseData& oadMergedResponse)
{
  std::vector<RexShoppingTrx::PortionMergeTvlVectType> portionMergeTvlVect;
  for (const auto* r3SeqConstraint : rec3Constraint)
  {
    portionMergeTvlVect.push_back(r3SeqConstraint->portionMerge);
  }

  ShoppingRexUtil::mergePortion(portionMergeTvlVect, oadMergedResponse.portion);
}

void
Cat31RestrictionMerger::mergeFlightNumber(const R3SeqsConstraintVec& rec3Constraint,
                       RexShoppingTrx::OADResponseData& oadMergedResponse)
{
  oadMergedResponse.flightNumberRestriction = rec3Constraint.empty() ? false :
                                              !std::any_of(rec3Constraint.begin(), rec3Constraint.end(),
                                              [](const auto* r3SeqConstraint)
                                              {
                                                return !r3SeqConstraint->flightNumberRestriction;
                                              });
}

void
Cat31RestrictionMerger::mergeFareByteCxrAppl(const R3SeqsConstraintVec& rec3Constraint,
                          RexShoppingTrx::OADResponseData& oadMergedResponse)
{

  RexShoppingTrx::FareByteCxrApplVect fareByteCxrApplVect;

  for (const auto* r3SeqConstraint : rec3Constraint)
  {
    fareByteCxrApplVect.push_back(r3SeqConstraint->fareByteCxrAppl);
  }

  RexShoppingTrx::FareByteCxrAppl fareByteCxrAppl;
  ShoppingRexUtil::mergeFareByteCxrApplRestrictions(fareByteCxrApplVect, fareByteCxrAppl);

  RexShoppingTrx::FareByteCxrApplData fareByteCxrApplData;
  const std::set<CarrierCode>& restrictedCxrAppl = fareByteCxrAppl.restCxr;
  const std::set<CarrierCode>& applicableCxrAppl = fareByteCxrAppl.applCxr;
  if (restrictedCxrAppl.empty())
  {
    if (applicableCxrAppl.count(DOLLAR_CARRIER) != 0)
    {
      // REST are empty and APPL has all CXR's
      fareByteCxrApplData.excluded = false;
    }
    else
    {
      // REST are empty and APPL has specific CXR's
      fareByteCxrApplData.cxrList = applicableCxrAppl;
      fareByteCxrApplData.excluded = false;
    }
  }
  else
  {
    // REST has specific CXR's
    fareByteCxrApplData.cxrList = restrictedCxrAppl;
    fareByteCxrApplData.excluded = true;
  }

  oadMergedResponse.fareByteCxrAppl = fareByteCxrApplData;
}

void
Cat31RestrictionMerger::mergeOutboundPortion(const R3SeqsConstraintVec& rec3Constraint,
                          RexShoppingTrx::OADResponseData& oadMergedResponse)
{
  std::vector<RexShoppingTrx::PortionMergeTvlVectType> rec3OutboundPortion;

  for (const auto* r3SeqConstraint : rec3Constraint)
    rec3OutboundPortion.push_back(r3SeqConstraint->outboundPortion);

  ShoppingRexUtil::mergePortion(rec3OutboundPortion, oadMergedResponse.outboundPortion);
}

void
Cat31RestrictionMerger::mergeChangeInd(const R3SeqsConstraintVec& rec3Constraint,
                    OADResponseData& oadMergedResponse,
                    const FareMarket& fareMarket)
{
  ChangeInd changeInd = J_RESTR;

  for (const auto& r3SeqConstraint : rec3Constraint)
  {
    changeInd = ChangeInd(changeInd & convert(r3SeqConstraint->changeInd));
  }

  switch(changeInd)
  {
  case J_RESTR:
    for (const PricingUnit* pu : _itin.farePath().front()->pricingUnit())
    {
      for (const FareUsage* fu : pu->fareUsage())
      {
        for (const TravelSeg* seg : fu->paxTypeFare()->fareMarket()->travelSeg())
        {
          oadMergedResponse.portion.push_back(seg->pnrSegment());
        }
      }
    }
    break;
  case PU_RESTR:
  {
    const PricingUnit* pu = getPricingUnit(*_itin.farePath().front(), fareMarket);
    for (const FareUsage* fu : pu->fareUsage())
    {
      for (const TravelSeg* seg : fu->paxTypeFare()->fareMarket()->travelSeg())
      {
        oadMergedResponse.portion.push_back(seg->pnrSegment());
      }
    }
    break;
  }
  case FC_RESTR:
    for (const TravelSeg* seg : fareMarket.travelSeg())
    {
      oadMergedResponse.portion.push_back(seg->pnrSegment());
    }
    break;
  case NO_RESTR:
    break;
  }
}

void
Cat31RestrictionMerger::mergeCalendarRange(const std::vector<R3SeqsConstraintVec>& rec3Constraints,
                                           std::vector<OADResponseData>& oadMergedResponseVec)
{
  if (oadMergedResponseVec.empty() || rec3Constraints.empty())
    return;

  for (std::size_t i = 0; i < oadMergedResponseVec.size(); ++i)
    oadMergedResponseVec[i].calendarRange = rec3Constraints[i].front()->calendarRange;

  if (fallback::exscCat31MergeCalendarRange(&_trx))
  {
    auto current = oadMergedResponseVec.begin();
    auto next = std::next(current);

    for (; next != oadMergedResponseVec.end(); ++next, ++current)
    {
      auto& currRange = current->calendarRange;
      auto& nextRange = next->calendarRange;

      if (nextRange.firstDate < currRange.firstDate)
      {
        nextRange.firstDate = currRange.lastDate.nextDay();
      }
      currRange.lastDate = nextRange.firstDate.subtractDays(1);
    }
  }
  else
  {
    std::map<std::reference_wrapper<const std::set<CarrierCode>>,
             std::vector<OADResponseData>,
             std::less<std::set<CarrierCode>>> carriersToOADMap;

    for (const OADResponseData& oad : oadMergedResponseVec)
    {
      carriersToOADMap[oad.fareByteCxrAppl.cxrList].push_back(oad);
    }

    auto areAlmostTheSame = [](OADResponseData& lhs, OADResponseData& rhs) -> bool
    { return lhs.portion == rhs.portion && lhs.outboundPortion == rhs.outboundPortion; };

    for (auto& oadVec : carriersToOADMap | boost::adaptors::map_values)
    {
      std::sort(oadVec.begin(), oadVec.end(),
                [&](const OADResponseData& lhsOAD, const OADResponseData& rhsOAD)
                {
                  return lhsOAD.calendarRange < rhsOAD.calendarRange;
                });

      auto current = oadVec.begin();
      auto next = std::next(current);

      auto getFirstDate = [](auto& oad) -> auto & { return oad.calendarRange.firstDate; };
      auto getLastDate = [](auto& oad) -> auto & { return oad.calendarRange.lastDate; };

      while (next != oadVec.end())
      {
        if (areAlmostTheSame(*current, *next) &&
            getLastDate(*current).nextDay() >= getFirstDate(*next))
        {
          getLastDate(*current) = std::max(getLastDate(*current), getLastDate(*next));
          next = oadVec.erase(next);
        }
        else
        {
          ++current;
          ++next;
        }
      }
    }

    oadMergedResponseVec.clear();
    bool isEXSCalendar = ExchShopCalendar::isEXSCalendar(_trx);
    for (auto& oadVec : carriersToOADMap | boost::adaptors::map_values)
    {
      if (!isEXSCalendar)
        for (auto& oad : oadVec)
          oad.calendarRange.reset();

      oadMergedResponseVec.insert(oadMergedResponseVec.cbegin(), oadVec.begin(), oadVec.end());
    }
  }
}

Cat31RestrictionMerger::ChangeInd
Cat31RestrictionMerger::convert(Indicator changeInd) const
{
  switch(changeInd)
  {
  case VoluntaryChanges::NOT_APPLY:
    return ChangeInd::NO_RESTR;
  case VoluntaryChanges::NOT_PERMITTED:
    return ChangeInd::FC_RESTR;
  case VoluntaryChanges::CHG_IND_P:
    return ChangeInd::PU_RESTR;
  case VoluntaryChanges::CHG_IND_J:
    if (_oneCarrierTicket)
      return ChangeInd::J_RESTR;
    else
      return ChangeInd::PU_RESTR;
  }
  return ChangeInd::NO_RESTR;
}

const PricingUnit*
Cat31RestrictionMerger::getPricingUnit(const FarePath& fp, const FareMarket& fareMarket) const
{
  for (const PricingUnit* pu : fp.pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      if (fu->paxTypeFare()->fareMarket() == &fareMarket)
        return pu;
    }
  }
  TSE_ASSERT(!"PricingUnit must exist");
}

} // tse
