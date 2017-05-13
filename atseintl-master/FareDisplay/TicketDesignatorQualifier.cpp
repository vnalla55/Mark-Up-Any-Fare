//----------------------------------------------------------------------------
//
//  File:           TicketDesignatorQualifier.cpp
//
//  Description:    Qualifies PaxTypeFare.
//
//  Updates:
//
//  Copyright Sabre 2006
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#include "FareDisplay/TicketDesignatorQualifier.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/TSELatencyData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareByRuleApp.h"
#include "Rules/RuleConst.h"

namespace tse
{
namespace
{
ConfigurableValue<bool>
removeTicketDesignatorCheck("FAREDISPLAY_SVC", "REMOVE_TKT_DESIGNATOR_CHECK", false);
}

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

TicketDesignatorQualifier::TicketDesignatorQualifier() : _removeTicketDesignatorCheck(false)
{
  _removeTicketDesignatorCheck = removeTicketDesignatorCheck.getValue();
}

TicketDesignatorQualifier::~TicketDesignatorQualifier()
{
}

const tse::PaxTypeFare::FareDisplayState
TicketDesignatorQualifier::qualify(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const
{
  TSELatencyData metrics(trx, "QUAL TICKET DESGNTR");

  const FareDisplayRequest* request = trx.getRequest();

  if (!request->ticketDesignator().empty())
  {
    // Create PaxTypeFare Fare Class for Comparison
    std::string ptFareFC = ptFare.createFareBasisCodeFD(trx);

    // Get Tkt Designator after "/" for comparison
    std::string tktDesg;

    // Find "/" in fare basis code
    std::string::size_type i = ptFareFC.find('/');

    if ((i != std::string::npos) && (i < ptFareFC.size() - 1))
    {
      // "/" exists
      tktDesg = ptFareFC.substr(i + 1, ptFareFC.size() - i - 1);
    }

    // match tktDesg in fare basis
    if (tktDesg == request->ticketDesignator())
      return retProc(trx, ptFare);

    // match tktDesg in FBR (may be different than fare basis)
    if (ptFare.isFareByRule())
    {
      const FareByRuleApp* fbrApp = ptFare.getFbrRuleData()->fbrApp();
      if (fbrApp && fbrApp->tktDesignator() == request->ticketDesignator())
        return retProc(trx, ptFare);
    }
    return PaxTypeFare::FD_TicketDesignator;
  }

  // Ticket Designator was not input
  else if (trx.isLongRD())
  {
    // Just return fare(s) without tkt designator
    //  Create PaxTypeFare Fare Basis code
    std::string ptFareFC = ptFare.createFareBasisCodeFD(trx);

    // Find "/" in fare basis code
    std::string::size_type i = ptFareFC.find('/');

    if ((i != std::string::npos) && (i < ptFareFC.size() - 1))
    {
      // "/" exists - assume there's a ticket designator
      return PaxTypeFare::FD_TicketDesignator;
    }
  }

  return retProc(trx, ptFare);
}

bool
TicketDesignatorQualifier::setup(FareDisplayTrx& trx)
{
  return true;
}
}
