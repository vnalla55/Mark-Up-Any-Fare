#include "FareDisplay/MileageAdapterImpl.h"

#include "DBAccess/DataHandle.h"
#include "FareDisplay/MPData.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MPMRetriever.h"
#include "Routing/Retriever.h"

namespace tse
{
namespace
{

class MileageGetter
{
public:
  MileageGetter(MPData& mpData, DataHandle& dh)
    : mpData_(mpData),
      dataHandle_(dh),
      mpmRetriever_(tse::Singleton<Retriever<MPMRetriever> >::instance())
  {
    dataHandle_.get(item_.city1());
    dataHandle_.get(item_.city2());
    if (const Loc* loc = dataHandle_.getLoc(mpData_.getBoardCity(), mpData_.getTravelDate()))
      *item_.city1() = *loc;
    if (const Loc* loc = dataHandle_.getLoc(mpData_.getOffCity(), mpData_.getTravelDate()))
      *item_.city2() = *loc;
    item_.travelDate() = mpData_.getTravelDate();
  }
  void operator()(GlobalDirection gd)
  {
    item_.mpmGlobalDirection() = gd;
    mpmRetriever_.retrieve(item_, dataHandle_);
    mpData_.setMPM(gd, item_.mpm());
  }

private:
  MPData& mpData_;
  DataHandle& dataHandle_;
  const Retriever<MPMRetriever>& mpmRetriever_;
  MileageRouteItem item_;
};
}

bool
MileageAdapterImpl::getMPMforGD(MPData& mpData) const
{
  if (mpData.getGlobals().empty())
    return false;
  DataHandle dh(mpData.getTicketingDate());
  const GlobalVec& globals(mpData.getGlobals());
  for_each(globals.begin(), globals.end(), MileageGetter(mpData, dh));
  return true;
}
}
