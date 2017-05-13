//----------------------------------------------------------------------------
//  File:        Diag688Collector.C
//  Authors:     Grzegorz Cholewiak
//  Created:     Sep 10 2007
//
//  Description: Diagnostic 689 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2007
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

#include "Diagnostic/Diag689Collector.h"

#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/ExcItin.h"
#include "DataModel/FareCompInfo.h"
#include "DataModel/FarePath.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RefundProcessInfo.h"
#include "DataModel/ReissueCharges.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/Currency.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/ReissueSequence.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "Diagnostic/FareTypeTablePresenter.h"
#include "Diagnostic/SeasonalityDOWTablePresenter.h"
#include "RexPricing/PenaltyAdjuster.h"
#include "RexPricing/RexFareBytesValidator.h"
#include "Rules/AdvResOverride.h"

namespace tse
{
FALLBACK_DECL(rexFareTypeTbl);

namespace
{
bool
checkIfOverrideNeeded(const FarePath& farePath)
{
  for (PricingUnit* pu : farePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      if (fu->paxTypeFare()->vendor() != "ATP" &&
          fu->paxTypeFare()->vendor() != "SITA")
      {
        return true;
      }
    }
  }

  return false;
}
}

const std::string Diag689Collector::PERMUTATION_ID = "ID";
const std::string Diag689Collector::RANGE_START = "RS";
const std::string Diag689Collector::RANGE_END = "RE";
const std::string Diag689Collector::TABLE_ITEMNO = "TI";
const std::string Diag689Collector::TABLE_SEQNO = "SN";
const std::string Diag689Collector::ADV_RES = "C5";

void
Diag689Collector::initialize()
{
  std::string diagRangeMin = rootDiag()->diagParamMapItem(RANGE_START);
  std::string diagRangeMax = rootDiag()->diagParamMapItem(RANGE_END);
  std::string diagPermIndex = rootDiag()->diagParamMapItem(PERMUTATION_ID);
  std::string diagItemNo = rootDiag()->diagParamMapItem(TABLE_ITEMNO);
  std::string diagSeqNo = rootDiag()->diagParamMapItem(TABLE_SEQNO);
  std::string bookStatus = rootDiag()->diagParamMapItem(Diagnostic::FARE_PATH);
  std::string cat5info = rootDiag()->diagParamMapItem(ADV_RES);

  if (cat5info == "ON")
    _cat5info = true;

  if (bookStatus == "BOOKED")
    _filterRebook = true;

  if (bookStatus == "REBOOKED")
    _filterAsbook = true;

  _minRange = std::atoi(diagRangeMin.c_str());
  _maxRange = std::atoi(diagRangeMax.c_str());
  _permutationIndex = std::atoi(diagPermIndex.c_str());
  _t988ItemNo = std::atoi(diagItemNo.c_str());
  _t988SeqNo = std::atoi(diagSeqNo.c_str());

  if (_minRange < 0)
    _minRange = 0;
  if (_maxRange < 0)
    _maxRange = 0;
  if (_permutationIndex < 0)
    _permutationIndex = 0;
  if (_t988ItemNo < 0)
    _t988ItemNo = 0;
  if (_t988SeqNo < 0)
    _t988SeqNo = 0;
}

typedef std::vector<PricingUnit*>::const_iterator PuIt;
typedef std::vector<FareUsage*>::const_iterator FuIt;

Diag689Collector&
Diag689Collector::operator<<(const FarePath& farePath)
{
  if (_active)
  {
    filterByFarePath(farePath);
    if (filterPassed())
    {
      printLine();
      DiagCollector::operator<<(farePath);
      if (isRebookSolution())
      {
        std::string rebookClasses;
        PuIt puIter = farePath.pricingUnit().begin();
        PuIt puIterEnd = farePath.pricingUnit().end();
        for (; puIter != puIterEnd; puIter++)
        {
          FuIt fuIter = (*puIter)->fareUsage().begin();
          FuIt fuIterEnd = (*puIter)->fareUsage().end();
          for (; fuIter != fuIterEnd; fuIter++)
          {
            size_t idx = 0;
            std::vector<PaxTypeFare::SegmentStatus>& segmentStatus = (*fuIter)->segmentStatus();
            const std::vector<TravelSeg*>& travelSeg = (*fuIter)->travelSeg();
            for (; idx < segmentStatus.size(); idx++)
            {
              if (segmentStatus[idx]._bkgCodeReBook.empty())
                continue;
              std::ostringstream str;
              str << (rebookClasses.empty() ? "" : " ") << travelSeg[idx]->pnrSegment()
                  << segmentStatus[idx]._bkgCodeReBook;
              rebookClasses += str.str();
            }
          }
        }

        *this << "REBOOK - " << rebookClasses << '\n';
      }
      else
        *this << "AS BOOKED\n";

      *this << "ITIN CHANGED: " << (farePath.ignoreReissueCharges() ? "N\n" : "Y\n");
      lineSkip(0);
    }
  }
  return *this;
}

void
Diag689Collector::printPermutationInfo(const ProcessTagInfo& pti, bool info, const FarePath& excFp)
{
  if (filterPassed())
  {
    DiagCollector& dc = *(DiagCollector*)this;
    const ReissueSequenceW& tbl988(*pti.reissueSequence());
    dc << pti.fareCompInfo()->fareCompNumber() << ":" << pti.fareMarket()->boardMultiCity() << "-"
       << pti.fareMarket()->offMultiCity() << " R3 ITEM " << pti.record3()->itemNo();

    if (pti.reissueSequence()->orig())
      dc << " T988 ITEM " << pti.itemNo() << " SEQ " << pti.seqNo() << '\n';
    else
      dc << " NO T988\n";

    if (pti.isOverriden())
    {
      FareCompInfo::OverridingIntlFcData* od =
          pti.fareCompInfo()->findOverridingData(pti.record3()->orig());

      dc << pti.fareCompInfo()->getOverridingFc(od) << ":"
         << "OVERIDN"
         << " R3 ITEM " << pti.record3()->overriding()->itemNo() << " T988 ITEM "
         << pti.reissueSequence()->overriding()->itemNo() << " SEQ "
         << pti.reissueSequence()->overriding()->seqNo() << '\n';
    }
    dc << *pti.paxTypeFare() << '\n';
    if (info && pti.reissueSequence()->orig())
    {
      dc << "REISSUE TO LOWER: " << tbl988.reissueToLower() << '\n'
         << "TERM: " << tbl988.terminalPointInd() << '\n'
         << "FIRST BREAK: " << tbl988.firstBreakInd() << '\n'
         << "FULLY FLOWN: " << tbl988.extendInd() << '\n' << "JOURNEY: " << tbl988.journeyInd()
         << '\n' << "SAME POINT TBL993 ITEM NO: " << tbl988.samePointTblItemNo() << '\n'
         << "OVERRIDE RESERVATION DATE TBL994 ITEM NO: " << pti.record3()->overrideDateTblItemNo()
         << '\n' << "RULE INDICATOR: " << tbl988.ruleInd() << "  "
         << "RULE NUMBER: " << tbl988.ruleNo() << '\n'
         << "CARRIER APPLICATION TABLE: " << tbl988.fareCxrApplTblItemNo() << '\n'
         << "TARIFF NUMBER: " << tbl988.ruleTariffNo() << " "
         << "EXCLUDE PRIVATE: " << tbl988.excludePrivate() << '\n'
         << "FARE TYPE/CLASS INDICATOR: " << tbl988.fareTypeInd()
         << " CLASS CODE: " << tbl988.fareClass() << '\n' << "  FARE TYPE: " << tbl988.fareType()
         << "  SAME: " << tbl988.sameInd() << '\n';

      RexPricingTrx& trx = static_cast<RexPricingTrx&>(*_trx);

      const DateTime& applicationDate = pti.fareMarket()->retrievalDate();

      bool changeVendorForDFFFares = checkIfOverrideNeeded(excFp);
      VendorCode vendor =
                  RexFareBytesValidator::getVendorCodeForFareTypeTbl(pti, changeVendorForDFFFares);

      FareTypeTablePresenter fttp(*this, trx);
      fttp.printFareType(vendor,
                         pti.reissueSequence()->fareTypeTblItemNo(),
                         applicationDate);

      dc << "FARE AMOUNT: " << tbl988.fareAmtInd() << " "
         << "NORMAL/SPECIAL: " << tbl988.normalspecialInd() << " "
         << "OWRT: " << tbl988.owrt() << '\n' << "ADV RES FROM: " << tbl988.fromAdvResInd()
         << "   TO: " << tbl988.toAdvResInd() << '\n'
         << "BOOKING CODE REVALIDATE IND: " << tbl988.bkgCdRevalInd() << '\n';
      if (!_isCAT31OptionN)
      {
        dc << "ADV RES BYTES FROM AND TO WILL BE IGNORED - PCG OPTION IS A OR B\n";
      }

      dc << "REVALIDATE RULES IND: " << tbl988.revalidationInd() << '\n' << " 1-"
         << tbl988.provision1() << "  2-" << tbl988.provision2() << "  3-" << tbl988.provision3()
         << "  4-" << tbl988.provision4() << "  5-" << tbl988.provision5() << "  6-"
         << tbl988.provision6() << "  7-" << tbl988.provision7() << "  8-" << tbl988.provision8()
         << "  9-" << tbl988.provision9() << " 10-" << tbl988.provision10() << '\n' << " 11-"
         << tbl988.provision11() << " 12-" << tbl988.provision12() << " 13-" << tbl988.provision13()
         << " 14-" << tbl988.provision14() << " 15-" << tbl988.provision15() << " 17-"
         << tbl988.provision17() << " 23-" << tbl988.provision18() << " 50-" << tbl988.provision50()
         << '\n';

      dc << "RESIDUAL/PENALTY INDICATOR: " << pti.record3()->residualInd() << "   HIERARCHY: ";

      if (pti.record3()->residualHierarchy() == ProcessTagPermutation::RESIDUAL_BLANK)
        dc << "MOST";
      else
        dc << "LEAST";

      dc << "\nFORM OF REFUND: " << pti.record3()->formOfRefund() << '\n'
         << "TKT/RES REQ: " << tbl988.ticketResvInd() << "  "
         << "BOTH: " << tbl988.departureInd() << '\n' << "AFTER RES  TOD: " << tbl988.reissueTOD()
         << "  "
         << "PERIOD: " << tbl988.reissuePeriod() << "  "
         << "UNIT: " << tbl988.reissueUnit() << '\n' << "BEFORE DEPT  OPT: " << tbl988.optionInd()
         << "  "
         << "DEPT: " << tbl988.departure() << "  "
         << "UNIT: " << tbl988.departureUnit() << '\n' << "STOP: " << tbl988.stopInd() << '\n'
         << "PORTION: " << tbl988.portionInd() << "\n"
         << "OUTBOUND PORTION OF TRAVEL IND: " << tbl988.outboundInd() << '\n'
         << "ELECTRONIC TICKET: " << tbl988.electronicTktInd() << '\n';

      dc << "EXPANDED KEEP: " << tbl988.expndKeep() << '\n';

      SeasonalityDOWPresenter sdowp(*this, trx);
      sdowp.printSeasonalityDOW(pti.reissueSequence()->vendor(),
                                pti.reissueSequence()->seasonalityDOWTblItemNo(),
                                pti.fareMarket()->ruleApplicationDate());
    }
  }
}

Diag689Collector&
Diag689Collector::operator <<(const ProcessTagPermutation& perm)
{
  filterByPermutationNumber(perm.number());
  filterByT988(perm);

  if (filterPassed())
  {
    *this << "PERMUTATION " << perm.number() << " UU "
          << fareApplticationToSymbol(perm.fareTypeSelection(UU)) << "/UN "
          << fareApplticationToSymbol(perm.fareTypeSelection(UN)) << "/FL "
          << fareApplticationToSymbol(perm.fareTypeSelection(FL)) << "/UC "
          << fareApplticationToSymbol(perm.fareTypeSelection(UC)) << std::endl;
  }

  return *this;
}

const char*
Diag689Collector::fareApplticationToSymbol(FareApplication fa)
{
  static const char* T = "T";
  static const char* H = "H";
  static const char* K = "K";
  static const char* C = "C";
  static const char* X = "X";
  static const char* UNKN = "-";
  switch (fa)
  {
  case TRAVEL_COMMENCEMENT:
    return T;
  case HISTORICAL:
    return H;
  case KEEP:
    return K;
  case CURRENT:
    return C;
  case CANCEL:
    return X;
  default:
    return UNKN;
  }
}

void
Diag689Collector::printPermutationValidationResult(const ProcessTagPermutation& permutation,
                                                   const std::string& result)
{
  if (filterPassed())
  {
    *this << "PERMUTATION " << permutation.number() << ": " << result << std::endl;
    lineSkip(0);
  }
}

void
Diag689Collector::printResultInformation(RepriceFareValidationResult r,
                                         const ProcessTagInfo& pti,
                                         const PaxTypeFare* ptf)
{
  if (r == REPRICE_PASS)
    return;

  if (filterPassed())
  {
    *this << "EXC FC " << pti.fareCompNumber() << " CHECK FAILED ON ";

    switch (r)
    {
    case RULE_INDICATOR:
      *this << "RULE NUMBER\n";
      break;
    case CARRIER_APPLICATION_TBL:
      *this << "CARRIER APPLICATION TABLE\n";
      break;
    case TARIFF_NUMBER:
      *this << "TARIFF NUMBER\n";
      break;
    case EXCLUDE_PRIVATE:
    {
      *this << "EXCLUDE PRIVATE\n";

      if (ptf) // will be null if cache used
        *this << DiagnosticUtil::tcrTariffCatToString(pti.paxTypeFare()->tcrTariffCat()) << " "
              << pti.paxTypeFare()->createFareBasis(nullptr) << " TO "
              << DiagnosticUtil::tcrTariffCatToString(ptf->tcrTariffCat()) << " "
              << ptf->createFareBasis(nullptr) << " EXCHANGE NOT PERMITTED\n";
      break;
    }
    case FARE_CLASS_CODE:
      *this << "FARE CLASS CODE\n";
      break;
    case FARE_TYPE_CODE:
      *this << "FARE TYPE\n";
      break;
    case FARE_TYPE_TABLE:
      *this << "FARE TYPE TABLE\n";
      break;
    case SAME_INDICATOR:
      *this << "SAME\n";
      break;
    case FARE_AMOUNT:
      *this << "FARE AMOUNT\n";
      break;
    case NORMAL_SPECIAL:
      *this << "NORMAL/SPECIAL\n";
      break;
    case OWRT_INDICATOR:
      *this << "OWRT\n";
      break;
    default:
      break;
    }
  }
}

bool
Diag689Collector::filterPassed() const
{
  return _active && _fpFilterPassed && _permFilterPassed;
}

void
Diag689Collector::filterByFarePath(const FarePath& farePath)
{
  if ((_filterRebook && farePath.rebookClassesExists()) ||
      (_filterAsbook && !farePath.rebookClassesExists()))
  {
    _fpFilterPassed = false;
    return;
  }

  bool foundFM = false;
  bool foundPTF = false;

  PuIt puIter = farePath.pricingUnit().begin();
  PuIt puIterEnd = farePath.pricingUnit().end();
  for (; puIter != puIterEnd; puIter++)
  {
    FuIt fuIter = (*puIter)->fareUsage().begin();
    FuIt fuIterEnd = (*puIter)->fareUsage().end();
    for (; fuIter != fuIterEnd; fuIter++)
    {
      const PaxTypeFare* ptf = (*fuIter)->paxTypeFare();
      foundFM |= rootDiag()->shouldDisplay(*ptf->fareMarket());
      foundPTF |= rootDiag()->shouldDisplay(*ptf);
      if (foundFM && foundPTF)
        break;
    }
  }
  _fpFilterPassed = foundPTF && foundFM;
}

void
Diag689Collector::filterByPermutationNumber(const int& number)
{
  if ((_permutationIndex > 0 && number != _permutationIndex) || (number < _minRange) ||
      (_maxRange > 0 && number > _maxRange))
    _permFilterPassed = false;
}

struct Diag689Collector::T988Finder
{
  T988Finder(int itemNo, int seqNo) : _itemNo(itemNo), _seqNo(seqNo) {}

  bool operator()(const ProcessTagInfo* pti) const
  {
    return (_itemNo == pti->itemNo()) && (_seqNo == 0 || _seqNo == pti->seqNo());
  }
  int _itemNo;
  int _seqNo;
};

void
Diag689Collector::filterByT988(const ProcessTagPermutation& permutation)
{
  const std::vector<ProcessTagInfo*>& pt = permutation.processTags();
  if (_t988ItemNo && find_if(pt.begin(), pt.end(), T988Finder(_t988ItemNo, _t988SeqNo)) == pt.end())
    _permFilterPassed = false;
}

void
Diag689Collector::displayRetrievalDate(const PaxTypeFare& fare)
{
  *this << " FTN1:" << fare.footNote1() << " FTN2:" << fare.footNote2() << '\n';

  DiagCollector::displayRetrievalDate(fare);

  if (static_cast<PricingTrx&>(*_trx).excTrxType() != PricingTrx::AF_EXC_TRX)
    *this << "\n FARE CREATE DATE " << fare.createDate().dateToString(YYYYMMDD, "-");
}

void
Diag689Collector::print(const ProcessTagInfo& pti,
                        Indicator applicationScenario,
                        const std::string& applicationStatus)
{
  this->setf(std::ios::right, std::ios::adjustfield);

  *this << pti.paxTypeFare()->fareMarket()->boardMultiCity() << "-"
        << pti.paxTypeFare()->fareMarket()->governingCarrier() << "-"
        << pti.paxTypeFare()->fareMarket()->offMultiCity() << std::setw(15)
        << Money(pti.paxTypeFare()->fareAmount(), pti.paxTypeFare()->currency())
        << applicationStatus << '\n';

  *this << " PENALTY1-" << std::setw(7);
  pti.record3()->penaltyAmt1() > EPSILON
      ? *this << Money(pti.record3()->penaltyAmt1(), pti.record3()->cur1())
      : *this << "0.00";

  *this << " PENALTY2-" << std::setw(7);
  pti.record3()->penaltyAmt2() > EPSILON
      ? *this << Money(pti.record3()->penaltyAmt2(), pti.record3()->cur2())
      : *this << "0.00";

  *this << " PERCENT-" << std::setw(5) << pti.record3()->percent() << " H/L-" << std::setw(2)
        << pti.record3()->highLowInd() << '\n';

  *this << "   J/PU/FC-" << std::setw(2) << pti.record3()->journeyInd() << " APPL-" << std::setw(2)
        << pti.record3()->feeAppl() << "   NEW APPL- " << applicationScenario << '\n';

  *this << "   MINAMT-" << std::setw(6) << pti.record3()->minAmt() << std::setw(4)
        << pti.record3()->minCur() << " DISCOUNT- " << pti.record3()->discountTag1() << ' '
        << pti.record3()->discountTag2() << ' ' << pti.record3()->discountTag3() << ' '
        << pti.record3()->discountTag4() << '\n';
}

void
Diag689Collector::print(const PenaltyFee* fee, bool charged)
{
  *this << " PENALTY FEE:    " << std::setw(8);

  if (fee)
  {
    *this << Money(fee->penaltyAmount, fee->penaltyCurrency) << (charged ? " CHARGED" : " ")
          << " DISCOUNT PERCENTAGE:    " << std::setw(8) << fee->applicableDiscount << '\n';
  }

  else
    *this << " "
          << " DISCOUNT PERCENTAGE:\n";
}

void
Diag689Collector::printTotalChangeFee(const Money& totalFee, Indicator applicationScenario)
{
  *this << "TOTAL CHANGE FEE:" << std::setw(8) << totalFee << "   APPL- " << applicationScenario
        << '\n';
}

void
Diag689Collector::printNonRefInfo(const FareUsage& fu, bool rounding)
{
  if (fu.isNonRefundable())
  {
    const Money sourceMoney =
        fu.getNonRefundableAmt(NUC, static_cast<RexBaseTrx&>(*_trx), rounding);

    *this << " NONREFUNDABLE: ";
    printMoney(static_cast<RexBaseTrx&>(*_trx)
                   .convertCurrency(sourceMoney, fu.paxTypeFare()->currency(), rounding));
    *this << '\n';
  }
}

namespace
{
const char* const
getPricingUnitType(const PricingUnit::Type& type)
{
  switch (type)
  {
  case PricingUnit::Type::OPENJAW:
    return " OJ";
  case PricingUnit::Type::ROUNDTRIP:
    return " RT";
  case PricingUnit::Type::CIRCLETRIP:
    return " CT";
  case PricingUnit::Type::ONEWAY:
    return " OW";
  default:
    ;
  }
  return " ";
}

} // namespace

void
Diag689Collector::printFareUsagesInfo(const PricingUnit& pu)
{
  RexBaseTrx* rexTrx = dynamic_cast<RexBaseTrx*>(_trx);
  if (rexTrx == nullptr)
  {
    for (const FareUsage* fu : pu.fareUsage())
    {
      DiagCollector::operator<<(*fu);
      *this << getPricingUnitType(pu.puType()) << '\n';
    }
    return;
  }

  if (rexTrx->trxPhase() == RexPricingTrx::PRICE_NEWITIN_PHASE)
  {
    for (const FareUsage* fu : pu.fareUsage())
    {
      DiagCollector::operator<<(*fu);
      *this << getPricingUnitType(pu.puType()) << '\n';
      displayRetrievalDate(*fu->paxTypeFare());
      *this << '\n';
      printNonRefInfo(*fu, rexTrx->itin().front()->useInternationalRounding());
    }
    return;
  }

  for (const FareUsage* fu : pu.fareUsage())
  {
    DiagCollector::operator<<(*fu);
    *this << getPricingUnitType(pu.puType()) << '\n';
    displayRetrievalDate(*fu->paxTypeFare());
    *this << '\n';
    printNonRefInfo(*fu, rexTrx->exchangeItin().front()->useInternationalRounding());
  }
}

// --- cat33 refund specyfic ---

void
Diag689Collector::print(const FarePath& farePath)
{
  filterByFarePath(farePath);
  if (filterPassed())
  {
    printLine();
    DiagCollector::operator<<(farePath);
  }
}

void
Diag689Collector::print(const RefundPermutation& permutation)
{
  _permFilterPassed = true;
  filterByPermutationNumber(permutation.number());

  if (filterPassed())
  {
    lineSkip(0);
    *this << "PERMUTATION " << permutation.number() << " \n";

    std::vector<RefundProcessInfo*>::const_iterator processInfo =
                                                        permutation.processInfos().begin(),
                                                    processInfoEnd =
                                                        permutation.processInfos().end();
    for (; processInfo != processInfoEnd; ++processInfo)
    {
      printHeader(**processInfo);
      print(**processInfo);
    }
  }
}

void
Diag689Collector::printMapping(const RefundProcessInfo& processInfo,
                               const std::vector<FareUsage*>& mappedFus)
{
  if (filterPassed() && !mappedFus.empty())
  {
    DiagCollector& baseCollector(*this);
    baseCollector << "EXC FC " << processInfo.fareCompNumber() << '\n';

    FuIt fu = mappedFus.begin();
    FuIt fuEnd = mappedFus.end();
    for (; fu != fuEnd; ++fu)
    {
      printMapping(*(**fu).paxTypeFare());
      baseCollector << *(**fu).paxTypeFare() << '\n';
    }
  }
}

void
Diag689Collector::printMapping(const PaxTypeFare& ptf)
{
  if (filterPassed())
  {
    *this << "MATCHING TO REPRICE FC " << ptf.fareMarket()->boardMultiCity() << "-"
          << ptf.fareMarket()->offMultiCity() << '\n';
  }
}

void
Diag689Collector::printNarrowHeader(const FareMarket& fm)
{
  *this << std::setw(4) << std::right << fm.fareCompInfo()->fareCompNumber() << ": "
        << fm.boardMultiCity() << "-" << fm.governingCarrier() << "-" << fm.offMultiCity();
}

// make it common!
void
Diag689Collector::printNarrowPtf(const PaxTypeFare& ptf)
{
  *this << " " << ptf.fareMarket()->boardMultiCity() << "-" << ptf.fareMarket()->governingCarrier()
        << "-" << ptf.fareMarket()->offMultiCity() << " " << ptf.createFareBasis(nullptr);
}

void
Diag689Collector::printAdvResOverrideData(const AdvResOverride& aro)
{
  *this << "   FROM DATE: " << aro.fromDate().toIsoExtendedString()
        << "\n   TO DATE: " << aro.toDate().toIsoExtendedString();

  if (aro.reissueSequence())
  {
    *this << "\n   T988: " << aro.reissueSequence()->itemNo()
          << " SEQ: " << aro.reissueSequence()->seqNo()
          << " BOTH: " << aro.reissueSequence()->departureInd()
          << "\n   IGNORE TKT BEFORE DEPARTUE: ";

    if (aro.ignoreTktDeforeDeptRestriction())
      *this << "YES";
    else
    {
      *this << "NO\n"
            << "    OPTION: " << aro.reissueSequence()->optionInd()
            << " DEPARTURE: " << aro.reissueSequence()->departure()
            << " UNIT: " << aro.reissueSequence()->departureUnit();
    }

    *this << "\n   IGNORE TKT AFTER RESERVATION: ";
    if (aro.ignoreTktAfterResRestriction())
      *this << "YES";
    else
    {
      *this << "NO\n"
            << "    TOD: " << aro.reissueSequence()->reissueTOD()
            << " PERIOD: " << aro.reissueSequence()->reissuePeriod()
            << " UNIT: " << aro.reissueSequence()->reissueUnit();
    }
  }

  *this << '\n';
}

void
Diag689Collector::printHeader(const RefundProcessInfo& processInfo)
{
  if (filterPassed())
  {
    printNarrowHeader(*processInfo.paxTypeFare().fareMarket());
    static_cast<DiagCollector&>(*this) << " R3 ITEM " << processInfo.record3().itemNo() << '\n';
  }
}

void
Diag689Collector::print(const RefundProcessInfo& processInfo)
{
  if (filterPassed())
  {
    setf(std::ios::left, std::ios::adjustfield);

    *this << "FARE BREAKS: " << processInfo.record3().fareBreakpoints() << '\n'
          << "REPRICE IND: " << processInfo.record3().repriceInd() << '\n'
          << "TARIFF NUMBER: " << std::setw(5) << processInfo.record3().ruleTariff()
          << " EXCLUDE PRIVATE/PUBLIC: " << processInfo.record3().ruleTariffInd() << '\n'
          << "RULE NUMBER: " << processInfo.record3().rule() << '\n'
          << "FARE CLASS/FAMILY IND: " << std::setw(3) << processInfo.record3().fareClassInd()
          << " FARE CLASS: " << processInfo.record3().fareClass() << '\n';

    FareTypeTablePresenter fttp(*this, static_cast<RexBaseTrx&>(*_trx));
    fttp.printFareType(processInfo.paxTypeFare().vendor(),
                       processInfo.record3().fareTypeTblItemNo(),
                       processInfo.fareMarket().ruleApplicationDate());

    *this << "SAME: " << std::setw(5) << processInfo.record3().sameFareInd()
          << " NORMAL/SPECIAL: " << std::setw(5) << processInfo.record3().nmlSpecialInd()
          << " OWRT: " << processInfo.record3().owrt() << '\n' << "FARE AMOUNT: " << std::setw(10)
          << processInfo.record3().fareAmountInd()
          << " RBD IND: " << processInfo.record3().bookingCodeInd() << '\n'
          << "FORM OF REFUND: " << processInfo.record3().formOfRefund() << '\n';
  }
}

inline void
Diag689Collector::printMoney(const Money& money)
{
  std::ostringstream os;
  os << money;
  *this << std::setw(17) << std::left << os.str();
}

void
Diag689Collector::printRecord3PenaltyPart(const VoluntaryRefundsInfo& r3)
{
  *this << " 100% PENALTY- " << r3.cancellationInd() << "  TAX NON REF- "
        << r3.taxNonrefundableInd() << '\n' << " PENALTY1- ";
  printMoney(r3.penalty1Amt(), r3.penalty1Cur());
  *this << " PENALTY2- ";
  printMoney(r3.penalty2Amt(), r3.penalty2Cur());
  *this << '\n' << " PERCENT- " << std::setw(5) << std::fixed << std::setprecision(2)
        << r3.penaltyPercent() << "  H/L- " << r3.highLowInd() << '\n' << " PU/FC- "
        << r3.reissueFeeInd() << "  CALC OPTION: " << r3.calcOption() << '\n' << " MINAMT- ";
  printMoney(r3.minimumAmt(), r3.minimumAmtCur());
  *this << " DISCOUNT: " << r3.discountTag1() << " " << r3.discountTag2() << " "
        << r3.discountTag3() << " " << r3.discountTag4() << '\n';
}

void
Diag689Collector::printFareComponent(const FareUsage& fu, const PenaltyAdjuster& adjuster)
{
  printNarrowHeader(*fu.paxTypeFare()->fareMarket());
  *this << " " << (fu.isInbound() ? "I" : "O") << "   ";

  const RefundPricingTrx& trx = *static_cast<RefundPricingTrx*>(_trx);
  printMoney(adjuster.adjustedFuAmt(fu), trx.exchangeItinCalculationCurrency());
  *this << '\n';
}

namespace
{
typedef std::vector<RefundProcessInfo*>::const_iterator RpIt;

struct HasPaxTypeFare
{
  HasPaxTypeFare(const PaxTypeFare* ptf) : _ptf(ptf) {}

  bool operator()(const RefundProcessInfo* info) const { return &info->paxTypeFare() == _ptf; }

protected:
  const PaxTypeFare* _ptf;
};

RpIt
getProcessInfo(const FareUsage& fu, const std::vector<RefundProcessInfo*>& infos)
{
  const PaxTypeFare* ptf = fu.paxTypeFare();
  return std::find_if(infos.begin(), infos.end(), HasPaxTypeFare(ptf));
}
}

void
Diag689Collector::printPricingUnit(const PricingUnit& pu, const RefundPermutation& perm)
{
  const PenaltyAdjuster adjuster = getAdjuster(pu);
  const std::vector<FareUsage*>& fu = pu.fareUsage();
  for (const auto elem : fu)
  {
    printFareComponent(*elem, adjuster);
    RpIt pi = getProcessInfo(*elem, perm.processInfos());
    if (pi == perm.processInfos().end())
      *this << "ERROR: NO RECORD3 FOR THIS FARE COMPONENT!\n";
    else
      printRecord3PenaltyPart((*pi)->record3());
  }
  RefundPermutation::PenaltyFees::const_iterator pen = perm.penaltyFees().find(&pu);
  if (pen != perm.penaltyFees().end())
    printPricingUnitPenaltys(*pen->second);
}

const PenaltyAdjuster
Diag689Collector::getAdjuster(const PricingUnit& pu) const
{
  const RefundPricingTrx& trx = *static_cast<RefundPricingTrx*>(_trx);

  if (trx.arePenaltiesAndFCsEqualToSumFromFareCalc())
    return PenaltyAdjuster(pu, PenaltyAdjuster::SUMARIZE_FC);

  return PenaltyAdjuster(pu, PenaltyAdjuster::SUMARIZE_FU);
  //                           trx.exchangeItin().front()->farePath().front()->plusUpAmount());
}

void
Diag689Collector::printPricingUnitPenaltys(const RefundPenalty& penalty)
{
  std::string level = (penalty.isPuScope() ? "PU" : penalty.isFcScope() ? "FC" : "PU/FC");

  *this << "PENALTY FEE CHARGED ON " << level << " LEVEL\n";
  for (unsigned i = 0; i < penalty.fee().size(); ++i)
    printFee(penalty.fee()[i], i + 1);
}

void
Diag689Collector::printFee(const RefundPenalty::Fee& fee, unsigned nr)
{
  *this << "PENALTY FEE " << nr << ": ";
  if (fee.nonRefundable())
  {
    *this << "100%\n";
    return;
  }

  printMoney(fee.amount());
  *this << ' ';
  const RefundPricingTrx& trx = *static_cast<RefundPricingTrx*>(_trx);
  printMoney(trx.convertCurrency(fee.amount(), trx.exchangeItinCalculationCurrency()));

  *this << " DISCOUNT: " << (fee.discount() ? "Y" : "N") << '\n';
}

void
Diag689Collector::printPenalty(const std::vector<PricingUnit*>& excPUs,
                               const RefundPermutation& perm,
                               const MoneyAmount farePathAmount)
{
  if (filterPassed())
  {
    unsigned i = 1;
    *this << "PENALTY CALCULATION: \n";
    for (PuIt p = excPUs.begin(); p != excPUs.end(); ++p, ++i)
    {
      *this << "PRICING UNIT " << i << ":\n";
      printPricingUnit(**p, perm);
    }

    *this << "\nTOTAL PENALTY: ";
    printMoney(perm.totalPenalty());

    const RefundPricingTrx& trx = *static_cast<RefundPricingTrx*>(_trx);

    if (perm.minimumPenalty().value() > EPSILON)
    {
      *this << "LESS THAN MINIMUM AMOUNT: ";
      printMoney(trx.convertCurrency(perm.minimumPenalty(), trx.exchangeItinCalculationCurrency()));
    }

    *this << "\nTOTAL REPRICE VALUE:";
    if (trx.fullRefund())
      *this << " N/A";
    else
      *this << " " << farePathAmount + perm.overallPenalty(trx) << " "
            << trx.exchangeItinCalculationCurrency();

    *this << "\nTAXES REFUNDABLE: ";
    perm.taxRefundable() ? *this << 'Y' : *this << 'N';

    *this << '\n';
  }
}

void
Diag689Collector::printHeader()
{
  if (!_active)
    return;

  *this << "*********************** DIAGNOSTIC 689 ***********************\n";
}

void
Diag689Collector::printForFullRefund(const std::vector<PricingUnit*>& excPUs,
                                     const std::vector<RefundPermutation*>& perm,
                                     const MoneyAmount farePathAmount)
{
  printHeader();

  for (const auto elem : perm)
  {
    print(*elem);
    if (filterPassed())
    {
      *this << "VALIDATING FARE BREAK LIMITATIONS: N/A\n"
            << "VALIDATING FARE RESTRICTIONS: N/A\n";
    }
    printPenalty(excPUs, *elem, farePathAmount);
    if (filterPassed())
    {
      *this << "PERMUTATION " << elem->number() << ": N/A\n";
    }
  }
  flushMsg();
}

void
Diag689Collector::printNewTicketEqualOrHigherValidation(const Money& excTotalAmount,
                                                        const Money& newTotalAmount,
                                                        const Money& newAdjustedTotalAmount,
                                                        const Money& excNonRefAmount,
                                                        const Money& newNonRefAmout,
                                                        const Money& newAdjustedNonrefAmount,
                                                        const bool isExcNet,
                                                        const bool isNewNet,
                                                        const Indicator byte,
                                                        const bool status)
{
  *this << "NONREFUNDABLE AMOUNT OF\n";
  printNewTicketEqualOrHigherAmounts(excNonRefAmount, newNonRefAmout,
                                     newAdjustedNonrefAmount, isExcNet, isNewNet);

  *this << "TOTAL AMOUNT OF\n";
  printNewTicketEqualOrHigherAmounts(excTotalAmount, newTotalAmount,
                                     newAdjustedTotalAmount, isExcNet, isNewNet);

  static const char* const msg = "NEW TICKET EQUAL OR HIGHER: ";
  *this << msg << (byte == ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BN
                       ? "BN"
                       : std::string(1, byte)) << '\n' << msg << (status ? "PASSED\n" : "FAILED\n");
}

void Diag689Collector::printNewFareEqualOrHigherValidation(int itemNo,
                                                           int seqNo,
                                                           Indicator indicator,
                                                           bool status,
                                                           MoneyAmount newFareAmount,
                                                           MoneyAmount prevFareAmount,
                                                           CurrencyCode currency)
{
  *this << "ITEM " << itemNo << " SEQ " << seqNo << "\n"
        << "NEW FARE EQUAL OR HIGHER: " << indicator << "\n";
  if (newFareAmount > EPSILON)
    *this << "NEW FARE AMOUNT:      " << newFareAmount << " " << currency << "\n";
  if (prevFareAmount > EPSILON)
    *this << "PREVIOUS FARE AMOUNT: " << prevFareAmount << " " << currency << "\n";
  *this << "NEW FARE EQUAL OR HIGHER: " << (status ? "PASSED" : "FAILED") << "\n";
}

namespace
{

inline
const char* const
getFareAmountLevel(bool isNet)
{
  return isNet ? "NL" : "SL";
}

} // namespace

void
Diag689Collector::printNewTicketEqualOrHigherAmounts(const Money& excAmount,
                                                     const Money& newAmount,
                                                     const Money& newAdjustedAmount,
                                                     const bool isExcNet,
                                                     const bool isNewNet)
{
  *this << "EXCHANGE TICKET: ";
  printMoney(excAmount);
  *this << ' ' << getFareAmountLevel(isExcNet) << '\n';
  *this << "NEW TICKET:      ";
  printMoney(newAmount);
  if (newAmount.code() != excAmount.code())
    printMoney(newAdjustedAmount);
  *this << ' ' << getFareAmountLevel(isNewNet) << '\n';
}

} // tse
