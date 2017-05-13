// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class WBuffer;
class RBuffer;

class TaxRulesRecord
{
public:
  typedef Code<3> TaxType;
  typedef Code<3> StopoverTimeTag;
  typedef Code<2> TaxProcessingApplTag;
  typedef Code<2> TaxMatchingApplTag;

  TaxRulesRecord();

  bool operator==(const TaxRulesRecord& rhs) const;

  void flattenize(Flattenizable::Archive& archive);

  static void dummyData(TaxRulesRecord& obj);

  WBuffer& write(WBuffer& os) const;

  RBuffer& read(RBuffer& is);

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr);

  std::ostream& print(std::ostream& out) const;

  VendorCode& vendor() { return _vendor; }
  const VendorCode& vendor() const { return _vendor; }

  NationCode& nation() { return _nation; }
  const NationCode& nation() const { return _nation; }

  TaxCode& taxCode() { return _taxCode; }
  const TaxCode& taxCode() const { return _taxCode; }

  Indicator& taxRemittanceId() { return _taxRemittanceId; }
  const Indicator& taxRemittanceId() const { return _taxRemittanceId; }

  TaxType& taxType() { return _taxType; }
  const TaxType& taxType() const { return _taxType; }

  Indicator& taxPointTag() { return _taxPointTag; }
  const Indicator& taxPointTag() const { return _taxPointTag; }

  Indicator& percentFlatTag() { return _percentFlatTag; }
  const Indicator& percentFlatTag() const { return _percentFlatTag; }

  int32_t& seqNo() { return _seqNo; }
  const int32_t& seqNo() const { return _seqNo; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  uint64_t& versionDate() { return _versionDate; }
  const uint64_t& versionDate() const { return _versionDate; }

  DateTime& expireDate() { return _expireDate; }
  const DateTime& expireDate() const { return _expireDate; }

  DateTime& lockDate() { return _lockDate; }
  const DateTime& lockDate() const { return _lockDate; }

  Indicator& taxableUnitTag1() { return _taxableUnitTag1; }
  const Indicator& taxableUnitTag1() const { return _taxableUnitTag1; }

  Indicator& taxableUnitTag2() { return _taxableUnitTag2; }
  const Indicator& taxableUnitTag2() const { return _taxableUnitTag2; }

  Indicator& taxableUnitTag3() { return _taxableUnitTag3; }
  const Indicator& taxableUnitTag3() const { return _taxableUnitTag3; }

  Indicator& taxableUnitTag4() { return _taxableUnitTag4; }
  const Indicator& taxableUnitTag4() const { return _taxableUnitTag4; }

  Indicator& taxableUnitTag5() { return _taxableUnitTag5; }
  const Indicator& taxableUnitTag5() const { return _taxableUnitTag5; }

  Indicator& taxableUnitTag6() { return _taxableUnitTag6; }
  const Indicator& taxableUnitTag6() const { return _taxableUnitTag6; }

  Indicator& taxableUnitTag7() { return _taxableUnitTag7; }
  const Indicator& taxableUnitTag7() const { return _taxableUnitTag7; }

  Indicator& taxableUnitTag8() { return _taxableUnitTag8; }
  const Indicator& taxableUnitTag8() const { return _taxableUnitTag8; }

  Indicator& taxableUnitTag9() { return _taxableUnitTag9; }
  const Indicator& taxableUnitTag9() const { return _taxableUnitTag9; }

  Indicator& taxableUnitTag10() { return _taxableUnitTag10; }
  const Indicator& taxableUnitTag10() const { return _taxableUnitTag10; }

  DateTime& effDate() { return _effDate; }
  const DateTime& effDate() const { return _effDate; }

  DateTime& discDate() { return _discDate; }
  const DateTime& discDate() const { return _discDate; }

  Indicator& travelDateAppTag() { return _travelDateAppTag; }
  const Indicator& travelDateAppTag() const { return _travelDateAppTag; }

  uint16_t& tvlFirstYear() { return _tvlFirstYear; }
  const uint16_t& tvlFirstYear() const { return _tvlFirstYear; }

  uint16_t& tvlFirstMonth() { return _tvlFirstMonth; }
  const uint16_t& tvlFirstMonth() const { return _tvlFirstMonth; }

  uint16_t& tvlFirstDay() { return _tvlFirstDay; }
  const uint16_t& tvlFirstDay() const { return _tvlFirstDay; }

  uint16_t& tvlLastYear() { return _tvlLastYear; }
  const uint16_t& tvlLastYear() const { return _tvlLastYear; }

  uint16_t& tvlLastMonth() { return _tvlLastMonth; }
  const uint16_t& tvlLastMonth() const { return _tvlLastMonth; }

  uint16_t& tvlLastDay() { return _tvlLastDay; }
  const uint16_t& tvlLastDay() const { return _tvlLastDay; }

  CarrierCode& taxCarrier() { return _taxCarrier; }
  const CarrierCode& taxCarrier() const { return _taxCarrier; }

  uint16_t& calcOrder() { return _calcOrder; }
  const uint16_t& calcOrder() const { return _calcOrder; }

  uint32_t& carrierApplItemNo1() { return _carrierApplItemNo1; }
  const uint32_t& carrierApplItemNo1() const { return _carrierApplItemNo1; }

  uint32_t& carrierFltItemNo2() { return _carrierFltItemNo2; }
  const uint32_t& carrierFltItemNo2() const { return _carrierFltItemNo2; }

  Indicator& rtnToOrig() { return _rtnToOrig; }
  const Indicator& rtnToOrig() const { return _rtnToOrig; }

  uint32_t& psgrTypeCodeItemNo() { return _psgrTypeCodeItemNo; }
  const uint32_t& psgrTypeCodeItemNo() const { return _psgrTypeCodeItemNo; }

  Indicator& ticketedPointTag() { return _ticketedPointTag; }
  const Indicator& ticketedPointTag() const { return _ticketedPointTag; }

  LocTypeCode& posLocType() { return _posLocType; }
  const LocTypeCode& posLocType() const { return _posLocType; }

  LocCode& posLoc() { return _posLoc; }
  const LocCode& posLoc() const { return _posLoc; }

  uint32_t& posLocZoneTblNo() { return _posLocZoneTblNo; }
  const uint32_t& posLocZoneTblNo() const { return _posLocZoneTblNo; }

  uint32_t& svcFeesSecurityItemNo() { return _svcFeesSecurityItemNo; }
  const uint32_t& svcFeesSecurityItemNo() const { return _svcFeesSecurityItemNo; }

  LocTypeCode& poTktLocType() { return _poTktLocType; }
  const LocTypeCode& poTktLocType() const { return _poTktLocType; }

  LocCode& poTktLoc() { return _poTktLoc; }
  const LocCode& poTktLoc() const { return _poTktLoc; }

  uint32_t& poTktLocZoneTblNo() { return _poTktLocZoneTblNo; }
  const uint32_t& poTktLocZoneTblNo() const { return _poTktLocZoneTblNo; }

  LocTypeCode& poDeliveryLocType() { return _poDeliveryLocType; }
  const LocTypeCode& poDeliveryLocType() const { return _poDeliveryLocType; }

  LocCode& poDeliveryLoc() { return _poDeliveryLoc; }
  const LocCode& poDeliveryLoc() const { return _poDeliveryLoc; }

  uint32_t& poDeliveryZoneTblNo() { return _poDeliveryZoneTblNo; }
  const uint32_t& poDeliveryZoneTblNo() const { return _poDeliveryZoneTblNo; }

  Indicator& jrnyInd() { return _jrnyInd; }
  const Indicator& jrnyInd() const { return _jrnyInd; }

  LocTypeCode& jrnyLoc1Type() { return _jrnyLoc1Type; }
  const LocTypeCode& jrnyLoc1Type() const { return _jrnyLoc1Type; }

  LocCode& jrnyLoc1() { return _jrnyLoc1; }
  const LocCode& jrnyLoc1() const { return _jrnyLoc1; }

  uint32_t& jrnyLoc1ZoneTblNo() { return _jrnyLoc1ZoneTblNo; }
  const uint32_t& jrnyLoc1ZoneTblNo() const { return _jrnyLoc1ZoneTblNo; }

  LocTypeCode& jrnyLoc2Type() { return _jrnyLoc2Type; }
  const LocTypeCode& jrnyLoc2Type() const { return _jrnyLoc2Type; }

  LocCode& jrnyLoc2() { return _jrnyLoc2; }
  const LocCode& jrnyLoc2() const { return _jrnyLoc2; }

  uint32_t& jrnyLoc2ZoneTblNo() { return _jrnyLoc2ZoneTblNo; }
  const uint32_t& jrnyLoc2ZoneTblNo() const { return _jrnyLoc2ZoneTblNo; }

  LocTypeCode& jrnyIncludesLocType() { return _jrnyIncludesLocType; }
  const LocTypeCode& jrnyIncludesLocType() const { return _jrnyIncludesLocType; }

  LocCode& jrnyIncludesLoc() { return _jrnyIncludesLoc; }
  const LocCode& jrnyIncludesLoc() const { return _jrnyIncludesLoc; }

  uint32_t& jrnyIncludesLocZoneTblNo() { return _jrnyIncludesLocZoneTblNo; }
  const uint32_t& jrnyIncludesLocZoneTblNo() const { return _jrnyIncludesLocZoneTblNo; }

  LocCode& trvlWhollyWithinLoc() { return _trvlWhollyWithinLoc; }
  const LocCode& trvlWhollyWithinLoc() const { return _trvlWhollyWithinLoc; }

  LocTypeCode& trvlWhollyWithinLocType() { return _trvlWhollyWithinLocType; }
  const LocTypeCode& trvlWhollyWithinLocType() const { return _trvlWhollyWithinLocType; }

  uint32_t& trvlWhollyWithinLocZoneTblNo() { return _trvlWhollyWithinLocZoneTblNo; }
  const uint32_t& trvlWhollyWithinLocZoneTblNo() const { return _trvlWhollyWithinLocZoneTblNo; }

  Indicator& taxPointLoc1IntDomInd() { return _taxPointLoc1IntDomInd; }
  const Indicator& taxPointLoc1IntDomInd() const { return _taxPointLoc1IntDomInd; }

  LocTypeCode& taxPointLoc1TrnsfrType() { return _taxPointLoc1TrnsfrType; }
  const LocTypeCode& taxPointLoc1TrnsfrType() const { return _taxPointLoc1TrnsfrType; }

  Indicator& taxPointLoc1StopoverTag() { return _taxPointLoc1StopoverTag; }
  const Indicator& taxPointLoc1StopoverTag() const { return _taxPointLoc1StopoverTag; }

  LocTypeCode& taxPointLoc1Type() { return _taxPointLoc1Type; }
  const LocTypeCode& taxPointLoc1Type() const { return _taxPointLoc1Type; }

  LocCode& taxPointLoc1() { return _taxPointLoc1; }
  const LocCode& taxPointLoc1() const { return _taxPointLoc1; }

  uint32_t& taxPointLoc1ZoneTblNo() { return _taxPointLoc1ZoneTblNo; }
  const uint32_t& taxPointLoc1ZoneTblNo() const { return _taxPointLoc1ZoneTblNo; }

  Indicator& taxPointLoc2IntlDomInd() { return _taxPointLoc2IntlDomInd; }
  const Indicator& taxPointLoc2IntlDomInd() const { return _taxPointLoc2IntlDomInd; }

  Indicator& taxPointLoc2Compare() { return _taxPointLoc2Compare; }
  const Indicator& taxPointLoc2Compare() const { return _taxPointLoc2Compare; }

  Indicator& taxPointLoc2StopoverTag() { return _taxPointLoc2StopoverTag; }
  const Indicator& taxPointLoc2StopoverTag() const { return _taxPointLoc2StopoverTag; }

  LocTypeCode& taxPointLoc2Type() { return _taxPointLoc2Type; }
  const LocTypeCode& taxPointLoc2Type() const { return _taxPointLoc2Type; }

  LocCode& taxPointLoc2() { return _taxPointLoc2; }
  const LocCode& taxPointLoc2() const { return _taxPointLoc2; }

  uint32_t& taxPointLoc2ZoneTblNo() { return _taxPointLoc2ZoneTblNo; }
  const uint32_t& taxPointLoc2ZoneTblNo() const { return _taxPointLoc2ZoneTblNo; }

  LocTypeCode& taxPointLoc3Type() { return _taxPointLoc3Type; }
  const LocTypeCode& taxPointLoc3Type() const { return _taxPointLoc3Type; }

  LocCode& taxPointLoc3() { return _taxPointLoc3; }
  const LocCode& taxPointLoc3() const { return _taxPointLoc3; }

  uint32_t& taxPointLoc3ZoneTblNo() { return _taxPointLoc3ZoneTblNo; }
  const uint32_t& taxPointLoc3ZoneTblNo() const { return _taxPointLoc3ZoneTblNo; }

  LocTypeCode& taxPointLoc3GeoType() { return _taxPointLoc3GeoType; }
  const LocTypeCode& taxPointLoc3GeoType() const { return _taxPointLoc3GeoType; }

  StopoverTimeTag& stopoverTimeTag() { return _stopoverTimeTag; }
  const StopoverTimeTag& stopoverTimeTag() const { return _stopoverTimeTag; }

  Indicator& stopoverTimeUnit() { return _stopoverTimeUnit; }
  const Indicator& stopoverTimeUnit() const { return _stopoverTimeUnit; }

  Indicator& connectionsTag1() { return _connectionsTag1; }
  const Indicator& connectionsTag1() const { return _connectionsTag1; }

  Indicator& connectionsTag2() { return _connectionsTag2; }
  const Indicator& connectionsTag2() const { return _connectionsTag2; }

  Indicator& connectionsTag3() { return _connectionsTag3; }
  const Indicator& connectionsTag3() const { return _connectionsTag3; }

  Indicator& connectionsTag4() { return _connectionsTag4; }
  const Indicator& connectionsTag4() const { return _connectionsTag4; }

  Indicator& connectionsTag5() { return _connectionsTag5; }
  const Indicator& connectionsTag5() const { return _connectionsTag5; }

  Indicator& connectionsTag6() { return _connectionsTag6; }
  const Indicator& connectionsTag6() const { return _connectionsTag6; }

  Indicator& connectionsTag7() { return _connectionsTag7; }
  const Indicator& connectionsTag7() const { return _connectionsTag7; }

  Indicator& serviceBaggageApplTag() { return _serviceBaggageApplTag; }
  const Indicator& serviceBaggageApplTag() const { return _serviceBaggageApplTag; }

  uint32_t& serviceBaggageItemNo() { return _serviceBaggageItemNo; }
  const uint32_t& serviceBaggageItemNo() const { return _serviceBaggageItemNo; }

  uint32_t& carrierFltItemNo1() { return _carrierFltItemNo1; }
  const uint32_t& carrierFltItemNo1() const { return _carrierFltItemNo1; }

  Indicator& sectorDetailApplTag() { return _sectorDetailApplTag; }
  const Indicator& sectorDetailApplTag() const { return _sectorDetailApplTag; }

  uint32_t& sectorDetailItemNo() { return _sectorDetailItemNo; }
  const uint32_t& sectorDetailItemNo() const { return _sectorDetailItemNo; }

  Indicator& tktValApplQualifier() { return _tktValApplQualifier; }
  const Indicator& tktValApplQualifier() const { return _tktValApplQualifier; }

  uint32_t& tktValMin() { return _tktValMin; }
  const uint32_t& tktValMin() const { return _tktValMin; }

  uint32_t& tktValMax() { return _tktValMax; }
  const uint32_t& tktValMax() const { return _tktValMax; }

  CurrencyCode& tktValCurrency() { return _tktValCurrency; }
  const CurrencyCode& tktValCurrency() const { return _tktValCurrency; }

  uint32_t& tktValCurDecimals() { return _tktValCurDecimals; }
  const uint32_t& tktValCurDecimals() const { return _tktValCurDecimals; }

  CurrencyCode& currencyOfSale() { return _currencyOfSale; }
  const CurrencyCode& currencyOfSale() const { return _currencyOfSale; }

  Indicator& paidBy3rdPartyTag() { return _paidBy3rdPartyTag; }
  const Indicator& paidBy3rdPartyTag() const { return _paidBy3rdPartyTag; }

  DateTime& ratdDate() { return _ratdDate; }
  const DateTime& ratdDate() const { return _ratdDate; }

  DateTime& historicSaleEffDate() { return _historicSaleEffDate; }
  const DateTime& historicSaleEffDate() const { return _historicSaleEffDate; }

  DateTime& historicSaleDiscDate() { return _historicSaleDiscDate; }
  const DateTime& historicSaleDiscDate() const { return _historicSaleDiscDate; }

  DateTime& historicTrvlEffDate() { return _historicTrvlEffDate; }
  const DateTime& historicTrvlEffDate() const { return _historicTrvlEffDate; }

  DateTime& historicTrvlDiscDate() { return _historicTrvlDiscDate; }
  const DateTime& historicTrvlDiscDate() const { return _historicTrvlDiscDate; }

  uint32_t& taxAmt() { return _taxAmt; }
  const uint32_t& taxAmt() const { return _taxAmt; }

  CurrencyCode& taxCurrency() { return _taxCurrency; }
  const CurrencyCode& taxCurrency() const { return _taxCurrency; }

  uint32_t& taxCurDecimals() { return _taxCurDecimals; }
  const uint32_t& taxCurDecimals() const { return _taxCurDecimals; }

  uint32_t& taxPercent() { return _taxPercent; }
  const uint32_t& taxPercent() const { return _taxPercent; }

  uint32_t& minTax() { return _minTax; }
  const uint32_t& minTax() const { return _minTax; }

  uint32_t& maxTax() { return _maxTax; }
  const uint32_t& maxTax() const { return _maxTax; }

  CurrencyCode& minMaxCurrency() { return _minMaxCurrency; }
  const CurrencyCode& minMaxCurrency() const { return _minMaxCurrency; }

  uint32_t& minMaxDecimals() { return _minMaxDecimals; }
  const uint32_t& minMaxDecimals() const { return _minMaxDecimals; }

  Indicator& vatInclusiveInd() { return _vatInclusiveInd; }
  const Indicator& vatInclusiveInd() const { return _vatInclusiveInd; }

  Indicator& taxAppliesToTagInd() { return _taxAppliesToTagInd; }
  const Indicator& taxAppliesToTagInd() const { return _taxAppliesToTagInd; }

  Indicator& taxAppLimit() { return _taxAppLimit; }
  const Indicator& taxAppLimit() const { return _taxAppLimit; }

  Indicator& netRemitApplTag() { return _netRemitApplTag; }
  const Indicator& netRemitApplTag() const { return _netRemitApplTag; }

  Indicator& taxRoundUnit() { return _taxRoundUnit; }
  const Indicator& taxRoundUnit() const { return _taxRoundUnit; }

  Indicator& taxRoundDir() { return _taxRoundDir; }
  const Indicator& taxRoundDir() const { return _taxRoundDir; }

  uint32_t& alternateRuleRefTag() { return _alternateRuleRefTag; }
  const uint32_t& alternateRuleRefTag() const { return _alternateRuleRefTag; }

  uint32_t& taxTextTblItemNo() { return _taxTextTblItemNo; }
  const uint32_t& taxTextTblItemNo() const { return _taxTextTblItemNo; }

  TaxProcessingApplTag& taxProcessingApplTag() { return _taxProcessingApplTag; }
  const TaxProcessingApplTag& taxProcessingApplTag() const { return _taxProcessingApplTag; }

  TaxMatchingApplTag& taxMatchingApplTag() { return _taxMatchingApplTag; }
  const TaxMatchingApplTag& taxMatchingApplTag() const { return _taxMatchingApplTag; }

  Indicator& outputTypeInd() { return _outputTypeInd; }
  const Indicator& outputTypeInd() const { return _outputTypeInd; }

  Indicator& exemptTag() { return _exemptTag; }
  const Indicator& exemptTag() const { return _exemptTag; }

private:
  TaxRulesRecord(const TaxRulesRecord&);
  TaxRulesRecord& operator=(const TaxRulesRecord&);

  VendorCode _vendor;
  NationCode _nation;
  TaxCode _taxCode;
  Indicator _taxRemittanceId;
  TaxType _taxType;
  Indicator _taxPointTag;
  Indicator _percentFlatTag;
  int32_t _seqNo;
  DateTime _createDate;
  uint64_t _versionDate;
  DateTime _expireDate;
  DateTime _lockDate;
  Indicator _taxableUnitTag1;
  Indicator _taxableUnitTag2;
  Indicator _taxableUnitTag3;
  Indicator _taxableUnitTag4;
  Indicator _taxableUnitTag5;
  Indicator _taxableUnitTag6;
  Indicator _taxableUnitTag7;
  Indicator _taxableUnitTag8;
  Indicator _taxableUnitTag9;
  Indicator _taxableUnitTag10;
  uint16_t _calcOrder;
  DateTime _effDate;
  DateTime _discDate;
  Indicator _travelDateAppTag;
  uint16_t _tvlFirstYear;
  uint16_t _tvlFirstMonth;
  uint16_t _tvlFirstDay;
  uint16_t _tvlLastYear;
  uint16_t _tvlLastMonth;
  uint16_t _tvlLastDay;
  CarrierCode _taxCarrier;

  uint32_t _carrierApplItemNo1;
  uint32_t _carrierFltItemNo2;
  Indicator _rtnToOrig;
  uint32_t _psgrTypeCodeItemNo;
  Indicator _ticketedPointTag;
  LocTypeCode _posLocType;
  LocCode _posLoc;
  uint32_t _posLocZoneTblNo;
  uint32_t _svcFeesSecurityItemNo;
  LocTypeCode _poTktLocType;
  LocCode _poTktLoc;
  uint32_t _poTktLocZoneTblNo;
  LocTypeCode _poDeliveryLocType;
  LocCode _poDeliveryLoc;
  uint32_t _poDeliveryZoneTblNo;

  Indicator _jrnyInd;
  LocTypeCode _jrnyLoc1Type;
  LocCode _jrnyLoc1;
  uint32_t _jrnyLoc1ZoneTblNo;
  LocTypeCode _jrnyLoc2Type;
  LocCode _jrnyLoc2;
  uint32_t _jrnyLoc2ZoneTblNo;
  LocTypeCode _jrnyIncludesLocType;
  LocCode _jrnyIncludesLoc;
  uint32_t _jrnyIncludesLocZoneTblNo;

  LocCode _trvlWhollyWithinLoc;
  LocTypeCode _trvlWhollyWithinLocType;
  uint32_t _trvlWhollyWithinLocZoneTblNo;

  Indicator _taxPointLoc1IntDomInd;
  Indicator _taxPointLoc1TrnsfrType;
  Indicator _taxPointLoc1StopoverTag;
  LocTypeCode _taxPointLoc1Type;
  LocCode _taxPointLoc1;
  uint32_t _taxPointLoc1ZoneTblNo;

  Indicator _taxPointLoc2IntlDomInd;
  Indicator _taxPointLoc2Compare;
  Indicator _taxPointLoc2StopoverTag;
  LocTypeCode _taxPointLoc2Type;
  LocCode _taxPointLoc2;
  uint32_t _taxPointLoc2ZoneTblNo;

  Indicator _taxPointLoc3Type;
  LocCode _taxPointLoc3;
  uint32_t _taxPointLoc3ZoneTblNo;
  LocTypeCode _taxPointLoc3GeoType;

  StopoverTimeTag _stopoverTimeTag;
  Indicator _stopoverTimeUnit;

  Indicator _connectionsTag1;
  Indicator _connectionsTag2;
  Indicator _connectionsTag3;
  Indicator _connectionsTag4;
  Indicator _connectionsTag5;
  Indicator _connectionsTag6;
  Indicator _connectionsTag7;

  Indicator _serviceBaggageApplTag;
  uint32_t _serviceBaggageItemNo;
  uint32_t _carrierFltItemNo1;
  Indicator _sectorDetailApplTag;
  uint32_t _sectorDetailItemNo;

  Indicator _tktValApplQualifier;
  uint32_t _tktValMin;
  uint32_t _tktValMax;
  CurrencyCode _tktValCurrency;
  uint32_t _tktValCurDecimals;

  CurrencyCode _currencyOfSale;
  Indicator _paidBy3rdPartyTag;
  DateTime _ratdDate;

  DateTime _historicSaleEffDate;
  DateTime _historicSaleDiscDate;
  DateTime _historicTrvlEffDate;
  DateTime _historicTrvlDiscDate;

  uint32_t _taxAmt;
  CurrencyCode _taxCurrency;
  uint32_t _taxCurDecimals;
  uint32_t _taxPercent;

  uint32_t _minTax;
  uint32_t _maxTax;
  CurrencyCode _minMaxCurrency;
  uint32_t _minMaxDecimals;

  Indicator _vatInclusiveInd;
  Indicator _taxAppliesToTagInd;
  Indicator _taxAppLimit;
  Indicator _netRemitApplTag;
  Indicator _taxRoundUnit;
  Indicator _taxRoundDir;
  uint32_t _taxTextTblItemNo;
  Indicator _outputTypeInd;

  uint32_t _alternateRuleRefTag;
  TaxProcessingApplTag _taxProcessingApplTag;
  TaxMatchingApplTag _taxMatchingApplTag;

  Indicator _exemptTag;

};
}
