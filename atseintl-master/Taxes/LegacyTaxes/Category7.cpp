// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
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

#include "Taxes/LegacyTaxes/Category7.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayItem.h"
#include "DBAccess/TaxExemptionCarrier.h"
#include "Rules/RuleConst.h"
#include <sstream>
#include <vector>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

namespace tse
{

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::TaxResponse
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category7::Category7()
  : _subCat1(EMPTY_STRING()), _subCat2(EMPTY_STRING()), _subCat3(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function TaxResponse::~TaxResponse
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category7::~Category7() {}
// ----------------------------------------------------------------------------
// <PRE>
//
// @function buildCategory7
//
// Description:  Complete all Category7 layouts
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category7::build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem)
{
  uint32_t maxLineSize = 48;
  boost::char_separator<char> sep(",");
  boost::tokenizer<boost::char_separator<char> >::iterator iter;
  std::string tempStr(EMPTY_STRING());

  _subCat3 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::AIRLINE);

  if (taxDisplayItem.taxCodeReg()->exemptionCxr().empty() && _subCat3.empty())
  {
    _subCat1 = "     NO AIRLINE RESTRICTIONS APPLY.\n";
    return;
  }

  std::string carriers = EMPTY_STRING();

  std::vector<TaxExemptionCarrier>::iterator taxExemptionCarrierIter =
      taxDisplayItem.taxCodeReg()->exemptionCxr().begin();
  std::vector<TaxExemptionCarrier>::iterator taxExemptionCarrierEndIter =
      taxDisplayItem.taxCodeReg()->exemptionCxr().end();

  for (; taxExemptionCarrierIter != taxExemptionCarrierEndIter; taxExemptionCarrierIter++)
  {
    if (((*taxExemptionCarrierIter).flight1() == 0) &&
        ((*taxExemptionCarrierIter).flight2() == 0) &&
        ((*taxExemptionCarrierIter).airport1().empty()) &&
        ((*taxExemptionCarrierIter).airport2().empty()))
    {
      if (_subCat1.empty())
      {
        _subCat1 = "* APPLIES ON CARRIER/S -";

        if (taxDisplayItem.taxCodeReg()->exempcxrExclInd() == YES)
          _subCat1 = "* EXCEPT ON CARRIER/S -";

        _subCat1 += "\n  ";
      }
      else
      {
        _subCat1 += ", ";
      }

      if ((_subCat1.size() - _subCat1.find_last_of("\n")) > 57)
        _subCat1 += "\n  ";

      _subCat1 += (*taxExemptionCarrierIter).carrier();
      continue;
    }

    std::string::size_type cxrSize = carriers.find((*taxExemptionCarrierIter).carrier());

    if (cxrSize != std::string::npos)
      continue;

    if (_subCat1.empty())
    {
      _subCat1 = "* APPLIES ON CARRIER/S  ";

      if (taxDisplayItem.taxCodeReg()->exempcxrExclInd() == YES)
        _subCat1 = "* EXCEPT ON CARRIER/S  ";

      _subCat1 += "\n";
    }

    carriers += (*taxExemptionCarrierIter).carrier();

    std::string between =
        cityPairFlightInfo(taxDisplayItem, (*taxExemptionCarrierIter).carrier(), 'B');

    if (!between.empty())
    {
      tempStr = EMPTY_STRING();
      _subCat2 += (*taxExemptionCarrierIter).carrier() + "-BETWEEN- ";

      // This section was added to format the output.
      // To indent and to properly wrap text while mainting indentation
      boost::tokenizer<boost::char_separator<char> > tokenizer(between, sep);
      for (iter = tokenizer.begin(); iter != tokenizer.end(); ++iter)
      {
        if ((tempStr.size() + (*iter).size()) < maxLineSize)
        {
          if ((*iter)[0] == ' ')
          {
            if (tempStr.empty())
            {
              tempStr = (*iter).substr(1);
            }
            else
            {
              tempStr += ", " + (*iter).substr(1);
            }
          }
          else
          {
            if (tempStr.empty())
            {
              tempStr = *iter;
            }
            else
            {
              tempStr += ", " + *iter;
            }
          }
        }
        else
        {
          _subCat2 += tempStr + ",\n" + "            ";

          if ((*iter)[0] == ' ')
          {
            tempStr = (*iter).substr(1);
          }
          else
          {
            tempStr = *iter;
          }
        }
      }

      if (tempStr.size() > maxLineSize)
      {
        _subCat2 += "\n            " + tempStr + ".";
      }
      else
      {
        _subCat2 += tempStr + ".";
      }
      _subCat2 += "\n";
    }

    std::string fromTo =
        cityPairFlightInfo(taxDisplayItem, (*taxExemptionCarrierIter).carrier(), 'F');

    if (!fromTo.empty())
    {
      tempStr = EMPTY_STRING();

      if (!between.empty())
      {
        _subCat2 += "  -FROM   - ";
      }
      else
      {
        _subCat2 += (*taxExemptionCarrierIter).carrier() + "-FROM-    ";
      }

      // This section was added to format the output.
      // To indent and to properly wrap text while mainting indentation
      boost::tokenizer<boost::char_separator<char> > tokenizer(fromTo, sep);
      for (iter = tokenizer.begin(); ((iter != tokenizer.end()) && (*iter != "\n")); ++iter)
      {
        if ((tempStr.size() + (*iter).size()) < maxLineSize)
        {
          if ((*iter)[0] == ' ')
          {
            if (tempStr.empty())
            {
              tempStr += (*iter).substr(1);
            }
            else
            {
              tempStr += ", " + (*iter).substr(1);
            }
          }
          else
          {
            if (tempStr.empty())
            {
              tempStr += *iter;
            }
            else
            {
              tempStr += ", " + *iter;
            }
          }
        }
        else
        {
          _subCat2 += tempStr + ",\n" + "            ";

          if ((*iter)[0] == ' ')
          {
            tempStr = (*iter).substr(1);
          }
          else
          {
            tempStr = *iter;
          }
        }
      }

      if (tempStr.size() > maxLineSize)
      {
        _subCat2 += "\n            " + tempStr + ".";
      }
      else
      {
        _subCat2 += tempStr + ".";
      }

      _subCat2 += "\n";
    }
    std::string flights =
        cityPairFlightInfo(taxDisplayItem, (*taxExemptionCarrierIter).carrier(), '*');

    if (!flights.empty())
    {
      tempStr = EMPTY_STRING();

      if (!between.empty() || !fromTo.empty())
      {
        _subCat2 += " - "; //"  -       - " PL 17886;
      }
      else
      {
        _subCat2 += (*taxExemptionCarrierIter).carrier() + "- "; //"-           " PL 17886
      }

      // This section was added to format the output.
      // To indent and to properly wrap text while mainting indentation
      boost::tokenizer<boost::char_separator<char> > tokenizer(flights, sep);
      for (iter = tokenizer.begin(); ((iter != tokenizer.end()) && (*iter != "\n")); ++iter)
      {
        if ((tempStr.size() + (*iter).size()) < maxLineSize)
        {
          if ((*iter)[0] == ' ')
          {
            if (tempStr.empty())
            {
              tempStr += (*iter).substr(1);
            }
            else
            {
              tempStr += ", " + (*iter).substr(1);
            }
          }
          else
          {
            if (tempStr.empty())
            {
              tempStr += *iter;
            }
            else
            {
              tempStr += ", " + *iter;
            }
          }
        }
        else
        {
          _subCat2 += tempStr + ",\n" + "            ";

          if ((*iter)[0] == ' ')
          {
            tempStr = (*iter).substr(1);
          }
          else
          {
            tempStr = *iter;
          }
        }
      }

      if (tempStr.size() > maxLineSize)
      {
        _subCat2 += "\n            " + tempStr + ".";
      }
      else
      {
        _subCat2 += tempStr + ".";
      }

      _subCat2 += "\n";
    }
  }

  if (!_subCat1.empty() && _subCat1.substr(_subCat1.size() - 1, 1) != "\n")
    _subCat1 += ".\n";
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function cityPairFlightInfo
//
// Description: Group city pair and flight under a carrier code
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category7::cityPairFlightInfo(TaxDisplayItem& taxDisplayItem,
                              CarrierCode carrierCode,
                              char direction)
{
  std::string cxrInfo = EMPTY_STRING();
  std::string tempInfo = EMPTY_STRING();

  std::vector<TaxExemptionCarrier>::iterator taxExemptionCarrierIter =
      taxDisplayItem.taxCodeReg()->exemptionCxr().begin();
  std::vector<TaxExemptionCarrier>::iterator taxExemptionCarrierEndIter =
      taxDisplayItem.taxCodeReg()->exemptionCxr().end();

  for (; taxExemptionCarrierIter != taxExemptionCarrierEndIter; taxExemptionCarrierIter++)
  {
    if (carrierCode != (*taxExemptionCarrierIter).carrier())
      continue;

    tempInfo = EMPTY_STRING();

    std::string flightText =
        flightInfo((*taxExemptionCarrierIter).flight1(), (*taxExemptionCarrierIter).flight2());

    if ((*taxExemptionCarrierIter).direction() == direction)
    {
      if (!(*taxExemptionCarrierIter).airport1().empty())
      {
        tempInfo += (*taxExemptionCarrierIter).airport1().c_str();
        if (direction == 'F')
          tempInfo += " TO ";
        else
          tempInfo += " AND ";

        if (!(*taxExemptionCarrierIter).airport2().empty())
        {
          tempInfo += (*taxExemptionCarrierIter).airport2();
        }
        else
        {
          tempInfo += "ANYWHERE";
        }
      }

      if (!flightText.empty())
      {
        tempInfo += RuleConst::BLANK;
        tempInfo += flightText;
      }
    }

    if ((direction == '*') && ((*taxExemptionCarrierIter).direction() == RuleConst::BLANK))
    {
      tempInfo += flightText;
    }

    if (!tempInfo.empty() && !cxrInfo.empty())
      cxrInfo += ", ";

    cxrInfo += tempInfo;
  }
  return cxrInfo;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function flightInfo
//
// Description: flight number strings
//
// </PRE>
// ----------------------------------------------------------------------------

std::string
Category7::flightInfo(uint16_t flight1, uint16_t flight2)
{
  std::string cxrInfo = EMPTY_STRING();

  if (flight1 != 0)
  {
    cxrInfo += "ON FLIGHT ";
    cxrInfo += boost::lexical_cast<std::string>(flight1);

    if (flight2 != 0)
      cxrInfo += " TO " + boost::lexical_cast<std::string>(flight2);
  }
  return cxrInfo;
}
}
