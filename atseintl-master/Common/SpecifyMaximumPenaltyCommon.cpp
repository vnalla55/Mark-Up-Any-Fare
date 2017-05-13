//----------------------------------------------------------------------------
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
//----------------------------------------------------------------------------

#include "Common/SpecifyMaximumPenaltyCommon.h"

#include "DataModel/FarePath.h"
#include "DataModel/MaxPenaltyInfo.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DBAccess/DateOverrideRuleItem.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/PenaltyInfo.h"
#include "DBAccess/VoluntaryChangesInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DiagManager.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Rules/VoluntaryChanges.h"
#include "Rules/RuleConst.h"
#include "Pricing/MaximumPenaltyValidator.h"

namespace tse {

namespace smp
{

namespace
{

inline bool
areFiltersEqual(const MaxPenaltyInfo::Filter& filter1,
                const MaxPenaltyInfo::Filter& filter2)
{
  return filter1._departure == filter2._departure &&
         filter1._maxFee == filter2._maxFee &&
         filter1._query == filter1._query;
}

} //namespace

RecordApplication operator|(const RecordApplication app1, const RecordApplication app2)
{
  return static_cast<RecordApplication>(static_cast<unsigned>(app1) | static_cast<unsigned>(app2));
}

RecordApplication& operator|=(RecordApplication& app1, const RecordApplication app2)
{
  return app1 = app1 | app2;
}

RecordApplication operator&(const RecordApplication app1, const RecordApplication app2)
{
  return static_cast<RecordApplication>(static_cast<unsigned>(app1) & static_cast<unsigned>(app2));
}

RecordApplication& operator&=(RecordApplication& app1, const RecordApplication app2)
{
  return app1 = app1 & app2;
}

bool
isPenaltyCalculationRequired(const PricingTrx& trx)
{
  for (const PaxType* paxType : trx.paxType())
  {
    if (paxType->maxPenaltyInfo())
    {
      return true;
    }
  }

  return false;
}

RecordApplication
getRecordApplication(const VoluntaryRefundsInfo& refundInfo, bool isFirstFc, bool isFirstPu)
{
  RecordApplication application = smp::BOTH;

  if (refundInfo.depOfJourneyInd() == DepartureAppl::BEFORE)
  {
    application = smp::BEFORE;
  }
  else if (refundInfo.depOfJourneyInd() == DepartureAppl::AFTER)
  {
    application = smp::AFTER;
  }

  if (refundInfo.fareComponentInd() == DepartureAppl::AFTER ||
      refundInfo.puInd() == DepartureAppl::AFTER)
    application &= smp::AFTER;

  bool isAdvCancelFromTo =
      refundInfo.advCancelFromTo() == AdvCancellationInd::JOURNEY ||
      (refundInfo.advCancelFromTo() == AdvCancellationInd::FARE_COMPONENT && isFirstFc) ||
      (refundInfo.advCancelFromTo() == AdvCancellationInd::PRICING_UNIT && isFirstPu);

  if ((refundInfo.fareComponentInd() == DepartureAppl::BEFORE && isFirstFc) ||
      (refundInfo.puInd() == DepartureAppl::BEFORE && isFirstPu) || isAdvCancelFromTo)
    application &= smp::BEFORE;

  return application;
}

RecordApplication
getRecordApplication(const VoluntaryChangesInfoW& changeInfo,
                     RecordApplication targetApplication,
                     bool isFirstFC,
                     bool isFirstPU)
{
  RecordApplication application = smp::BOTH;
  if (changeInfo.departureInd() == DepartureAppl::BEFORE)
  {
    application = smp::BEFORE;
  }
  else if (changeInfo.departureInd() == DepartureAppl::AFTER)
  {
    application = smp::AFTER;
  }

  auto checkByte = [&](Indicator byteValue, bool isFirst)
  {
    if (targetApplication == smp::BEFORE || (targetApplication == smp::AFTER && isFirst))
    {
      if(byteValue == DepartureAppl::BEFORE)
      {
        application = smp::BEFORE;
      }
      else if(byteValue == DepartureAppl::AFTER)
      {
        application = smp::AFTER;
      }
    }
  };

  checkByte(changeInfo.fareComponentInd(), isFirstFC);
  checkByte(changeInfo.priceableUnitInd(), isFirstPU);

  return application;
}

smp::RecordApplication
getRecordApplication(const PenaltyInfo& penaltyInfo)
{
  if (penaltyInfo.penaltyAppl() == PenaltyAppl::ANYTIME_BLANK ||
      penaltyInfo.penaltyAppl() == PenaltyAppl::ANYTIME ||
      penaltyInfo.penaltyAppl() == PenaltyAppl::ANYTIME_CHILD_DSC)
  {
    return smp::BOTH;
  }
  else if (penaltyInfo.penaltyAppl() == PenaltyAppl::BEFORE_DEP ||
           penaltyInfo.penaltyAppl() == PenaltyAppl::BEFORE_DEP_CHILD_DSC)
  {
    return smp::BEFORE;
  }
  else
  {
    return smp::AFTER;
  }
}

bool
isDepartureMatching(RecordApplication app1, RecordApplication app2)
{
  return app1 & app2;
}

bool
voluntaryChangesNotPermitted(const VoluntaryChangesInfoW& changeInfo)
{
  return changeInfo.changeInd() == VoluntaryChanges::NOT_PERMITTED ||
         changeInfo.changeInd() == VoluntaryChanges::CHG_IND_J ||
         changeInfo.changeInd() == VoluntaryChanges::CHG_IND_P;
}

void
validatePenaltyInputInformation(PricingTrx& trx)
{
  for (PaxType* paxType : trx.paxType())
  {
    if (!paxType->maxPenaltyInfo())
    {
      std::string errMsg = "MISSING PENALTY PARAMETRS FOR PASSENGER " +
                            paxType->paxType();

      throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED, errMsg.c_str());
    }
  }

  //for phase 1 all passengers have to have same penalty processing information
  if (trx.paxType().size() > 1)
  {
    const MaxPenaltyInfo& firstPax = *trx.paxType().front()->maxPenaltyInfo();

    for (auto paxIter = ++trx.paxType().begin(); paxIter < trx.paxType().end(); ++paxIter)
    {
      bool isValid = areFiltersEqual(firstPax._changeFilter,
                                     (*paxIter)->maxPenaltyInfo()->_changeFilter) &&
                     areFiltersEqual(firstPax._refundFilter,
                                     (*paxIter)->maxPenaltyInfo()->_refundFilter);
      if (!isValid)
      {
        std::string errMsg = "PENALTY PARAMETRS AREN'T SAME FOR ALL PASSENGER TYPES";
        throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED, errMsg.c_str());
      }
    }
  }
}

void
preValidateFareMarket(PricingTrx& trx, Itin& itin, FareMarket& fareMarket)
{
  RuleControllerWithChancelor<FareMarketRuleController> ruleController(
       LoadRecords, std::vector<uint16_t>(1, RuleConst::PENALTIES_RULE), &trx);
  ruleController.validate(trx, fareMarket, itin);

  MaximumPenaltyValidator calc(trx);
  calc.prevalidate16(fareMarket);
}

template <typename Record3>
bool
printRecord3(const PricingTrx& trx,
             const Record3& record,
             smp::RecordApplication departureInd,
             DiagManager& diag)
{
  bool matchFilter =
      trx.diagnostic().diagParamIsSet(Diagnostic::IDENTIFICATION, "ALL") ||
      trx.diagnostic().diagParamIsSet(Diagnostic::IDENTIFICATION, std::to_string(record.itemNo()));

  if (matchFilter)
  {
    diag << "    RECORD ID : " << record.itemNo()
         << " DEP : " << (departureInd == smp::BOTH ? '-' : toDepartureAppl(departureInd)) << "\n";
    diag << record;
  }

  return matchFilter;
}

template bool
printRecord3<PenaltyInfo>(const PricingTrx& trx,
                          const PenaltyInfo& record,
                          smp::RecordApplication departureInd,
                          DiagManager& diag);
template bool
printRecord3<VoluntaryChangesInfoW>(const PricingTrx& trx,
                                    const VoluntaryChangesInfoW& record,
                                    smp::RecordApplication departureInd,
                                    DiagManager& diag);
template bool
printRecord3<VoluntaryRefundsInfo>(const PricingTrx& trx,
                                   const VoluntaryRefundsInfo& record,
                                   smp::RecordApplication departureInd,
                                   DiagManager& diag);

bool
validateOverrideDateTable(PricingTrx& trx,
                          const VendorCode& vendor,
                          const uint32_t& overrideDateTblItemNo,
                          const DateTime& applicationDate)
{
  if (overrideDateTblItemNo == 0)
    return true;

  const std::vector<DateOverrideRuleItem*>& dorItemVec =
                                   trx.dataHandle().getDateOverrideRuleItem(vendor,
                                                                            overrideDateTblItemNo,
                                                                            applicationDate);
  if (dorItemVec.empty())
    return true;

  const DateTime& ticketingDate = trx.getRequest()->ticketingDT();

  for(const auto& dorItem: dorItemVec)
  {
    if ((!dorItem->tktEffDate().isValid() || dorItem->tktEffDate() <= ticketingDate) &&
        (!dorItem->tktDiscDate().isValid() || dorItem->tktDiscDate() >= ticketingDate))
    {
      return true;
    }
  }

  return false;
}

template <class RecordType>
bool
isPsgMatch(const PaxType& paxType, const RecordType& record)
{
  return paxType.requestedPaxType() == record.psgType() || paxType.paxType() == record.psgType() ||
         record.psgType().empty();
}

template bool
isPsgMatch<VoluntaryChangesInfoW>(const PaxType& paxType, const VoluntaryChangesInfoW& record);

template bool
isPsgMatch<VoluntaryRefundsInfo>(const PaxType& paxType, const VoluntaryRefundsInfo& record);

const std::pair<const PaxTypeFare*, const int16_t>
grabMissingDataFareInformationAndCleanUp(const FareInfo& fareInfo,
                                         std::vector<const PaxTypeFare*>& allPTFs)
{
  auto foundPTF = std::find_if(allPTFs.begin(),
                               allPTFs.end(),
                               [&fareInfo](const PaxTypeFare* ptf)
                               { return ptf && ptf->fare()->fareInfo() == &fareInfo; });

  if (foundPTF == allPTFs.end())
  {
    throw ErrorResponseException(ErrorResponseException::DATA_ERROR_DETECTED,
                                 std::string("PAX TYPE FAR NOT FOUND FOR GIVEN FAREINFO : #") +
                                     fareInfo.fareClass());
  }
  else
  {
    const PaxTypeFare* ptf = *foundPTF;
    *foundPTF = nullptr;
    return std::make_pair(ptf, foundPTF - allPTFs.begin());
  }
}

bool ptfHasCat31Rec2(const PaxTypeFare& ptf)
{
  const PaxTypeFare* ptfToCheck =
      (ptf.isFareByRule() && ptf.fareByRuleInfo().ovrdcat31() == 'B' ? ptf.baseFare() : &ptf);

  if (ptfToCheck->paxTypeFareGfrData(RuleConst::VOLUNTARY_EXCHANGE_RULE))
  {
    return true;
  }
  else
  {
    const auto* fareRule = ptfToCheck->paxTypeFareRuleData(RuleConst::VOLUNTARY_EXCHANGE_RULE);
    return fareRule && fareRule->categoryRuleItemInfoSet();
  }
}

std::string
printRecordApplication(const smp::RecordApplication& application)
{
  switch(application)
  {
  case smp::AFTER:
    return "AFTER";

  case smp::BEFORE:
    return "BEFORE";

  case smp::BOTH:
    return "BOTH";

  case smp::INVALID:
    return "INVALID";
  }
  return "N\\A";
}

CurrencyCode
getOriginCurrency(const FarePath& farePath)
{
  for (const PricingUnit* pu : farePath.pricingUnit())
    for (const FareUsage* fu : pu->fareUsage())
      if (fu->paxTypeFare()->isInternational())
        return fu->paxTypeFare()->currency();
  return farePath.pricingUnit().front()->fareUsage().front()->paxTypeFare()->currency();
}

template <typename Fees>
void
printDiagnosticFareFees(DiagManager& diag,
                        const PaxTypeFare& ptf,
                        const Fees& fees,
                        const bool additionalInfo)
{
  diag << "  " << (additionalInfo ? DiagnosticUtil::printPaxTypeFare(ptf) : ptf.createFareBasis(0))
       << " " << fees << '\n';
}

template void
printDiagnosticFareFees<MaxPenaltyResponse::Fees>(DiagManager& diag,
                                                  const PaxTypeFare& ptf,
                                                  const MaxPenaltyResponse::Fees& fees,
                                                  const bool additionalInfo);

static std::vector<const FareUsage*>
prepareFareUsageVector(const FarePath& farePath)
{
  std::vector<const FareUsage*> fareUsages;

  for (const PricingUnit* pu : farePath.pricingUnit())
    for (const FareUsage* fu : pu->fareUsage())
      fareUsages.push_back(fu);

  return fareUsages;
}

// returns vector containing PaxTypeFares in correct order
std::vector<const PaxTypeFare*> getAllPaxTypeFares(const FarePath& farePath)
{
  std::vector<const FareUsage*> fareUsages = prepareFareUsageVector(farePath);
  std::vector<const PaxTypeFare*> fares;
  fares.reserve(fareUsages.size());

  for (const FareMarket* fm : farePath.itin()->fareMarket())
  {
    auto foundFu = std::find_if(fareUsages.begin(),
                                fareUsages.end(),
                                [=](const FareUsage* fu)
                                { return fu->paxTypeFare()->fareMarket() == fm; });
    if (foundFu != fareUsages.end())
    {
      fares.push_back((*foundFu)->paxTypeFare());
    }
  }
  return fares;
}

std::vector<const PaxTypeFare*>
getAllPaxTypeFaresForSfrMultipax(const FarePath& farePath, const MultiPaxFCMapping& fcMap)
{
  std::vector<const FareUsage*> fareUsages = prepareFareUsageVector(farePath);
  std::vector<const PaxTypeFare*> fares;
  fares.reserve(fareUsages.size());

  auto fcVectorIt = fcMap.find(farePath.paxType());
  if (fcVectorIt != fcMap.end())
  {
    for (auto fc : fcVectorIt->second)
    {
      const FareMarket* fm = fc->fareMarket();
      auto foundFu = std::find_if(fareUsages.begin(),
                                  fareUsages.end(),
                                  [=](const FareUsage* fu)
                                  { return fu->paxTypeFare()->fareMarket() == fm; });
      if (foundFu != fareUsages.end())
      {
        fares.push_back((*foundFu)->paxTypeFare());
      }
    }
  }
  return fares;
}

} /* namespace smp */
} /* namespace tse */
