#pragma once

#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Flattenizable.h"
#include "DBAccess/Record2Types.h"

namespace tse
{
enum ReferenceType
{
  BINDINGSNONE = -1,
  BINDINGSFOOTNOTE1 // 0
  ,
  BINDINGSFOOTNOTE2 // 1
  ,
  BINDINGSFARERULE // 2
  ,
  BINDINGSGENERALRULE // 3
  ,
  NUMBERBINDINGTYPES // 4
};

class PricingTrx;
typedef std::pair<bool, bool> BindingResult;
typedef std::pair<bool, CategoryRuleInfo*> BindingResultCRIP;

// { 10 | 9000000 | B/F | 0-3 }
class Record2Reference
{
public:
  Record2Reference();
  Record2Reference(uint16_t catNumber,
                   SequenceNumberLong sequenceNumber,
                   Indicator flipIndicator,
                   MATCHTYPE matchType);
  virtual ~Record2Reference();

  bool operator<(const Record2Reference& other) const
  {
    if (_catNumber < other._catNumber)
      return true;
    if (_catNumber > other._catNumber)
      return false;
    if (_matchType < other._matchType)
      return true;
    // if (_matchType > other._matchType) return false;
    return false;
  }
  Record2Reference* clone(class DataHandle& dataHandle) const;
  Record2Reference* clone() const;

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _catNumber);
    FLATTENIZE(archive, _sequenceNumber);
    FLATTENIZE(archive, _matchType);
    FLATTENIZE(archive, _flipIndicator);
  }

  uint16_t _catNumber;
  SequenceNumberLong _sequenceNumber;
  MATCHTYPE _matchType;
  Indicator _flipIndicator;

  bool operator==(const Record2Reference& rhs) const
  {
    bool eq = ((_catNumber == rhs._catNumber) && (_sequenceNumber == rhs._sequenceNumber) &&
               (_matchType == rhs._matchType) && (_flipIndicator == rhs._flipIndicator));

    return eq;
  }

  static void dummyData(Record2Reference& obj)
  {
    obj._catNumber = 1;
    obj._sequenceNumber = 2;
    obj._matchType = FARERULE;
    obj._flipIndicator = 'A';
  }

};

std::ostream& operator<<(std::ostream& os, const Record2Reference& ref);

typedef std::vector<Record2Reference> Record2ReferenceVector;
class AdditionalInfoContainer
{
public:
  AdditionalInfoContainer();
  virtual ~AdditionalInfoContainer();

  AdditionalInfoContainer* clone() const;

  template <typename T>
  BindingResultCRIP checkBindings(const PricingTrx& trx,
                                  int cat,
                                  const std::vector<T*>& catRuleInfoList,
                                  bool& bLocationSwapped,
                                  MATCHTYPE matchType) const
  {
    return _checkImpl(trx, cat, catRuleInfoList, bLocationSwapped, matchType);
  }

  BindingResult checkBindings(const PricingTrx& trx,
                              const CategoryRuleInfo& item,
                              bool& bLocationSwapped,
                              MATCHTYPE matchType) const;
  BindingResult hasCat(const PricingTrx& trx, uint16_t cat, MATCHTYPE matchType) const;
  bool needGeneralRuleValidation(const PricingTrx& trx, int category) const;
  bool checkBookingCodes() const;
  const std::vector<BookingCode>* getBookingCodes(const PricingTrx& trx) const;
  bool isWebFare(bool bTravelocity) const;
  Indicator negViaAppl() const { return _negViaAppl; }
  Indicator nonstopDirectInd() const { return _nonstopDirectInd; }
  const PaxTypeCode& getPaxType() const { return _paxType; }

  TariffNumber _ruleTariff;
  TariffNumber _routingTariff;
  FareType _fareType;
  PaxTypeCode _paxType;
  Indicator _negViaAppl;
  Indicator _nonstopDirectInd;
  bool _sameCarrier102;
  bool _sameCarrier103;
  bool _sameCarrier104;
  bool _travelocityWebfare;
  bool _expediaWebfare;
  Indicator _tariffType;
  Indicator _domInternInd;
  std::vector<BookingCode> _bookingCodes;
  Record2ReferenceVector _references;

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, _ruleTariff);
    FLATTENIZE(archive, _routingTariff);
    FLATTENIZE(archive, _fareType);
    FLATTENIZE(archive, _paxType);
    FLATTENIZE(archive, _negViaAppl);
    FLATTENIZE(archive, _nonstopDirectInd);
    FLATTENIZE(archive, _sameCarrier102);
    FLATTENIZE(archive, _sameCarrier103);
    FLATTENIZE(archive, _sameCarrier104);
    FLATTENIZE(archive, _travelocityWebfare);
    FLATTENIZE(archive, _expediaWebfare);
    FLATTENIZE(archive, _tariffType);
    FLATTENIZE(archive, _domInternInd);
    FLATTENIZE(archive, _bookingCodes);
    FLATTENIZE(archive, _references);
  }

  bool operator==(const AdditionalInfoContainer& rhs) const
  {
    bool eq =
        ((_ruleTariff == rhs._ruleTariff) && (_routingTariff == rhs._routingTariff) &&
         (_fareType == rhs._fareType) && (_paxType == rhs._paxType) &&
         (_negViaAppl == rhs._negViaAppl) && (_nonstopDirectInd == rhs._nonstopDirectInd) &&
         (_sameCarrier102 == rhs._sameCarrier102) && (_sameCarrier103 == rhs._sameCarrier103) &&
         (_sameCarrier104 == rhs._sameCarrier104) &&
         (_travelocityWebfare == rhs._travelocityWebfare) &&
         (_expediaWebfare == rhs._expediaWebfare) && (_tariffType == rhs._tariffType) &&
         (_domInternInd == rhs._domInternInd) &&
         (_bookingCodes.size() == rhs._bookingCodes.size()) &&
         (_references.size() == rhs._references.size()));

    for (size_t i = 0; (eq && (i < _bookingCodes.size())); ++i)
    {
      eq = (_bookingCodes[i] == rhs._bookingCodes[i]);
    }

    for (size_t j = 0; (eq && (j < _references.size())); ++j)
    {
      eq = (_references[j] == rhs._references[j]);
    }

    return eq;
  }

  static void dummyData(AdditionalInfoContainer& obj)
  {
    obj._ruleTariff = 1;
    obj._routingTariff = 2;
    obj._fareType = "ABCDEFGH";
    obj._paxType = "IJK";
    obj._negViaAppl = 'L';
    obj._nonstopDirectInd = 'M';
    obj._sameCarrier102 = false;
    obj._sameCarrier103 = true;
    obj._sameCarrier104 = false;
    obj._travelocityWebfare = true;
    obj._expediaWebfare = false;
    obj._tariffType = 'N';
    obj._domInternInd = 'O';

    obj._bookingCodes.push_back("PQ");
    obj._bookingCodes.push_back("RS");

    Record2Reference r2r1;
    Record2Reference r2r2;

    Record2Reference::dummyData(r2r1);
    Record2Reference::dummyData(r2r2);

    obj._references.push_back(r2r1);
    obj._references.push_back(r2r2);
  }

private:
  template <typename T>
  BindingResultCRIP _checkImpl(const PricingTrx& trx,
                               int cat,
                               const std::vector<T*>& catRuleInfoList,
                               bool& bLocationSwapped,
                               MATCHTYPE matchType) const;
  // not implemented
  AdditionalInfoContainer& operator=(const AdditionalInfoContainer&);
};
std::ostream& operator<<(std::ostream& os, const AdditionalInfoContainer& cntnr);

} // tse
