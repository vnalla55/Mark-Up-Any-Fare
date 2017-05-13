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

#include "DBAccess/FareInfo.h"

#include "Common/ObjectComparison.h"
#include "Common/TSEException.h"
#include "DBAccess/DataHandle.h"

using namespace tse;

FareInfo::~FareInfo() { delete _pAdditionalInfoContainer; }

FareInfo*
FareInfo::clone(DataHandle& dataHandle) const
{
  FareInfo* cloneObj = nullptr;
  dataHandle.get(cloneObj);
  if (LIKELY(cloneObj))
  {
    _clone(*cloneObj);
  }
  return cloneObj;
}

void
FareInfo::clone(FareInfo& cloneObj) const
{
  _clone(cloneObj);
}

void
FareInfo::_clone(FareInfo& cloneObj) const
{
  _effInterval.cloneDateInterval(cloneObj.effInterval());

  cloneObj._vendor = _vendor;
  cloneObj._carrier = _carrier;
  cloneObj._market1 = _market1;
  cloneObj._market2 = _market2;
  cloneObj._lastModDate = _lastModDate;
  cloneObj._originalFareAmount = _originalFareAmount;
  cloneObj._fareAmount = _fareAmount;
  cloneObj._fareClass = _fareClass;
  cloneObj._fareTariff = _fareTariff;
  cloneObj._linkNumber = _linkNumber;
  cloneObj._sequenceNumber = _sequenceNumber;
  cloneObj._noDec = _noDec;
  cloneObj._currency = _currency;
  cloneObj._footnote1 = _footnote1;
  cloneObj._footnote2 = _footnote2;
  cloneObj._owrt = _owrt;
  cloneObj._directionality = _directionality;
  cloneObj._ruleNumber = _ruleNumber;
  cloneObj._routingNumber = _routingNumber;
  cloneObj._globalDirection = _globalDirection;
  cloneObj._constructionInd = _constructionInd;
  cloneObj._inhibit = _inhibit;
  cloneObj._increasedFareAmtTag = _increasedFareAmtTag;
  cloneObj._reducedFareAmtTag = _reducedFareAmtTag;
  cloneObj._footnoteTag = _footnoteTag;
  cloneObj._routingTag = _routingTag;
  cloneObj._mpmTag = _mpmTag;
  cloneObj._effectiveDateTag = _effectiveDateTag;
  cloneObj._currencyCodeTag = _currencyCodeTag;
  cloneObj._ruleTag = _ruleTag;
  if (UNLIKELY(_pAdditionalInfoContainer != nullptr))
  {
    cloneObj._pAdditionalInfoContainer = _pAdditionalInfoContainer->clone();
  }
  cloneObj._vendorFWS = _vendorFWS;
}

const BindingResult
_empty(false, false);

BindingResult
FareInfo::checkBindings(const PricingTrx& trx,
                        const CategoryRuleInfo& item,
                        bool& bLocationSwapped,
                        MATCHTYPE matchType) const
{
  return nullptr == _pAdditionalInfoContainer
             ? _empty
             : _pAdditionalInfoContainer->checkBindings(trx, item, bLocationSwapped, matchType);
}

BindingResult
FareInfo::hasCat(const PricingTrx& trx, uint16_t cat, MATCHTYPE matchType) const
{
  return nullptr == _pAdditionalInfoContainer ? _empty
                                        : _pAdditionalInfoContainer->hasCat(trx, cat, matchType);
}

bool
FareInfo::needGeneralRuleValidation(const PricingTrx& trx, int cat) const
{
  return nullptr == _pAdditionalInfoContainer
             ? true
             : _pAdditionalInfoContainer->needGeneralRuleValidation(trx, cat);
}

bool
FareInfo::checkBookingCodes() const
{
  bool bResult(false);
  if (_pAdditionalInfoContainer != nullptr)
  {
    bResult = _pAdditionalInfoContainer->checkBookingCodes();
  }
  return bResult;
}

const std::vector<BookingCode>*
FareInfo::getBookingCodes(const PricingTrx& trx) const
{
  return nullptr == _pAdditionalInfoContainer ? nullptr : _pAdditionalInfoContainer->getBookingCodes(trx);
}

const PaxTypeCode&
FareInfo::getPaxType() const
{
  static const PaxTypeCode _empty;
  return nullptr == _pAdditionalInfoContainer ? _empty : _pAdditionalInfoContainer->getPaxType();
}

bool
FareInfo::isWebFare(bool bTravelocity) const
{
  return nullptr == _pAdditionalInfoContainer ? false
                                        : _pAdditionalInfoContainer->isWebFare(bTravelocity);
}

bool
FareInfo::isExpediaWebFare() const
{
  return nullptr == _pAdditionalInfoContainer ? false : _pAdditionalInfoContainer->_expediaWebfare;
}

Indicator
FareInfo::negViaAppl() const
{
  return nullptr == _pAdditionalInfoContainer ? ' ' : _pAdditionalInfoContainer->_negViaAppl;
}

Indicator
FareInfo::nonstopDirectInd() const
{
  return nullptr == _pAdditionalInfoContainer ? ' ' : _pAdditionalInfoContainer->_nonstopDirectInd;
}

TariffNumber
FareInfo::getRuleTariff() const
{
  return nullptr == _pAdditionalInfoContainer ? 0 : _pAdditionalInfoContainer->_ruleTariff;
}

TariffNumber
FareInfo::getRoutingTariff() const
{
  return nullptr == _pAdditionalInfoContainer ? 0 : _pAdditionalInfoContainer->_routingTariff;
}

const FareType&
FareInfo::getFareType() const
{
  const static FareType _none;
  return nullptr == _pAdditionalInfoContainer ? _none : _pAdditionalInfoContainer->_fareType;
}

Indicator
FareInfo::getTariffType() const
{
  return nullptr == _pAdditionalInfoContainer ? ' ' : _pAdditionalInfoContainer->_tariffType;
}

Indicator
FareInfo::getDomInternInd() const
{
  return nullptr == _pAdditionalInfoContainer ? ' ' : _pAdditionalInfoContainer->_domInternInd;
}

const bool
FareInfo::sameCarrier102() const
{
  return (nullptr == _pAdditionalInfoContainer) ? false : _pAdditionalInfoContainer->_sameCarrier102;
}

const bool
FareInfo::sameCarrier103() const
{
  return (nullptr == _pAdditionalInfoContainer) ? false : _pAdditionalInfoContainer->_sameCarrier103;
}

const bool
FareInfo::sameCarrier104() const
{
  return (nullptr == _pAdditionalInfoContainer) ? false : _pAdditionalInfoContainer->_sameCarrier104;
}

const Record2ReferenceVector*
FareInfo::references() const
{
  return (nullptr == _pAdditionalInfoContainer) ? nullptr : &(_pAdditionalInfoContainer->_references);
}

void
FareInfo::dumpFareInfo(std::ostream& os) const
{
  os << "[";
  ::dumpObject(os, _effInterval);
  os << "|" << _vendor << "|" << _carrier << "|" << _market1 << "|" << _market2 << "|"
     << _lastModDate << "|" << _originalFareAmount << "|" << _fareAmount << "|" << _fareClass << "|"
     << _fareTariff << "|" << _linkNumber << "|" << _sequenceNumber << "|" << _noDec << "|"
     << _currency << "|" << _footnote1 << "|" << _footnote2 << "|" << _owrt << "|"
     << _directionality << "|" << _ruleNumber << "|" << _routingNumber << "|" << _globalDirection
     << "|" << _constructionInd << "|" << _inhibit << "|" << _increasedFareAmtTag << "|"
     << _reducedFareAmtTag << "|" << _footnoteTag << "|" << _routingTag << "|" << _mpmTag << "|"
     << _effectiveDateTag << "|" << _currencyCodeTag << "|" << _ruleTag << "|" << _vendorFWS;
  if (_pAdditionalInfoContainer)
  {
    os << "|" << *_pAdditionalInfoContainer;
  }
}

void
FareInfo::dumpObject(std::ostream& os) const
{
  dumpFareInfo(os);
  os << "]";
}

void
FareInfo::dummyData()
{
  TSEDateInterval::dummyData(_effInterval);

  _vendor = "ABCD";
  _carrier = "EFG";
  _market1 = "aaaaaaaa";
  _market2 = "bbbbbbbb";
  _lastModDate = time(nullptr);
  _originalFareAmount = 1.11;
  _fareAmount = 2.22;
  _fareClass = "HIJKLMNO";
  _fareTariff = 3;
  _linkNumber = 4;
  _sequenceNumber = 70000000007;
  _noDec = 6;
  _currency = "PQR";
  _footnote1 = "ST";
  _footnote2 = "UV";
  _owrt = 'W';
  _directionality = TO;
  _ruleNumber = "XYZa";
  _routingNumber = "bcde";
  _globalDirection = GlobalDirection::US;
  _constructionInd = 'f';
  _inhibit = 'g';
  _increasedFareAmtTag = 'h';
  _reducedFareAmtTag = 'i';
  _footnoteTag = 'j';
  _routingTag = 'k';
  _mpmTag = 'l';
  _effectiveDateTag = 'm';
  _currencyCodeTag = 'n';
  _ruleTag = 'o';

  _pAdditionalInfoContainer = new AdditionalInfoContainer;
  AdditionalInfoContainer::dummyData(*_pAdditionalInfoContainer);
  _vendorFWS = true;
}
