//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Pricing/Shopping/Utils/SopDistributor.h"

#include "Common/Assert.h"
#include "Common/Logger.h"
#include "DataModel/ShoppingTrx.h"
#include "Pricing/Shopping/Utils/ShoppingUtils.h"

#include <sstream>

namespace tse
{

namespace utils
{

namespace
{
Logger
logger("atseintl.Pricing.ShoppingUtils.SopDistributor");
}

void
SopDistributor::addCollector(ICollector<SopCandidate>* collector)
{
  TSE_ASSERT(collector != nullptr);
  _registeredCollectors.push_back(collector);
}

void
SopDistributor::distributeSops(const ShoppingTrx& trx) const
{
  LOG4CXX_DEBUG(logger,
                "Starting to distribute with " << _registeredCollectors.size()
                                               << " registered collectors");
  TSE_ASSERT(_registeredCollectors.size() > 0);

  SopCandidate currentInfo;

  // Iterate over legs
  const std::vector<ShoppingTrx::Leg>& legs = trx.legs();
  for (unsigned int i = 0; i < legs.size(); ++i)
  {
    const ShoppingTrx::Leg& currentLeg = legs[i];

    // Iterate over sops in single leg
    // Warning: we only iterate over first
    // currentLeg.requestSops() sops which is
    // the collection of sops originally contained
    // in the request
    for (unsigned int j = 0; j < currentLeg.requestSops(); ++j)
    {
      const ShoppingTrx::SchedulingOption& aSop = currentLeg.sop()[j];

      // Use index in the sop() vector as sopId
      // - this is the way PO understands SOP ids
      // during FOS construction.
      //
      // aSop.sopId() may return zero here because the
      // sop Id field may be not initialized
      currentInfo.sopId = j;
      currentInfo.legId = i;
      currentInfo.carrierCode = aSop.governingCarrier();
      currentInfo.isFlightDirect = isSopDirect(aSop);

      if (IS_DEBUG_ENABLED(logger))
      {
        std::ostringstream out;
        out << "New SOP candidate " << currentInfo;
        out << " (" << aSop << ")";
        LOG4CXX_DEBUG(logger, out.str());
      }

      callCollectorsWithSopCandidate(currentInfo);
    }
  }
}

void
SopDistributor::callCollectorsWithSopCandidate(const SopCandidate& candidate) const
{
  for (auto& elem : _registeredCollectors)
  {
    elem->collect(candidate);
  }
}

} // namespace utils

} // namespace tse
