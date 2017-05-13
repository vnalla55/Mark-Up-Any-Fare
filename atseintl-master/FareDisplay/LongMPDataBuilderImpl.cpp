#include "FareDisplay/LongMPDataBuilderImpl.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/MPData.h"

#include <algorithm>
#include <iterator>
#include <set>

using std::set;
using std::bind2nd;
using std::not_equal_to;
using std::vector;

namespace tse
{
static Logger
logger("atseintl.FareDisplay.LongMPDataBuilderImpl");

namespace
{

struct GDExtractor
{
  GlobalDirection operator()(const PaxTypeFare* fare) const
  {
    GlobalDirection gd(GlobalDirection::ZZ);
    if (fare != nullptr)
      gd = fare->globalDirection();
    return gd;
  }
} gdExtractor;

bool
isValidGD(const set<GlobalDirection>& uniqueGD)
{
  if (uniqueGD.empty())
    return false;
  return find_if(uniqueGD.begin(),
                 uniqueGD.end(),
                 bind2nd(not_equal_to<GlobalDirection>(), GlobalDirection::ZZ)) != uniqueGD.end();
}

class GDAdder
{
public:
  GDAdder(MPData& mpData) : mpData_(mpData) {}
  void operator()(GlobalDirection gd) { mpData_.setMPM(gd, 0); }

private:
  MPData& mpData_;
};
}

MPData*
LongMPDataBuilderImpl::buildMPData(FareDisplayTrx& trx) const
{
  MPData* mpData(nullptr);
  trx.dataHandle().get(mpData);
  mpData->initialize(trx.boardMultiCity(),
                     trx.offMultiCity(),
                     trx.getRequest()->calcFareAmount(),
                     trx.getOptions()->currencyOverride(),
                     trx.travelDate(),
                     trx.ticketingDate());
  if (trx.getRequest()->globalDirection() != GlobalDirection::ZZ)
    mpData->setMPM(trx.getRequest()->globalDirection(), 0);
  else
  {
    set<GlobalDirection> uniqueGD;
    const vector<PaxTypeFare*>& fares(trx.allPaxTypeFare());
    if (!fares.empty())
    {
      transform(fares.begin(), fares.end(), inserter(uniqueGD, uniqueGD.end()), gdExtractor);
      if (isValidGD(uniqueGD))
        for_each(uniqueGD.begin(), uniqueGD.end(), GDAdder(*mpData));
      else
      {
        LOG4CXX_ERROR(logger, "No valid global direction in paxTypeFares");
        trx.errorResponse() << "NO MPM TO DISPLAY" << std::endl;
      }
    }
    else
    {
      trx.errorResponse() << "NO FARES FOR CARRIER " << std::endl;
      LOG4CXX_ERROR(logger, "allPaxTypeFare vector empty");
    }
  }
  return mpData;
}
}
