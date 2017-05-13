#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/TSEDateInterval.h"
#include "DBAccess/CommissionLocSegInfo.h"
#include "DBAccess/CommissionMarketSegInfo.h"
#include "DBAccess/CommissionTravelDatesSegInfo.h"

namespace tse
{

class CommissionProgramInfo
{
 public:

  CommissionProgramInfo()
    : _programId(0)
    , _pointOfSaleItemNo(-1)
    , _pointOfOriginItemNo(-1)
    , _travelDatesItemNo(-1)
    , _marketItemNo(-1)
    , _contractId(-1)
    , _qSurchargeInd(' ')
    , _throughFareInd(' ')
    , _maxConnectionTime(-1)
    , _landAgreementInd(' ')
    , _inhibit(' ')
    , _validityInd(' ')
  {
  }

  ~CommissionProgramInfo()
  {
    for (auto loc : _pointOfSale)
    {
      delete loc;
    }
    for (auto loc : _pointOfOrigin)
    {
      delete loc;
    }
    for (auto travelDate : _travelDates)
    {
      delete travelDate;
    }
    for (auto market : _markets)
    {
      delete market;
    }
  }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  uint64_t& programId() { return _programId; }
  uint64_t programId() const { return _programId; }

  TSEDateInterval& effInterval() { return _effInterval; }
  const TSEDateInterval& effInterval() const { return _effInterval; }

  DateTime& createDate() { return _effInterval.createDate(); }
  DateTime createDate() const { return _effInterval.createDate(); }

  DateTime& effDate() { return _effInterval.effDate(); }
  DateTime effDate() const { return _effInterval.effDate(); }

  DateTime& expireDate() { return _effInterval.expireDate(); }
  DateTime expireDate() const { return _effInterval.expireDate(); }

  DateTime& discDate() { return _effInterval.discDate(); }
  DateTime discDate() const { return _effInterval.discDate(); }

  std::string& programName() { return _programName; }
  const std::string& programName() const { return _programName; }

  int64_t& pointOfSaleItemNo() { return _pointOfSaleItemNo; }
  int64_t pointOfSaleItemNo() const { return _pointOfSaleItemNo; }

  int64_t& pointOfOriginItemNo() { return _pointOfOriginItemNo; }
  int64_t pointOfOriginItemNo() const { return _pointOfOriginItemNo; }

  int64_t& travelDatesItemNo() { return _travelDatesItemNo; }
  int64_t travelDatesItemNo() const { return _travelDatesItemNo; }

  DateTime& startTktDate() { return _startTktDate; }
  DateTime startTktDate() const { return _startTktDate; }

  DateTime& endTktDate() { return _endTktDate; }
  DateTime endTktDate() const { return _endTktDate; }

  int64_t& marketItemNo() { return _marketItemNo; }
  int64_t marketItemNo() const { return _marketItemNo; }

  int64_t& contractId() { return _contractId; }
  int64_t contractId() const { return _contractId; }

  Indicator& qSurchargeInd() { return _qSurchargeInd; }
  Indicator qSurchargeInd() const { return _qSurchargeInd; }

  Indicator& throughFareInd() { return _throughFareInd; }
  Indicator throughFareInd() const { return _throughFareInd; }

  int64_t& maxConnectionTime() { return _maxConnectionTime; }
  int64_t maxConnectionTime() const { return _maxConnectionTime; }

  Indicator& landAgreementInd() { return _landAgreementInd; }
  Indicator landAgreementInd() const { return _landAgreementInd; }

  Indicator& inhibit() { return _inhibit; }
  Indicator inhibit() const { return _inhibit; }

  Indicator& validityInd() { return _validityInd; }
  Indicator validityInd() const { return _validityInd; }

  std::vector<CommissionLocSegInfo*>& pointOfSale() { return _pointOfSale; }
  const std::vector<CommissionLocSegInfo*>& pointOfSale() const { return _pointOfSale; }

  std::vector<CommissionLocSegInfo*>& pointOfOrigin() { return _pointOfOrigin; }
  const std::vector<CommissionLocSegInfo*>& pointOfOrigin() const { return _pointOfOrigin; }

  std::vector<CommissionTravelDatesSegInfo*>& travelDates() { return _travelDates; }
  const std::vector<CommissionTravelDatesSegInfo*>& travelDates() const { return _travelDates; }

  std::vector<CommissionMarketSegInfo*>& markets() { return _markets; }
  const std::vector<CommissionMarketSegInfo*>& markets() const { return _markets; }

  bool operator ==(const CommissionProgramInfo& rhs) const
  {
    bool eq(_vendor == rhs._vendor
            && _programId == rhs._programId
            && _effInterval == rhs._effInterval
            && _programName == rhs._programName
            && _pointOfSaleItemNo == rhs._pointOfSaleItemNo
            && _pointOfOriginItemNo == rhs._pointOfOriginItemNo
            && _travelDatesItemNo == rhs._travelDatesItemNo
            && _startTktDate == rhs._startTktDate
            && _endTktDate == rhs._endTktDate
            && _marketItemNo == rhs._marketItemNo
            && _contractId == rhs._contractId
            && _qSurchargeInd == rhs._qSurchargeInd
            && _throughFareInd == rhs._throughFareInd
            && _maxConnectionTime == rhs._maxConnectionTime
            && _landAgreementInd == rhs._landAgreementInd
            && _inhibit == rhs._inhibit
            && _validityInd == rhs._validityInd
            && _pointOfSale.size() == rhs._pointOfSale.size()
            && _pointOfOrigin.size() == rhs._pointOfOrigin.size()
            && _travelDates.size() == rhs._travelDates.size()
            && _markets.size() == rhs._markets.size());
    for (size_t i = 0; i < _pointOfSale.size() && eq; ++i)
    {
      eq = *_pointOfSale[i] == *rhs._pointOfSale[i];
    }
    for (size_t i = 0; i < _pointOfOrigin.size() && eq; ++i)
    {
      eq = *_pointOfOrigin[i] == *rhs._pointOfOrigin[i];
    }
    for (size_t i = 0; i < _travelDates.size() && eq; ++i)
    {
      eq = *_travelDates[i] == *rhs._travelDates[i];
    }
    for (size_t i = 0; i < _markets.size() && eq; ++i)
    {
      eq = *_markets[i] == *rhs._markets[i];
    }
    return eq;
  }

  static void dummyData(CommissionProgramInfo& obj)
  {
    obj._vendor = "COS";
    obj._programId = 12345678912;
    TSEDateInterval::dummyData(obj._effInterval);
    obj._programName = "Abcdefgh ijkl";
    obj._pointOfSaleItemNo = 7932154321;
    obj._pointOfOriginItemNo = 2132154321;
    obj._travelDatesItemNo = 1232154321;
    obj._startTktDate = std::time(0);
    obj._endTktDate = std::time(0);
    obj._marketItemNo = 987654321;
    obj._contractId = 987;
    obj._qSurchargeInd = 'N';
    obj._throughFareInd = 'Y';
    obj._maxConnectionTime = 2400;
    obj._landAgreementInd = 'A';
    obj._inhibit = 'N';
    obj._validityInd = 'Y';
    CommissionLocSegInfo* loc1(new CommissionLocSegInfo);
    CommissionLocSegInfo::dummyData(*loc1);
    obj._pointOfSale.push_back(loc1);
    CommissionLocSegInfo* loc2(new CommissionLocSegInfo);
    CommissionLocSegInfo::dummyData(*loc2);
    obj._pointOfSale.push_back(loc2);
    CommissionLocSegInfo* origin1(new CommissionLocSegInfo);
    CommissionLocSegInfo::dummyData(*origin1);
    obj._pointOfOrigin.push_back(origin1);
    CommissionLocSegInfo* origin2(new CommissionLocSegInfo);
    CommissionLocSegInfo::dummyData(*origin2);
    obj._pointOfOrigin.push_back(origin2);
    CommissionTravelDatesSegInfo* travelDate1(new CommissionTravelDatesSegInfo);
    CommissionTravelDatesSegInfo::dummyData(*travelDate1);
    obj._travelDates.push_back(travelDate1);
    CommissionMarketSegInfo* market1(new CommissionMarketSegInfo);
    CommissionMarketSegInfo::dummyData(*market1);
    obj._markets.push_back(market1);
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _programId);
    FLATTENIZE(archive, _effInterval);
    FLATTENIZE(archive, _programName);
    FLATTENIZE(archive, _pointOfSaleItemNo);
    FLATTENIZE(archive, _pointOfOriginItemNo);
    FLATTENIZE(archive, _travelDatesItemNo);
    FLATTENIZE(archive, _startTktDate);
    FLATTENIZE(archive, _endTktDate);
    FLATTENIZE(archive, _marketItemNo);
    FLATTENIZE(archive, _contractId);
    FLATTENIZE(archive, _qSurchargeInd);
    FLATTENIZE(archive, _throughFareInd);
    FLATTENIZE(archive, _maxConnectionTime);
    FLATTENIZE(archive, _landAgreementInd);
    FLATTENIZE(archive, _inhibit);
    FLATTENIZE(archive, _validityInd);
    FLATTENIZE(archive, _pointOfSale);
    FLATTENIZE(archive, _pointOfOrigin);
    FLATTENIZE(archive, _travelDates);
    FLATTENIZE(archive, _markets);
  }

 private:

  VendorCode _vendor;
  uint64_t _programId;
  TSEDateInterval _effInterval;
  std::string _programName;
  int64_t _pointOfSaleItemNo;
  int64_t _pointOfOriginItemNo;
  int64_t _travelDatesItemNo;
  DateTime _startTktDate;
  DateTime _endTktDate;
  int64_t _marketItemNo;
  int64_t _contractId;
  Indicator _qSurchargeInd;
  Indicator _throughFareInd;
  int64_t _maxConnectionTime;
  Indicator _landAgreementInd;
  Indicator _inhibit;
  Indicator _validityInd;
  std::vector<CommissionLocSegInfo*> _pointOfSale;
  std::vector<CommissionLocSegInfo*> _pointOfOrigin;
  std::vector<CommissionTravelDatesSegInfo*> _travelDates;
  std::vector<CommissionMarketSegInfo*> _markets;
};

}// tse
