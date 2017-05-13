// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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

#include <sstream>

#include "SectorDetailServiceServer.h"

namespace tax
{
typedef std::shared_ptr<const SectorDetail> SharedConstValue;

SharedConstValue
SectorDetailServiceServer::getSectorDetail(const type::Vendor& vendor, type::Index itemNo) const
{
  for (const SectorDetail& sd : _sectorDetail)
  {
    if (sd.vendor == vendor && sd.itemNo == itemNo)
    {
      return SharedConstValue(new SectorDetail(sd));
    }
  }
  return SharedConstValue();
}

} // namespace tax
