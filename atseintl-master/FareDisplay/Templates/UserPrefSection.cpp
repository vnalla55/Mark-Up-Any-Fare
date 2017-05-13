//-------------------------------------------------------------------
//
//  File:        UserPrefSection.cpp
//  Description: Display Diagnostic for User Preferences
//
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/UserPrefSection.h"

#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayTrx.h"

#include <sstream>
#include <string>

namespace tse
{
void
UserPrefSection::buildDisplay()
{
  _trx.response().clear();

  displayUserPref(_trx);

  if ((!_trx.getOptions()->surchargeTypes().empty()) &&
      (_trx.getOptions()->applySurcharges() == YES))
  {
    _trx.response() << " USER SURCHARGES TYPES : ";

    for (const auto surchargeTypeInd : _trx.getOptions()->surchargeTypes())
      _trx.response() << surchargeTypeInd << " ";
  }
  else if (_trx.getOptions()->applySurcharges() == YES)
  {
    _trx.response() << " NO USER SURCHARGES FOUND : USE DEFAULT";
  }

  _trx.response() << " "
                  << "\n";
  _trx.response() << "************************************************************"
                  << "\n";
  _trx.response() << "                     END DIAGNOSTIC INFO"
                  << "\n";
  _trx.response() << "      NODE: ";

  char hostName[1024];

  if (::gethostname(hostName, 1023) < 0)
  {
    std::cout << "hostName: '" << hostName << "'";
    _trx.response() << "HOST NAME ERROR"
                    << "\n";
  }
  else
  {
    std::string aString(hostName);
    std::transform(aString.begin(), aString.end(), aString.begin(), (int (*)(int))toupper);
    _trx.response() << aString << "\n";
  }

  _trx.response() << "************************************************************" << std::endl;
}

void
UserPrefSection::displayUserPref(FareDisplayTrx& trx) const
{

  trx.response() << std::endl;

  trx.response() << " **************** FARE DISPLAY USER PREFERENCES *************"
                 << "\n";

  trx.response() << " TABLES - SPTFP                  FAREDISPPREF/FAREDISPPREFSEG"
                 << "\n";

  trx.response() << " "
                 << "\n";

  trx.response() << " SINGLE CXR TEMPLATE ID                                   ";
  trx.response() << trx.getOptions()->singleCxrTemplateId() << "\n";

  trx.response() << " MULTI CXR TEMPLATE ID                                    ";
  trx.response() << trx.getOptions()->multiCxrTemplateId() << "\n";

  trx.response() << " FL TEMPLATE ID                                           ";
  trx.response() << trx.getOptions()->taxTemplateId() << "\n";

  trx.response() << " ADD ON TEMPLATE ID                                       ";
  trx.response() << trx.getOptions()->addOnTemplateId() << "\n";
  trx.response() << " "
                 << "\n";

  trx.response() << " DISPLAY ROUTINGS IN SINGLE CARRIER DISPLAY               ";
  displayYesNo(trx, trx.getOptions()->showRoutings());
  trx.response() << " "
                 << "\n";

  trx.response() << " VALIDATE CAT 15 LOCALES FOR PUBLIC FARES                 ";
  displayYesNo(trx, trx.getOptions()->validateLocaleForPublFares());
  trx.response() << " "
                 << "\n";

  trx.response() << " ADD SURCHARGES TO BASE FARE                              ";
  displayYesNo(trx, trx.getOptions()->applySurcharges());
  trx.response() << " "
                 << "\n";

  trx.response() << " SUPPRESS FARES WITH FUTURE FIRST SALES DATES             ";
  displayYesNo(trx, trx.getOptions()->noFutureSalesDate());
  trx.response() << " "
                 << "\n";

  trx.response() << " SERVICE COUNTS OR ALTERNATE CARRIER LIST       "
                 << "\n";
  trx.response() << "   SINGLE CARRIER PREFERENCE                              ";
  displayYesNo(trx, trx.getOptions()->singleCarrierSvcSched());
  trx.response() << "   MULTI CARRIER PREFERENCE                               ";
  displayYesNo(trx, trx.getOptions()->multiCarrierSvcSched());
  trx.response() << " "
                 << "\n";

  trx.response() << "   A  - DISPLAY ALTERNATE CARRIER LIST "
                 << "\n";
  trx.response() << "   S  - DISPLAY SERVICE COUNTS         "
                 << "\n";
  trx.response() << "   NO - DISPLAY NEITHER                "
                 << "\n";
  trx.response() << " "
                 << "\n";

  trx.response() << " THESE PREFERENCES APPLY ONLY TO TWO COLUMN DISPLAYS"
                 << "\n";

  trx.response() << "     DOUBLE OW /X/ FARES AND DISPLAY IN RT COLUMN"
                 << "\n";
  trx.response() << "     WHEN NO JOURNEY TYPE OR RT REQUESTED                 ";
  displayYesNo(trx, trx.getOptions()->doubleForRoundTrip());
  trx.response() << " "
                 << "\n";

  trx.response() << "     DISPLAY JOURNEY INDICATORS ON ALL RT FARES"
                 << "\n";
  trx.response() << "     WHEN NO JOURNEY TYPE REQUESTED                       ";
  displayYesNo(trx, trx.getOptions()->journeyInd());
  trx.response() << " "
                 << "\n";

  trx.response() << "     APPLY DAY OF WEEK VALIDATION TO ONE WAY FARES        ";
  displayYesNo(trx, trx.getOptions()->applyDOWvalidationToOWFares());
  trx.response() << " "
                 << "\n";
}

void
UserPrefSection::displayYesNo(FareDisplayTrx& trx, Indicator ind) const
{
  if (ind == 'Y')
  {
    trx.response() << "YES"
                   << "\n";
  }
  else if (ind == 'N')
  {
    trx.response() << "NO"
                   << "\n";
  }
  else
  {
    trx.response() << ind << "\n";
  }
}
} // tse namespace
