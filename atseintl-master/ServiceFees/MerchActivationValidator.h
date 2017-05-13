//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"

#include <vector>

namespace tse
{
class PricingTrx;
class MerchActivationInfo;
class Diag875Collector;

class MerchActivationValidator
{
  friend class MerchActivationValidatorTest;
  friend class OptionalFeeCollector;

public:
  MerchActivationValidator(PricingTrx& trx, Diag875Collector* _diag875);
  virtual ~MerchActivationValidator() {}

  bool validate(const CarrierCode& carrier) const;

protected:
  virtual const std::vector<MerchActivationInfo*>&
  getMerchActivationInfo(const CarrierCode& carrier) const;

  bool validateMerchActivationData(const MerchActivationInfo& merchActInfo) const;
  bool checkTravelDate(const MerchActivationInfo& merchActInfo) const;
  virtual const std::vector<MerchActivationInfo*>&
  getMerchActivation(const CarrierCode& carrier, const PseudoCityCode& pcc);
  bool retrieveMerchActivaionData(const CarrierCode& carrier,
                                  std::vector<ServiceGroup*>& groupCode,
                                  std::vector<ServiceGroup*>& dispOnlyGroups);
  bool processMerchActivaionDataforHC(const std::vector<MerchActivationInfo*>& mmInfoVec,
                                      const CarrierCode& carrier,
                                      std::vector<ServiceGroup*>& groupCode,
                                      std::vector<ServiceGroup*>& dispOnlyGroups);
  bool processMerchActivaionDataforTN(const std::vector<MerchActivationInfo*>& mmInfoVec,
                                      const CarrierCode& carrier,
                                      std::vector<ServiceGroup*>& groupCode,
                                      std::vector<ServiceGroup*>& dispOnlyGroups);
  bool processMAdataForHCAndTN(const std::vector<MerchActivationInfo*>& mmInfoVec,
                               const CarrierCode& cXr,
                               std::vector<ServiceGroup*>& groups,
                               std::vector<ServiceGroup*>& dispOnlyGroups);
  bool isAllMAdataPositive(const std::vector<MerchActivationInfo*>& mmInfoVec);
  bool isAllMAdataNegative(const std::vector<MerchActivationInfo*>& mmInfoVec);
  void addMerchGroupCodes(const std::vector<MerchActivationInfo*>& mmInfoVec,
                          std::vector<ServiceGroup*>& svcGroupsVec,
                          std::vector<ServiceGroup*>& dispOnlyGroups);
  void removeMerchGroupCodes(std::vector<ServiceGroup*>& svcGroupsVec, ServiceGroup& svcGroup);
  virtual void getMerchValidGroupCodes(std::vector<ServiceGroup*>& validGroups);
  void processMerchGroupCodes(const std::vector<MerchActivationInfo*>& mmInfoVec,
                              std::vector<ServiceGroup*>& svcGroupsVec,
                              std::vector<ServiceGroup*>& dispOnlyGroups);
  void processDisplayOnlyGroupCodes(const std::vector<MerchActivationInfo*>& mmInfoVec,
                                    const std::vector<ServiceGroup*>& svcGroupsVec,
                                    std::vector<ServiceGroup*>& dispOnlyGroups);
  bool
  satisfyMMRecordHCCondition(const MerchActivationInfo& merchActInfo, const CarrierCode& carrier);
  bool satisfyDefaultMMRecordHCCondition(const MerchActivationInfo& merchActInfo,
                                         const CarrierCode& carrier);
  bool satisfyDefaultMMRecordConditionBlank(const MerchActivationInfo& merchActInfo,
                                            const CarrierCode& carrier);
  bool
  satisfyMMRecordTNCondition(const MerchActivationInfo& merchActInfo, const CarrierCode& carrier);
  void printCxrNoMerchAdata(const CarrierCode cxr) const;
  void printActiveCxrGroup(const CarrierCode cxr,
                           bool active,
                           const std::string& group = EMPTY_STRING()) const;
  void printMerchActiveCxrGroups(const CarrierCode cxr,
                                 bool active,
                                 std::vector<ServiceGroup*>& groupCode);

private:
  PricingTrx& _trx;
  Diag875Collector* _diag875;
};
}

