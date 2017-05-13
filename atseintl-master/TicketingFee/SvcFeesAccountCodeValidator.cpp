//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "TicketingFee/SvcFeesAccountCodeValidator.h"

#include "DataModel/PricingTrx.h"
#include "DBAccess/SvcFeesAccCodeInfo.h"
#include "Diagnostic/SvcFeesDiagCollector.h"

namespace tse
{
const std::string SvcFeesAccountCodeValidator::ASTERISK = "*";

SvcFeesAccountCodeValidator::SvcFeesAccountCodeValidator(PricingTrx& trx,
                                                         SvcFeesDiagCollector* diag)
  : _trx(trx), _diag((diag && diag->isActive()) ? diag : nullptr)
{
}

bool
SvcFeesAccountCodeValidator::validate(int itemNo) const
{
  if (UNLIKELY(_diag))
    _diag->printAccountCodeTable172Header(itemNo);

  const std::vector<SvcFeesAccCodeInfo*>& accCodeInfos = getSvcFeesAccCodeInfo(itemNo);

  for (const auto accCodeInfo : accCodeInfos)
  {
    bool rc = validateAccCode(*accCodeInfo);
    if (UNLIKELY(_diag))
    {
      (*_diag) << *accCodeInfo;
      _diag->printValidationStatus(rc);
    }
    if (rc)
      return true;
  }

  return false;
}

const std::vector<SvcFeesAccCodeInfo*>&
SvcFeesAccountCodeValidator::getSvcFeesAccCodeInfo(int itemNo) const
{
  return _trx.dataHandle().getSvcFeesAccountCode(ATPCO_VENDOR_CODE, itemNo);
}

bool
SvcFeesAccountCodeValidator::validateAccCode(const SvcFeesAccCodeInfo& accCode) const
{
  if (UNLIKELY(accCode.accountCode().empty()))
    return false;

  if (UNLIKELY(accCode.accountCode() == ASTERISK))
  {
    return _trx.getRequest()->isMultiAccCorpId() || !_trx.getRequest()->accountCode().empty() ||
           !_trx.getRequest()->corporateID().empty();
  }

  if (_trx.getRequest()->isMultiAccCorpId()) // multiple AccCode/CorpID request ?
  {
    return validateMultiAccCode(accCode);
  }
  else
  {
    return validateSingleAccCode(accCode);
  }
}

bool
SvcFeesAccountCodeValidator::validateMultiAccCode(const SvcFeesAccCodeInfo& accCode) const
{
  const std::vector<std::string>& corpIdVec = _trx.getRequest()->corpIdVec();
  const std::vector<std::string>& accCodeVec = _trx.getRequest()->accCodeVec();
  bool isMatched = false;
  if (UNLIKELY(corpIdVec.empty() && accCodeVec.empty()))
    return isMatched;

  std::vector<std::string>::const_iterator vecIter;
  for (vecIter = corpIdVec.begin(); vecIter != corpIdVec.end() && !isMatched; vecIter++)
  {
    isMatched = (accCode.accountCode() == *vecIter ||
                 matchAccountCode(accCode.accountCode().c_str(), (*vecIter).c_str()));
  }

  for (vecIter = accCodeVec.begin(); vecIter != accCodeVec.end() && !isMatched; vecIter++)
  {
    isMatched = (accCode.accountCode() == *vecIter ||
                 matchAccountCode(accCode.accountCode().c_str(), (*vecIter).c_str()));
  }
  return isMatched;
}

bool
SvcFeesAccountCodeValidator::validateSingleAccCode(const SvcFeesAccCodeInfo& accCode) const
{
  const std::string& accCodeInput = _trx.getRequest()->accountCode().empty()
                                        ? _trx.getRequest()->corporateID()
                                        : _trx.getRequest()->accountCode();

  if (LIKELY(accCodeInput.empty()))
    return false;
  if (!matchAccountCode(accCode.accountCode().c_str(), accCodeInput.c_str()))
    return false;
  return true;
}

bool
SvcFeesAccountCodeValidator::matchAccountCode(const char* accCodeT172CStr,
                                              const char* accCodeInputCStr)
{
  if (UNLIKELY(0 == strlen(accCodeT172CStr)))
    return false; // to protect
  if (UNLIKELY(0 == strcmp(accCodeT172CStr, accCodeInputCStr)))
    return true;

  // The following logic was cloned from RuleUtil::matchFareBasis()
  const char* posHPtr = strchr(accCodeT172CStr, ASTERISK[0]);
  if (LIKELY(posHPtr == nullptr))
    return false;

  size_t posH = posHPtr - accCodeT172CStr;
  size_t classRuleSize = strlen(accCodeT172CStr) - 1;
  size_t fareRuleSize = strlen(accCodeInputCStr);

  if (fareRuleSize < classRuleSize)
    return false;

  size_t posSecond = fareRuleSize - 1;
  if (posH != classRuleSize)
  {
    char second[classRuleSize + 1];
    size_t sizeSecond = classRuleSize - posH;

    // Using sizeSecond to copy the right number of bytes.
    memcpy(second, accCodeT172CStr + posH + 1, sizeSecond);
    second[sizeSecond] = 0;
    size_t posSecondEnd = 0;

    // If the asterisk is at the beginning of the T172/T173 rule then the
    //  match cannot start at the first character of the inpit AccCode.
    posSecondEnd = posH == 0 ? 1 : posH;

    while (true)
    {
      const char* posSecondPtr = strstr(accCodeInputCStr + posSecondEnd, second);
      if (posSecondPtr == nullptr)
        return false;

      posSecond = posSecondPtr - accCodeInputCStr;
      posSecondEnd = posSecond + sizeSecond - 1;
      // When matching on a numeric, if the preceeding
      //  or succeeding characters are also numeric, then
      //  it is not a match.
      // For example: *E70 matches BHE70NR but does not match
      //  BHE701.
      if ((posSecond > 0 && beginsWithTwoDigits(accCodeInputCStr + posSecond - 1)) ||
          (posSecondEnd < (fareRuleSize - 1) &&
           beginsWithTwoDigits(accCodeInputCStr + posSecondEnd)))
      {
        ++posSecondEnd;
        continue;
      }
      // If we get here, then we have a valid match so
      //  break out of the loop.
      break;
    }
    if (posH == 0)
      return true;
  }

  size_t posFirstEnd = posH - 1;

  if (memcmp(accCodeT172CStr, accCodeInputCStr, posH) != 0 ||
      beginsWithTwoDigits(accCodeInputCStr + posFirstEnd))
    return false;

  return posFirstEnd <= posSecond;
}

bool
SvcFeesAccountCodeValidator::beginsWithTwoDigits(const char* text)
{
  return isdigit(*text) && isdigit(*(text + 1));
}
}
