//----------------------------------------------------------------------------
//       � 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//       and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//       or transfer of this software/documentation, in any medium, or incorporation of this
//       software/documentation into any system or publication, is strictly prohibited
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/Flattenizable.h"

namespace tse
{

class Customer
{
public:
  Customer()
    : _branchAccInd(' '),
      _curConvInd(' '),
      _cadSubscriberInd(' '),
      _webSubscriberInd(' '),
      _btsSubscriberInd(' '),
      _sellingFareInd(' '),
      _tvlyInternetSubriber(' '),
      _tvlyLocation(' '),
      _tvlyOnlineLocation(' '),
      _availabilityIgRul2St(' '),
      _availabilityIgRul3St(' '),
      _channelId(' '),
      _availIgRul2StWpnc(' '),
      _activateJourneyPricing(' '),
      _activateJourneyShopping(' '),
      _noRollupPfaBulkTkt(' '),
      _doNotApplySegmentFee(' '),
      _ssgGroupNo(0),
      _optInAgency(' '),
      _privateFareInd(' '),
      _doNotApplyObTktFees(' '),
      _eTicketCapable(' '),
      _fareQuoteCur(' '),
      _pricingApplTag1(' '),
      _pricingApplTag2(' '),
      _pricingApplTag3(' '),
      _pricingApplTag4(' '),
      _pricingApplTag5(' '),
      _pricingApplTag6(' '),
      _pricingApplTag7(' '),
      _pricingApplTag8(' '),
      _pricingApplTag9(' '),
      _pricingApplTag10(' '),
      _cat05OverrideCode(" ")
  {
  }
  PseudoCityCode& pseudoCity() { return _pseudoCity; }
  const PseudoCityCode& pseudoCity() const { return _pseudoCity; }

  PseudoCityCode& homePseudoCity() { return _homePseudoCity; }
  const PseudoCityCode& homePseudoCity() const { return _homePseudoCity; }

  std::string& arcNo() { return _arcNo; }
  const std::string& arcNo() const { return _arcNo; }

  std::string& homeArcNo() { return _homeArcNo; }
  const std::string& homeArcNo() const { return _homeArcNo; }

  LocCode& requestCity() { return _requestCity; }
  const LocCode& requestCity() const { return _requestCity; }

  LocCode& aaCity() { return _aaCity; }
  const LocCode& aaCity() const { return _aaCity; }

  CurrencyCode& defaultCur() { return _defaultCur; }
  const CurrencyCode& defaultCur() const { return _defaultCur; }

  std::string& lbtCustomerGroup() { return _lbtCustomerGroup; }
  const std::string& lbtCustomerGroup() const { return _lbtCustomerGroup; }

  std::string& newLbtCustomerGroup() { return _newLbtCustomerGroup; }
  const std::string& newLbtCustomerGroup() const { return _newLbtCustomerGroup; }

  std::string& agencyName() { return _agencyName; }
  const std::string& agencyName() const { return _agencyName; }

  PseudoCityCode& alternateHomePseudo() { return _alternateHomePseudo; }
  const PseudoCityCode& alternateHomePseudo() const { return _alternateHomePseudo; }

  Indicator& branchAccInd() { return _branchAccInd; }
  const Indicator& branchAccInd() const { return _branchAccInd; }

  Indicator& curConvInd() { return _curConvInd; }
  const Indicator& curConvInd() const { return _curConvInd; }

  Indicator& cadSubscriberInd() { return _cadSubscriberInd; }
  const Indicator& cadSubscriberInd() const { return _cadSubscriberInd; }

  Indicator& webSubscriberInd() { return _webSubscriberInd; }
  const Indicator& webSubscriberInd() const { return _webSubscriberInd; }

  Indicator& btsSubscriberInd() { return _btsSubscriberInd; }
  const Indicator& btsSubscriberInd() const { return _btsSubscriberInd; }

  Indicator& sellingFareInd() { return _sellingFareInd; }
  const Indicator& sellingFareInd() const { return _sellingFareInd; }

  Indicator& tvlyInternetSubriber() { return _tvlyInternetSubriber; }
  const Indicator& tvlyInternetSubriber() const { return _tvlyInternetSubriber; }

  Indicator& tvlyLocation() { return _tvlyLocation; }
  const Indicator& tvlyLocation() const { return _tvlyLocation; }

  Indicator& tvlyOnlineLocation() { return _tvlyOnlineLocation; }
  const Indicator& tvlyOnlineLocation() const { return _tvlyOnlineLocation; }

  Indicator& availabilityIgRul2St() { return _availabilityIgRul2St; }
  const Indicator& availabilityIgRul2St() const { return _availabilityIgRul2St; }

  Indicator& availabilityIgRul3St() { return _availabilityIgRul3St; }
  const Indicator& availabilityIgRul3St() const { return _availabilityIgRul3St; }

  std::string& erspNumber() { return _erspNumber; }
  const std::string& erspNumber() const { return _erspNumber; }

  Indicator& channelId() { return _channelId; }
  const Indicator& channelId() const { return _channelId; }

  Indicator& availIgRul2StWpnc() { return _availIgRul2StWpnc; }
  const Indicator& availIgRul2StWpnc() const { return _availIgRul2StWpnc; }

  Indicator& activateJourneyPricing() { return _activateJourneyPricing; }
  const Indicator& activateJourneyPricing() const { return _activateJourneyPricing; }

  Indicator& activateJourneyShopping() { return _activateJourneyShopping; }
  const Indicator& activateJourneyShopping() const { return _activateJourneyShopping; }

  std::string& ownerId() { return _ownerId; }
  const std::string& ownerId() const { return _ownerId; }

  std::string& cruisePfaCur() { return _cruisePfaCur; }
  const std::string& cruisePfaCur() const { return _cruisePfaCur; }

  Indicator& noRollupPfaBulkTkt() { return _noRollupPfaBulkTkt; }
  const Indicator& noRollupPfaBulkTkt() const { return _noRollupPfaBulkTkt; }

  std::string& crsCarrier() { return _crsCarrier; }
  const std::string& crsCarrier() const { return _crsCarrier; }

  Indicator& doNotApplySegmentFee() { return _doNotApplySegmentFee; }
  const Indicator& doNotApplySegmentFee() const { return _doNotApplySegmentFee; }

  TJRGroup& ssgGroupNo() { return _ssgGroupNo; }
  const TJRGroup& ssgGroupNo() const { return _ssgGroupNo; }

  std::string& hostName() { return _hostName; }
  const std::string& hostName() const { return _hostName; }

  Indicator& optInAgency() { return _optInAgency; }
  const Indicator& optInAgency() const { return _optInAgency; }

  Indicator& privateFareInd() { return _privateFareInd; }
  const Indicator& privateFareInd() const { return _privateFareInd; }

  PseudoCityCode& snapHomeAgencyPcc() { return _snapHomeAgencyPcc; }
  const PseudoCityCode& snapHomeAgencyPcc() const { return _snapHomeAgencyPcc; }

  Indicator& doNotApplyObTktFees() { return _doNotApplyObTktFees; }
  const Indicator& doNotApplyObTktFees() const { return _doNotApplyObTktFees; }

  DateTime& createDate() { return _createDate; }
  const DateTime& createDate() const { return _createDate; }

  PaxTypeCode& defaultPassengerType() { return _defaultPassengerType; }
  const PaxTypeCode& defaultPassengerType() const { return _defaultPassengerType; }

  Indicator& eTicketCapable() { return _eTicketCapable; }
  const Indicator& eTicketCapable() const { return _eTicketCapable; }

  Indicator& fareQuoteCur() { return _fareQuoteCur; }
  const Indicator& fareQuoteCur() const { return _fareQuoteCur; }

  Indicator& pricingApplTag1() { return _pricingApplTag1; }
  const Indicator& pricingApplTag1() const { return _pricingApplTag1; }

  Indicator& pricingApplTag2() { return _pricingApplTag2; }
  const Indicator& pricingApplTag2() const { return _pricingApplTag2; }

  Indicator& pricingApplTag3() { return _pricingApplTag3; }
  const Indicator& pricingApplTag3() const { return _pricingApplTag3; }

  Indicator& pricingApplTag4() { return _pricingApplTag4; }
  const Indicator& pricingApplTag4() const { return _pricingApplTag4; }

  Indicator& pricingApplTag5() { return _pricingApplTag5; }
  const Indicator& pricingApplTag5() const { return _pricingApplTag5; }

  Indicator& pricingApplTag6() { return _pricingApplTag6; }
  const Indicator& pricingApplTag6() const { return _pricingApplTag6; }

  Indicator& pricingApplTag7() { return _pricingApplTag7; }
  const Indicator& pricingApplTag7() const { return _pricingApplTag7; }

  Indicator& pricingApplTag8() { return _pricingApplTag8; }
  const Indicator& pricingApplTag8() const { return _pricingApplTag8; }

  Indicator& pricingApplTag9() { return _pricingApplTag9; }
  const Indicator& pricingApplTag9() const { return _pricingApplTag9; }

  Indicator& pricingApplTag10() { return _pricingApplTag10; }
  const Indicator& pricingApplTag10() const { return _pricingApplTag10; }

  Alpha3Char& cat05OverrideCode() { return _cat05OverrideCode; }
  const Alpha3Char& cat05OverrideCode() const { return _cat05OverrideCode; }

  std::string& settlementPlans() { return _settlementPlans; }
  const std::string& settlementPlans() const { return _settlementPlans; }

  bool isArcUser() const { return 'Y' == pricingApplTag2(); }

  bool isMultiSettlementPlanUser() const { return !settlementPlans().empty(); }

  bool operator==(const Customer& rhs) const
  {
    return (
        (_pseudoCity == rhs._pseudoCity) && (_homePseudoCity == rhs._homePseudoCity) &&
        (_arcNo == rhs._arcNo) && (_homeArcNo == rhs._homeArcNo) &&
        (_requestCity == rhs._requestCity) && (_aaCity == rhs._aaCity) &&
        (_defaultCur == rhs._defaultCur) && (_lbtCustomerGroup == rhs._lbtCustomerGroup) &&
        (_newLbtCustomerGroup == rhs._newLbtCustomerGroup) && (_agencyName == rhs._agencyName) &&
        (_alternateHomePseudo == rhs._alternateHomePseudo) &&
        (_branchAccInd == rhs._branchAccInd) && (_curConvInd == rhs._curConvInd) &&
        (_cadSubscriberInd == rhs._cadSubscriberInd) &&
        (_webSubscriberInd == rhs._webSubscriberInd) &&
        (_btsSubscriberInd == rhs._btsSubscriberInd) && (_sellingFareInd == rhs._sellingFareInd) &&
        (_tvlyInternetSubriber == rhs._tvlyInternetSubriber) &&
        (_tvlyLocation == rhs._tvlyLocation) && (_tvlyOnlineLocation == rhs._tvlyOnlineLocation) &&
        (_availabilityIgRul2St == rhs._availabilityIgRul2St) &&
        (_availabilityIgRul3St == rhs._availabilityIgRul3St) && (_erspNumber == rhs._erspNumber) &&
        (_channelId == rhs._channelId) && (_availIgRul2StWpnc == rhs._availIgRul2StWpnc) &&
        (_activateJourneyPricing == rhs._activateJourneyPricing) &&
        (_activateJourneyShopping == rhs._activateJourneyShopping) && (_ownerId == rhs._ownerId) &&
        (_cruisePfaCur == rhs._cruisePfaCur) && (_noRollupPfaBulkTkt == rhs._noRollupPfaBulkTkt) &&
        (_crsCarrier == rhs._crsCarrier) && (_doNotApplySegmentFee == rhs._doNotApplySegmentFee) &&
        (_ssgGroupNo == rhs._ssgGroupNo) && (_hostName == rhs._hostName) &&
        (_optInAgency == rhs._optInAgency) && (_privateFareInd == rhs._privateFareInd) &&
        (_snapHomeAgencyPcc == rhs._snapHomeAgencyPcc) &&
        (_doNotApplyObTktFees == rhs._doNotApplyObTktFees) && (_createDate == rhs._createDate) &&
        (_defaultPassengerType == rhs._defaultPassengerType) &&
        (_eTicketCapable == rhs._eTicketCapable) && (_fareQuoteCur == rhs._fareQuoteCur) &&
        (_pricingApplTag1 == rhs._pricingApplTag1) && (_pricingApplTag2 == rhs._pricingApplTag2) &&
        (_pricingApplTag3 == rhs._pricingApplTag3) && (_pricingApplTag4 == rhs._pricingApplTag4) &&
        (_pricingApplTag5 == rhs._pricingApplTag5) && (_pricingApplTag6 == rhs._pricingApplTag6) &&
        (_pricingApplTag7 == rhs._pricingApplTag7) && (_pricingApplTag8 == rhs._pricingApplTag8) &&
        (_pricingApplTag9 == rhs._pricingApplTag9) &&
        (_pricingApplTag10 == rhs._pricingApplTag10) &&
        (_cat05OverrideCode == rhs._cat05OverrideCode) &&
        (_settlementPlans == rhs._settlementPlans));
  }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  static void dummyData(Customer& obj)
  {
    obj._pseudoCity = "ABCDE";
    obj._homePseudoCity = "FGHIJ";
    obj._arcNo = "aaaaaaaa";
    obj._homeArcNo = "bbbbbbbb";
    obj._requestCity = "KLMNO";
    obj._aaCity = "PQRST";
    obj._defaultCur = "UVW";
    obj._lbtCustomerGroup = "cccccccc";
    obj._newLbtCustomerGroup = "dddddddd";
    obj._agencyName = "eeeeeeee";
    obj._alternateHomePseudo = "XYZab";
    obj._branchAccInd = 'c';
    obj._curConvInd = 'd';
    obj._cadSubscriberInd = 'e';
    obj._webSubscriberInd = 'f';
    obj._btsSubscriberInd = 'g';
    obj._sellingFareInd = 'h';
    obj._tvlyInternetSubriber = 'i';
    obj._tvlyLocation = 'j';
    obj._tvlyOnlineLocation = 'k';
    obj._availabilityIgRul2St = 'l';
    obj._availabilityIgRul3St = 'm';
    obj._erspNumber = "ffffffff";
    obj._channelId = 'n';
    obj._availIgRul2StWpnc = 'o';
    obj._activateJourneyPricing = 'p';
    obj._activateJourneyShopping = 'q';
    obj._ownerId = "gggggggg";
    obj._cruisePfaCur = "hhhhhhhh";
    obj._noRollupPfaBulkTkt = 'r';
    obj._crsCarrier = "iiiiiiii";
    obj._doNotApplySegmentFee = 's';
    obj._ssgGroupNo = 1;
    obj._hostName = "jjjjjjjj";
    obj._optInAgency = 't';
    obj._privateFareInd = 'u';
    obj._snapHomeAgencyPcc = "AAAAA";
    obj._createDate = time(nullptr);
    obj._defaultPassengerType = "ADT";
    obj._eTicketCapable = 'Y';
    obj._fareQuoteCur = 'Y';
    obj._pricingApplTag1 = 'Y';
    obj._pricingApplTag2 = 'N';
    obj._pricingApplTag3 = 'N';
    obj._pricingApplTag4 = 'N';
    obj._pricingApplTag5 = 'N';
    obj._pricingApplTag6 = 'N';
    obj._pricingApplTag7 = 'N';
    obj._pricingApplTag8 = 'N';
    obj._pricingApplTag9 = 'N';
    obj._pricingApplTag10 = 'N';
    obj._cat05OverrideCode = ' ';
    obj._settlementPlans = "SP1SP2SP3SP4";
  }

protected:
  PseudoCityCode _pseudoCity;
  PseudoCityCode _homePseudoCity;
  std::string _arcNo;
  std::string _homeArcNo;
  LocCode _requestCity;
  LocCode _aaCity;
  CurrencyCode _defaultCur;
  std::string _lbtCustomerGroup;
  std::string _newLbtCustomerGroup;
  std::string _agencyName;
  PseudoCityCode _alternateHomePseudo;
  Indicator _branchAccInd;
  Indicator _curConvInd;
  Indicator _cadSubscriberInd;
  Indicator _webSubscriberInd;
  Indicator _btsSubscriberInd;
  Indicator _sellingFareInd;
  Indicator _tvlyInternetSubriber;
  Indicator _tvlyLocation;
  Indicator _tvlyOnlineLocation;
  Indicator _availabilityIgRul2St;
  Indicator _availabilityIgRul3St;
  std::string _erspNumber;
  Indicator _channelId;
  Indicator _availIgRul2StWpnc;
  Indicator _activateJourneyPricing;
  Indicator _activateJourneyShopping;
  std::string _ownerId;
  std::string _cruisePfaCur;
  Indicator _noRollupPfaBulkTkt;
  std::string _crsCarrier;
  Indicator _doNotApplySegmentFee;
  TJRGroup _ssgGroupNo;
  std::string _hostName;
  Indicator _optInAgency;
  Indicator _privateFareInd;
  PseudoCityCode _snapHomeAgencyPcc;
  Indicator _doNotApplyObTktFees;
  DateTime _createDate;
  PaxTypeCode _defaultPassengerType;
  Indicator _eTicketCapable;
  Indicator _fareQuoteCur;
  Indicator _pricingApplTag1;
  Indicator _pricingApplTag2;
  Indicator _pricingApplTag3;
  Indicator _pricingApplTag4;
  Indicator _pricingApplTag5;
  Indicator _pricingApplTag6;
  Indicator _pricingApplTag7;
  Indicator _pricingApplTag8;
  Indicator _pricingApplTag9;
  Indicator _pricingApplTag10;
  Alpha3Char _cat05OverrideCode;
  std::string _settlementPlans;

public:
  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _pseudoCity);
    FLATTENIZE(archive, _homePseudoCity);
    FLATTENIZE(archive, _arcNo);
    FLATTENIZE(archive, _homeArcNo);
    FLATTENIZE(archive, _requestCity);
    FLATTENIZE(archive, _aaCity);
    FLATTENIZE(archive, _defaultCur);
    FLATTENIZE(archive, _lbtCustomerGroup);
    FLATTENIZE(archive, _newLbtCustomerGroup);
    FLATTENIZE(archive, _agencyName);
    FLATTENIZE(archive, _alternateHomePseudo);
    FLATTENIZE(archive, _branchAccInd);
    FLATTENIZE(archive, _curConvInd);
    FLATTENIZE(archive, _cadSubscriberInd);
    FLATTENIZE(archive, _webSubscriberInd);
    FLATTENIZE(archive, _btsSubscriberInd);
    FLATTENIZE(archive, _sellingFareInd);
    FLATTENIZE(archive, _tvlyInternetSubriber);
    FLATTENIZE(archive, _tvlyLocation);
    FLATTENIZE(archive, _tvlyOnlineLocation);
    FLATTENIZE(archive, _availabilityIgRul2St);
    FLATTENIZE(archive, _availabilityIgRul3St);
    FLATTENIZE(archive, _erspNumber);
    FLATTENIZE(archive, _channelId);
    FLATTENIZE(archive, _availIgRul2StWpnc);
    FLATTENIZE(archive, _activateJourneyPricing);
    FLATTENIZE(archive, _activateJourneyShopping);
    FLATTENIZE(archive, _ownerId);
    FLATTENIZE(archive, _cruisePfaCur);
    FLATTENIZE(archive, _noRollupPfaBulkTkt);
    FLATTENIZE(archive, _crsCarrier);
    FLATTENIZE(archive, _doNotApplySegmentFee);
    FLATTENIZE(archive, _ssgGroupNo);
    FLATTENIZE(archive, _hostName);
    FLATTENIZE(archive, _optInAgency);
    FLATTENIZE(archive, _privateFareInd);
    FLATTENIZE(archive, _snapHomeAgencyPcc);
    FLATTENIZE(archive, _doNotApplyObTktFees);
    FLATTENIZE(archive, _createDate);
    FLATTENIZE(archive, _defaultPassengerType);
    FLATTENIZE(archive, _eTicketCapable);
    FLATTENIZE(archive, _fareQuoteCur);
    FLATTENIZE(archive, _pricingApplTag1);
    FLATTENIZE(archive, _pricingApplTag2);
    FLATTENIZE(archive, _pricingApplTag3);
    FLATTENIZE(archive, _pricingApplTag4);
    FLATTENIZE(archive, _pricingApplTag5);
    FLATTENIZE(archive, _pricingApplTag6);
    FLATTENIZE(archive, _pricingApplTag7);
    FLATTENIZE(archive, _pricingApplTag8);
    FLATTENIZE(archive, _pricingApplTag9);
    FLATTENIZE(archive, _pricingApplTag10);
    FLATTENIZE(archive, _cat05OverrideCode);
    FLATTENIZE(archive, _settlementPlans);
  }
private:

  template <typename B, typename T> static B& convert(B& buffer,
                                                      T ptr)
  {
    return buffer
           & ptr->_pseudoCity
           & ptr->_homePseudoCity
           & ptr->_arcNo
           & ptr->_homeArcNo
           & ptr->_requestCity
           & ptr->_aaCity
           & ptr->_defaultCur
           & ptr->_lbtCustomerGroup
           & ptr->_newLbtCustomerGroup
           & ptr->_agencyName
           & ptr->_alternateHomePseudo
           & ptr->_branchAccInd
           & ptr->_curConvInd
           & ptr->_cadSubscriberInd
           & ptr->_webSubscriberInd
           & ptr->_btsSubscriberInd
           & ptr->_sellingFareInd
           & ptr->_tvlyInternetSubriber
           & ptr->_tvlyLocation
           & ptr->_tvlyOnlineLocation
           & ptr->_availabilityIgRul2St
           & ptr->_availabilityIgRul3St
           & ptr->_erspNumber
           & ptr->_channelId
           & ptr->_availIgRul2StWpnc
           & ptr->_activateJourneyPricing
           & ptr->_activateJourneyShopping
           & ptr->_ownerId
           & ptr->_cruisePfaCur
           & ptr->_noRollupPfaBulkTkt
           & ptr->_crsCarrier
           & ptr->_doNotApplySegmentFee
           & ptr->_ssgGroupNo
           & ptr->_hostName
           & ptr->_optInAgency
           & ptr->_privateFareInd
           & ptr->_snapHomeAgencyPcc
           & ptr->_doNotApplyObTktFees
           & ptr->_createDate
           & ptr->_defaultPassengerType
           & ptr->_eTicketCapable
           & ptr->_fareQuoteCur
           & ptr->_pricingApplTag1
           & ptr->_pricingApplTag2
           & ptr->_pricingApplTag3
           & ptr->_pricingApplTag4
           & ptr->_pricingApplTag5
           & ptr->_pricingApplTag6
           & ptr->_pricingApplTag7
           & ptr->_pricingApplTag8
           & ptr->_pricingApplTag9
           & ptr->_pricingApplTag10
           & ptr->_cat05OverrideCode
           & ptr->_settlementPlans;
  }

};
}
