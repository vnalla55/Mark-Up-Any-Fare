#include "Common/FxCnException.h"

#include "Common/FallbackUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"

#include <boost/regex.hpp>

namespace tse
{
/**
 * Check if FareX Chinese Exception is applicable
 *
 * Input : PricingTrx
 * Output: return true to indicate exception applies
 *         false otherwise
 **/
bool
FxCnException::
operator()()
{
  static CarrierCode Carriers[] = { "CX", "GE", "KA", "NX" };

  // Doesn't apply if ticketing (validating) carrier is not one of the
  // participating carriers
  CarrierCode cxr = _itin.validatingCarrier();

  if (cxr.empty() || !TseUtil::isMember(cxr, Carriers))
    return false;

  // Doesn't apply if not FareX or Abacus
  if (!_trx.getOptions()->fareX() && !_trx.getRequest()->ticketingAgent()->abacusUser())
  {
    return false;
  }

  // Check fare basis - there should be only one
  if (!checkSingleFareBasis(_trx))
    return false;

  if (!checkThruMarket(_itin.travelSeg()))
    return false;

  return true;
}

bool
FxCnException::checkThruMarket(const std::vector<TravelSeg*>& tvlSeg)
{
  // Doesn't apply if originate from MFM, HKG, or in China
  const TravelSeg* ts = TravelSegUtil::firstAirSeg(tvlSeg);
  if (!ts || ts->origin()->nation().equalToConst("CN") || ts->origin()->loc() == "MFM" ||
      ts->origin()->loc() == "HKG")
  {
    return false;
  }

  CarrierCode cxr = _itin.validatingCarrier();

  // Doesn't apply if does not travel via "MFM" or "HKG" to a point in CN
  std::vector<TravelSeg*>::const_iterator tsI = std::find(tvlSeg.begin(), tvlSeg.end(), ts);
  if (tsI == tvlSeg.end())
    return false;
  for (++tsI; tsI != tvlSeg.end(); ++tsI)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(*tsI);
    if (!airSeg)
      continue;

    if ((airSeg->origin()->loc() == "MFM" && (cxr.equalToConst("GE") || cxr.equalToConst("NX"))) ||
        (airSeg->origin()->loc() == "HKG" && (cxr.equalToConst("KA") || cxr.equalToConst("CX"))))
      break;

  }
  if (tsI == tvlSeg.end())
    return false;

  for (; tsI != tvlSeg.end(); ++tsI)
  {
    if ((*tsI)->destination()->nation().equalToConst("CN"))
      break;
  }
  if (tsI == tvlSeg.end())
    return false;

  return true;
}

/**
 * Check if there is a valid thru fare in the CN exception market
 * which could be used for FareX or Abacus command pricing
 *
 * @return true if thru fare does not exist, false otherwise.
 **/
bool
FxCnException::checkThruFare()
{
  for (std::vector<FareMarket*>::const_iterator i = _itin.fareMarket().begin(),
                                                iend = _itin.fareMarket().end();
       i != iend;
       ++i)
  {
    const FareMarket& fm = **i;
    if (!checkThruMarket(fm.travelSeg()))
      continue;

    const std::vector<PaxTypeBucket>& ptCortegeVect = fm.paxTypeCortege();

    if (fm.failCode() != ErrorResponseException::NO_ERROR || ptCortegeVect.empty())
    {
      return true;
    }

    for (std::vector<PaxTypeBucket>::const_iterator j = ptCortegeVect.begin(),
                                                    jend = ptCortegeVect.end();
         j != jend;
         ++j)
    {
      const PaxTypeBucket& ptc = (*j);

      for (std::vector<PaxTypeFare*>::const_iterator k = ptc.paxTypeFare().begin(),
                                                     kend = ptc.paxTypeFare().end();
           k != kend;
           ++k)
      {
        // check if the fare is published and valid for (normal) command pricing.
        if ((*k)->isPublished() && (*k)->validForCmdPricing(false))
        {
          return false;
        }
      }
    }
  }

  return true;
}

unsigned int
FxCnException::countPatternMatch(const char* str, const boost::regex& regex) const
{
  unsigned int count = 0;
  boost::cmatch m;
  const char* start = str;
  while (start && boost::regex_search(start, m, regex))
  {
    count++;
    start = m[0].second;
  }
  return count;
}

bool
FxCnException::checkSingleFareBasis(const PricingTrx& trx) const
{
  const boost::regex fbRegex("S\\d+((-\\d+)|(/\\d+)+)?\\*Q\\w+");

  std::string entry = trx.getOptions()->lineEntry();
  int qCount = countPatternMatch(entry.data(), fbRegex);

  if (qCount == 1)
    return true;
  else if (qCount > 1)
    return false;

  const std::vector<TravelSeg*>& tvlSeg = _itin.travelSeg();
  const TravelSeg* ts = TravelSegUtil::firstAirSeg(tvlSeg);

  // Check fare basis - there should be only one
  if (!ts || ts->fareBasisCode().empty())
    return false;

  for (std::vector<TravelSeg*>::const_iterator i = tvlSeg.begin(), iend = tvlSeg.end(); i != iend;
       ++i)
  {
    AirSeg* as = dynamic_cast<AirSeg*>(*i);
    if (!as)
    {
      continue;
    }
    if (ts->fareBasisCode() != (*i)->fareBasisCode())
      return false;
  }
  return true;
}
}
