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

#include "PassengerMapperServer.h"

namespace tax
{

PassengerMapperServer::PassengerMapperServer(boost::ptr_vector<PaxTypeMapping> mappings)
{
  for(const PaxTypeMapping & mapping : mappings)
  {
    const type::Vendor vendor = mapping.vendor;
    const type::CarrierCode validatingCarrier = mapping.validatingCarrier;
    for(const PaxTypeMappingItem & item : mapping.items)
    {
      MapType::key_type key(vendor, validatingCarrier, item.paxFrom);
      _mapping[key].insert(item.paxTo);
    }
  }
}

bool PassengerMapperServer::matches(const type::Vendor& vendor,
                                    const type::CarrierCode& validatingCarrier,
                                    const type::PassengerCode& ticketPtc,
                                    const type::PassengerCode& rulePtc) const
{

  if (ticketPtc == rulePtc || rulePtc.empty())
  {
    return true;
  }

  MapType::key_type key(vendor, validatingCarrier, ticketPtc);
  MapType::const_iterator it = _mapping.find(key);
  if (it == _mapping.end())
  {
    return false;
  }

  std::set<type::PassengerCode>::const_iterator it2 = it->second.find(rulePtc);

  return it2 != it->second.end();
}

} // namespace tax
