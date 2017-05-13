/*----------------------------------------------------------------------------
 *  Copyright Sabre 2015
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/

#include "Xform/FarePathSFRDataFormatter.h"

#include "Common/DateTime.h"
#include "Common/XMLConstruct.h"
#include "Xform/PricingResponseXMLTags.h"

namespace tse
{
void
FarePathSFRDataFormatter::format(const MostRestrictiveJourneySFRData& data)
{
  printJourneyData(data);
}

void
FarePathSFRDataFormatter::printCat5Data(const DateTime& mostRestrictiveAdvResData,
                                        const DateTime& mostRestrictiveTktData)
{
  _commonFormatter.printCat5Data(mostRestrictiveAdvResData, mostRestrictiveTktData);
}

void
FarePathSFRDataFormatter::printJourneyData(const MostRestrictiveJourneySFRData& data)
{
  _xml.openElement(xml2::MostRestrictiveJourneyData);
  printCat5Data(data._advanceReservation, data._advanceTicketing);
  _xml.closeElement();
}
}
