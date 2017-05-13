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
 *
 *  Due to specific way of creating a service (functions with C linkage)
 *   it is impossible to include a service's header file in scope
 *   where header of another service is inluded. Thus it was necessary to
 *   create a helper file where XformClientXML.h was safe to include (it
 *   does not conflict with RequestResponce.h).
 */

#pragma once

#include <string>

namespace tse
{
class PricingTrx;
class ConfigMan;

std::string
createResponse(PricingTrx& trx, ConfigMan& config);

} // namespace tse;

