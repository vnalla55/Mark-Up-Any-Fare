
// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2008
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

#include "Taxes/Pfc/PfcDisplayData.h"
#include "Common/TseConsts.h"
#include "DataModel/Agent.h"
#include "DBAccess/Customer.h"

#include <vector>

using namespace tse;

const std::string PfcDisplayData::AXESS_PREFIX = "VD ";

const std::string PfcDisplayData::HDQ = "HDQ";

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayData::PfcDisplayData
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayData::PfcDisplayData(TaxTrx* trx, PfcDisplayDb* db) : _trx(trx), _db(db) {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayData::PfcDisplayData
//
// Description:  Destructor
//
// </PRE>
// ----------------------------------------------------------------------------
PfcDisplayData::~PfcDisplayData() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function PfcDisplayData::getAxesSPrefix
//
// Description:  Get AXESS Prefix.
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
PfcDisplayData::getAxessPrefix() const
{
  std::string prefix;

  const Agent* agent = trx()->pfcDisplayRequest()->ticketingAgent();

  if (agent->mainTvlAgencyPCC().empty() || agent->mainTvlAgencyPCC() == HDQ)
  {
    return prefix;
  }

  const Customer* customer = db()->getCustomer(agent->mainTvlAgencyPCC());

  if (!customer)
  {
    return prefix;
  }

  if (customer->crsCarrier() == AXESS_MULTIHOST_ID)
  {
    prefix = AXESS_PREFIX + "\n";
  }

  return prefix;
}
