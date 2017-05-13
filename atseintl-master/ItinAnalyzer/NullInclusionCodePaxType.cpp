//----------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "ItinAnalyzer/NullInclusionCodePaxType.h"

#include "Common/NonFatalErrorResponseException.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{
static Logger
logger("atseintl.ItinAnalyzerService.InclusionCodePaxType");

void
NullInclusionCodePaxType::getPaxType(FareDisplayTrx& trx)
{
  LOG4CXX_ERROR(logger, " Invalid Inclusion Code Requested " << trx.getRequest()->inclusionCode());
  throw NonFatalErrorResponseException(ErrorResponseException::INVALID_INCLUSION_CODE);
}
}
