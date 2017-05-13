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
#include "Common/SafeEnumToString.h"
#include "Rules/RuleDescriptionFormater.h"
#include "DataModel/Services/SectorDetail.h"
#include "DataModel/Services/ServiceBaggage.h"
#include "DataModel/Common/CodeIO.h"

#include <boost/format.hpp>
#include <sstream>

namespace tax
{
void
RuleDescriptionFormater::format(std::ostringstream& buf,
                                std::shared_ptr<SectorDetail const> sectorDetail)
{
  if (sectorDetail != nullptr)
  {
    if (sectorDetail->entries.size() != 0)
    {
      buf << " CONTENT:\n";
      boost::format formatter(" %|=4| %|=3| %|=14| %|=5| %|=4| %|=8| %|=4| %|=4| %|=4| %|=6|"
                              " %|=9| %|=17| %|=7|\n");
      buf << formatter % "APPL" % "EQP" % "FAREOWNCARRIER" % "CABIN" % "RULE" % "FARETYPE" %
                 "RBD1" % "RBD2" % "RBD3" % "TARIFF" % "TARIFFIND" % "BASISTKTDESIGNATOR" %
                 "TKTCODE";
      for (const SectorDetailEntry& entry: sectorDetail->entries)
      {
        buf << formatter % entry.applTag % entry.equipmentCode % entry.fareOwnerCarrier %
                   entry.cabinCode % entry.rule % entry.fareType % entry.reservationCodes[0] %
                   entry.reservationCodes[1] % entry.reservationCodes[2] % entry.tariff.asNumber() %
                   entry.tariff.asCode() % entry.fareBasisTktDesignator % entry.ticketCode;
      }
    }
    else
    {
      buf << " IS EMPTY\n";
    }
  }
  else // sectorDetail == 0
  {
    buf << " DOES NOT EXIST\n";
  }
}

void
RuleDescriptionFormater::format(std::ostringstream& buf,
                                std::shared_ptr<ServiceBaggage const> serviceBaggage)
{
  if (!serviceBaggage)
  {
    buf << " DOES NOT EXIST";
    return;
  }
  if (serviceBaggage->entries.size() != 0)
  {
    buf << " CONTENT:\n";
    boost::format formatter(" %|=7| %|=2| %|=3| %|=7| %|=5| %|=8| %|=11|\n");
    buf << formatter % "APPLTAG" % "TC" % "TT" % "SVCTYPE" % "GROUP" % "SUBGROUP" % "FEEOWNERCXR";
    for (const ServiceBaggageEntry& entry: serviceBaggage->entries)
    {
      buf << formatter % entry.applTag % entry.taxCode % entry.taxTypeSubcode %
                 entry.optionalServiceTag % entry.group % entry.subGroup % entry.feeOwnerCarrier;
    }
  }
  else // entries.size() == 0
  {
    buf << " IS EMPTY";
  }
}

} // namespace tax
