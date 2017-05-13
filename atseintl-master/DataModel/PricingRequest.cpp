//-------------------------------------------------------------------
//
//  File:        PricingRequest.cpp
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
//          03/08/04 - VN - file created.
//          04/05/04 - Mike Carroll - Added initialization for new members.
//          02/14/05 - Quan Ta - Renamed from "Request" to PricingRequest.
//
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

#include "DataModel/PricingRequest.h"

#include "Common/NonFatalErrorResponseException.h"
#include "DataModel/AvailData.h"
#include "DataModel/FrequentFlyerAccount.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ReservationData.h"
#include "DataModel/Traveler.h"
#include "DataModel/TravelSeg.h"

#include <boost/tokenizer.hpp>

namespace tse
{
/**
* Get discount percentage for a travel segment
*/
const Percent
PricingRequest::discountPercentage(const int16_t segmentOrder) const
{
  std::map<int16_t, Percent>::const_iterator i = _discPercentages.find(segmentOrder);
  if (i != _discPercentages.end())
    return i->second;

  return 0;
}

const Percent*
PricingRequest::discountPercentageNew(const int16_t segmentOrder, const PricingTrx& trx) const
{
  return discountsNew().getPercentage(segmentOrder, trx.isMip());
}

/**
* Get discount percentage for a travel segment
*/
const Percent*
Discounts::getPercentage(const int16_t segmentOrder, const bool isMip) const
{
  if (isMip)
    return _percentages.empty() ? nullptr : &(_percentages.begin()->second);

  std::map<int16_t, Percent>::const_iterator i = _percentages.find(segmentOrder);
  if (i != _percentages.end())
    return &(i->second);

  return nullptr;
}

/**
 * Get discount amount containing a travel segment
 */
const DiscountAmount*
PricingRequest::discountAmount(const int16_t startSegOrder) const
{
  std::vector<DiscountAmount>::const_iterator i = _discAmounts.begin();
  for (; i != _discAmounts.end(); i++)
  {
    if (startSegOrder >= (*i).startSegmentOrder && startSegOrder <= (*i).endSegmentOrder)
      return &(*i);
  }

  return nullptr;
}

/**
 * Get discount amount containing a travel segment
 */
const DiscountAmount*
Discounts::getAmount(const int16_t startSegOrder) const
{
  for (const auto& amount : _amounts)
    if (startSegOrder >= amount.startSegmentOrder && startSegOrder <= amount.endSegmentOrder)
      return &amount;

  return nullptr;
}

bool
Discounts::isPPEntry() const
{
  return std::any_of(_percentages.begin(), _percentages.end(), [](auto& p){return p.second < 0;});
}

bool
Discounts::isDPEntry() const
{
  return std::any_of(_percentages.begin(), _percentages.end(), [](auto& p){return p.second > 0;});
}

bool
Discounts::isPAEntry() const
{
  return std::any_of(_amounts.begin(), _amounts.end(), [](auto& a){return a.amount < 0;});
}

bool
Discounts::isDAEntry() const
{
  return std::any_of(_amounts.begin(), _amounts.end(), [](auto& a){return a.amount > 0;});
}

void
Discounts::addPercentage(int16_t segmentOrder, Percent percentage)
{
  _percentages.emplace(segmentOrder, percentage);
}

void
Discounts::addAmount(size_t setNo,
                     const int16_t segmentOrder,
                     const MoneyAmount amount,
                     const std::string& currencyCode)
{
  if (!setNo)
  {
    setNo = _amounts.size() + 1;
    for (uint16_t i = 0; i < _amounts.size(); ++i)
    {
      if (_amounts[i].amount == amount)
      {
        setNo = i + 1;
        break;
      }
    }
  }

  if (setNo > _amounts.size()) // New set of PA or DA
    _amounts.resize(setNo);

  DiscountAmount& curAmount = _amounts[setNo - 1];
  curAmount.amount = amount;
  curAmount.currencyCode = currencyCode;

  if (!curAmount.startSegmentOrder || curAmount.startSegmentOrder > segmentOrder)
    curAmount.startSegmentOrder = segmentOrder;

  if (!curAmount.endSegmentOrder || curAmount.endSegmentOrder < segmentOrder)
    curAmount.endSegmentOrder = segmentOrder;
}

/**
 * Check if the travel segment is YY override
 */
bool
PricingRequest::industryFareOverride(const int16_t segmentOrder)
{
  std::vector<int16_t>::const_iterator i =
      std::find(_industryFareOverrides.begin(), _industryFareOverrides.end(), segmentOrder);
  return i != _industryFareOverrides.end();
}

/**
 * Check if the travel segment is governing carrier override
 */
const CarrierCode
PricingRequest::governingCarrierOverride(const int16_t segmentOrder) const
{
  std::map<int16_t, CarrierCode>::const_iterator i = _governingCarrierOverrides.find(segmentOrder);
  if (i != _governingCarrierOverrides.end())
    return i->second;

  return "";
}

/**
 * Check if the travel segment is Ticket Designator
 */
const TktDesignator
PricingRequest::tktDesignator(const int16_t segmentOrder) const
{
  std::map<int16_t, TktDesignator>::const_iterator i = _tktDesignator.find(segmentOrder);
  if (UNLIKELY(i != _tktDesignator.end()))
    return i->second;

  return "";
}

/**
 * Check if the travel segment has specified Ticket Designator
 */
const TktDesignator
PricingRequest::specifiedTktDesignator(const int16_t segmentOrder) const
{
  std::map<int16_t, TktDesignator>::const_iterator i = _specifiedTktDesignator.find(segmentOrder);
  if (UNLIKELY(i != _specifiedTktDesignator.end()))
    return i->second;

  return "";
}

void
PricingRequest::addDiscAmount(const int16_t setNo,
                              const int16_t segmentOrder,
                              const MoneyAmount discAmount,
                              const std::string& discCurrencyCode)
{
  if (setNo > 0) // For XML2 with Discount Group Number provided
  {
    if (setNo > (int16_t) getDiscountAmounts().size()) // New set of DA
    {
      _discAmounts.resize(setNo);
    }

    DiscountAmount& curDiscAmount = _discAmounts[setNo - 1];
    curDiscAmount.amount = discAmount;
    curDiscAmount.currencyCode = discCurrencyCode;

    if (curDiscAmount.startSegmentOrder == 0 || curDiscAmount.startSegmentOrder > segmentOrder)
      curDiscAmount.startSegmentOrder = segmentOrder;

    if (curDiscAmount.endSegmentOrder == 0 || curDiscAmount.endSegmentOrder < segmentOrder)
      curDiscAmount.endSegmentOrder = segmentOrder;
  }
  else // For XML1 only
  {
    // Temp code to check if same amount to decide if same set.
    std::vector<DiscountAmount>::reverse_iterator i = _discAmounts.rbegin();
    for (; i != _discAmounts.rend(); i++)
    {
      if ((*i).amount == discAmount) // Same set
      {
        DiscountAmount& curDiscAmount = *i;

        if (curDiscAmount.startSegmentOrder == 0 || curDiscAmount.startSegmentOrder > segmentOrder)
          curDiscAmount.startSegmentOrder = segmentOrder;

        if (curDiscAmount.endSegmentOrder == 0 || curDiscAmount.endSegmentOrder < segmentOrder)
          curDiscAmount.endSegmentOrder = segmentOrder;

        break;
      }
    }

    if (i == _discAmounts.rend()) // Not found, create new one
    { // lint -e{530}
      size_t size = _discAmounts.size() + 1; // lint !e578
      _discAmounts.resize(size);

      DiscountAmount& curDiscAmount = _discAmounts[size - 1];
      curDiscAmount.amount = discAmount;
      curDiscAmount.currencyCode = discCurrencyCode;

      if (curDiscAmount.startSegmentOrder == 0 || curDiscAmount.startSegmentOrder > segmentOrder)
        curDiscAmount.startSegmentOrder = segmentOrder;

      if (curDiscAmount.endSegmentOrder == 0 || curDiscAmount.endSegmentOrder < segmentOrder)
        curDiscAmount.endSegmentOrder = segmentOrder;
    }
  }
}

void
PricingRequest::brandedFareAllBookingCode(const uint16_t index, std::vector<BookingCode>& bkg) const
{
  const std::vector<BookingCode>& fareBkg = _brandedFares.fareBookingCode(index);
  const std::vector<BookingCode>& fareSecBkg = _brandedFares.fareSecondaryBookingCode(index);
  bkg.reserve(fareBkg.size() + fareSecBkg.size());
  bkg.insert(bkg.end(), fareBkg.begin(), fareBkg.end());
  bkg.insert(bkg.end(), fareSecBkg.begin(), fareSecBkg.end());
}

bool
PricingRequest::checkSchemaVersion(short major, short minor, short revision) const
{
  if (major < majorSchemaVersion())
    return true;
  else if (major == majorSchemaVersion())
    return minor < minorSchemaVersion() ||
           (minor == minorSchemaVersion() && revision <= revisionSchemaVersion());
  else
    return false;
}

  std::string
  PricingRequest::jumpCabinLogicStatusToString(const JumpCabinLogic status)
  {
    switch (status)
    {
      case JumpCabinLogic::ENABLED:
        return "ENABLED";
      case JumpCabinLogic::ONLY_MIXED:
        return "ONLY_MIXED";
      default:
        return "DISABLED";
    }
  }

uint8_t
PricingRequest::setRCQValues(const std::string& xmlString)
{
  if (xmlString.empty())
    return 0;

  uint8_t rcqCount = 0;

  boost::char_separator<char> separator(",");
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  tokenizer tokens(xmlString, separator);
  uint16_t maxSize = prmValue() ? 1 : MAX_NUM_OF_ELEMENT_RCQ;
  std::stringstream ss;

  for (tokenizer::iterator tokenI = tokens.begin(); tokenI != tokens.end(); ++tokenI)
  {
    if (++rcqCount > maxSize)
    {
      ss <<  "MAXIMUM " << maxSize << " RETAILER RULE QUALIFIERS PERMITTED";
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, ss.str().c_str());
    }

    std::string token = ((std::string)tokenI->data());
    size_t size = token.size();

    if ((size < MIN_LENGTH_OF_RCQ) || (size > MAX_LENGTH_OF_RCQ))
    {
      ss << "FORMAT - RETAILER RULE QUALIFIER MUST BE " << PricingRequest::MIN_LENGTH_OF_RCQ
         << "-" << PricingRequest::MAX_LENGTH_OF_RCQ << " ALPHANUMERIC";
      throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, ss.str().c_str());
    }

    for (char c : token)
      if (!isalnum(c))
      {
        ss << "FORMAT - RETAILER RULE QUALIFIER MUST BE " << PricingRequest::MIN_LENGTH_OF_RCQ
           << "-" << PricingRequest::MAX_LENGTH_OF_RCQ << " ALPHANUMERIC";
        throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INPUT, ss.str().c_str());
      }

     _rcqs.push_back(token);
  }

  return rcqCount;
}

bool
PricingRequest::isMatch(const FareRetailerCode& retailerCode) const
{
  if (prmValue() && !_rcqs.empty())
    return _rcqs.front() == retailerCode;

  if (retailerCode.empty())
    return true;

  for(const auto& singleRC: _rcqs)
    if (singleRC == retailerCode)
      return true;

  return false;

}

bool
PricingRequest::isNSPCxr(const CarrierCode& mktCxr) const
{
  if(!(spvInd() == tse::spValidator::noSMV_noIEV ||
       spvInd() == tse::spValidator::noSMV_IEV))
    return false;
  if(spvCxrsCode().empty())
    return true;
  else if(std::find(spvCxrsCode().begin(), spvCxrsCode().end(), mktCxr) != spvCxrsCode().end())
    return true;
  else
    return false;

}

} // tse
