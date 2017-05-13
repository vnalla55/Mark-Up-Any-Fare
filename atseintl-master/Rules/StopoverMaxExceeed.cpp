#include "Rules/StopoverMaxExceeed.h"
#include "Common/FallbackUtil.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/StopoversInfo.h"
#include "Rules/RuleUtil.h"
#include "Rules/Stopovers.h"
#include "Rules/StopoversInfoWrapper.h"

using std::string;

namespace tse
{
void
StopoverMaxExceeed::reset()
{
  _isRtw = false;
  _numStopsMax = 0;
  _numStopsMaxUnlimited = false;
  _numStopsInMax = 0;
  _numStopsOutMax = 0;
  _isInStopsUnlimited = false;
  _isOutStopsUnlimited = false;
}

bool
StopoverMaxExceeed::newCheckMaxExceed(const PricingUnit& pu,
                                      PricingTrx& trx,
                                      StopoversTravelSegWrapperMap& stopoversTravelSegWrappers,
                                      const FareUsage& fu)
{
  bool anyMaxExceed = false;
  bool notMatched = false;
  int16_t stopCount = 0;
  int16_t matchCount = 0;

  anyMaxExceed = checkMaxExceedAndNotMatched(
      pu, trx, notMatched, stopCount, matchCount, stopoversTravelSegWrappers);
  if (!anyMaxExceed && notMatched)
  {
    anyMaxExceed = checkFinalMaxExceed(pu, trx, stopCount, matchCount, stopoversTravelSegWrappers,
                                       fu);
  }

  return anyMaxExceed;
}

/*
 * check for notMatched stopover segment and finds stopover count
 * return true if max exceeded
 */
bool
StopoverMaxExceeed::checkMaxExceedAndNotMatched(
    const PricingUnit& pu,
    PricingTrx& trx,
    bool& notMatched,
    int16_t& stopCount,
    int16_t& matchCount,
    StopoversTravelSegWrapperMap& stopoversTravelSegWrappers)
{
  for (const FareUsage* fareUsage : pu.fareUsage())
  {
    for (const TravelSeg* ts: fareUsage->travelSeg())
    {
      StopoversTravelSegWrapperMapCI stswIter = stopoversTravelSegWrappers.find(ts);
      if (stswIter != stopoversTravelSegWrappers.end())
      {
        const StopoversTravelSegWrapper& stsw = (*stswIter).second;
        if (!stsw.passedValidation() && stsw.maxSOExceeded() > 0)
        {
          return true;
        }

        if (stsw.isMatched())
        {
          matchCount++;
        }
        else if (stsw.isNotMatched())
        {
          notMatched = true;
        }

        if (stsw.isMatched() || stsw.isNotMatched())
        {
          stopCount++;
        }
      }
    }
  }
  return false;
}

bool
StopoverMaxExceeed::checkFinalMaxExceed(const PricingUnit& pu,
                                        PricingTrx& trx,
                                        const int16_t& stopCount,
                                        const int16_t& matchCount,
                                        StopoversTravelSegWrapperMap& stopoversTravelSegWrappers,
                                        const FareUsage& checkingFareUsage)
{
  if (_numStopsMaxUnlimited)
    return false;

  bool anyMaxExceed = false;
  if (stopCount > _numStopsMax)
  {
    anyMaxExceed = true;
    setTotalMaxExceed(pu, matchCount, stopoversTravelSegWrappers);
    return anyMaxExceed;
  }

  uint16_t totalOutSO, totalInSO, inboundMatchCount, outboundMatchCount;
  totalOutSO = totalInSO = inboundMatchCount = outboundMatchCount = 0;

  getTotalFareComponentStopovers(pu,
                                 trx,
                                 totalInSO,
                                 inboundMatchCount,
                                 totalOutSO,
                                 outboundMatchCount,
                                 stopoversTravelSegWrappers);


  //APO36029: Cumulative inbound and outbound  stopover counts should be validated only at FC level.
  //validate outbound counts when checking fareusage is outbound,dothe samewhen checking fu is inbound
  if ( &pu)
  {
     if (checkingFareUsage.isOutbound())
     {
        if (!_isOutStopsUnlimited && totalOutSO > _numStopsOutMax)
        {
           anyMaxExceed = true;
           setOutboundMaxExceed(pu, _numStopsOutMax - outboundMatchCount,
                                                 stopoversTravelSegWrappers);
           return anyMaxExceed;
        }
     }
     if (checkingFareUsage.isInbound())
     {
        if (!_isInStopsUnlimited && totalInSO > _numStopsInMax)
        {
           anyMaxExceed = true;
           setInboundMaxExceed(pu, _numStopsInMax - inboundMatchCount, stopoversTravelSegWrappers);
           return anyMaxExceed;
        }
     }
     return anyMaxExceed;
  }
  else  //fallback
  {
    if (!_isOutStopsUnlimited && totalOutSO > _numStopsOutMax)
    {
       anyMaxExceed = true;
       setOutboundMaxExceed(pu, _numStopsOutMax - outboundMatchCount, stopoversTravelSegWrappers);
       return anyMaxExceed;
    }

    if (!_isInStopsUnlimited && totalInSO > _numStopsInMax)
    {
       anyMaxExceed = true;
       setInboundMaxExceed(pu, _numStopsInMax - inboundMatchCount, stopoversTravelSegWrappers);
       return anyMaxExceed;
    }
  return anyMaxExceed;
  }
}

void
StopoverMaxExceeed::setTotalMaxExceed(const PricingUnit& pu,
                                      const int16_t matchCount,
                                      StopoversTravelSegWrapperMap& stopoversTravelSegWrappers)
    const
{
  int16_t numNotMatched = _numStopsMax - matchCount;

  std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();
  for (; fuI != fuEnd; ++fuI)
  {
    const FareUsage* fareUsage = *fuI;
    setMaxExceed(fareUsage,
                 stopoversTravelSegWrappers,
                 numNotMatched,
                 StopoversInfoWrapper::TOTAL_MAX_EXECEED);
  }
}

// Can we just loop through StopoversTravelSegWrapperMap and cound all stopovers limit?
void
StopoverMaxExceeed::getTotalFareComponentStopovers(
    const PricingUnit& pu,
    PricingTrx& trx,
    uint16_t& totalInSO,
    uint16_t& inboundMatchCount,
    uint16_t& totalOutSO,
    uint16_t& outboundMatchCount,
    StopoversTravelSegWrapperMap& stopoversTravelSegWrappers) const
{
  std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();

  for (; fuI != fuEnd; ++fuI)
  {
    const FareUsage* fareUsage = *fuI;

    std::vector<TravelSeg*>::const_iterator tsI = fareUsage->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tsEnd = fareUsage->travelSeg().end();

    for (; tsI != tsEnd; ++tsI)
    {
      const TravelSeg* ts = *tsI;
      StopoversTravelSegWrapperMapCI stswIter = stopoversTravelSegWrappers.find(ts);
      if (stswIter != stopoversTravelSegWrappers.end())
      {
        const StopoversTravelSegWrapper& stsw = (*stswIter).second;
        if (stsw.isMatched() || stsw.isNotMatched())
        {
          if (fareUsage->isInbound())
          {
            ++totalInSO;
          }
          else
          {
            ++totalOutSO;
          }
        }

        if (stsw.isMatched())
        {
          if (fareUsage->isInbound())
          {
            ++inboundMatchCount;
          }
          else
          {
            ++outboundMatchCount;
          }
        }
      }
    }
  }
  return;
}

void
StopoverMaxExceeed::setMaxExceed(const FareUsage* fareUsage,
                                 StopoversTravelSegWrapperMap& stopoversTravelSegWrappers,
                                 int16_t& numOutNotMatched,
                                 const int16_t& val) const
{
  std::vector<TravelSeg*>::const_iterator tsI = fareUsage->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tsEnd = fareUsage->travelSeg().end();
  for (; tsI != tsEnd; ++tsI)
  {
    const TravelSeg* ts = *tsI;
    StopoversTravelSegWrapperMapI stswIter = stopoversTravelSegWrappers.find(ts);
    if (stswIter != stopoversTravelSegWrappers.end())
    {
      StopoversTravelSegWrapper& stsw = (*stswIter).second;
      if (stsw.isNotMatched())
      {
        if (numOutNotMatched > 0)
          numOutNotMatched--;
        else if (numOutNotMatched == 0)
          stsw.maxSOExceeded() = val;
      }
    }
  }
}

void
StopoverMaxExceeed::setOutboundMaxExceed(const PricingUnit& pu,
                                         int16_t numOutNotMatched,
                                         StopoversTravelSegWrapperMap& stopoversTravelSegWrappers)
    const
{
  std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();
  for (; fuI != fuEnd; ++fuI)
  {
    const FareUsage* fareUsage = *fuI;
    if (fareUsage->isInbound())
      continue;
    setMaxExceed(fareUsage,
                 stopoversTravelSegWrappers,
                 numOutNotMatched,
                 StopoversInfoWrapper::OUT_MAX_EXECEED);
  }
}

void
StopoverMaxExceeed::setInboundMaxExceed(const PricingUnit& pu,
                                        int16_t numInNotMatched,
                                        StopoversTravelSegWrapperMap& stopoversTravelSegWrappers)
    const
{
  std::vector<FareUsage*>::const_iterator fuI = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fuEnd = pu.fareUsage().end();
  for (; fuI != fuEnd; ++fuI)
  {
    const FareUsage* fareUsage = *fuI;
    if (!fareUsage->isInbound())
      continue;

    setMaxExceed(fareUsage,
                 stopoversTravelSegWrappers,
                 numInNotMatched,
                 StopoversInfoWrapper::IN_MAX_EXECEED);
  }
}

void
StopoverMaxExceeed::sumNumStopoversMax(const uint32_t relationalInd, const StopoversInfo* soInfo)
{
  int16_t numStopsMax = 0;
  int16_t numStopsInMax = 0;
  int16_t numStopsOutMax = 0;
  const std::string& noStopsMax = soInfo->noStopsMax();
  const std::string& noStopsOutbound = soInfo->noStopsOutbound();
  const std::string& noStopsInbound = soInfo->noStopsInbound();

  if (UNLIKELY(_isRtw))
  {
    if (!noStopsMax.empty())
    {
      _isOutStopsUnlimited = true;
      _isInStopsUnlimited = true;
    }
    else if (!noStopsOutbound.empty() || !noStopsInbound.empty())
      return;
  }

  if (!_numStopsMaxUnlimited && (noStopsMax == Stopovers::NUM_STOPS_UNLIMITED))
  {
    _numStopsMaxUnlimited = true;
  }

  if (!_isInStopsUnlimited && (noStopsInbound == Stopovers::NUM_STOPS_UNLIMITED))
  {
    _isInStopsUnlimited = true;
  }
  else
  {
    numStopsInMax = atoi(soInfo->noStopsInbound().c_str());
  }

  if (!_isOutStopsUnlimited && (noStopsOutbound == Stopovers::NUM_STOPS_UNLIMITED))
  {
    _isOutStopsUnlimited = true;
  }
  else
  {
    numStopsOutMax = atoi(soInfo->noStopsOutbound().c_str());
  }

  if (!_numStopsMaxUnlimited)
  {
    if (!noStopsMax.empty())
    {
      numStopsMax = atoi(noStopsMax.c_str());
      if (relationalInd == CategoryRuleItemInfo::AND)
      {
        _numStopsMax += numStopsMax;
      }
      else
      {
        _numStopsMax = numStopsMax;
      }

      if (noStopsOutbound.empty())
      {
        _numStopsOutMax += numStopsMax;
      }

      if (noStopsInbound.empty())
      {
        _numStopsInMax += numStopsMax;
      }
    }
    else
    {
      if (_isInStopsUnlimited || _isOutStopsUnlimited)
      {
        _numStopsMaxUnlimited = true;
      }

      if (!_numStopsMaxUnlimited)
      {

        if (!noStopsOutbound.empty())
        {
          _numStopsMax += numStopsOutMax;
        }

        if (!noStopsInbound.empty())
        {
          _numStopsMax += numStopsInMax;
        }
      }
    }
  }

  if (!_isInStopsUnlimited)
  {
    if (!noStopsInbound.empty())
    {
      if (relationalInd == CategoryRuleItemInfo::AND)
      {
        _numStopsInMax += numStopsInMax;
      }
      else
      {
        _numStopsInMax = numStopsInMax;
      }
    }
  }

  if (!_isOutStopsUnlimited)
  {

    if (!noStopsOutbound.empty())
    {
      if (relationalInd == CategoryRuleItemInfo::AND)
      {
        _numStopsOutMax += numStopsOutMax;
      }
      else
      {
        _numStopsOutMax = numStopsOutMax;
      }
    }
  }
}
}
