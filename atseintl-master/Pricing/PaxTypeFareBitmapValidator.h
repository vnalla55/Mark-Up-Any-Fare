// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "BookingCode/BCETuning.h"
#include "DataModel/ShoppingTrx.h"

#include <vector>

namespace tse
{

class BitmapOpOrderer;
class PaxTypeFare;
class FareUsageMatrixMap;

class PaxTypeFareBitmapValidator
{
  friend class PaxTypeFareBitmapValidatorTest;

public:
  class SkippedBitValidator
  {
  public:
    SkippedBitValidator() = default;

    SkippedBitValidator(const SkippedBitValidator&) = delete;
    void operator=(const SkippedBitValidator&) = delete;

    virtual ~SkippedBitValidator() = default;

    virtual void operator()(ItinIndex::ItinIndexIterator& itinIt,
                            PaxTypeFare* ptFare,
                            ShoppingTrx::Leg& leg,
                            uint32_t beginLeg,
                            uint32_t endLeg,
                            uint32_t bit,
                            std::vector<BCETuning>& bceTuning,
                            const DatePair* dates,
                            const ItinIndex::Key carrierKey) = 0;

    virtual bool foundHighestFarePath() const = 0;
    virtual uint32_t maxFlightsForRuleValidation() const = 0;
    virtual uint32_t altDateMaxPassedBit() const = 0;

  protected:
    bool isAltDateFltBitFailedBKCRTGGLB(PaxTypeFare* ptFare,
                                        const int bit,
                                        bool& bkcProcessed,
                                        bool& rtgProcessed,
                                        bool& glbProcessed);

  };

  PaxTypeFareBitmapValidator(ShoppingTrx& trx,
                             PaxTypeFare& paxTypeFare,
                             const DatePair* dates,
                             SkippedBitValidator* skippedBitValidator,
                             FareUsageMatrixMap* fareUsageMatrixMap);

  PaxTypeFareBitmapValidator(const PaxTypeFareBitmapValidator&) = delete;
  void operator=(const PaxTypeFareBitmapValidator&) = delete;

  bool validate(bool considerNonStopsOnly);

  bool validate(const uint32_t bit, const ItinIndex::Key& key);

  static SkippedBitValidator*
  createSoloSkippedBitValidator(ShoppingTrx* trx,
                                BitmapOpOrderer* bOrder,
                                const std::vector<uint32_t>& cxrKeyVector);

private:
  bool isNonStopItin(const ItinIndex::ItinIndexIterator& itinIt) const;
  bool validate(ItinIndex::ItinIndexIterator& itinIt, const ItinIndex::Key& carrierKey);
  bool validate(ItinIndex::Key key, bool considerNonStopsOnly);
  bool validate(CarrierCode governingCarrier, bool considerNonStopsOnly);

  ShoppingTrx& _trx;
  PaxTypeFare& _ptFare;
  const int _legIndex;
  ShoppingTrx::Leg& _leg;
  const bool _acrossStopOver;
  const int _startIndex;
  const int _endIndex;
  ShoppingTrx::FareMarketRuleMap::iterator _fmrmIT;
  std::vector<std::vector<BCETuning> > _defaultSBCE; // use default tuning block if it is not in the
                                                     // map
  std::vector<std::vector<BCETuning> >& _shoppingBCETuningData;
  uint32_t _bitCount = 0;
  bool _atLeastOneSopValid = false;
  const DatePair* _dates;
  bool _variabledDuration;
  uint64_t _duration;
  SkippedBitValidator* _skippedBitValidator;
  FareUsageMatrixMap* _fareUsageMatrixMap;
};
}
