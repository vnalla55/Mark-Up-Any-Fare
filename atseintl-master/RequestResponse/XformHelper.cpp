/**
 *  Copyright Sabre 2005
 *
 *          The copyright to the computer program(s) herein
 *          is the property of Sabre.
 *          The program(s) may be used and/or copied only with
 *          the written permission of Sabre or in accordance
 *          with the terms and conditions stipulated in the
 *          agreement/contract under which the program(s)
 *          have been supplied.
 *
 */

#include "RequestResponse/XformHelper.h"

#include "Common/Config/ConfigMan.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Xform/XformClientXML.h"

#include <fstream>
#include <string>

namespace tse
{
std::string
createResponse(PricingTrx& trx, ConfigMan& config)
{
  XformClientXML xform("Xform internal", config);
  std::string response;
  xform.convert(trx, response);
  return response;
}
}
