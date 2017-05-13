#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class CommissionContractInfo
{
 public:

  CommissionContractInfo()
    : _contractId(0)
    , _inhibit(' ')
    , _validityInd(' ')
  {
  }

  ~CommissionContractInfo()
  {
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  uint64_t& contractId() { return _contractId; }
  uint64_t contractId() const { return _contractId; }

  DateTime& effDateTime() { return _effDateTime; }
  DateTime effDateTime() const { return _effDateTime; }

  DateTime& expireDate() { return _expireDate; }
  DateTime expireDate() const { return _expireDate; }

  std::string& description() { return _description; }
  const std::string& description() const { return _description; }

  std::string& contractType() { return _contractType; }
  const std::string& contractType() const { return _contractType; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  PseudoCityCode& pcc() { return _pcc; }
  const PseudoCityCode& pcc() const { return _pcc; }

  PseudoCityCode& sourcePCC() { return _sourcePcc; }
  const PseudoCityCode& sourcePCC() const { return _sourcePcc; }

  Indicator& inhibit() { return _inhibit; }
  Indicator inhibit() const { return _inhibit; }

  Indicator& validityInd() { return _validityInd; }
  Indicator validityInd() const { return _validityInd; }

  bool operator ==(const CommissionContractInfo& rhs) const
  {
    bool eq(_vendor == rhs._vendor
            && _contractId == rhs._contractId
            && _effDateTime == rhs._effDateTime
            && _expireDate == rhs._expireDate
            && _description == rhs._description
            && _contractType == rhs._contractType
            && _carrier == rhs._carrier
            && _pcc == rhs._pcc
            && _sourcePcc == rhs._sourcePcc
            && _inhibit == rhs._inhibit
            && _validityInd == rhs._validityInd);
    return eq;
  }

  static void dummyData(CommissionContractInfo& obj)
  {
    obj._vendor = "COS";
    obj._contractId = 12345678912;
    obj._effDateTime = std::time(0);
    obj._expireDate = std::time(0);
    obj._description = "Abcdefgh ijkl";
    obj._contractType = "7932154321";
    obj._carrier = "AA";
    obj._pcc = "W0S";
    obj._sourcePcc = "W0S";
    obj._inhibit = 'N';
    obj._validityInd = 'Y';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _contractId);
    FLATTENIZE(archive, _effDateTime);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _description);
    FLATTENIZE(archive, _contractType);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _pcc);
    FLATTENIZE(archive, _sourcePcc);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _validityInd);
  }

 private:

  VendorCode _vendor;
  uint64_t _contractId;
  DateTime _effDateTime;
  DateTime _expireDate;
  std::string _description;
  std::string _contractType;
  CarrierCode _carrier;
  PseudoCityCode _pcc; // PCC that is sending the request
  PseudoCityCode _sourcePcc; // PCC that created comm rules
  Indicator _inhibit;
  Indicator _validityInd;
};

}// tse
