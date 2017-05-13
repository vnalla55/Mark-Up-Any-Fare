//-------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseConsts.h"
#include "DBAccess/BoundFareAdditionalInfo.h"
#include "DBAccess/CompressedDataUtils.h"
#include "DBAccess/FareInfoFactory.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/TSEDateInterval.h"

#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/seq/for_each.hpp>

#include <vector>

#define FARE_INFO_MEMBERS                                                                          \
  (_effInterval)(_vendor)(_carrier)(_market1)(_market2)(_lastModDate)(_originalFareAmount)(        \
      _fareAmount)(_fareClass)(_fareTariff)(_linkNumber)(_sequenceNumber)(_noDec)(_currency)(      \
      _footnote1)(_footnote2)(_owrt)(_directionality)(_ruleNumber)(_routingNumber)(                \
      _globalDirection)(_constructionInd)(_inhibit)(_increasedFareAmtTag)(_reducedFareAmtTag)(     \
      _footnoteTag)(_routingTag)(_mpmTag)(_effectiveDateTag)(_currencyCodeTag)(_ruleTag)(          \
      _pAdditionalInfoContainer)(_vendorFWS)

namespace tse
{
class FareInfo;

namespace flattenizer
{
template <class FareInfo>
inline void
flatten(Flattenizable::Archive& archive, const std::vector<FareInfo*>& v);
template <class FareInfo>
inline void
flatten(Flattenizable::Archive& archive, const std::vector<const FareInfo*>& v);
template <class FareInfo>
inline void
unflatten(Flattenizable::Archive& archive, std::vector<FareInfo*>& v);
template <class FareInfo>
inline void
unflatten(Flattenizable::Archive& archive, std::vector<const FareInfo*>& v);
template <class FareInfo>
inline void
calcmem(Flattenizable::Archive& archive, const std::vector<FareInfo*>& v);
template <class FareInfo>
inline void
calcmem(Flattenizable::Archive& archive, const std::vector<const FareInfo*>& v);
}
}

//-------------------------------------------------------------------
// ** NOTICE **
//
// This class has a clone method, if you add member variables
// please make sure you update the clone method as well!
//-------------------------------------------------------------------

namespace tse
{
class DataHandle;

class FareInfo
{
public:
  FareInfo() = default;
  FareInfo(const FareInfo& rhs) = delete;
  FareInfo& operator=(const FareInfo& rhs) = delete;
  virtual ~FareInfo();

  virtual const eFareInfoType objectType() const { return eFareInfo; }

  /**
   * This methods obtains a new FareInfo pointer from
   * the data handle and populates it to be 'equal'
   * to the current object
   *
   * @param dataHandle
   *
   * @return new object
   */
  virtual FareInfo* clone(DataHandle& dataHandle) const;

  /**
   * This methods populates a given FareInfo to be
   * 'equal' to the current object
   *
   * @param FareInfo - object to populate
   */
  virtual void clone(FareInfo& cloneObj) const;

  virtual const bool isSITA() const { return false; }

  // accessors
  // =========

  TSEDateInterval& effInterval() { return _effInterval; }
  const TSEDateInterval& effInterval() const { return _effInterval; }

  DateTime& createDate() { return _effInterval.createDate(); }
  const DateTime& createDate() const { return _effInterval.createDate(); }

  DateTime& effDate() { return _effInterval.effDate(); }
  const DateTime& effDate() const { return _effInterval.effDate(); }

  DateTime& expireDate() { return _effInterval.expireDate(); }
  const DateTime& expireDate() const { return _effInterval.expireDate(); }

  DateTime& discDate() { return _effInterval.discDate(); }
  const DateTime& discDate() const { return _effInterval.discDate(); }

  const VendorCode& vendor() const { return _vendor; }
  VendorCode& vendor() { return _vendor; }

  const CarrierCode& carrier() const { return _carrier; }
  CarrierCode& carrier() { return _carrier; }

  const LocCode& market1() const { return _market1; }
  LocCode& market1() { return _market1; }

  const LocCode& market2() const { return _market2; }
  LocCode& market2() { return _market2; }

  const DateTime& lastModDate() const { return _lastModDate; }
  DateTime& lastModDate() { return _lastModDate; }

  const MoneyAmount originalFareAmount() const { return _originalFareAmount; }
  MoneyAmount& originalFareAmount() { return _originalFareAmount; }

  const MoneyAmount fareAmount() const { return _fareAmount; }
  MoneyAmount& fareAmount() { return _fareAmount; }

  const FareClassCode& fareClass() const { return _fareClass; }
  FareClassCode& fareClass() { return _fareClass; }

  const TariffNumber fareTariff() const { return _fareTariff; }
  TariffNumber& fareTariff() { return _fareTariff; }

  const LinkNumber linkNumber() const { return _linkNumber; }
  LinkNumber& linkNumber() { return _linkNumber; }

  const SequenceNumberLong sequenceNumber() const { return _sequenceNumber; }
  SequenceNumberLong& sequenceNumber() { return _sequenceNumber; }

  const int noDec() const { return _noDec; }
  int& noDec() { return _noDec; }

  const CurrencyCode& currency() const { return _currency; }
  CurrencyCode& currency() { return _currency; }

  const Footnote& footNote1() const { return _footnote1; }
  Footnote& footNote1() { return _footnote1; }

  const Footnote& footNote2() const { return _footnote2; }
  Footnote& footNote2() { return _footnote2; }

  const Indicator owrt() const { return _owrt; }
  Indicator& owrt() { return _owrt; }

  const Directionality& directionality() const { return _directionality; }
  Directionality& directionality() { return _directionality; }

  const RuleNumber& ruleNumber() const { return _ruleNumber; }
  RuleNumber& ruleNumber() { return _ruleNumber; }

  const RoutingNumber& routingNumber() const { return _routingNumber; }
  RoutingNumber& routingNumber() { return _routingNumber; }

  const GlobalDirection globalDirection() const { return _globalDirection; }
  GlobalDirection& globalDirection() { return _globalDirection; }

  Indicator& constructionInd() { return _constructionInd; }
  const Indicator constructionInd() const { return _constructionInd; }

  const Indicator inhibit() const { return _inhibit; }
  Indicator& inhibit() { return _inhibit; }

  const Indicator increasedFareAmtTag() const { return _increasedFareAmtTag; }
  Indicator& increasedFareAmtTag() { return _increasedFareAmtTag; }

  const Indicator reducedFareAmtTag() const { return _reducedFareAmtTag; }
  Indicator& reducedFareAmtTag() { return _reducedFareAmtTag; }

  const Indicator footnoteTag() const { return _footnoteTag; }
  Indicator& footnoteTag() { return _footnoteTag; }

  const Indicator routingTag() const { return _routingTag; }
  Indicator& routingTag() { return _routingTag; }

  const Indicator mpmTag() const { return _mpmTag; }
  Indicator& mpmTag() { return _mpmTag; }

  const Indicator effectiveDateTag() const { return _effectiveDateTag; }

  Indicator& effectiveDateTag() { return _effectiveDateTag; }

  const Indicator currencyCodeTag() const { return _currencyCodeTag; }
  Indicator& currencyCodeTag() { return _currencyCodeTag; }

  const Indicator ruleTag() const { return _ruleTag; }
  Indicator& ruleTag() { return _ruleTag; }

  const bool vendorFWS() const { return _vendorFWS; }
  bool& vendorFWS() { return _vendorFWS; }

  BindingResult checkBindings(const PricingTrx& trx,
                              const CategoryRuleInfo& item,
                              bool& bLocationSwapped,
                              MATCHTYPE matchType) const;
  template <typename T>
  BindingResultCRIP checkBindings(const PricingTrx& trx,
                                  int cat,
                                  const std::vector<T*>& catRuleInfoList,
                                  bool& bLocationSwapped,
                                  MATCHTYPE matchType) const
  {
    BindingResultCRIP emptyCRIP(false, nullptr);
    return nullptr == _pAdditionalInfoContainer
               ? emptyCRIP
               : _pAdditionalInfoContainer->checkBindings(
                     trx, cat, catRuleInfoList, bLocationSwapped, matchType);
  }

  BindingResult hasCat(const PricingTrx& trx, uint16_t cat, MATCHTYPE matchType) const;
  bool needGeneralRuleValidation(const PricingTrx& trx, int category) const;
  bool checkBookingCodes() const;
  const std::vector<BookingCode>* getBookingCodes(const PricingTrx& trx) const;
  bool isWebFare(bool bTravelocity) const;
  bool isExpediaWebFare() const;
  bool isRoundTrip() const { return _owrt == ROUND_TRIP_MAYNOT_BE_HALVED; }
  TariffNumber getRuleTariff() const;
  TariffNumber getRoutingTariff() const;
  const PaxTypeCode& getPaxType() const;
  const FareType& getFareType() const;
  Indicator getTariffType() const;
  Indicator getDomInternInd() const;
  Indicator negViaAppl() const;
  Indicator nonstopDirectInd() const;
  const bool sameCarrier102() const;
  const bool sameCarrier103() const;
  const bool sameCarrier104() const;
  const Record2ReferenceVector* references() const;
  void resetToOriginalAmount() { _fareAmount = _originalFareAmount; }

public:
  TSEDateInterval _effInterval;
  VendorCode _vendor; // fare/rules vendor
  CarrierCode _carrier; // carrier code
  LocCode _market1; // origin or destination market
  LocCode _market2; // origin or destination market
  DateTime _lastModDate; // last date modified
  MoneyAmount _originalFareAmount = 0; // published amount of the fare
  MoneyAmount _fareAmount = 0; // one way amount of the fare
  FareClassCode _fareClass; // fare class code
  TariffNumber _fareTariff = 0; // fare tariff
  LinkNumber _linkNumber = 0; // link number
  SequenceNumberLong _sequenceNumber = 0; // sequence number
  CurrencyNoDec _noDec = 0; // number of decimal places to be
  CurrencyCode _currency; // currency code
  Footnote _footnote1; // footnote #1
  Footnote _footnote2; // footnote #2
  Indicator _owrt = ' '; // one-way/round-trip indicator
  Directionality _directionality = Directionality::TO; // fare directionality (from/to _market1)
  RuleNumber _ruleNumber; // rule number
  RoutingNumber _routingNumber; // routing number
  GlobalDirection _globalDirection = GlobalDirection::ZZ; // applicable global direction
  Indicator _constructionInd = ' '; // SMF & SITA only
  Indicator _inhibit = ' '; // Inhibit now checked at App Level

  // Change tags                                           intl         dom
  Indicator _increasedFareAmtTag = ' '; //   2           1
  Indicator _reducedFareAmtTag = ' '; //   3           2
  Indicator _footnoteTag = ' '; //   5           3
  Indicator _routingTag = ' '; //   6           4
  Indicator _mpmTag = ' '; //   8           5
  Indicator _effectiveDateTag = ' '; //  10          N/A
  Indicator _currencyCodeTag = ' '; //  17          N/A
  Indicator _ruleTag = ' '; //  N/A         11

  AdditionalInfoContainer* _pAdditionalInfoContainer = nullptr;
  bool _vendorFWS = false;

  virtual bool operator==(const FareInfo& rhs) const
  {
    bool eq =
        ((_effInterval == rhs._effInterval) && (_vendor == rhs._vendor) &&
         (_carrier == rhs._carrier) && (_market1 == rhs._market1) && (_market2 == rhs._market2) &&
         (_lastModDate == rhs._lastModDate) && (_originalFareAmount == rhs._originalFareAmount) &&
         (_fareAmount == rhs._fareAmount) && (_fareClass == rhs._fareClass) &&
         (_fareTariff == rhs._fareTariff) && (_linkNumber == rhs._linkNumber) &&
         (_sequenceNumber == rhs._sequenceNumber) && (_noDec == rhs._noDec) &&
         (_currency == rhs._currency) && (_footnote1 == rhs._footnote1) &&
         (_footnote2 == rhs._footnote2) && (_owrt == rhs._owrt) &&
         (_directionality == rhs._directionality) && (_ruleNumber == rhs._ruleNumber) &&
         (_routingNumber == rhs._routingNumber) && (_globalDirection == rhs._globalDirection) &&
         (_constructionInd == rhs._constructionInd) && (_inhibit == rhs._inhibit) &&
         (_increasedFareAmtTag == rhs._increasedFareAmtTag) &&
         (_reducedFareAmtTag == rhs._reducedFareAmtTag) && (_footnoteTag == rhs._footnoteTag) &&
         (_routingTag == rhs._routingTag) && (_mpmTag == rhs._mpmTag) &&
         (_effectiveDateTag == rhs._effectiveDateTag) &&
         (_currencyCodeTag == rhs._currencyCodeTag) && (_ruleTag == rhs._ruleTag) &&
         (_vendorFWS == rhs._vendorFWS));

    if (eq)
    {
      if (_pAdditionalInfoContainer == nullptr)
      {
        eq = (rhs._pAdditionalInfoContainer == nullptr);
      }
      else
      {
        eq = ((rhs._pAdditionalInfoContainer != nullptr) &&
              (*_pAdditionalInfoContainer == *(rhs._pAdditionalInfoContainer)));
      }
    }

    return eq;
  }

  static void dummyData(FareInfo& obj) { obj.dummyData(); }

  virtual WBuffer& write(WBuffer& os, size_t* memSize) const
  {
    os.write(static_cast<boost::uint8_t>(eFareInfo));
    if (UNLIKELY(memSize))
    {
      *memSize += sizeof(FareInfo);
    }
    return convert(os, this);
  }

  virtual RBuffer& read(RBuffer& is) { return convert(is, this); }

  virtual void flattenize(Flattenizable::Archive& archive)
  {
#define READ_WRITE(r, d, member) FLATTENIZE(archive, member);
    BOOST_PP_SEQ_FOR_EACH(READ_WRITE, BOOST_PP_EMPTY(), FARE_INFO_MEMBERS)
#undef READ_WRITE
  }

  friend inline std::ostream& dumpObject(std::ostream& os, const FareInfo& obj)
  {
    obj.dumpObject(os);
    return os;
  }

protected:
  void dumpFareInfo(std::ostream& os) const;
  virtual void dumpObject(std::ostream& os) const;
  virtual void dummyData();

  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_effInterval & ptr->_vendor & ptr->_carrier & ptr->_market1 &
           ptr->_market2 & ptr->_lastModDate & ptr->_originalFareAmount & ptr->_fareAmount &
           ptr->_fareClass & ptr->_fareTariff & ptr->_linkNumber & ptr->_sequenceNumber &
           ptr->_noDec & ptr->_currency & ptr->_footnote1 & ptr->_footnote2 & ptr->_owrt &
           ptr->_directionality & ptr->_ruleNumber & ptr->_routingNumber & ptr->_globalDirection &
           ptr->_constructionInd & ptr->_inhibit & ptr->_increasedFareAmtTag &
           ptr->_reducedFareAmtTag & ptr->_footnoteTag & ptr->_routingTag & ptr->_mpmTag &
           ptr->_effectiveDateTag & ptr->_currencyCodeTag & ptr->_ruleTag &
           ptr->_pAdditionalInfoContainer & ptr->_vendorFWS;
  }

private:
  void _clone(FareInfo& cloneObj) const;
};

typedef std::vector<const FareInfo*> FareInfoVec;
typedef FareInfoVec::iterator FareInfoVecI;

} // namespace tse

namespace tse
{
namespace flattenizer
{
template <>
inline void
flatten(Flattenizable::Archive& archive, const std::vector<const FareInfo*>& v)
{
  size_t item_counter(0);
  archive.append(v.size());
  for (const auto elem : v)
  {
    archive.flattenizeSubItemBegin();
    FLATTENIZE_PUSH(archive, item_counter++);
    archive.append(static_cast<size_t>(elem->objectType()));
    flatten(archive, *elem);
    archive.flattenizePop();
    archive.flattenizeSubItemEnd();
  }
}

template <>
inline void
unflatten(Flattenizable::Archive& archive, std::vector<const FareInfo*>& v)
{
  size_t item_counter(0);
  size_t sz(0);
  archive.extract(sz);
  v.clear();

  while (sz--)
  {
    archive.flattenizeSubItemBegin();
    FLATTENIZE_PUSH(archive, item_counter++);
    size_t type;
    archive.extract(type);

    FareInfo* info(FareInfoFactory::create(static_cast<eFareInfoType>(type)));

    unflatten(archive, *info);
    v.push_back(info);
    archive.flattenizePop();
    archive.flattenizeSubItemEnd();
  }
}

template <>
inline void
calcmem(Flattenizable::Archive& archive, const std::vector<const FareInfo*>& v)
{
  size_t item_counter(0);
  archive.addSize(sizeof(size_t));
  for (const auto elem : v)
  {
    archive.flattenizeSubItemBegin();
    FLATTENIZE_PUSH(archive, item_counter++);
    archive.addSize(sizeof(size_t));
    calcmem(archive, *elem);
    archive.flattenizePop();
    archive.flattenizeSubItemEnd();
  }
}
} // namespace flattenizer
} // namespace tse

