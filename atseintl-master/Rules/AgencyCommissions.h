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
//-------------------------------------------------------------------

/**
 * \class AgencyCommissions
 *
 * \brief Calculate agency commission
 *  This class is an entry point for authorizing, validating, and calculating agency commission.
 *
 * Created on: Friday Sep 24 14:48:40 2015
 */

#pragma once

#include "Common/CommissionKeys.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PricingTrx.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleUtil.h"

#include <boost/logic/tribool.hpp>

namespace tse
{
class CommissionContractInfo;
class CommissionRuleInfo;
class Diag867Collector;
class FarePath;
class FareUsage;

namespace amc
{

class CommissionRuleKey;
class CommissionRuleCollector;
using FcCommissionInfoPairVec = std::vector<std::pair<FareUsage*, FcCommissionData*>>;
using FcCommissionRulePairVec = std::vector<std::pair<FareUsage*, CommissionRuleData>>;

class AgencyCommissions
{
  friend class AgencyCommissionsTest;
  public:
    AgencyCommissions(PricingTrx& trx, Diag867Collector* diag867);
    virtual ~AgencyCommissions();

    /** This method will be called by the client code to caculate agency commission.
      * This method will check security, find valid commision rule to apply and
      * calcuate commission and store them in FarePath.
      */
    bool getAgencyCommissions(
        FarePath& farePath,
        const std::vector<CustomerSecurityHandshakeInfo*>& csHsInfoCol,
        VCFMCommissionPrograms& vcfmCommProgs,
        VCFMPTFCommissionRules& vcfmptfCommRules);

  protected:
    // Struct used to store original FC of a CommissionRuleInfo selected
    class SelectedFcCommRuleData
    {
      private:
        const FcCommissionRuleData* _fcCommRuleData; // @todo remove it with phase 1 fallback
        CommissionRuleData _selectedCrd;

      public:
        SelectedFcCommRuleData()
          : _fcCommRuleData(nullptr), _selectedCrd(CommissionRuleData())
        { }

        SelectedFcCommRuleData(
            const FcCommissionRuleData* fcCommRuleData,
            const CommissionRuleData& crd)
          : _fcCommRuleData(fcCommRuleData), _selectedCrd(crd)
        {
        }

        const FcCommissionRuleData* fcCommRuleData() const { return _fcCommRuleData; }
        const CommissionRuleData& commissionRuleData() const { return _selectedCrd; }

        bool operator< (const SelectedFcCommRuleData& other) const
        {
          const CommissionRuleData& otherCrd = other.commissionRuleData();
          if (!_selectedCrd.commRuleInfo() && !otherCrd.commRuleInfo())
            return false;

          if (_selectedCrd.commRuleInfo() && !otherCrd.commRuleInfo())
            return true;

          if (!_selectedCrd.commRuleInfo() && otherCrd.commRuleInfo())
            return false;

          // One with q-surcharge comes first
          if (fabs(_selectedCrd.commRuleInfo()->commissionValue() -
                otherCrd.commRuleInfo()->commissionValue()) <= EPSILON)
          {
            if (_selectedCrd.isQSurchargeApplicable())
              return true;

            if (otherCrd.isQSurchargeApplicable())
              return false;
          }

          return _selectedCrd.commRuleInfo()->commissionValue() > otherCrd.commRuleInfo()->commissionValue();
        }
    };

    // delete it and related code with fallbackAMC2
    using FcCommRuleDataCol = boost::container::flat_set<SelectedFcCommRuleData>;

  private:
    boost::logic::tribool isCommRuleDataColHasSurcharge(const SortedCommRuleDataVec& crdCol) const;
    bool calculateCommission(
        FarePath& fp,
        const VCFMPTFCommRulesPerValCxr& commRulesPerKey);

    void collectCommissionRuleData(
        const CarrierCode& valCxr,
        const PseudoCityCode& pcc,
        const std::vector<CustomerSecurityHandshakeInfo*>& csHsInfoCol,
        const ContractIdCol& anyCxrContractIdCol,
        CommissionRuleCollector& crCollector) const;

    void collectContractIdForAgency(
        const CarrierCode& v,
        const PseudoCityCode& p,
        const std::vector<CustomerSecurityHandshakeInfo*>& csHsInfoCol,
        ContractIdCol& contractIdCol) const;

    bool checkCommType(
        const VCFMPTFCommissionRules& crdPerFc,
        ContractId& contId,
        CommissionTypeId ct) const;

    bool checkForContractAndCommType(
        const VCFMPTFCommissionRules& vcfmptfCommRules,
        ContractId contId,
        CommissionTypeId ct) const;

    bool findMaxCommRulesForCT9(
        const FarePath& fpath,
        const VCFMPTFCommissionRules& vcfmptfCommRules,
        FcCommRuleDataCol& fcCrdCol,
        ContractId contId) const;

    bool findMaxCommRulesForCT9(
        const FarePath& fpath,
        const VCFMPTFCommissionRules& vcfmptfCommRules,
        const ContractId contId,
        FcCommissionRulePairVec& fcRuleCol) const;

    bool checkCommType11(
        const VCFMPTFCommissionRules& crdPerFc,
        uint16_t& nonCommFcCnt,
        ContractId& contId) const;

    bool checkForContractAndCommType11(
        const VCFMPTFCommissionRules& vcfmptfCommRules,
        const ContractId& contId,
        uint16_t& nonCommFcCnt) const;

    bool calculateMaxCommForSingleFc(
        FarePath& fp,
        const VCFMPTFCommissionRules& commRuleDataPerFc,
        FcCommissionInfoPairVec& fcAgcyCommDataCol,
        MoneyAmount& maxAmt) const;

    bool calculateMaxCommForMultipleFc(
        FarePath& fpath,
        const VCFMPTFCommissionRules& vcfmptfCommRules,
        FcCommissionInfoPairVec& fcAgcyCommDataCol,
        MoneyAmount& maxAmt) const;

    MoneyAmount collectCT9FromSelectedCommRuleData(
        FarePath& fpath,
        FcCommissionRulePairVec& ct9FcCrdCol,
        const CommissionRuleData& selectedCrd,
        FcCommissionInfoPairVec& fcAgcyCommDataCol) const;

    bool calculateCommissionForCT9(
        FarePath& fp,
        const VCFMPTFCommissionRules& commRuleDataPerFc,
        FcCommissionInfoPairVec& fcAgcyCommDataCol,
        MoneyAmount& maxAmt) const;

    bool calculateCommissionForCT10(
        FarePath& fp,
        const VCFMPTFCommissionRules& crdPerFc,
        FcCommissionInfoPairVec& fcAgcyCommDataCol,
        MoneyAmount& maxAmt) const;

    bool calculateCommissionForCT11(
        FarePath& fp,
        const VCFMPTFCommissionRules& crdPerFc,
        FcCommissionInfoPairVec& fcAgcyCommDataCol,
        MoneyAmount& maxAmt) const;

    bool collectCommForCT11(
        const FarePath& fpath,
        const FareUsage& fu,
        const FcCommissionRuleData& fcCrd,
        const uint16_t nonCommFcCnt,
        CommissionRuleData& selectedCrd,
        MoneyAmount& commAmt) const;

    bool collectCommForCT10(
        const FarePath& fpath,
        const FareUsage& fu,
        const FcCommissionRuleData& fcCrd,
        CommissionRuleData& selectedCrd,
        MoneyAmount& commAmt) const;

    FareUsage* findFareUsage(const FarePath& fp, const CommissionRuleKey& crKey) const;

    const std::vector<CommissionContractInfo*>&
      getCommissionContractInfo(const CarrierCode& valCxr, const PseudoCityCode& pcc) const
    {
      return _trx.dataHandle().getCommissionContract(COS_VENDOR_CODE, valCxr, pcc);
    }

    MoneyAmount convertCurrency(
        const FarePath& fp,
        MoneyAmount sourceAmount,
        const CurrencyCode& sourceCurrency,
        const CurrencyCode& targetCurrency,
        bool doNonIataRounding=false)const
    {
      return RuleUtil::convertCurrency(_trx, fp, sourceAmount, sourceCurrency, targetCurrency, doNonIataRounding);
    }

    MoneyAmount getNetTotalAmount(
        const FarePath& fp,
        const FareUsage& fu,
        bool applyQSurcharge) const;

    MoneyAmount getMaxCommAmount(
        const FarePath& fp,
        const FareUsage& fu,
        const SortedCommRuleDataVec& crdCol,
        CommissionRuleData& selectedCrd) const;

    MoneyAmount getMaxCommAmountWithoutSurcharge(
        const FarePath& fp,
        const FareUsage& fu,
        const SortedCommRuleDataVec& crdCol,
        CommissionRuleData& selectedCrd) const;

    MoneyAmount getMaxCommAmountWithSurcharge(
        const FarePath& fp,
        const FareUsage& fu,
        const SortedCommRuleDataVec& crdCol,
        CommissionRuleData& selectedCrd) const;

    bool checkApplyQSurcharge(
        const FcCommRuleDataCol& selectedFcCrdCol,
        const FcCommissionRuleData* fcCommRuleData) const;

    void storeFcCommissionDataInFC(
        const CarrierCode& valCxr,
        const FcCommissionInfoPairVec& fcAgcyCommDataCol) const;
    void storeFcCommissionData(
        FcCommissionInfoPairVec& fcAgcyCommDataCol,
        FareUsage& fu,
        const CommissionRuleData& crd,
        const MoneyAmount amt) const;

    PricingTrx& _trx;
    Diag867Collector* _diag867;
};

} // amc
} // tse
