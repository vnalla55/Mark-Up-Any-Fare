#include "DBAccess/BoundFareAdditionalInfo.h"

#include "Allocator/TrxMalloc.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FootNoteCtrlInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"

namespace tse
{
Record2Reference::Record2Reference()
  : _catNumber(0), _sequenceNumber(0), _matchType(MATCHNONE), _flipIndicator(' ')
{
}

Record2Reference::Record2Reference(uint16_t catNumber,
                                   SequenceNumberLong sequenceNumber,
                                   Indicator flipIndicator,
                                   MATCHTYPE matchType)
  : _catNumber(catNumber),
    _sequenceNumber(sequenceNumber),
    _matchType(matchType),
    _flipIndicator(flipIndicator)
{
}

Record2Reference::~Record2Reference() {}

std::ostream& operator<<(std::ostream& os, const Record2Reference& ref)
{
  os << ref._catNumber << ',' << ref._sequenceNumber << ',' << ref._matchType << ','
     << ref._flipIndicator;
  return os;
}
// AdditionalInfoContainer
template BindingResultCRIP
AdditionalInfoContainer::_checkImpl<GeneralFareRuleInfo>(
    const PricingTrx& trx,
    int cat,
    const std::vector<GeneralFareRuleInfo*>& catRuleInfoList,
    bool& bLocationSwapped,
    MATCHTYPE matchType) const;
template BindingResultCRIP
AdditionalInfoContainer::_checkImpl<FootNoteCtrlInfo>(
    const PricingTrx& trx,
    int cat,
    const std::vector<FootNoteCtrlInfo*>& catRuleInfoList,
    bool& bLocationSwapped,
    MATCHTYPE matchType) const;
AdditionalInfoContainer::AdditionalInfoContainer()
  : _ruleTariff(0),
    _routingTariff(0),
    _paxType("ADT"),
    _negViaAppl(' '),
    _nonstopDirectInd(' '),
    _sameCarrier102(false),
    _sameCarrier103(false),
    _sameCarrier104(false),
    _travelocityWebfare(false),
    _expediaWebfare(false),
    _tariffType(' '),
    _domInternInd(' ')
{
}

AdditionalInfoContainer::~AdditionalInfoContainer() {}

AdditionalInfoContainer*
AdditionalInfoContainer::clone() const
{
  const MallocContextDisabler context;

  return new AdditionalInfoContainer(*this);
}

std::ostream& operator<<(std::ostream& os, const AdditionalInfoContainer& cntnr)
{
  os << cntnr._ruleTariff << ',' << cntnr._routingTariff << ',' << cntnr._fareType << ','
     << cntnr._paxType << ',' << cntnr._negViaAppl << ',' << cntnr._nonstopDirectInd << ','
     << cntnr._sameCarrier102 << ',' << cntnr._sameCarrier103 << ',' << cntnr._sameCarrier104 << ','
     << cntnr._travelocityWebfare << ',' << cntnr._expediaWebfare << ',' << cntnr._tariffType << ','
     << cntnr._domInternInd << ';';
  std::ostream_iterator<BookingCode> ostreamChIt(os, "|");
  std::copy(cntnr._bookingCodes.begin(), cntnr._bookingCodes.end(), ostreamChIt);
  os << ';';
  std::ostream_iterator<Record2Reference> ostreamIt(os, ";");
  std::copy(cntnr._references.begin(), cntnr._references.end(), ostreamIt);
  return os;
}
BindingResult
AdditionalInfoContainer::checkBindings(const PricingTrx& trx,
                                       const CategoryRuleInfo& item,
                                       bool& bLocationSwapped,
                                       MATCHTYPE matchType) const
{
  // std::cerr << "AdditionalInfoContainer::checkBindings:item" << std::endl;
  BindingResult result(trx.getValidateUsingBindings(), false);
  if (!result.first)
  {
    return result;
  }
  uint16_t catNumber(item.categoryNumber());
  SequenceNumberLong sequenceNumber(item.sequenceNumber());
  for (const auto& elem : _references)
  {
    if (catNumber == elem._catNumber && matchType == elem._matchType)
    {
      if (sequenceNumber == elem._sequenceNumber)
      {
        result.second = true;
        bLocationSwapped = 'T' == elem._flipIndicator;
        // std::cerr << "AdditionalInfoContainer::checkBindings:item:found match" << std::endl;
      }
      break;
    }
  }
  return result;
}

BindingResult
AdditionalInfoContainer::hasCat(const PricingTrx& trx, uint16_t catNumber, MATCHTYPE matchType)
    const
{
  BindingResult result(trx.getValidateUsingBindings(), false);
  if (!result.first)
  {
    return result;
  }
  for (const auto& elem : _references)
  {
    if (catNumber == elem._catNumber && matchType == elem._matchType)
    {
      result.second = true;
      // std::cerr << "AdditionalInfoContainer::hasCat:found match" << std::endl;
      break;
    }
  }
  return result;
}
template <typename T>
BindingResultCRIP
AdditionalInfoContainer::_checkImpl(const PricingTrx& trx,
                                    int cat,
                                    const std::vector<T*>& catRuleInfoList,
                                    bool& bLocationSwapped,
                                    MATCHTYPE matchType) const
{
  BindingResultCRIP result(trx.getValidateUsingBindings(), nullptr);
  if (!result.first)
  {
    return result;
  }
  for (const auto& el : _references)
  {
    if (cat == el._catNumber && matchType == el._matchType)
    {
      SequenceNumberLong bindingsSequenceNumber(el._sequenceNumber);
      for (const auto elem : catRuleInfoList)
      {
        if (elem->sequenceNumber() == bindingsSequenceNumber)
        {
          result.second = elem;
          bLocationSwapped = 'T' == el._flipIndicator;
          return result;
        }
      }
      break;
    }
  }
  return result;
}

bool
AdditionalInfoContainer::needGeneralRuleValidation(const PricingTrx& trx, int category) const

{
  return !trx.getValidateUsingBindings();
}

bool
AdditionalInfoContainer::checkBookingCodes() const
{
  // std::ostream &os = std::cerr;
  // std::ostream_iterator<BookingCode> outIter(os, " ");
  // std::copy(_bookingCodes.begin(), _bookingCodes.end(), outIter);
  // os << std::endl;
  return true;
}

const std::vector<BookingCode>*
AdditionalInfoContainer::getBookingCodes(const PricingTrx& trx) const
{
  if (trx.getValidateBookingCodeBF())
  {
    return &_bookingCodes;
  }
  else
  {
    return nullptr;
  }
}

bool
AdditionalInfoContainer::isWebFare(bool bTravelocity) const
{
  return bTravelocity ? _travelocityWebfare : false;
}

} // tse
