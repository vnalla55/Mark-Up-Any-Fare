// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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
#include <iosfwd>
#include <memory>

namespace tax
{
class SectorDetail;
class ServiceBaggage;

class RuleDescriptionFormater
{
public:
  static void format(std::ostringstream& buf, std::shared_ptr<SectorDetail const> sectorDetail);

  static void format(std::ostringstream& buf, std::shared_ptr<ServiceBaggage const> serviceBaggage);
};
}

