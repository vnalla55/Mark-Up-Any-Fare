//----------------------------------------------------------------------------
//   2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//   or transfer of this software/documentation, in any medium, or incorporation of this
//   software/documentation into any system or publication, is strictly prohibited
//
//   ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/LocKey.h"

namespace tse
{

class TicketingFeesInfo
{
public:
  TicketingFeesInfo() = default;

  TicketingFeesInfo(const TicketingFeesInfo&) = delete;
  TicketingFeesInfo& operator=(const TicketingFeesInfo&) = delete;

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  DateTime& ticketEffDate() { return _ticketEffDate; }
  const DateTime& ticketEffDate() const { return _ticketEffDate; }

  DateTime& ticketDiscDate() { return _ticketDiscDate; }
  const DateTime& ticketDiscDate() const { return _ticketDiscDate; }

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  CarrierCode& carrier() { return _carrier; }
  const CarrierCode& carrier() const { return _carrier; }

  ServiceTypeCode& serviceTypeCode() { return _serviceTypeCode; }
  const ServiceTypeCode& serviceTypeCode() const { return _serviceTypeCode; }

  ServiceSubTypeCode& serviceSubTypeCode() { return _serviceSubTypeCode; }
  const ServiceSubTypeCode& serviceSubTypeCode() const { return _serviceSubTypeCode; }

  int& seqNo() { return _seqNo; }
  const int& seqNo() const { return _seqNo; }

  Indicator& publicPrivateInd() { return _publicPrivateInd; }
  const Indicator& publicPrivateInd() const { return _publicPrivateInd; }

  PaxTypeCode& paxType() { return _paxType; }
  const PaxTypeCode& paxType() const { return _paxType; }

  int& svcFeesAccCodeTblItemNo() { return _svcFeesAccCodeTblItemNo; }
  const int& svcFeesAccCodeTblItemNo() const { return _svcFeesAccCodeTblItemNo; }

  int& svcFeesTktDsgnTblItemNo() { return _svcFeesTktDsgnTblItemNo; }
  const int& svcFeesTktDsgnTblItemNo() const { return _svcFeesTktDsgnTblItemNo; }

  int& svcFeesSecurityTblItemNo() { return _svcFeesSecurityTblItemNo; }
  const int& svcFeesSecurityTblItemNo() const { return _svcFeesSecurityTblItemNo; }

  Indicator& journeyInd() { return _journeyInd; }
  const Indicator& journeyInd() const { return _journeyInd; }

  LocKey& loc1() { return _loc1; }
  const LocKey& loc1() const { return _loc1; }

  LocCode& loc1ZoneItemNo() { return _loc1ZoneItemNo; }
  const LocCode& loc1ZoneItemNo() const { return _loc1ZoneItemNo; }

  LocKey& loc2() { return _loc2; }
  const LocKey& loc2() const { return _loc2; }

  LocCode& loc2ZoneItemNo() { return _loc2ZoneItemNo; }
  const LocCode& loc2ZoneItemNo() const { return _loc2ZoneItemNo; }

  LocKey& locWhlWithin() { return _locWhlWithin; }
  const LocKey& locWhlWithin() const { return _locWhlWithin; }

  LocCode& locZoneWhlWithinItemNo() { return _locZoneWhlWithinItemNo; }
  const LocCode& locZoneWhlWithinItemNo() const { return _locZoneWhlWithinItemNo; }

  LocKey& locVia() { return _locVia; }
  const LocKey& locVia() const { return _locVia; }

  LocCode& locZoneViaItemNo() { return _locZoneViaItemNo; }
  const LocCode& locZoneViaItemNo() const { return _locZoneViaItemNo; }

  Indicator& fareInd() { return _fareInd; }
  const Indicator& fareInd() const { return _fareInd; }

  CarrierCode& primaryFareCarrier() { return _primaryFareCarrier; }
  const CarrierCode& primaryFareCarrier() const { return _primaryFareCarrier; }

  FareBasisCode& fareBasis() { return _fareBasis; }
  const FareBasisCode& fareBasis() const { return _fareBasis; }

  FopBinNumber& fopBinNumber() { return _fopBinNumber; }
  const FopBinNumber& fopBinNumber() const { return _fopBinNumber; }

  Indicator& refundReissue() { return _refundReissue; }
  const Indicator& refundReissue() const { return _refundReissue; }

  Indicator& commission() { return _commission; }
  const Indicator& commission() const { return _commission; }

  Indicator& interline() { return _interline; }
  const Indicator& interline() const { return _interline; }

  Indicator& noCharge() { return _noCharge; }
  const Indicator& noCharge() const { return _noCharge; }

  int& noDec() { return _noDec; }
  const int& noDec() const { return _noDec; }

  MoneyAmount& feeAmount() { return _feeAmount; }
  const MoneyAmount& feeAmount() const { return _feeAmount; }

  CurrencyCode& cur() { return _cur; }
  const CurrencyCode& cur() const { return _cur; }

  int& feePercentNoDec() { return _feePercentNoDec; }
  const int& feePercentNoDec() const { return _feePercentNoDec; }

  Percent& feePercent() { return _feePercent; }
  const Percent& feePercent() const { return _feePercent; }

  Indicator& taxInclude() { return _taxInclude; }
  const Indicator& taxInclude() const { return _taxInclude; }

  MoneyAmount& maxFeeAmount() { return _maxFeeAmount; }
  const MoneyAmount& maxFeeAmount() const { return _maxFeeAmount; }

  CurrencyCode& maxFeeCur() { return _maxFeeCur; }
  const CurrencyCode& maxFeeCur() const { return _maxFeeCur; }

  int& maxFeeNoDec() { return _maxFeeNoDec; }
  const int& maxFeeNoDec() const { return _maxFeeNoDec; }

  std::string& commercialName() { return _commercialName; }
  const std::string& commercialName() const { return _commercialName; }

  bool operator==(const TicketingFeesInfo& rhs) const
  {
    return ((_vendor == rhs._vendor) && (_carrier == rhs._carrier) &&
            (_serviceTypeCode == rhs._serviceTypeCode) &&
            (_serviceSubTypeCode == rhs._serviceSubTypeCode) && (_seqNo == rhs._seqNo) &&
            (_publicPrivateInd == rhs._publicPrivateInd) && (_expireDate == rhs._expireDate) &&
            (_createDate == rhs._createDate) && (_effDate == rhs._effDate) &&
            (_discDate == rhs._discDate) && (_ticketEffDate == rhs._ticketEffDate) &&
            (_ticketDiscDate == rhs._ticketDiscDate) && (_paxType == rhs._paxType) &&
            (_svcFeesAccCodeTblItemNo == rhs._svcFeesAccCodeTblItemNo) &&
            (_svcFeesTktDsgnTblItemNo == rhs._svcFeesTktDsgnTblItemNo) &&
            (_svcFeesSecurityTblItemNo == rhs._svcFeesSecurityTblItemNo) &&
            (_journeyInd == rhs._journeyInd) && (_loc1 == rhs._loc1) &&
            (_loc1ZoneItemNo == rhs._loc1ZoneItemNo) && (_loc2 == rhs._loc2) &&
            (_loc2ZoneItemNo == rhs._loc2ZoneItemNo) && (_locWhlWithin == rhs._locWhlWithin) &&
            (_locZoneWhlWithinItemNo == rhs._locZoneWhlWithinItemNo) && (_locVia == rhs._locVia) &&
            (_locZoneViaItemNo == rhs._locZoneViaItemNo) && (_fareInd == rhs._fareInd) &&
            (_primaryFareCarrier == rhs._primaryFareCarrier) && (_fareBasis == rhs._fareBasis) &&
            (_fopBinNumber == rhs._fopBinNumber) && (_refundReissue == rhs._refundReissue) &&
            (_commission == rhs._commission) && (_interline == rhs._interline) &&
            (_noCharge == rhs._noCharge) && (_noDec == rhs._noDec) &&
            (_feeAmount == rhs._feeAmount) && (_cur == rhs._cur) &&
            (_feePercentNoDec == rhs._feePercentNoDec) && (_feePercent == rhs._feePercent) &&
            (_taxInclude == rhs._taxInclude) && (_maxFeeAmount == rhs._maxFeeAmount) &&
            (_maxFeeCur == rhs._maxFeeCur) && (_maxFeeNoDec == rhs._maxFeeNoDec)) &&
            (_commercialName == rhs._commercialName);
  }

  bool compareFopBin(const FopBinNumber& fbn) const
  {
    for (int i = 0; i < 6; ++i)
    {
      if (_fopBinNumber[i] == fbn[i])
        continue;
      if (_fopBinNumber[i] == '*')
        return true;
      return false;
    }
    return true;
  }

  void getValuableData(const TicketingFeesInfo& obj)
  {
    _serviceTypeCode = obj.serviceTypeCode();
    _serviceSubTypeCode = obj.serviceSubTypeCode();
    _fopBinNumber = obj.fopBinNumber();
    _refundReissue = obj.refundReissue();
    _commission = obj.commission();
    _interline = obj.interline();
    _noCharge = obj.noCharge();
    _feePercent = obj.feePercent();
    _feePercentNoDec = obj.feePercentNoDec();
    _maxFeeAmount = obj.maxFeeAmount();
    _maxFeeNoDec = obj.maxFeeNoDec();
    _commercialName = obj.commercialName();
  }

  static void dummyData(TicketingFeesInfo& obj)
  {
    obj._vendor = "ATP";
    obj._carrier = "UA";
    obj._serviceTypeCode = "OB";
    obj._serviceSubTypeCode = "FCA";
    obj._seqNo = 1;
    obj._publicPrivateInd = 'P';
    obj._expireDate = time(nullptr);
    obj._createDate = time(nullptr);
    obj._effDate = time(nullptr);
    obj._discDate = time(nullptr);
    obj._ticketEffDate = time(nullptr);
    obj._ticketDiscDate = time(nullptr);
    obj._paxType = "CNN";
    obj._svcFeesAccCodeTblItemNo = 12;
    obj._svcFeesTktDsgnTblItemNo = 1234;
    obj._svcFeesSecurityTblItemNo = 0;
    obj._journeyInd = ' ';
    LocKey::dummyData(obj._loc1);
    obj._loc1ZoneItemNo = "0000001";
    LocKey::dummyData(obj._loc2);
    LocKey::dummyData(obj._locWhlWithin);
    obj._locZoneWhlWithinItemNo = "";
    LocKey::dummyData(obj._locVia);
    obj._locZoneViaItemNo = "1234567";
    obj._fareInd = 'A';
    obj._primaryFareCarrier = "AA";
    obj._fareBasis = "Y26";
    obj._fopBinNumber = "12****";
    obj._refundReissue = 'N';
    obj._commission = 'N';
    obj._interline = 'N';
    obj._noCharge = 'X';
    obj._noDec = 2;
    obj._feeAmount = 5.95;
    obj._cur = "USD";
    obj._feePercentNoDec = 2;
    obj._feePercent = 0;
    obj._taxInclude = ' ';
    obj._maxFeeAmount = 20.55;
    obj._maxFeeCur = "GBP";
    obj._maxFeeNoDec = 2;
    obj._commercialName = "1234567890";
  }

private:
  VendorCode _vendor;
  CarrierCode _carrier;
  ServiceTypeCode _serviceTypeCode;
  ServiceSubTypeCode _serviceSubTypeCode;
  int _seqNo = 0;
  Indicator _publicPrivateInd = ' ';
  DateTime _expireDate;
  DateTime _createDate;
  DateTime _effDate;
  DateTime _discDate;
  DateTime _ticketEffDate;
  DateTime _ticketDiscDate;
  PaxTypeCode _paxType;
  int _svcFeesAccCodeTblItemNo = 0;
  int _svcFeesTktDsgnTblItemNo = 0;
  int _svcFeesSecurityTblItemNo = 0;
  Indicator _journeyInd = ' ';
  LocKey _loc1;
  LocCode _loc1ZoneItemNo;
  LocKey _loc2;
  LocCode _loc2ZoneItemNo;
  LocKey _locWhlWithin;
  LocCode _locZoneWhlWithinItemNo;
  LocKey _locVia;
  LocCode _locZoneViaItemNo;
  Indicator _fareInd = ' ';
  CarrierCode _primaryFareCarrier;
  FareBasisCode _fareBasis;
  FopBinNumber _fopBinNumber;
  Indicator _refundReissue = ' ';
  Indicator _commission = ' ';
  Indicator _interline = ' ';
  Indicator _noCharge = ' ';
  int _noDec = 0;
  MoneyAmount _feeAmount = 0;
  CurrencyCode _cur;
  int _feePercentNoDec = 0;
  Percent _feePercent = 0;
  Indicator _taxInclude = ' ';
  MoneyAmount _maxFeeAmount = 0;
  CurrencyCode _maxFeeCur;
  int _maxFeeNoDec = 0;
  std::string _commercialName;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _vendor);
    FLATTENIZE(archive, _carrier);
    FLATTENIZE(archive, _serviceTypeCode);
    FLATTENIZE(archive, _serviceSubTypeCode);
    FLATTENIZE(archive, _seqNo);
    FLATTENIZE(archive, _publicPrivateInd);
    FLATTENIZE(archive, _expireDate);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _effDate);
    FLATTENIZE(archive, _discDate);
    FLATTENIZE(archive, _ticketEffDate);
    FLATTENIZE(archive, _ticketDiscDate);
    FLATTENIZE(archive, _paxType);
    FLATTENIZE(archive, _svcFeesAccCodeTblItemNo);
    FLATTENIZE(archive, _svcFeesTktDsgnTblItemNo);
    FLATTENIZE(archive, _svcFeesSecurityTblItemNo);
    FLATTENIZE(archive, _journeyInd);
    FLATTENIZE(archive, _loc1);
    FLATTENIZE(archive, _loc1ZoneItemNo);
    FLATTENIZE(archive, _loc2);
    FLATTENIZE(archive, _loc2ZoneItemNo);
    FLATTENIZE(archive, _locWhlWithin);
    FLATTENIZE(archive, _locZoneWhlWithinItemNo);
    FLATTENIZE(archive, _locVia);
    FLATTENIZE(archive, _locZoneViaItemNo);
    FLATTENIZE(archive, _fareInd);
    FLATTENIZE(archive, _primaryFareCarrier);
    FLATTENIZE(archive, _fareBasis);
    FLATTENIZE(archive, _fopBinNumber);
    FLATTENIZE(archive, _refundReissue);
    FLATTENIZE(archive, _commission);
    FLATTENIZE(archive, _interline);
    FLATTENIZE(archive, _noCharge);
    FLATTENIZE(archive, _noDec);
    FLATTENIZE(archive, _feeAmount);
    FLATTENIZE(archive, _cur);
    FLATTENIZE(archive, _feePercentNoDec);
    FLATTENIZE(archive, _feePercent);
    FLATTENIZE(archive, _taxInclude);
    FLATTENIZE(archive, _maxFeeAmount);
    FLATTENIZE(archive, _maxFeeCur);
    FLATTENIZE(archive, _maxFeeNoDec);
    FLATTENIZE(archive, _commercialName);
  }
};
}
