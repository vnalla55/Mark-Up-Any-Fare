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

#include "DBAccess/TaxRulesRecord.h"

#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

TaxRulesRecord::TaxRulesRecord()
  : _vendor(""),
    _nation(""),
    _taxCode(""),
    _taxRemittanceId('\0'),
    _taxType(""),
    _taxPointTag('\0'),
    _percentFlatTag('\0'),
    _seqNo(0),
    _createDate(2001, 1, 1),
    _versionDate(0),
    _expireDate(2001, 1, 1),
    _lockDate(2001, 1, 1),
    _taxableUnitTag1('\0'),
    _taxableUnitTag2('\0'),
    _taxableUnitTag3('\0'),
    _taxableUnitTag4('\0'),
    _taxableUnitTag5('\0'),
    _taxableUnitTag6('\0'),
    _taxableUnitTag7('\0'),
    _taxableUnitTag8('\0'),
    _taxableUnitTag9('\0'),
    _taxableUnitTag10('\0'),
    _calcOrder(0),
    _effDate(2001, 1, 1),
    _discDate(2001, 1, 1),
    _travelDateAppTag('\0'),
    _tvlFirstYear(0),
    _tvlFirstMonth(0),
    _tvlFirstDay(0),
    _tvlLastYear(0),
    _tvlLastMonth(0),
    _tvlLastDay(0),
    _taxCarrier(""),
    _carrierApplItemNo1(0),
    _carrierFltItemNo2(0),
    _rtnToOrig('\0'),
    _psgrTypeCodeItemNo(0),
    _ticketedPointTag('\0'),
    _posLocType('\0'),
    _posLoc(""),
    _posLocZoneTblNo(0),
    _svcFeesSecurityItemNo(0),
    _poTktLocType('\0'),
    _poTktLoc(""),
    _poTktLocZoneTblNo(0),
    _poDeliveryLocType('\0'),
    _poDeliveryLoc(""),
    _poDeliveryZoneTblNo(0),
    _jrnyInd('\0'),
    _jrnyLoc1Type('\0'),
    _jrnyLoc1(""),
    _jrnyLoc1ZoneTblNo(0),
    _jrnyLoc2Type('\0'),
    _jrnyLoc2(""),
    _jrnyLoc2ZoneTblNo(0),
    _jrnyIncludesLocType('\0'),
    _jrnyIncludesLoc(""),
    _jrnyIncludesLocZoneTblNo(0),
    _trvlWhollyWithinLoc(""),
    _trvlWhollyWithinLocType('\0'),
    _trvlWhollyWithinLocZoneTblNo(0),
    _taxPointLoc1IntDomInd('\0'),
    _taxPointLoc1TrnsfrType('\0'),
    _taxPointLoc1StopoverTag('\0'),
    _taxPointLoc1Type('\0'),
    _taxPointLoc1(""),
    _taxPointLoc1ZoneTblNo(0),
    _taxPointLoc2IntlDomInd('\0'),
    _taxPointLoc2Compare('\0'),
    _taxPointLoc2StopoverTag('\0'),
    _taxPointLoc2Type('\0'),
    _taxPointLoc2(""),
    _taxPointLoc2ZoneTblNo(0),
    _taxPointLoc3Type('\0'),
    _taxPointLoc3(""),
    _taxPointLoc3ZoneTblNo(0),
    _taxPointLoc3GeoType('\0'),
    _stopoverTimeTag('\0'),
    _stopoverTimeUnit('\0'),
    _connectionsTag1('\0'),
    _connectionsTag2('\0'),
    _connectionsTag3('\0'),
    _connectionsTag4('\0'),
    _connectionsTag5('\0'),
    _connectionsTag6('\0'),
    _connectionsTag7('\0'),
    _serviceBaggageApplTag('\0'),
    _serviceBaggageItemNo(0),
    _carrierFltItemNo1(0),
    _sectorDetailApplTag('\0'),
    _sectorDetailItemNo(0),
    _tktValApplQualifier('\0'),
    _tktValMin(0),
    _tktValMax(0),
    _tktValCurrency(""),
    _tktValCurDecimals(0),
    _currencyOfSale(""),
    _paidBy3rdPartyTag('\0'),
    _ratdDate(0),
    _historicSaleEffDate(0),
    _historicSaleDiscDate(0),
    _historicTrvlEffDate(0),
    _historicTrvlDiscDate(0),
    _taxAmt(0),
    _taxCurrency(""),
    _taxCurDecimals(0),
    _taxPercent(0),
    _minTax(0),
    _maxTax(0),
    _minMaxCurrency(""),
    _minMaxDecimals(0),
    _vatInclusiveInd('\0'),
    _taxAppliesToTagInd('\0'),
    _taxAppLimit(0),
    _netRemitApplTag('\0'),
    _taxRoundUnit(0),
    _taxRoundDir(0),
    _taxTextTblItemNo(0),
    _outputTypeInd('\0'),
    _alternateRuleRefTag(0),
    _taxProcessingApplTag(""),
    _taxMatchingApplTag(""),
    _exemptTag('\0')
{
}

bool
TaxRulesRecord::
operator==(const TaxRulesRecord& rhs) const
{
  bool eq(
      (_vendor == rhs._vendor) && (_nation == rhs._nation) && (_taxCode == rhs._taxCode) &&
      (_taxRemittanceId == rhs._taxRemittanceId) && (_taxType == rhs._taxType) &&
      (_taxPointTag == rhs._taxPointTag) && (_percentFlatTag == rhs._percentFlatTag) &&
      (_seqNo == rhs._seqNo) && (_createDate == rhs._createDate) &&
      (_versionDate == rhs._versionDate) && (_expireDate == rhs._expireDate) &&
      (_lockDate == rhs._lockDate) && (_taxableUnitTag1 == rhs._taxableUnitTag1) &&
      (_taxableUnitTag2 == rhs._taxableUnitTag2) && (_taxableUnitTag3 == rhs._taxableUnitTag3) &&
      (_taxableUnitTag4 == rhs._taxableUnitTag4) && (_taxableUnitTag5 == rhs._taxableUnitTag5) &&
      (_taxableUnitTag6 == rhs._taxableUnitTag6) && (_taxableUnitTag7 == rhs._taxableUnitTag7) &&
      (_taxableUnitTag8 == rhs._taxableUnitTag8) && (_taxableUnitTag9 == rhs._taxableUnitTag9) &&
      (_taxableUnitTag10 == rhs._taxableUnitTag10) && (_calcOrder == rhs._calcOrder) &&
      (_effDate == rhs._effDate) && (_discDate == rhs._discDate) &&
      (_travelDateAppTag == rhs._travelDateAppTag) && (_tvlFirstYear == rhs._tvlFirstYear) &&
      (_tvlFirstMonth == rhs._tvlFirstMonth) && (_tvlFirstDay == rhs._tvlFirstDay) &&
      (_tvlLastYear == rhs._tvlLastYear) && (_tvlLastMonth == rhs._tvlLastMonth) &&
      (_tvlLastDay == rhs._tvlLastDay) && (_taxCarrier == rhs._taxCarrier) &&
      (_carrierApplItemNo1 == rhs._carrierApplItemNo1) &&
      (_carrierFltItemNo2 == rhs._carrierFltItemNo2) && (_rtnToOrig == rhs._rtnToOrig) &&
      (_psgrTypeCodeItemNo == rhs._psgrTypeCodeItemNo) &&
      (_ticketedPointTag == rhs._ticketedPointTag) && (_posLocType == rhs._posLocType) &&
      (_posLoc == rhs._posLoc) && (_posLocZoneTblNo == rhs._posLocZoneTblNo) &&
      (_svcFeesSecurityItemNo == rhs._svcFeesSecurityItemNo) &&
      (_poTktLocType == rhs._poTktLocType) && (_poTktLoc == rhs._poTktLoc) &&
      (_poTktLocZoneTblNo == rhs._poTktLocZoneTblNo) &&
      (_poDeliveryLocType == rhs._poDeliveryLocType) && (_poDeliveryLoc == rhs._poDeliveryLoc) &&
      (_poDeliveryZoneTblNo == rhs._poDeliveryZoneTblNo) && (_jrnyInd == rhs._jrnyInd) &&
      (_jrnyLoc1Type == rhs._jrnyLoc1Type) && (_jrnyLoc1 == rhs._jrnyLoc1) &&
      (_jrnyLoc1ZoneTblNo == rhs._jrnyLoc1ZoneTblNo) && (_jrnyLoc2Type == rhs._jrnyLoc2Type) &&
      (_jrnyLoc2 == rhs._jrnyLoc2) && (_jrnyLoc2ZoneTblNo == rhs._jrnyLoc2ZoneTblNo) &&
      (_jrnyIncludesLocType == rhs._jrnyIncludesLocType) &&
      (_jrnyIncludesLoc == rhs._jrnyIncludesLoc) &&
      (_jrnyIncludesLocZoneTblNo == rhs._jrnyIncludesLocZoneTblNo) &&
      (_trvlWhollyWithinLoc == rhs._trvlWhollyWithinLoc) &&
      (_trvlWhollyWithinLocType == rhs._trvlWhollyWithinLocType) &&
      (_trvlWhollyWithinLocZoneTblNo == rhs._trvlWhollyWithinLocZoneTblNo) &&
      (_taxPointLoc1IntDomInd == rhs._taxPointLoc1IntDomInd) &&
      (_taxPointLoc1TrnsfrType == rhs._taxPointLoc1TrnsfrType) &&
      (_taxPointLoc1StopoverTag == rhs._taxPointLoc1StopoverTag) &&
      (_taxPointLoc1Type == rhs._taxPointLoc1Type) && (_taxPointLoc1 == rhs._taxPointLoc1) &&
      (_taxPointLoc1ZoneTblNo == rhs._taxPointLoc1ZoneTblNo) &&
      (_taxPointLoc2IntlDomInd == rhs._taxPointLoc2IntlDomInd) &&
      (_taxPointLoc2Compare == rhs._taxPointLoc2Compare) &&
      (_taxPointLoc2StopoverTag == rhs._taxPointLoc2StopoverTag) &&
      (_taxPointLoc2Type == rhs._taxPointLoc2Type) && (_taxPointLoc2 == rhs._taxPointLoc2) &&
      (_taxPointLoc2ZoneTblNo == rhs._taxPointLoc2ZoneTblNo) &&
      (_taxPointLoc3Type == rhs._taxPointLoc3Type) && (_taxPointLoc3 == rhs._taxPointLoc3) &&
      (_taxPointLoc3ZoneTblNo == rhs._taxPointLoc3ZoneTblNo) &&
      (_taxPointLoc3GeoType == rhs._taxPointLoc3GeoType) &&
      (_stopoverTimeTag == rhs._stopoverTimeTag) && (_stopoverTimeUnit == rhs._stopoverTimeUnit) &&
      (_connectionsTag1 == rhs._connectionsTag1) && (_connectionsTag2 == rhs._connectionsTag2) &&
      (_connectionsTag3 == rhs._connectionsTag3) && (_connectionsTag4 == rhs._connectionsTag4) &&
      (_connectionsTag5 == rhs._connectionsTag5) && (_connectionsTag6 == rhs._connectionsTag6) &&
      (_connectionsTag7 == rhs._connectionsTag7) &&
      (_serviceBaggageApplTag == rhs._serviceBaggageApplTag) &&
      (_serviceBaggageItemNo == rhs._serviceBaggageItemNo) &&
      (_carrierFltItemNo1 == rhs._carrierFltItemNo1) &&
      (_sectorDetailApplTag == rhs._sectorDetailApplTag) &&
      (_sectorDetailItemNo == rhs._sectorDetailItemNo) &&
      (_tktValApplQualifier == rhs._tktValApplQualifier) && (_tktValMin == rhs._tktValMin) &&
      (_tktValMax == rhs._tktValMax) && (_tktValCurrency == rhs._tktValCurrency) &&
      (_tktValCurDecimals == rhs._tktValCurDecimals) && (_currencyOfSale == rhs._currencyOfSale) &&
      (_paidBy3rdPartyTag == rhs._paidBy3rdPartyTag) && (_ratdDate == rhs._ratdDate) &&
      (_historicSaleEffDate == rhs._historicSaleEffDate) &&
      (_historicSaleDiscDate == rhs._historicSaleDiscDate) &&
      (_historicTrvlEffDate == rhs._historicTrvlEffDate) &&
      (_historicTrvlDiscDate == rhs._historicTrvlDiscDate) && (_taxAmt == rhs._taxAmt) &&
      (_taxCurrency == rhs._taxCurrency) && (_taxCurDecimals == rhs._taxCurDecimals) &&
      (_taxPercent == rhs._taxPercent) && (_minTax == rhs._minTax) && (_maxTax == rhs._maxTax) &&
      (_minMaxCurrency == rhs._minMaxCurrency) && (_minMaxDecimals == rhs._minMaxDecimals) &&
      (_vatInclusiveInd == rhs._taxAppliesToTagInd) &&
      (_taxAppliesToTagInd == rhs._taxAppliesToTagInd) && (_taxAppLimit == rhs._taxAppLimit) &&
      (_netRemitApplTag == rhs._netRemitApplTag) && (_taxRoundUnit == rhs._taxRoundUnit) &&
      (_taxRoundDir == rhs._taxRoundDir) && (_alternateRuleRefTag == rhs._alternateRuleRefTag) &&
      (_taxTextTblItemNo == rhs._taxTextTblItemNo) &&
      (_taxProcessingApplTag == rhs._taxProcessingApplTag) &&
      (_taxMatchingApplTag == rhs._taxMatchingApplTag) && (_outputTypeInd == rhs._outputTypeInd) &&
      (_exemptTag == rhs._exemptTag));

  return eq;
}

void
TaxRulesRecord::flattenize(Flattenizable::Archive& archive)
{
  FLATTENIZE(archive, _vendor);
  FLATTENIZE(archive, _nation);
  FLATTENIZE(archive, _taxCode);
  FLATTENIZE(archive, _taxRemittanceId);
  FLATTENIZE(archive, _taxType);
  FLATTENIZE(archive, _taxPointTag);
  FLATTENIZE(archive, _percentFlatTag);
  FLATTENIZE(archive, _seqNo);
  FLATTENIZE(archive, _createDate);
  FLATTENIZE(archive, _versionDate);
  FLATTENIZE(archive, _expireDate);
  FLATTENIZE(archive, _lockDate);
  FLATTENIZE(archive, _taxableUnitTag1);
  FLATTENIZE(archive, _taxableUnitTag2);
  FLATTENIZE(archive, _taxableUnitTag3);
  FLATTENIZE(archive, _taxableUnitTag4);
  FLATTENIZE(archive, _taxableUnitTag5);
  FLATTENIZE(archive, _taxableUnitTag6);
  FLATTENIZE(archive, _taxableUnitTag7);
  FLATTENIZE(archive, _taxableUnitTag8);
  FLATTENIZE(archive, _taxableUnitTag9);
  FLATTENIZE(archive, _taxableUnitTag10);
  FLATTENIZE(archive, _calcOrder);
  FLATTENIZE(archive, _effDate);
  FLATTENIZE(archive, _discDate);
  FLATTENIZE(archive, _travelDateAppTag);
  FLATTENIZE(archive, _tvlFirstYear);
  FLATTENIZE(archive, _tvlFirstMonth);
  FLATTENIZE(archive, _tvlFirstDay);
  FLATTENIZE(archive, _tvlLastYear);
  FLATTENIZE(archive, _tvlLastMonth);
  FLATTENIZE(archive, _tvlLastDay);
  FLATTENIZE(archive, _taxCarrier);
  FLATTENIZE(archive, _carrierApplItemNo1);
  FLATTENIZE(archive, _carrierFltItemNo2);
  FLATTENIZE(archive, _rtnToOrig);
  FLATTENIZE(archive, _psgrTypeCodeItemNo);
  FLATTENIZE(archive, _ticketedPointTag);
  FLATTENIZE(archive, _posLocType);
  FLATTENIZE(archive, _posLoc);
  FLATTENIZE(archive, _posLocZoneTblNo);
  FLATTENIZE(archive, _svcFeesSecurityItemNo);
  FLATTENIZE(archive, _poTktLocType);
  FLATTENIZE(archive, _poTktLoc);
  FLATTENIZE(archive, _poTktLocZoneTblNo);
  FLATTENIZE(archive, _poDeliveryLocType);
  FLATTENIZE(archive, _poDeliveryLoc);
  FLATTENIZE(archive, _poDeliveryZoneTblNo);
  FLATTENIZE(archive, _jrnyInd);
  FLATTENIZE(archive, _jrnyLoc1Type);
  FLATTENIZE(archive, _jrnyLoc1);
  FLATTENIZE(archive, _jrnyLoc1ZoneTblNo);
  FLATTENIZE(archive, _jrnyLoc2Type);
  FLATTENIZE(archive, _jrnyLoc2);
  FLATTENIZE(archive, _jrnyLoc2ZoneTblNo);
  FLATTENIZE(archive, _jrnyIncludesLocType);
  FLATTENIZE(archive, _jrnyIncludesLoc);
  FLATTENIZE(archive, _jrnyIncludesLocZoneTblNo);
  FLATTENIZE(archive, _trvlWhollyWithinLoc);
  FLATTENIZE(archive, _trvlWhollyWithinLocType);
  FLATTENIZE(archive, _trvlWhollyWithinLocZoneTblNo);
  FLATTENIZE(archive, _taxPointLoc1IntDomInd);
  FLATTENIZE(archive, _taxPointLoc1TrnsfrType);
  FLATTENIZE(archive, _taxPointLoc1StopoverTag);
  FLATTENIZE(archive, _taxPointLoc1Type);
  FLATTENIZE(archive, _taxPointLoc1);
  FLATTENIZE(archive, _taxPointLoc1ZoneTblNo);
  FLATTENIZE(archive, _taxPointLoc2IntlDomInd);
  FLATTENIZE(archive, _taxPointLoc2Compare);
  FLATTENIZE(archive, _taxPointLoc2StopoverTag);
  FLATTENIZE(archive, _taxPointLoc2Type);
  FLATTENIZE(archive, _taxPointLoc2);
  FLATTENIZE(archive, _taxPointLoc2ZoneTblNo);
  FLATTENIZE(archive, _taxPointLoc3Type);
  FLATTENIZE(archive, _taxPointLoc3);
  FLATTENIZE(archive, _taxPointLoc3ZoneTblNo);
  FLATTENIZE(archive, _taxPointLoc3GeoType);
  FLATTENIZE(archive, _stopoverTimeTag);
  FLATTENIZE(archive, _stopoverTimeUnit);
  FLATTENIZE(archive, _connectionsTag1);
  FLATTENIZE(archive, _connectionsTag2);
  FLATTENIZE(archive, _connectionsTag3);
  FLATTENIZE(archive, _connectionsTag4);
  FLATTENIZE(archive, _connectionsTag5);
  FLATTENIZE(archive, _connectionsTag6);
  FLATTENIZE(archive, _connectionsTag7);
  FLATTENIZE(archive, _serviceBaggageApplTag);
  FLATTENIZE(archive, _serviceBaggageItemNo);
  FLATTENIZE(archive, _carrierFltItemNo1);
  FLATTENIZE(archive, _sectorDetailApplTag);
  FLATTENIZE(archive, _sectorDetailItemNo);
  FLATTENIZE(archive, _tktValApplQualifier);
  FLATTENIZE(archive, _tktValMin);
  FLATTENIZE(archive, _tktValMax);
  FLATTENIZE(archive, _tktValCurrency);
  FLATTENIZE(archive, _tktValCurDecimals);
  FLATTENIZE(archive, _currencyOfSale);
  FLATTENIZE(archive, _paidBy3rdPartyTag);
  FLATTENIZE(archive, _ratdDate);
  FLATTENIZE(archive, _historicSaleEffDate);
  FLATTENIZE(archive, _historicSaleDiscDate);
  FLATTENIZE(archive, _historicTrvlEffDate);
  FLATTENIZE(archive, _historicTrvlDiscDate);
  FLATTENIZE(archive, _taxAmt);
  FLATTENIZE(archive, _taxCurrency);
  FLATTENIZE(archive, _taxCurDecimals);
  FLATTENIZE(archive, _taxPercent);
  FLATTENIZE(archive, _minTax);
  FLATTENIZE(archive, _maxTax);
  FLATTENIZE(archive, _minMaxCurrency);
  FLATTENIZE(archive, _minMaxDecimals);
  FLATTENIZE(archive, _vatInclusiveInd);
  FLATTENIZE(archive, _taxAppliesToTagInd);
  FLATTENIZE(archive, _taxAppLimit);
  FLATTENIZE(archive, _netRemitApplTag);
  FLATTENIZE(archive, _taxRoundUnit);
  FLATTENIZE(archive, _taxRoundDir);
  FLATTENIZE(archive, _alternateRuleRefTag);
  FLATTENIZE(archive, _taxTextTblItemNo);
  FLATTENIZE(archive, _taxProcessingApplTag);
  FLATTENIZE(archive, _taxMatchingApplTag);
  FLATTENIZE(archive, _outputTypeInd);
  FLATTENIZE(archive, _exemptTag);
}

void
TaxRulesRecord::dummyData(TaxRulesRecord& obj)
{
  obj._nation = "PL";
  obj._taxCode = "XX";
  obj._taxType = "001";
  obj._taxRemittanceId = 'S';
  obj._taxPointTag = 'D';
  obj._percentFlatTag = 'F';
  obj._seqNo = 100;
  obj._createDate = time(nullptr);
  obj._versionDate = time(nullptr);
  obj._expireDate = time(nullptr);
  obj._lockDate = time(nullptr);
  obj._taxableUnitTag1 = '\0';
  obj._taxableUnitTag2 = '\0';
  obj._taxableUnitTag3 = '\0';
  obj._taxableUnitTag4 = '\0';
  obj._taxableUnitTag5 = '\0';
  obj._taxableUnitTag6 = '\0';
  obj._taxableUnitTag7 = '\0';
  obj._taxableUnitTag8 = '\0';
  obj._taxableUnitTag9 = '\0';
  obj._taxableUnitTag10 = '\0';
  obj._calcOrder = 0;
  obj._effDate = time(nullptr);
  obj._discDate = time(nullptr);
  obj._travelDateAppTag = 'J';
  obj._tvlFirstYear = 0;
  obj._tvlFirstMonth = 0;
  obj._tvlFirstDay = 0;
  obj._tvlLastYear = 0;
  obj._tvlLastMonth = 0;
  obj._tvlLastDay = 0;
  obj._taxCarrier = "LO";
  obj._svcFeesSecurityItemNo = 7683;
}

WBuffer&
TaxRulesRecord::write(WBuffer& os) const
{
  return convert(os, this);
}

RBuffer&
TaxRulesRecord::read(RBuffer& is)
{
  return convert(is, this);
}

template <typename B, typename T>
B&
TaxRulesRecord::convert(B& buffer, T ptr)
{
  return buffer & ptr->_vendor & ptr->_nation & ptr->_taxCode & ptr->_taxRemittanceId &
         ptr->_taxType & ptr->_taxPointTag & ptr->_percentFlatTag & ptr->_seqNo & ptr->_createDate &
         ptr->_versionDate & ptr->_expireDate & ptr->_lockDate & ptr->_taxableUnitTag1 &
         ptr->_taxableUnitTag2 & ptr->_taxableUnitTag3 & ptr->_taxableUnitTag4 &
         ptr->_taxableUnitTag5 & ptr->_taxableUnitTag6 & ptr->_taxableUnitTag7 &
         ptr->_taxableUnitTag8 & ptr->_taxableUnitTag9 & ptr->_taxableUnitTag10 & ptr->_calcOrder &
         ptr->_effDate & ptr->_discDate & ptr->_travelDateAppTag & ptr->_tvlFirstYear &
         ptr->_tvlFirstMonth & ptr->_tvlFirstDay & ptr->_tvlLastYear & ptr->_tvlLastMonth &
         ptr->_tvlLastDay & ptr->_taxCarrier & ptr->_carrierApplItemNo1 & ptr->_carrierFltItemNo2 &
         ptr->_rtnToOrig & ptr->_psgrTypeCodeItemNo & ptr->_ticketedPointTag & ptr->_posLocType &
         ptr->_posLoc & ptr->_posLocZoneTblNo & ptr->_svcFeesSecurityItemNo & ptr->_poTktLocType &
         ptr->_poTktLoc & ptr->_poTktLocZoneTblNo & ptr->_poDeliveryLocType & ptr->_poDeliveryLoc &
         ptr->_poDeliveryZoneTblNo & ptr->_jrnyInd & ptr->_jrnyLoc1Type & ptr->_jrnyLoc1 &
         ptr->_jrnyLoc1ZoneTblNo & ptr->_jrnyLoc2Type & ptr->_jrnyLoc2 & ptr->_jrnyLoc2ZoneTblNo &
         ptr->_jrnyIncludesLocType & ptr->_jrnyIncludesLoc & ptr->_jrnyIncludesLocZoneTblNo &
         ptr->_trvlWhollyWithinLoc & ptr->_trvlWhollyWithinLocType &
         ptr->_trvlWhollyWithinLocZoneTblNo & ptr->_taxPointLoc1IntDomInd &
         ptr->_taxPointLoc1TrnsfrType & ptr->_taxPointLoc1StopoverTag & ptr->_taxPointLoc1Type &
         ptr->_taxPointLoc1 & ptr->_taxPointLoc1ZoneTblNo & ptr->_taxPointLoc2IntlDomInd &
         ptr->_taxPointLoc2Compare & ptr->_taxPointLoc2StopoverTag & ptr->_taxPointLoc2Type &
         ptr->_taxPointLoc2 & ptr->_taxPointLoc2ZoneTblNo & ptr->_taxPointLoc3Type &
         ptr->_taxPointLoc3 & ptr->_taxPointLoc3ZoneTblNo & ptr->_taxPointLoc3GeoType &
         ptr->_stopoverTimeTag & ptr->_stopoverTimeUnit & ptr->_connectionsTag1 &
         ptr->_connectionsTag2 & ptr->_connectionsTag3 & ptr->_connectionsTag4 &
         ptr->_connectionsTag5 & ptr->_connectionsTag6 & ptr->_connectionsTag7 &
         ptr->_serviceBaggageApplTag & ptr->_serviceBaggageItemNo & ptr->_carrierFltItemNo1 &
         ptr->_sectorDetailApplTag & ptr->_sectorDetailItemNo & ptr->_tktValApplQualifier &
         ptr->_tktValMin & ptr->_tktValMax & ptr->_tktValCurrency & ptr->_tktValCurDecimals &
         ptr->_currencyOfSale & ptr->_paidBy3rdPartyTag & ptr->_ratdDate &
         ptr->_historicSaleEffDate & ptr->_historicSaleDiscDate & ptr->_historicTrvlEffDate &
         ptr->_historicTrvlDiscDate & ptr->_taxAmt & ptr->_taxCurrency & ptr->_taxCurDecimals &
         ptr->_taxPercent & ptr->_minTax & ptr->_maxTax & ptr->_minMaxCurrency &
         ptr->_minMaxDecimals & ptr->_vatInclusiveInd & ptr->_taxAppliesToTagInd &
         ptr->_taxAppLimit & ptr->_netRemitApplTag & ptr->_taxRoundUnit & ptr->_taxRoundDir &
         ptr->_alternateRuleRefTag & ptr->_taxTextTblItemNo & ptr->_taxProcessingApplTag &
         ptr->_taxMatchingApplTag & ptr->_outputTypeInd & ptr->_exemptTag;
}

std::ostream&
TaxRulesRecord::print(std::ostream& out) const
{
  out << "\nVENDOR:           " << _vendor;
  out << "\nNATION:           " << _nation;
  out << "\nTAXCODE:          " << _taxCode;
  out << "\nTAXREMITTANCEID:  " << _taxRemittanceId;
  out << "\nTAXTYPE:          " << _taxType;
  out << "\nTAXPOINTTAG:      " << _taxPointTag;
  out << "\nPERCENTFLATTAG:   " << _percentFlatTag;
  out << "\nTAXCARRIER:       " << _taxCarrier;
  out << "\nTICKETEDPOINTTAG: " << _ticketedPointTag;
  out << "\nCALCORDER:        " << _calcOrder;
  out << "\nSEQNO:            " << _seqNo;
  out << "\nCREATEDATE:       " << _createDate;
  out << "\nVERSIONDATE:      " << _versionDate;
  out << "\nEXPIREDATE:       " << _expireDate;
  out << "\nLOCKDATE:         " << _lockDate;
  out << "\nEFFDATE:          " << _effDate;
  out << "\nDISCDATE:         " << _discDate;
  out << "\nTAXABLEUNITTAG1:  " << _taxableUnitTag1;
  out << "\nTAXABLEUNITTAG2:  " << _taxableUnitTag2;
  out << "\nTAXABLEUNITTAG3:  " << _taxableUnitTag3;
  out << "\nTAXABLEUNITTAG4:  " << _taxableUnitTag4;
  out << "\nTAXABLEUNITTAG5:  " << _taxableUnitTag5;
  out << "\nTAXABLEUNITTAG6:  " << _taxableUnitTag6;
  out << "\nTAXABLEUNITTAG7:  " << _taxableUnitTag7;
  out << "\nTAXABLEUNITTAG8:  " << _taxableUnitTag8;
  out << "\nTAXABLEUNITTAG9:  " << _taxableUnitTag9;
  out << "\nTAXABLEUNITTAG10: " << _taxableUnitTag10;
  out << "\nTRAVELDATEAPPTAG: " << _travelDateAppTag;
  out << "\nTVLFIRSTYEAR:     " << _tvlFirstYear;
  out << "\nTVLLASTYEAR:      " << _tvlLastYear;
  out << "\nTVLFIRSTMONTH:    " << _tvlFirstMonth;
  out << "\nTVLLASTMONTH:     " << _tvlLastMonth;
  out << "\nTVLFIRSTDAY:      " << _tvlFirstDay;
  out << "\nTVLLASTDAY:       " << _tvlLastDay;
  out << "\nPOSLOCTYPE:       " << _posLocType;
  out << "\nPOSLOC:           " << _posLoc;
  out << "\nPOSLOCZONETBLNO:  " << _posLocZoneTblNo;
  out << "\nSVCFEESSECURITYITEMNO: " << _svcFeesSecurityItemNo;
  out << "\nPOTKTLOCTYPE:          " << _poTktLocType;
  out << "\nPOTKTLOC:              " << _poTktLoc;
  out << "\nPOTKTLOCZONETBLNO:     " << _poTktLocZoneTblNo;
  out << "\nPODELIVERYLOCTYPE:     " << _poDeliveryLocType;
  out << "\nPODELIVERYLOC:         " << _poDeliveryLoc;
  out << "\nPODELIVERYZONETBLNO:   " << _poDeliveryZoneTblNo;
  out << "\nRTNTOORIG:             " << _rtnToOrig;
  out << "\nJRNYIND:               " << _jrnyInd;
  out << "\nJRNYLOC1TYPE:          " << _jrnyLoc1Type;
  out << "\nJRNYLOC1:              " << _jrnyLoc1;
  out << "\nJRNYLOC1ZONETBLNO:     " << _jrnyLoc1ZoneTblNo;
  out << "\nJRNYLOC2TYPE:          " << _jrnyLoc2Type;
  out << "\nJRNYLOC2:              " << _jrnyLoc2;
  out << "\nJRNYLOC2ZONETBLNO:     " << _jrnyLoc2ZoneTblNo;
  out << "\nTRVLWHOLLYWITHINLOCTYPE:      " << _trvlWhollyWithinLocType;
  out << "\nTRVLWHOLLYWITHINLOC:          " << _trvlWhollyWithinLoc;
  out << "\nTRVLWHOLLYWITHINLOCZONETBLNO: " << _trvlWhollyWithinLocZoneTblNo;
  out << "\nJRNYINCLUDESLOCTYPE:          " << _jrnyIncludesLocType;
  out << "\nJRNYINCLUDESLOC:              " << _jrnyIncludesLoc;
  out << "\nJRNYINCLUDESLOCZONETBLNO:     " << _jrnyIncludesLocZoneTblNo;
  out << "\nHISTORICSALEEFFDATE:     " << _historicSaleEffDate;
  out << "\nHISTORICSALEDISCDATE:    " << _historicSaleDiscDate;
  out << "\nHISTORICTRVLEFFDATE:     " << _historicTrvlEffDate;
  out << "\nHISTORICTRVLDISCDATE:    " << _historicTrvlDiscDate;
  out << "\nTAXPOINTLOC1INTDOMIND:   " << _taxPointLoc1IntDomInd;
  out << "\nTAXPOINTLOC1TRNSFRTYPE:  " << _taxPointLoc1TrnsfrType;
  out << "\nTAXPOINTLOC1STOPOVERTAG: " << _taxPointLoc1StopoverTag;
  out << "\nTAXPOINTLOC1TYPE:        " << _taxPointLoc1Type;
  out << "\nTAXPOINTLOC1:            " << _taxPointLoc1;
  out << "\nTAXPOINTLOC1ZONETBLNO:   " << _taxPointLoc1ZoneTblNo;
  out << "\nTAXPOINTLOC2INTLDOMIND:  " << _taxPointLoc2IntlDomInd;
  out << "\nTAXPOINTLOC2COMPARE:     " << _taxPointLoc2Compare;
  out << "\nTAXPOINTLOC2STOPOVERTAG: " << _taxPointLoc2StopoverTag;
  out << "\nTAXPOINTLOC1TYPE:        " << _taxPointLoc2Type;
  out << "\nTAXPOINTLOC1:            " << _taxPointLoc2;
  out << "\nTAXPOINTLOC1ZONETBLNO:   " << _taxPointLoc2ZoneTblNo;
  out << "\nTAXPOINTLOC3GEOTYPE:     " << _taxPointLoc3GeoType;
  out << "\nTAXPOINTLOC3TYPE:        " << _taxPointLoc3Type;
  out << "\nTAXPOINTLOC3:            " << _taxPointLoc3;
  out << "\nTAXPOINTLOC3ZONETBLNO:   " << _taxPointLoc3ZoneTblNo;
  out << "\nSTOPOVERTIMETAG:  " << _stopoverTimeTag;
  out << "\nSTOPOVERTIMEUNIT: " << _stopoverTimeUnit;
  out << "\nCONNECTIONSTAG1:  " << _connectionsTag1;
  out << "\nCONNECTIONSTAG2:  " << _connectionsTag2;
  out << "\nCONNECTIONSTAG3:  " << _connectionsTag3;
  out << "\nCONNECTIONSTAG4:  " << _connectionsTag4;
  out << "\nCONNECTIONSTAG5:  " << _connectionsTag5;
  out << "\nCONNECTIONSTAG6:  " << _connectionsTag6;
  out << "\nCONNECTIONSTAG7:  " << _connectionsTag7;
  out << "\nCURRENCYOFSALE:   " << _currencyOfSale;
  out << "\nTAXCURRENCY:      " << _taxCurrency;
  out << "\nTAXCURDECIMALS:   " << _taxCurDecimals;
  out << "\nTAXAMT:           " << _taxAmt;
  out << "\nTAXPERCENT:       " << _taxPercent;
  out << "\nTAXROUNDUNIT:     " << _taxRoundUnit;
  out << "\nTAXROUNDDIR:      " << _taxRoundDir;
  out << "\nMINTAX:           " << _minTax;
  out << "\nMAXTAX:           " << _maxTax;
  out << "\nMINMAXCURRENCY:   " << _minMaxCurrency;
  out << "\nMINMAXDECIMALS:   " << _minMaxDecimals;
  out << "\nTKTVALAPPLQUALIFIER:   " << _tktValApplQualifier;
  out << "\nTKTVALMIN:             " << _tktValMin;
  out << "\nTKTVALMAX:             " << _tktValMax;
  out << "\nTKTVALCURRENCY:        " << _tktValCurrency;
  out << "\nTKTVALCURDECIMALS:     " << _tktValCurDecimals;
  out << "\nCARRIERAPPLITEMNO1:    " << _carrierApplItemNo1;
  out << "\nCARRIERFLTITEMNO1:     " << _carrierFltItemNo1;
  out << "\nCARRIERFLTITEMNO2:     " << _carrierFltItemNo2;
  out << "\nPSGRTYPECODEITEMNO:    " << _psgrTypeCodeItemNo;
  out << "\nSERVICEBAGGAGEAPPLTAG: " << _serviceBaggageApplTag;
  out << "\nSERVICEBAGGAGEITEMNO:  " << _serviceBaggageItemNo;
  out << "\nSECTORDETAILAPPLTAG:   " << _sectorDetailApplTag;
  out << "\nSECTORDETAILITEMNO:    " << _sectorDetailItemNo;
  out << "\nTAXTEXTTBLITEMNO:      " << _taxTextTblItemNo;
  out << "\nTAXAPPLIMIT:           " << _taxAppLimit;
  out << "\nNETREMITAPPLTAG:       " << _netRemitApplTag;
  out << "\nTAXAPPLIESTOTAGIND:    " << _taxAppliesToTagInd;
  out << "\nALTERNATERULEREFTAG:   " << _alternateRuleRefTag;
  out << "\nTAXPROCESSINGAPPLTAG:  " << _taxProcessingApplTag;
  out << "\nTAXMATCHINGAPPLTAG:    " << _taxProcessingApplTag;
  out << "\nOUTPUTTYPEIND:         " << _outputTypeInd;
  out << "\nPAIDBY3RDPARTYTAG:     " << _paidBy3rdPartyTag;
  out << "\nRATDDATE:              " << _ratdDate;
  out << "\nVATINCLUSIVEIND:       " << _vatInclusiveInd;
  out << "\nEXEMPTTAG:             " << _exemptTag;
  out << "\n";
  return out;
}

} // tse
