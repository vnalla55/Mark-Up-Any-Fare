//-------------------------------------------------------------------
//
//  File:        HeaderMsgSection.cpp
//  Authors:     Doug Batchelor
//  Created:     May 3, 2005
//  Description: This class abstracts a section.  It maintains
//               all the data and methods necessary to describe
//               and realize a particular set of header
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

#include "FareDisplay/Templates/HeaderMsgSection.h"

#include "FareDisplay/FDHeaderMsgController.h"
#include "FareDisplay/FDHeaderMsgText.h"
#include "Common/ItinUtil.h"

namespace tse
{
const std::string
HeaderMsgSection::CENTER("CT");
const std::string
HeaderMsgSection::RIGHT("RJ");
const std::string
HeaderMsgSection::BLANK_SPACE(" ");

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  HeaderMsgSection::buildDisplay()
//
// This is the main processing method of the HeaderMsgSection class.
// It requires a reference to the Fare Display Transaction. It is
// a non-standard template class that has no field classes and
// displays its data by writing directly into the response object.
//
//
//
// @return  void.
//
// </PRE>
// -------------------------------------------------------------------------

void
HeaderMsgSection::buildDisplay()
{
  std::vector<tse::FDHeaderMsgText*> headerMsgList;
  FDHeaderMsgController obj(_trx, _preferredCarriers);
  obj.getHeaderMsgs(headerMsgList);

  if (headerMsgList.empty())
  {
    displayAlternateCurrencyMessages(_trx);
    return;
  }
  // for use in XForm SDS response
  _trx.fdResponse()->headerMsgs() = headerMsgList;

  std::ostringstream& p0 = _trx.response();

  for (const auto fdHeaderMsg : headerMsgList)
  {

    int16_t size;
    const std::string& pos = fdHeaderMsg->startPoint();

    // Considering the length of a line is 63
    if (pos == CENTER)
      size = (63 - fdHeaderMsg->text().length()) / 2;
    else if (pos == RIGHT)
      size = (63 - fdHeaderMsg->text().length()) + 1;
    else
      size = atoi(pos.c_str());

    for (int16_t i = 1; i < size; i++)
      p0 << BLANK_SPACE;

    p0 << fdHeaderMsg->text() << std::endl;
  }

  displayAlternateCurrencyMessages(_trx);
}

// Display Alternate Currency header message when fares are published in more than one currency
void
HeaderMsgSection::displayAlternateCurrencyMessages(FareDisplayTrx& trx) const
{
  Itin& itin = *(trx.itin().front());

  // If domestic markets or if multiple currencies allowed
  // do not display Alternate Currency Header messages
  if (trx.isDomestic() || ItinUtil::applyMultiCurrencyPricing(&trx, itin))
    return;

  // Get alternate currencies
  std::set<CurrencyCode> alternateCurrencies;
  trx.getAlternateCurrencies(alternateCurrencies);

  if (alternateCurrencies.empty())
    return;

  std::ostringstream& p0 = trx.response();
  p0 << "MORE FARES FILED IN ";
  for (const auto& altCurrencyCode : alternateCurrencies)
    p0 << altCurrencyCode << BLANK_SPACE;

  p0 << std::endl;
}
} // tse namespace
