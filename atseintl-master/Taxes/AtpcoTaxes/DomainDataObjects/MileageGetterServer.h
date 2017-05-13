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

#include "DataModel/Common/Types.h"
#include "ServiceInterfaces/MileageGetter.h"
#include "DataModel/Services/Distance.h"

namespace tax
{

class MileageGetterServer : public MileageGetter
{
public:
  type::Index const& geoPathRefId() const
  {
    return _geoPathRefId;
  };

  type::Index& geoPathRefId()
  {
    return _geoPathRefId;
  };

  boost::ptr_vector<Distance>& distances()
  {
    return _distances;
  };

  boost::ptr_vector<Distance> const& distances() const
  {
    return _distances;
  };

  type::Miles getSingleDistance(const type::Index& from, const type::Index& to) const override;
  type::GlobalDirection
  getSingleGlobalDir(const type::Index& from, const type::Index& to) const override;

private:
  type::Index _geoPathRefId;
  boost::ptr_vector<Distance> _distances;
};
}
