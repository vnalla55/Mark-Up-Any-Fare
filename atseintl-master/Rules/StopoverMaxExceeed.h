#pragma once

#include "Rules/StopoversTravelSegWrapper.h"

namespace tse
{
class PricingUnit;
class PricingTrx;
class StopoversInfo;
class FareUsage;

class StopoverMaxExceeed
{
public:
  StopoverMaxExceeed()
    : _isRtw(false),
      _numStopsMaxUnlimited(false),
      _isInStopsUnlimited(false),
      _isOutStopsUnlimited(false),
      _numStopsMax(0),
      _numStopsInMax(0),
      _numStopsOutMax(0) {}

  void reset();
  void sumNumStopoversMax(const uint32_t relationalInd, const StopoversInfo* soInfo);

  bool newCheckMaxExceed(const PricingUnit& pu,
                         PricingTrx& trx,
                         StopoversTravelSegWrapperMap& stopoversTravelSegWrappers,
                         const FareUsage& fu);

  void setRtw(bool rtw) { _isRtw = rtw; }

private:
  bool checkMaxExceedAndNotMatched(const PricingUnit& pu,
                                   PricingTrx& trx,
                                   bool& nonMatched,
                                   int16_t& stopCount,
                                   int16_t& matchCount,
                                   StopoversTravelSegWrapperMap& stopoversTravelSegWrappers);

  bool checkFinalMaxExceed(const PricingUnit& pu,
                           PricingTrx& trx,
                           const int16_t& stopCount,
                           const int16_t& matchCount,
                           StopoversTravelSegWrapperMap& stopoversTravelSegWrappers,
                           const FareUsage& checkingFareUsage);

  void
  getTotalFareComponentStopovers(const PricingUnit& pu,
                                 PricingTrx& trx,
                                 uint16_t& inboundStopCount,
                                 uint16_t& inboundMatchCount,
                                 uint16_t& outboundStopCount,
                                 uint16_t& outboundMatchCount,
                                 StopoversTravelSegWrapperMap& stopoversTravelSegWrappers) const;

  void setOutboundMaxExceed(const PricingUnit& pu,
                            int16_t numOutNotMatched,
                            StopoversTravelSegWrapperMap& stopoversTravelSegWrappers) const;
  void setTotalMaxExceed(const PricingUnit& pu,
                         const int16_t matchCount,
                         StopoversTravelSegWrapperMap& stopoversTravelSegWrappers) const;
  void setInboundMaxExceed(const PricingUnit& pu,
                           int16_t numInNotMatched,
                           StopoversTravelSegWrapperMap& stopoversTravelSegWrappers) const;
  void setMaxExceed(const FareUsage* fareUsage,
                    StopoversTravelSegWrapperMap& stopoversTravelSegWrappers,
                    int16_t& numOutNotMatched,
                    const int16_t& val) const;

private:
  bool _isRtw;
  bool _numStopsMaxUnlimited;
  bool _isInStopsUnlimited;
  bool _isOutStopsUnlimited;
  int16_t _numStopsMax;
  int16_t _numStopsInMax;
  int16_t _numStopsOutMax;
};
}

