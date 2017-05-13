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
#pragma once

#include <boost/ptr_container/ptr_vector.hpp>

#include "DataModel/Services/SectorDetail.h"
#include "DataModel/Common/Types.h"
#include "ServiceInterfaces/SectorDetailService.h"

#include <memory>

namespace tax
{
class SectorDetailServiceServer : public SectorDetailService
{
public:
  SectorDetailServiceServer() {}
  ~SectorDetailServiceServer() {}

  std::shared_ptr<const SectorDetail>
  getSectorDetail(const type::Vendor& vendor, type::Index itemNo) const final;

  boost::ptr_vector<SectorDetail>& sectorDetail() { return _sectorDetail; }

  const boost::ptr_vector<SectorDetail>& sectorDetail() const { return _sectorDetail; };

private:
  boost::ptr_vector<SectorDetail> _sectorDetail;
};
}
