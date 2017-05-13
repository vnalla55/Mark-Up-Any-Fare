// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "DataModel/Itin.h"
#include "DataModel/TravelSeg.h"
#include "Taxes/LegacyFacades/TaxPointUtil.h"

namespace tse
{

void
TaxPointUtil::setTravelSegIndices(
    tax::type::Index taxPointBegin,
    tax::type::Index taxPointEnd,
    const tse::Itin& itin,
    uint16_t& travelSegStartIndex,
    uint16_t& travelSegEndIndex)
{
  if (taxPointBegin > taxPointEnd)
    std::swap(taxPointBegin, taxPointEnd);

  tax::type::Index taxPointIndex = 0;
  for (uint16_t i = 0; i < itin.travelSeg().size(); ++i)
  {
    if(itin.travelSeg()[i]->isArunk())
    {
      continue;
    }

    if (taxPointIndex == taxPointBegin)
      travelSegStartIndex = i;

    taxPointIndex += itin.travelSeg()[i]->hiddenStops().size() * 2 + 1;

    if (taxPointIndex == taxPointEnd)
      travelSegEndIndex = i;

    ++taxPointIndex;
  }
}

}
