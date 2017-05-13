//-------------------------------------------------------------------
//
//  File:        RefundPermutationGenerator.h
//  Created:     July 29, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#include "RexPricing/RefundPermutationGenerator.h"

#include "Common/FallbackUtil.h"
#include "Common/SpecifyMaximumPenaltyCommon.h"
#include "DataModel/ExcItin.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundPricingTrx.h"
#include "DataModel/RefundProcessInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag688Collector.h"
#include "Diagnostic/DiagManager.h"
#include "Rules/OriginallyScheduledFlightValidator.h"
#include "Rules/VoluntaryRefunds.h"
#include "Util/CartesianProduct.h"

namespace tse
{
FALLBACK_DECL(smpSkipDepartureMatchingForRecordValidation)

namespace
{
inline bool
departureIndMatch(const VoluntaryRefundsInfo& record3, smp::RecordApplication departureInd)
{
  // we don't care about firstFC/firstPU processing in getRecordApplication at this stage hence
  // "false" literals
  smp::RecordApplication application = smp::getRecordApplication(record3, false, false);
  return smp::isDepartureMatching(application, departureInd);
}

inline bool
isWaiverCode(const VoluntaryRefundsInfo& record)
{
  return record.waiverTblItemNo() != 0;
}

inline bool skipFullyFlownRecord(const VoluntaryRefundsInfo* record)
{
  return record->fullyFlown() == VoluntaryRefunds::FULLY_FLOWN;
}

inline bool
skipAfterScheduledFlight(const VoluntaryRefundsInfo* r3)
{
  return r3->origSchedFlt() == OriginallyScheduledFlightValidator::ANYTIME_AFTER ||
         r3->origSchedFlt() == OriginallyScheduledFlightValidator::DAY_AFTER;
}
} // namespace

RefundPermutationGenerator::RefundPermutationGenerator(PricingTrx& trx, log4cxx::LoggerPtr logger)
  : _dataHandle(trx.dataHandle()),
    _dc(nullptr),
    _logger(logger),
    _trx(&trx),
    _diagEnabled(trx.diagnostic().diagParamIsSet(Diagnostic::DISPLAY_DETAIL, "MAXPEN"))
{
  if (trx.diagnostic().diagnosticType() == Diagnostic688)
  {
    if ((_dc = dynamic_cast<Diag688Collector*>(DCFactory::instance()->create(trx))))
    {
      _dc->enable(Diagnostic688);
    }
  }
}

RefundPermutationGenerator::~RefundPermutationGenerator()
{
  if (_dc)
  {
    _dc->flushMsg();
  }
}

void
RefundPermutationGenerator::process(Permutations& permutations,
                                    const FarePath& farePath,
                                    const RefundPricingTrx::Options* refundOptions,
                                    smp::RecordApplication departureInd)
{
  std::vector<const PaxTypeFare*> fares;
  RuleUtil::getAllPTFs(fares, farePath);
  generate(refundOptions, fares, permutations, departureInd);
  if (!_dc)
    return;
  _dc->printRefundPermutations(permutations);
}

void RefundPermutationGenerator::processForSMP(Permutations& permutations,
                                               const FarePath& farePath,
                                               FCtoSequence& seqsByFC,
                                               smp::RecordApplication departureInd)
{
  std::vector<const PaxTypeFare*> fares;
  RuleUtil::getAllPTFs(fares, farePath);
  generateForSMP(fares, permutations, *farePath.pricingUnit().front(), seqsByFC, departureInd);
  if (!_dc)
    return;
  _dc->printRefundPermutations(permutations);
}

void RefundPermutationGenerator::generateForSMP(const std::vector<const PaxTypeFare*>& fares,
                                                Permutations& permutations,
                                                const PricingUnit& pricingUnit,
                                                FCtoSequence& seqsByFC,
                                                smp::RecordApplication departureInd) const
{
  int fareCompIndex = 0;

  DiagManager diag(*_trx);

  if (_diagEnabled)
  {
    diag.activate(Diagnostic555);
  }

  for (const PaxTypeFare* ptf : fares)
  {
    diag << "   " << DiagnosticUtil::printPaxTypeFare(*ptf) << " - CAT33\n";

    if (ptf->getVoluntaryRefundsInfo().empty())
    {
      diag << "    NO RECORDS - CAT33\n";
      return;
    }

    for (const VoluntaryRefundsInfo* r3 : ptf->getVoluntaryRefundsInfo())
    {
      if (_diagEnabled)
        smp::printRecord3(*_trx, *r3, departureInd, diag);

      if ((!fallback::smpSkipDepartureMatchingForRecordValidation(_trx) ||
           departureIndMatch(*r3, departureInd)) &&
          smp::isPsgMatch(*ptf->paxType(), *r3) && !isWaiverCode(*r3) &&
          !skipAfterScheduledFlight(r3) && !skipFullyFlownRecord(r3))
      {
        diag << "    RECORD PASSED - " << r3->itemNo() << "\n";
        RefundProcessInfo* p = _dataHandle.create<RefundProcessInfo>();
        p->assign(r3, ptf, 0);
        seqsByFC[fareCompIndex].push_back(p);
      }
      else
      {
        diag << "    RECORD FAILED - " << r3->itemNo() << "\n";
      }
    }
    ++fareCompIndex;
  }

  if (fares.size() == seqsByFC.size())
    storePermutations(permutations, seqsByFC);
}

void
RefundPermutationGenerator::generate(const RefundPricingTrx::Options* refundOptions,
                                     const std::vector<const PaxTypeFare*>& fares,
                                     Permutations& permutations,
                                     smp::RecordApplication departureInd) const
{
  FCtoSequence seqsByFC;

  auto createRefundProcessInfo = [&](const FareCompInfo* fc,
                                     const PaxTypeFare* ptf,
                                     const VoluntaryRefundsInfo* r3)
      {
        if (departureIndMatch(*r3, departureInd))
        {
          RefundProcessInfo* p = _dataHandle.create<RefundProcessInfo>();
          p->assign(r3, ptf, fc);
          seqsByFC[fc->fareCompNumber()].push_back(p);
        }
      };

  for (const PaxTypeFare* ptf : fares)
  {
    const FareCompInfo* fc = ptf->fareMarket()->fareCompInfo();

    if (refundOptions)
    {
      std::pair<OptionsIt, OptionsIt> range = findInOptions(*refundOptions, ptf);

      for (OptionsIt i = range.first; i != range.second; ++i)
      {
        createRefundProcessInfo(fc, ptf, i->second);
      }
    }
    else
    {
      for (const VoluntaryRefundsInfo* r3 : ptf->getVoluntaryRefundsInfo())
      {
        createRefundProcessInfo(fc, ptf, r3);
      }
    }
  }

  storePermutations(permutations, seqsByFC);
}


void
RefundPermutationGenerator::storePermutations(Permutations& permutations, FCtoSequence& seqsByFC) const
{
  typedef CartesianProduct<std::vector<RefundProcessInfo*>> CartesianProduct;

  CartesianProduct cp;

  for (auto& elem : seqsByFC)
    cp.addSet(elem.second);

  CartesianProduct::ProductType permSelector = cp.getNext();
  unsigned permNumber = 0;
  for (; !permSelector.empty(); permSelector = cp.getNext())
  {
    RefundPermutation* p = _dataHandle.create<RefundPermutation>();
    p->assign(++permNumber, permSelector);
    p->setRepriceIndicator();
    permutations.push_back(p);
  }
}

void
RefundPermutationGenerator::missingFareError() const
{
  LOG4CXX_ERROR(_logger, "Missing fare in Refund Options");
  throw ErrorResponseException(ErrorResponseException::UNKNOWN_EXCEPTION);
}

std::pair<RefundPermutationGenerator::OptionsIt, RefundPermutationGenerator::OptionsIt>
RefundPermutationGenerator::findInOptions(const RefundPricingTrx::Options& refundOptions,
                                          const PaxTypeFare* ptf) const
{
  std::pair<OptionsIt, OptionsIt> range = refundOptions.equal_range(ptf);

  if (range.first != range.second)
    return range;

  if (!ptf->isFareByRule())
    missingFareError();

  range = refundOptions.equal_range(ptf->baseFare());

  if (range.first == range.second)
    missingFareError();

  return range;
}
} // tse
