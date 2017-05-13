//-------------------------------------------------------------------
//
//  File:        ActualPaxTypeSection.cpp
//  Author:      Svetlana Tsarkova
//  Created:     October 10, 2005
//  Description: Diagnostic for Actual Passenger Type Map
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

#include "FareDisplay/Templates/ActualPaxTypeSection.h"

#include "DataModel/Billing.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxType.h"

#include <unistd.h>
#include <string>
#include <sstream>

namespace tse
{

void
ActualPaxTypeSection::buildDisplay()
{
  _trx.response().clear();

  displayActualPaxTypeMap();

  _trx.response() << "\n";

  _trx.response() << "*************************************************************"
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

  _trx.response() << "*************************************************************" << std::endl;
}

void
ActualPaxTypeSection::displayActualPaxTypeMap() const
{
  _trx.response() << "******************** ACTUAL PAX TYPE MAP ********************"
                  << "\n";

    // vector<PaxType*> loop
  for (const auto paxType : _trx.paxType())
  {
    _trx.response() << " "
                    << "\n"; // Always start from a new line
    _trx.response() << "FOR A PASSENGER TYPE: " << paxType->paxType();
    _trx.response() << "  AND VENDOR: " << paxType->vendorCode() << "\n";

    std::map<CarrierCode, std::vector<PaxType*>*>& paxTypeMap = paxType->actualPaxType();
    // Map loop
    for (const auto& paxTypeMapItem : paxTypeMap)
    {
      _trx.response() << "CARRIER: ";
      _trx.response() << paxTypeMapItem.first << "\n";
      _trx.response() << "ACTUAL PAX TYPES: ";
      const std::vector<PaxType*>& pVector = *(paxTypeMapItem.second);

      if (!pVector.empty())
      {
        std::vector<PaxType*>::const_iterator vectorBegin = pVector.begin();
        std::vector<PaxType*>::const_iterator vectorEnd = pVector.end();
        int count = 0;
        for (; vectorBegin != vectorEnd; vectorBegin++)
        {
          if ((vectorBegin != pVector.begin()) && count == 0)
          {
            _trx.response() << "                  ";
          }
          PaxType* psgPointer = *vectorBegin;
          _trx.response() << psgPointer->paxType() << " ";
          ++count;
          if (count == 5)
          {
            _trx.response() << "\n";
            count = 0;
          }
        } // vector loop
      } // if pVector
    } // End Map loop
  } //  end vector<PaxType> loop
}
} // tse namespace
