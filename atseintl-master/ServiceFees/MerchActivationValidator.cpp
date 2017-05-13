//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "ServiceFees/MerchActivationValidator.h"

#include "Common/ServiceFeeUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/MerchActivationInfo.h"
#include "Diagnostic/Diag875Collector.h"
#include "ServiceFees/ServiceFeesGroup.h"

namespace tse
{
MerchActivationValidator::MerchActivationValidator(PricingTrx& trx, Diag875Collector* diag875)
  : _trx(trx), _diag875(diag875)
{
}

bool
MerchActivationValidator::validate(const CarrierCode& carrier) const
{
  const std::vector<MerchActivationInfo*>& merchActInfos = getMerchActivationInfo(carrier);

  for (const auto merchActInfo : merchActInfos)
  {
    if (validateMerchActivationData(*merchActInfo))
      return true;
  }

  return false;
}

const std::vector<MerchActivationInfo*>&
MerchActivationValidator::getMerchActivationInfo(const CarrierCode& carrier) const
{
  return _trx.dataHandle().getMerchActivation(
      PRODUCT_ID_4, carrier, EMPTY_STRING(), _trx.ticketingDate());
}

bool
MerchActivationValidator::validateMerchActivationData(const MerchActivationInfo& merchActInfo) const
{
  if (!checkTravelDate(merchActInfo))
    return false;

  return true;
}

bool
MerchActivationValidator::checkTravelDate(const MerchActivationInfo& merchActInfo) const
{
  return ServiceFeeUtil::checkIsDateBetween(
             merchActInfo.effDate(), merchActInfo.discDate(), _trx.ticketingDate()) &&
         ServiceFeeUtil::checkIsDateBetween(
             merchActInfo.createDate(), merchActInfo.expireDate(), _trx.ticketingDate());
}

const std::vector<MerchActivationInfo*>&
MerchActivationValidator::getMerchActivation(const CarrierCode& carrier, const PseudoCityCode& pcc)
{
  return _trx.dataHandle().getMerchActivation(PRODUCT_ID_4, carrier, pcc);
}

bool
MerchActivationValidator::retrieveMerchActivaionData(const CarrierCode& carrier,
                                                     std::vector<ServiceGroup*>& groupCode,
                                                     std::vector<ServiceGroup*>& dispOnlyGroups)
{
  const std::vector<MerchActivationInfo*>& mmInfoVec = getMerchActivationInfo(carrier);

  if (!mmInfoVec.empty())
  {
    if (TrxUtil::isRequestFromAS(_trx)) // requestor is hosted airline
      return processMerchActivaionDataforHC(mmInfoVec, carrier, groupCode, dispOnlyGroups);
    else if (ServiceFeeUtil::isRequestFromTN(_trx)) // requestor is TN
      return processMerchActivaionDataforTN(mmInfoVec, carrier, groupCode, dispOnlyGroups);
    else
    {
      carrier.empty();
      return false; // Not an HC or TN //need to throw false
    }
  } // mmInfoVec not empty
  else
  {
    printCxrNoMerchAdata(carrier); // Carrier needs to be removed from the list
    return false;
  }
  return true;
}

bool
MerchActivationValidator::processMerchActivaionDataforHC(
    const std::vector<MerchActivationInfo*>& mmInfoVec,
    const CarrierCode& carrier,
    std::vector<ServiceGroup*>& groupCode,
    std::vector<ServiceGroup*>& dispOnlyGroups)
{
  std::vector<MerchActivationInfo*> mmInfoVecHC;
  std::vector<MerchActivationInfo*> mmInfoVecHCDefault;
  std::vector<MerchActivationInfo*> mmInfoVecDefaultBlank;

  bool isHCVec = false;
  bool isHCVecDefault = false;
  bool isHCVecDefaultBlank = false;

  std::vector<MerchActivationInfo*>::const_iterator mmItrHC = mmInfoVec.begin();
  for (; mmItrHC != mmInfoVec.end(); ++mmItrHC)
  {
    if (!validateMerchActivationData(**mmItrHC)) // validate MerchActivationData
      continue;

    MerchActivationInfo* mmInfo = *mmItrHC;
    if (mmInfo->includeInd() == NO)
      printActiveCxrGroup(carrier, false, mmInfo->groupCode());

    if (satisfyMMRecordHCCondition(*mmInfo, carrier))
    {
      mmInfoVecHC.push_back(mmInfo);
      isHCVec = true;
      continue;
    }
    else if (satisfyDefaultMMRecordHCCondition(*mmInfo, carrier)) // C - SABR
    {
      mmInfoVecHCDefault.push_back(mmInfo);
      isHCVecDefault = true;
      continue;
    }
    else if (satisfyDefaultMMRecordConditionBlank(*mmInfo, carrier)) // Blank
    {
      mmInfoVecDefaultBlank.push_back(mmInfo);
      isHCVecDefaultBlank = true;
      continue;
    }
    else
      continue;
  }

  if (isHCVec)
    return processMAdataForHCAndTN(mmInfoVecHC, carrier, groupCode, dispOnlyGroups);
  else if (isHCVecDefault)
    return processMAdataForHCAndTN(mmInfoVecHCDefault, carrier, groupCode, dispOnlyGroups);
  else if (isHCVecDefaultBlank)
    return processMAdataForHCAndTN(mmInfoVecDefaultBlank, carrier, groupCode, dispOnlyGroups);
  else
    return false; // remove carrier when no valid records in MA or no records that satisfies HC
                  // conditions

  return true;
}

bool
MerchActivationValidator::processMerchActivaionDataforTN(
    const std::vector<MerchActivationInfo*>& mmInfoVec,
    const CarrierCode& carrier,
    std::vector<ServiceGroup*>& groupCode,
    std::vector<ServiceGroup*>& dispOnlyGroups)
{
  std::vector<MerchActivationInfo*> mmInfoVecTN;
  std::vector<MerchActivationInfo*> mmInfoVecTNDefault;
  bool isTNVec = false;
  bool isTNVecDefault = false;

  std::vector<MerchActivationInfo*>::const_iterator mmItrTN = mmInfoVec.begin();
  for (; mmItrTN != mmInfoVec.end(); ++mmItrTN)
  {
    if (!validateMerchActivationData(**mmItrTN)) // validate MerchActivationData
      continue;

    MerchActivationInfo* mmInfo = *mmItrTN;
    if (mmInfo->includeInd() == NO)
      printActiveCxrGroup(carrier, false, mmInfo->groupCode());

    if (satisfyMMRecordTNCondition(*mmInfo, carrier))
    {
      mmInfoVecTN.push_back(mmInfo);
      isTNVec = true;
      continue;
    }
    else if (satisfyDefaultMMRecordConditionBlank(*mmInfo, carrier))
    {
      mmInfoVecTNDefault.push_back(mmInfo);
      isTNVecDefault = true;
      continue;
    }
    else
      continue;
  }

  if (isTNVec)
    return processMAdataForHCAndTN(mmInfoVecTN, carrier, groupCode, dispOnlyGroups);
  else if (isTNVecDefault)
    return processMAdataForHCAndTN(mmInfoVecTNDefault, carrier, groupCode, dispOnlyGroups);
  else
    return false; // remove carrier when no valid records in MA or no records that satisfies TN
                  // conditions

  return true;
}

bool
MerchActivationValidator::processMAdataForHCAndTN(
    const std::vector<MerchActivationInfo*>& mmInfoVec,
    const CarrierCode& cXr,
    std::vector<ServiceGroup*>& groups,
    std::vector<ServiceGroup*>& dispOnlyGroups)
{

  if (isAllMAdataPositive(mmInfoVec))
    addMerchGroupCodes(mmInfoVec, groups, dispOnlyGroups);
  else if (isAllMAdataNegative(mmInfoVec))
    return false; // remove carrier from the list as there is no group that is to be processed
  else
    processMerchGroupCodes(mmInfoVec, groups, dispOnlyGroups);

  if (!groups.empty())
  {
    printMerchActiveCxrGroups(cXr, true, groups);
    return true;
  }
  else
    return false; // remove carrier when there are no valid groups to be processed
}

// check if all records are positive
bool
MerchActivationValidator::isAllMAdataPositive(const std::vector<MerchActivationInfo*>& mmInfoVec)
{
  std::vector<MerchActivationInfo*>::const_iterator mmItr = mmInfoVec.begin();

  for (; mmItr != mmInfoVec.end(); ++mmItr)
  {
    MerchActivationInfo* mmInfo = *mmItr;
    if (mmInfo->includeInd() == NO)
      return false;
    else
      continue;
  }
  return true;
}

// check if all records are negative
bool
MerchActivationValidator::isAllMAdataNegative(const std::vector<MerchActivationInfo*>& mmInfoVec)
{
  std::vector<MerchActivationInfo*>::const_iterator mmItr = mmInfoVec.begin();

  for (; mmItr != mmInfoVec.end(); ++mmItr)
  {
    MerchActivationInfo* mmInfo = *mmItr;
    if (mmInfo->includeInd() == YES)
      return false;
    else
      continue;
  }
  return true;
}

// if all records are Positive add groups
void
MerchActivationValidator::addMerchGroupCodes(const std::vector<MerchActivationInfo*>& mmInfoVec,
                                             std::vector<ServiceGroup*>& svcGroupsVec,
                                             std::vector<ServiceGroup*>& dispOnlyGroups)
{
  std::vector<MerchActivationInfo*>::const_iterator mmItr = mmInfoVec.begin();
  std::vector<ServiceGroup*> validGroups;

  getMerchValidGroupCodes(validGroups);

  for (; mmItr != mmInfoVec.end(); ++mmItr)
  {
    MerchActivationInfo* mmInfo = *mmItr;
    if (mmInfo->includeInd() == YES && mmInfo->groupCode() != "**")
      svcGroupsVec.push_back(&mmInfo->groupCode());
    else if (mmInfo->includeInd() == YES && mmInfo->groupCode().equalToConst("**"))
    {
      svcGroupsVec.swap(validGroups);
      break;
    }
  }

  processDisplayOnlyGroupCodes(mmInfoVec, svcGroupsVec, dispOnlyGroups);
}

void
MerchActivationValidator::removeMerchGroupCodes(std::vector<ServiceGroup*>& svcGroupsVec,
                                                ServiceGroup& svcGroup)
{
  std::vector<ServiceGroup*>::iterator it = svcGroupsVec.begin();

  while (it != svcGroupsVec.end())
  {
    if ((**it) == svcGroup)
      it = svcGroupsVec.erase(it);
    else
      ++it;
  }
}

void
MerchActivationValidator::getMerchValidGroupCodes(std::vector<ServiceGroup*>& validGroups)
{
  std::vector<ServiceFeesGroup*>& groups = _trx.itin().front()->ocFeesGroup();

  typedef std::vector<ServiceFeesGroup*>::const_iterator SrvFeesGrpI;
  for (SrvFeesGrpI srvFeesGrpI = groups.begin(); srvFeesGrpI != groups.end(); ++srvFeesGrpI)
  {
    if ((*srvFeesGrpI)->groupCode().empty() || (*srvFeesGrpI)->groupDescription().empty())
      continue;
    else
      validGroups.push_back(&(*srvFeesGrpI)->groupCode());
  }
}

// When both Positive and Negative records exists
void
MerchActivationValidator::processMerchGroupCodes(const std::vector<MerchActivationInfo*>& mmInfoVec,
                                                 std::vector<ServiceGroup*>& svcGroupsVec,
                                                 std::vector<ServiceGroup*>& dispOnlyGroups)
{

  std::vector<ServiceGroup*> validGroups;
  getMerchValidGroupCodes(validGroups);
  // addMerchGroupCodes(mmInfoVec, svcGroupsVec);

  for (const auto mmInfo : mmInfoVec)
  {
    if (mmInfo->includeInd() == YES && mmInfo->groupCode() != "**")
      svcGroupsVec.push_back(&mmInfo->groupCode());
  }

  for (const auto mmInfo : mmInfoVec)
  {
    if (mmInfo->includeInd() == NO && mmInfo->groupCode() != "**")
    {
      if (!validGroups.empty())
        removeMerchGroupCodes(validGroups, mmInfo->groupCode());

      if (!svcGroupsVec.empty())
        removeMerchGroupCodes(svcGroupsVec, mmInfo->groupCode());
    }
  }

  for (const auto mmInfo : mmInfoVec)
  {
    if (mmInfo->includeInd() == NO && mmInfo->groupCode().equalToConst("**"))
      validGroups.clear();
  }

  for (const auto mmInfo : mmInfoVec)
  {
    if (mmInfo->includeInd() == YES && mmInfo->groupCode().equalToConst("**"))
      svcGroupsVec.swap(validGroups);
  }

  processDisplayOnlyGroupCodes(mmInfoVec, svcGroupsVec, dispOnlyGroups);
}

void
MerchActivationValidator::processDisplayOnlyGroupCodes(
    const std::vector<MerchActivationInfo*>& mmInfoVec,
    const std::vector<ServiceGroup*>& svcGroupsVec,
    std::vector<ServiceGroup*>& dispOnlyGroups)
{
  for (const auto mmInfo : mmInfoVec)
  {
    if (mmInfo->displayOnly() == YES && mmInfo->groupCode() != "**")
      dispOnlyGroups.push_back(&mmInfo->groupCode());
    else if (mmInfo->displayOnly() == YES && mmInfo->groupCode().equalToConst("**"))
    {
      dispOnlyGroups.assign(svcGroupsVec.begin(), svcGroupsVec.end());
      break;
    }
  }
}

bool
MerchActivationValidator::satisfyMMRecordHCCondition(const MerchActivationInfo& merchActInfo,
                                                     const CarrierCode& carrier)
{
  if (merchActInfo.userApplType() == MULTIHOST_USER_APPL &&
      merchActInfo.userAppl() == _trx.billing()->partitionID() &&
      merchActInfo.userAppl() == carrier)
    return true;

  return false;
}

bool
MerchActivationValidator::satisfyDefaultMMRecordHCCondition(const MerchActivationInfo& merchActInfo,
                                                            const CarrierCode& carrier)
{
  if ((merchActInfo.userApplType() == CRS_USER_APPL && merchActInfo.userAppl() == SABRE_USER))
    return true;

  return false;
}

bool
MerchActivationValidator::satisfyDefaultMMRecordConditionBlank(
    const MerchActivationInfo& merchActInfo, const CarrierCode& carrier)
{
  if (merchActInfo.userApplType() == NO_PARAM && merchActInfo.userAppl() == EMPTY_STRING())
    return true;

  return false;
}

bool
MerchActivationValidator::satisfyMMRecordTNCondition(const MerchActivationInfo& merchActInfo,
                                                     const CarrierCode& carrier)
{
  if (merchActInfo.userApplType() == CRS_USER_APPL &&
      merchActInfo.userAppl() == _trx.getRequest()->ticketingAgent()->agentTJR()->hostName())
    return true;

  return false;
}

void
MerchActivationValidator::printCxrNoMerchAdata(const CarrierCode cxr) const
{
  if (_diag875 != nullptr)
    _diag875->displayCxrNoMAdata(cxr);
}

void
MerchActivationValidator::printActiveCxrGroup(const CarrierCode cxr,
                                              bool active,
                                              const std::string& group) const
{
  if (_diag875 != nullptr)
    _diag875->displayActiveCxrGroup(cxr, active, group);
}

void
MerchActivationValidator::printMerchActiveCxrGroups(const CarrierCode cxr,
                                                    bool active,
                                                    std::vector<ServiceGroup*>& groupCode)
{
  if (_diag875 != nullptr)
  {
    std::string group("");
    std::vector<ServiceGroup*>::iterator it = groupCode.begin();
    std::vector<ServiceGroup*>::iterator itE = groupCode.end();

    for (; it != itE; ++it)
    {
      group.append(**it);
      group.append(" ");
    }

    _diag875->displayActiveCxrGroup(cxr, active, group);
  }
}
}
