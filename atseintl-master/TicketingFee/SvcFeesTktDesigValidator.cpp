//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "TicketingFee/SvcFeesTktDesigValidator.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/SvcFeesTktDesignatorInfo.h"
#include "Diagnostic/SvcFeesDiagCollector.h"
#include "Util/IteratorRange.h"

namespace tse
{
FALLBACK_DECL(tktDesignatorCoreFix)

SvcFeesTktDesigValidator::SvcFeesTktDesigValidator(PricingTrx& trx, SvcFeesDiagCollector* diag)
  : _trx(trx), _diag((diag && diag->isActive()) ? diag : nullptr)
{
}

bool
SvcFeesTktDesigValidator::validate(int itemNo) const
{
  if (UNLIKELY(_diag))
    _diag->printTktDesignatorTable173Header(itemNo);

  if (itemNo <= 0)
    return true;

  std::set<TktDesignator> inputTktDesignator;
  if (!getTicketDesignators(inputTktDesignator))
    return false;

  return validate(inputTktDesignator, getSvcFeesTicketDesignator(itemNo));
}

bool
SvcFeesTktDesigValidator::validate(const std::set<TktDesignator>& tktDesignators,
                                   const std::vector<SvcFeesTktDesignatorInfo*>& svcFeeesTktDesig)
    const
{
  for (const SvcFeesTktDesignatorInfo* t173 : svcFeeesTktDesig)
  {
    const TktDesignator& svcFeesTktD = t173->tktDesignator();
    if (UNLIKELY(svcFeesTktD.empty()))
    {
      if (UNLIKELY(_diag))
        _diag->printTktDesignatorFailInfo(FAIL_ON_BLANK);
      return false;
    }
    bool rc = compare(tktDesignators, svcFeesTktD);
    if (UNLIKELY(_diag))
    {
      *_diag << *t173;
      _diag->printValidationStatus(rc);
    }
    if (UNLIKELY(rc))
      return true;
  }
  return false;
}

const std::vector<SvcFeesTktDesignatorInfo*>&
SvcFeesTktDesigValidator::getSvcFeesTicketDesignator(const int itemNo) const
{
  return _trx.dataHandle().getSvcFeesTicketDesignator(ATPCO_VENDOR_CODE, itemNo);
}

bool
SvcFeesTktDesigValidator::compare(const std::set<TktDesignator>& tktDesignators,
                                  const TktDesignator& svcFeesTktDesig) const
{
  const bool wildCard = (svcFeesTktDesig.find(ASTERISK) != std::string::npos);

  for (const TktDesignator& tktD : tktDesignators)
    if (UNLIKELY((wildCard && wildCardMatch(svcFeesTktDesig, tktD)) || svcFeesTktDesig == tktD))
      return true;
  return false;
}

bool
SvcFeesTktDesigValidator::wildCardMatch(const TktDesignator& sTable, const TktDesignator& sInput)
    const
{
  const size_t asteriskPos = sTable.find(ASTERISK);

  // wildcard is in the beginning of an alphanumeric string
  if (asteriskPos == 0)
    return wildCardMatchLeft(sTable, sInput);

  if (asteriskPos + 1 < sTable.size())
    return wildCardMatchMiddle(sTable, sInput, asteriskPos);

  if (asteriskPos + 1 == sTable.size())
    return wildCardMatchRight(sTable, sInput, asteriskPos);

  return false;
}

static bool
checkWildcardDigitCondition(const TktDesignator& input,
                            const size_t wildCardMatchBoundary)
{
  // From DataApp:
  // The asterisk will not be used to replace a number when the immediately following or preceding
  // character in the Ticket Designator is also a number

  if (wildCardMatchBoundary == 0 || wildCardMatchBoundary == input.size())
    return true;

  return !isdigit(input[wildCardMatchBoundary - 1]) || !isdigit(input[wildCardMatchBoundary]);
}


bool
SvcFeesTktDesigValidator::wildCardMatchLeft(const TktDesignator& sTable,
                                            const TktDesignator& sInput) const
{
  if (sTable.size() == 1 || sTable[1] == '\0' || sTable[1] == ' ') // wildcard is alone
    return true;

  if (fallback::tktDesignatorCoreFix(&_trx))
  {
    size_t strEnd = sTable.size() - 1;

    TktDesignator pattern = sTable.substr(1, strEnd).c_str();
    size_t left = sInput.find(pattern, 0);

    if (left == std::string::npos || left == 0 || !wildCardPassOnDigit(sInput, pattern, left) ||
        !wildCardPassOnDigit(sInput, pattern, left, true))
    {
      return false;
    }

    return true;
  }

  // pattern coded as *XXX => processed like ?*XXX*
  const TktDesignator fixedPart = sTable.substr(1);
  const size_t fixedPartPos = sInput.find(fixedPart);

  return fixedPartPos != std::string::npos && fixedPartPos != 0 &&
         checkWildcardDigitCondition(sInput, fixedPartPos) &&
         checkWildcardDigitCondition(sInput, fixedPartPos + fixedPart.size());
}

bool
SvcFeesTktDesigValidator::wildCardMatchMiddle(const TktDesignator& sTable,
                                              const TktDesignator& sInput,
                                              size_t asteriskPos) const
{
  if (fallback::tktDesignatorCoreFix(&_trx))
  {
    // wildcard is in the middle of an alphanumeric string
    TktDesignator patternLeft = sTable.substr(0, asteriskPos).c_str();
    size_t left = sInput.find(patternLeft);
    if (left == std::string::npos || left != 0)
      return false;

    size_t sizePatternLeft = patternLeft.size();
    if (isdigit(atoi((sInput.substr(left + sizePatternLeft, 1)).c_str())) != 0 &&
        isdigit(atoi((sInput.substr(left + sizePatternLeft - 1, 1)).c_str())) != 0)
      return false;

    size_t strEnd = sTable.size() - 1;
    TktDesignator patternRight = sTable.substr(asteriskPos + 1, strEnd - asteriskPos).c_str();
    size_t right = sInput.find(patternRight, left + sizePatternLeft);

    if (right == std::string::npos || !wildCardPassOnDigit(sInput, patternRight, right) ||
        !wildCardPassOnDigit(sInput, patternRight, right, true))
    {
      return false;
    }

    return true;
  }

  // pattern coded as XXX*YYY => processed like XXX*YYY*
  const TktDesignator leftFixedPart = sTable.substr(0, asteriskPos);
  const TktDesignator rightFixedPart = sTable.substr(asteriskPos + 1);

  const size_t leftFixedPartPos = sInput.find(leftFixedPart);

  if (leftFixedPartPos != 0)
    return false;

  const size_t rightFixedPartPos =
      sInput.find(rightFixedPart, leftFixedPartPos + leftFixedPart.size());

  if (rightFixedPartPos == std::string::npos)
    return false;

  return checkWildcardDigitCondition(sInput, leftFixedPartPos + leftFixedPart.size()) &&
         checkWildcardDigitCondition(sInput, rightFixedPartPos) &&
         checkWildcardDigitCondition(sInput, rightFixedPartPos + rightFixedPart.size());
}

bool
SvcFeesTktDesigValidator::wildCardMatchRight(const TktDesignator& sTable,
                                             const TktDesignator& sInput,
                                             size_t asteriskPos) const
{
  if (fallback::tktDesignatorCoreFix(&_trx))
  {
    TktDesignator pattern = sTable.substr(0, asteriskPos).c_str();
    size_t right = sInput.find(pattern, 0);

    return right != std::string::npos && wildCardPassOnDigit(sInput, pattern, right, true);
  }

  const TktDesignator fixedPart = sTable.substr(0, asteriskPos);
  const size_t fixedPartPos = sInput.find(fixedPart);

  return fixedPartPos != std::string::npos &&
         checkWildcardDigitCondition(sInput, fixedPartPos + fixedPart.size());
}

bool
SvcFeesTktDesigValidator::wildCardPassOnDigit(const TktDesignator& sInput,
                                              const TktDesignator& pattern,
                                              size_t startPoint,
                                              bool right) const
{
  if (right)
  {
    startPoint += pattern.size();
  }

  return !isdigit(sInput[startPoint]) || !isdigit(sInput[startPoint - 1]);
}

// SvcFeesInputTktDesigValidator class
SvcFeesInputTktDesigValidator::SvcFeesInputTktDesigValidator(PricingTrx& trx,
                                                             const Itin& itin,
                                                             SvcFeesDiagCollector* diag)
  : SvcFeesTktDesigValidator(trx, diag), _itin(itin)
{
}

void
SvcFeesInputTktDesigValidator::getInputTktDesignators(std::set<TktDesignator>& inputTktDesignator)
    const
{
  inputTktDesignator.clear();

  for (const TravelSeg* ts : _itin.travelSeg())
  {
    const TktDesignator tktD = _trx.getRequest()->tktDesignator(ts->segmentOrder());
    if (!tktD.empty())
      inputTktDesignator.insert(tktD);
  }
}

bool
SvcFeesInputTktDesigValidator::getTicketDesignators(std::set<TktDesignator>& tktDesignators)
    const
{
  if (!_trx.getRequest()->isTktDesignatorEntry())
  {
    if (UNLIKELY(_diag))
      _diag->printTktDesignatorFailInfo(FAIL_NO_INPUT);
    return false;
  }

  // collect all input Tkt designators
  getInputTktDesignators(tktDesignators);

  if (tktDesignators.empty())
  {
    if (UNLIKELY(_diag))
      _diag->printTktDesignatorFailInfo(FAIL_NO_INPUT);
    return false;
  }

  return true;
}

// SvcFeesOutputTktDesigValidator class
SvcFeesOutputTktDesigValidator::SvcFeesOutputTktDesigValidator(PricingTrx& trx,
                                                               const PaxTypeFare& ptf,
                                                               SvcFeesDiagCollector* diag)
  : SvcFeesTktDesigValidator(trx, diag), _ptf(&ptf)
{
}

SvcFeesOutputTktDesigValidator::SvcFeesOutputTktDesigValidator(
    PricingTrx& trx,
    const FarePath& farePath,
    const std::vector<TravelSeg*>::const_iterator segI,
    const std::vector<TravelSeg*>::const_iterator segIE,
    SvcFeesDiagCollector* diag)
  : SvcFeesTktDesigValidator(trx, diag), _farePath(&farePath), _segI(segI), _segIE(segIE)
{
}

bool
SvcFeesOutputTktDesigValidator::getTicketDesignators(std::set<TktDesignator>& tktDesignators) const
{
  tktDesignators.clear();

  if (!_farePath && !_ptf)
    return false;

  if (_ptf)
    return getTktDesigFromPtf(*_ptf, tktDesignators);

  for (const PricingUnit* pu : _farePath->pricingUnit())
  {
    for (const FareUsage* fu : pu->fareUsage())
    {
      const PaxTypeFare& ptf = *fu->paxTypeFare();
      if (!isFareProcessed(ptf))
        continue;
      if (!getTktDesigFromPtf(ptf, tktDesignators))
        return false;
    }
  }
  return true;
}

bool
SvcFeesOutputTktDesigValidator::getTktDesigFromPtf(const PaxTypeFare& ptf,
                                                   std::set<TktDesignator>& tktDesignators) const
{
  const TravelSeg* lastTs = ptf.fareMarket()->travelSeg().back();
  TktDesignator inputTktD = _trx.getRequest()->specifiedTktDesignator(lastTs->segmentOrder());

  if (!inputTktD.empty())
  {
    tktDesignators.insert(inputTktD); // tkt des from DP
    return true;
  }

  const std::string fareBasis = getFareBasis(ptf);
  const size_t tktDesPos = fareBasis.find("/");

  if (tktDesPos == std::string::npos)
  {
    if (UNLIKELY(_diag))
      _diag->printTktDesignatorFailInfo(FAIL_NO_OUTPUT);
    return false; // no tkt des
  }

  tktDesignators.emplace(fareBasis.substr(tktDesPos + 1));
  return true;
}

bool
SvcFeesOutputTktDesigValidator::isFareProcessed(const PaxTypeFare& ptf) const
{
  for (const TravelSeg* ptfSeg : ptf.fareMarket()->travelSeg())
    for (const TravelSeg* ts : makeIteratorRange(_segI, _segIE))
      if (LIKELY(ptfSeg->segmentOrder() == ts->segmentOrder()))
        return true;
  return false;
}

bool
SvcFeesOutputTktDesigValidator::validate(
    const std::set<TktDesignator>& tktDesignators,
    const std::vector<SvcFeesTktDesignatorInfo*>& svcFeeesTktDesig) const
{
  // All tkt designators have to succeed
  for (const TktDesignator& tktD : tktDesignators)
  {
    // check, every designator passes
    const std::set<TktDesignator> tktDSet = {tktD};
    const bool ret = SvcFeesTktDesigValidator::validate(tktDSet, svcFeeesTktDesig);

    if (UNLIKELY(_diag))
    {
      *_diag << "    " << tktD;
      _diag->printValidationStatus(ret);
    }

    if (!ret)
      return false;
  }

  if (UNLIKELY(_diag))
    *_diag << "          ALL OUTPUT TKT DESIGNATORS PASSED\n";

  return true;
}

std::string
SvcFeesOutputTktDesigValidator::getFareBasis(const PaxTypeFare& paxTypeFare) const
{
  return paxTypeFare.createFareBasis(_trx);
}
}
