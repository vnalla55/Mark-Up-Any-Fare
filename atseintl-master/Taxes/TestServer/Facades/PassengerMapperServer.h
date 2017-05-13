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

#include "DataModel/Common/Types.h"
#include "DataModel/Services/PaxTypeMapping.h"
#include "ServiceInterfaces/PassengerMapper.h"
#include "ServiceInterfaces/Services.h"

#include <boost/ptr_container/ptr_vector.hpp>
#include <tuple>

#include <map>
#include <set>

namespace tax
{

class PassengerMapperServer : public PassengerMapper
{
public:
  PassengerMapperServer(boost::ptr_vector<PaxTypeMapping> mappings);
  virtual ~PassengerMapperServer()
  {
  }

  virtual bool matches(const type::Vendor& vendor, const type::CarrierCode& validatingCarrier,
                       const type::PassengerCode& ticketPtc,
                       const type::PassengerCode& rulePtc) const;

private:
  typedef std::map<std::tuple<type::Vendor, type::CarrierCode, type::PassengerCode>,
                   std::set<type::PassengerCode> > MapType;

  MapType _mapping;
};

} // namespace tax:w

