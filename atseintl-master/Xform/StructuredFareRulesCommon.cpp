/*----------------------------------------------------------------------------
 *  Copyright Sabre 2016
 *
 *     The copyright to the computer program(s) herein
 *     is the property of Sabre.
 *     The program(s) may be used and/or copied only with
 *     the written permission of Sabre or in accordance
 *     with the terms and conditions stipulated in the
 *     agreement/contract under which the program(s)
 *     have been supplied.
 *-------------------------------------------------------------------------*/

#include "Xform/StructuredFareRulesCommon.h"

#include "Common/XMLConstruct.h"
#include "Xform/PricingResponseXMLTags.h"

namespace tse
{
void
StructuredFareRulesCommon::printCat5Data(const DateTime& advResDate, const DateTime& advTktDate)
{
  if (!(advResDate.isValid() || advTktDate.isValid()))
    return;

  _xml.openElement(xml2::AdvanceReservationData);

  if (advResDate.isValid())
  {
    _xml.addAttribute(xml2::LastDayToBook, advResDate.dateToString(YYYYMMDD, "-"));
    _xml.addAttribute(xml2::LastTimeToBook, advResDate.timeToString(HHMM, ":"));
  }

  if (advTktDate.isValid())
  {
    _xml.addAttribute(xml2::LastDayToPurchaseTicket, advTktDate.dateToString(YYYYMMDD, "-"));
    _xml.addAttribute(xml2::LastTimeToPurchaseTicket, advTktDate.timeToString(HHMM, ":"));
  }

  _xml.closeElement();
}

void
StructuredFareRulesCommon::printCat7Data(const MaxStayMap& maxStayData)
{
  for (const auto& mapElem : maxStayData)
  {
    const MaxStayData& maxStayData = mapElem.second;
    _xml.openElement(xml2::MaximumStay);
    if (maxStayData._mustCommence.isValid())
    {
      _xml.addAttribute(xml2::MaximumStayDate,
                        maxStayData._mustCommence.dateToString(YYYYMMDD, "-"));
      _xml.addAttribute(xml2::MaximumStayTime, maxStayData._mustCommence.timeToString(HHMM, ":"));
    }
    if (maxStayData._mustComplete.isValid())
    {
      _xml.addAttribute(xml2::LastDayReturnMustBeCompleted,
                        maxStayData._mustComplete.dateToString(YYYYMMDD, "-"));
      _xml.addAttribute(xml2::LastTimeReturnMustBeCompleted,
                        maxStayData._mustComplete.timeToString(HHMM, ":"));
    }
    _xml.addAttribute(xml2::Location, maxStayData._location);
    _xml.closeElement();
  }
}

void
StructuredFareRulesCommon::printCat6Data(const DateTime& date, const LocCode& location)
{
  if (!date.isValid())
    return;
  _xml.openElement(xml2::MinimumStay);
  _xml.addAttribute(xml2::MinimumStayDate, date.dateToString(YYYYMMDD, "-"));
  _xml.addAttribute(xml2::MinimumStayTime, date.timeToString(HHMM, ":"));
  _xml.addAttribute(xml2::Location, location);
  _xml.closeElement();
}
}
