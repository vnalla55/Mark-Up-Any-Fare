#pragma once

#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CommissionContractInfo.h"
#include "DBAccess/CommissionProgramInfo.h"
#include "DBAccess/CommissionRuleInfo.h"

#include <map>
#include <boost/container/flat_set.hpp>

namespace tse
{
class FareMarket;
class PaxTypeFare;

namespace amc
{
using CommissionTypeId = unsigned;
using CommissionValue = unsigned long;
using ContractId = unsigned long;

static const CommissionTypeId CT9=9;
static const CommissionTypeId CT10=10;
static const CommissionTypeId CT11=11;
static const CommissionTypeId CT12=12;

struct CommissionProgramKey
{
  const CarrierCode& valCxr;
  const FareMarket* fareMarket;

  CommissionProgramKey(const CarrierCode& c, const FareMarket* fm)
    : valCxr(c), fareMarket(fm) { }

  bool operator< (const CommissionProgramKey& other) const
  {
    if (this->valCxr < other.valCxr)
      return true;
    if (this->valCxr > other.valCxr)
      return false;

    if (this->fareMarket < other.fareMarket)
      return true;
    if (this->fareMarket > other.fareMarket)
      return false;

    return false;
  }
};

struct CommissionRuleKey
{
  const CarrierCode& valCxr;
  const FareMarket* fareMarket;
  const PaxTypeFare* ptFare;

  CommissionRuleKey(const CarrierCode& c, const FareMarket* fm, const PaxTypeFare* ptf)
    : valCxr(c), fareMarket(fm), ptFare(ptf) { }

  bool operator< (const CommissionRuleKey& other) const
  {
    if (this->valCxr < other.valCxr)
      return true;
    if (this->valCxr > other.valCxr)
      return false;

    if (this->fareMarket < other.fareMarket)
      return true;
    if (this->fareMarket > other.fareMarket)
      return false;

    if (this->ptFare < other.ptFare)
      return true;
    if (this->ptFare > other.ptFare)
      return false;

    return false;
  }
};

/// Holds all comm-programs for given comm-id
struct ContractCommProgramData
{
  const CommissionContractInfo& ccInfo;
  std::vector<const CommissionProgramInfo*> cpInfoCol;

  ContractCommProgramData(const CommissionContractInfo& c)
    : ccInfo(c) {}
};

class CommissionRuleData
{
  public:
    CommissionRuleData()
      : _crInfo(nullptr), _cpInfo(nullptr), _ccInfo(nullptr), _qSurchargeApplicable(false)
    {}

    CommissionRuleData(
        const CommissionRuleInfo* cri,
        const CommissionProgramInfo* cpi,
        const CommissionContractInfo* cci)
      : _crInfo(cri), _cpInfo(cpi), _ccInfo(cci), _qSurchargeApplicable(cpi && cpi->qSurchargeInd()=='Y')
    { }

    bool isQSurchargeApplicable() const { return _qSurchargeApplicable; }
    ContractId contractId() const { return _ccInfo ? _ccInfo->contractId():-1; }
    const CommissionRuleInfo* commRuleInfo() const { return _crInfo; }
    const CommissionProgramInfo* commProgInfo() const { return _cpInfo; }
    const CommissionContractInfo* commContInfo() const { return _ccInfo; }
    //MoneyAmount commissionValue() const { return cri ? cri->commissionValue():0; }
    //int commissionTypeId() const { return cri ? cri->commissionTypeId():0; }

    bool operator< (const CommissionRuleData& other) const
    {
      if (!_crInfo && !other.commRuleInfo())
        return false;

      if (_crInfo && !other.commRuleInfo())
        return true;

      if (!_crInfo && other.commRuleInfo())
        return false;

      // One with q-surcharge comes first
      if (fabs(_crInfo->commissionValue() - other.commRuleInfo()->commissionValue()) <= EPSILON)
      {
        if (isQSurchargeApplicable())
          return true;

        if (other.isQSurchargeApplicable())
          return false;
      }

      return _crInfo->commissionValue() > other.commRuleInfo()->commissionValue();
    }

  private:
    const CommissionRuleInfo* _crInfo;
    const CommissionProgramInfo* _cpInfo;
    const CommissionContractInfo* _ccInfo;
    bool _qSurchargeApplicable;
};

struct compareMoneyAmount  : public std::binary_function<MoneyAmount, MoneyAmount, bool>
{
  bool operator()(const MoneyAmount a, const MoneyAmount b) const
  {
    MoneyAmount diff = b - a;
    if( fabs(diff) < EPSILON )
      return false;
    return a > b;
  }
};

struct util
{

static std::string
concatenateValCxrForCommMsg(const std::vector<CarrierCode>& cxrs)
{
  std::string res("");
  for (std::vector<CarrierCode>::const_iterator it = cxrs.begin();
      it != cxrs.end();
      ++it)
  {
    res += *it;
    if (it != cxrs.end() - 1)
      res += "/";
  }
  return res;
}

};

/// Collect commission data for FC. This object is stored in FareUsage
class FcCommissionData
{
  public:
    FcCommissionData() {}
    FcCommissionData(
        MoneyAmount fcCommAmt,
        const amc::CommissionRuleData& commRuleData)
      : _fcCommAmt(fcCommAmt), _commRuleData(commRuleData) { }

    MoneyAmount& fcCommAmt() { return _fcCommAmt; }
    const MoneyAmount fcCommAmt() const { return _fcCommAmt; }

    amc::CommissionRuleData&  commRuleData() { return _commRuleData; }
    const amc::CommissionRuleData&  commRuleData() const { return _commRuleData; }

  private:
    MoneyAmount _fcCommAmt = 0;
    amc::CommissionRuleData _commRuleData;
};

using SortedCommRuleDataVec = boost::container::flat_set<CommissionRuleData>;
using CommRuleDataColPerCT = std::map<CommissionTypeId, SortedCommRuleDataVec>;
//using ContractCommRuleDataColPerCTMap = std::map<ContractId, CommRuleDataColPerCT>;

class FcCommissionRuleData
{
  public:
    friend class FcCommissionRuleDataTest;
    FcCommissionRuleData() { }
    virtual ~ FcCommissionRuleData() { }

    const CommRuleDataColPerCT& commRuleDataColPerCommType() const { return _crdColPerCt; }
    CommRuleDataColPerCT& commRuleDataColPerCommType() { return _crdColPerCt; }

    bool doesFcHaveCommissionType(CommissionTypeId ct) const
    {
      return _crdColPerCt.find(ct) != _crdColPerCt.end();
    }

  private:
    CommRuleDataColPerCT _crdColPerCt;
};

//@todo Rename it. Can it be a vector?
using ContractIdCol = boost::container::flat_set<const CommissionContractInfo*>;
using VCFMCommProgPair =
  std::pair<CommissionProgramKey, std::vector<ContractCommProgramData>>;
using VCFMCommissionPrograms =
  std::map<CommissionProgramKey, std::vector<ContractCommProgramData>>;

using ContractFcCommRuleDataMap = std::map<ContractId, FcCommissionRuleData*>;
using ContractFcCommRuleDataPair = std::pair<ContractId, FcCommissionRuleData*>;

using VCFMPTFCommissionRules = std::map<CommissionRuleKey, ContractFcCommRuleDataMap>;
using VCFMPTFCommRulePair = std::pair<CommissionRuleKey, ContractFcCommRuleDataMap>;

using VCFMPTFCommRulesPerValCxr = std::map<CarrierCode, VCFMPTFCommissionRules>;
using VCFMPTFCommRulesPerValCxrPair = std::pair<CarrierCode, VCFMPTFCommissionRules>;

} // amc
} // tse

