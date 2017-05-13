//----------------------------------------------------------------------------
//
//  File:  DataModelMap.cpp
//  Description: See DataModelMap.h file
//  Created:  March 22, 2004
//  Authors:  Mike Carroll
//
//  Copyright Sabre 2003
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

#include "Xform/DataModelMap.h"

#include "Common/CurrencyUtil.h"
#include "Common/ItinUtil.h"
#include "Common/Logger.h"
#include "Common/XMLChString.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Loc.h"
#include "DBAccess/MultiTransport.h"
#include "Xform/DynamicConfig.h"

#include <boost/regex.hpp>

#include <fstream>
#include <set>
#include <string>
#include <vector>

#include <time.h>

namespace tse
{

static Logger
logger("atseintl.Xform.DataModelMap");

std::string
DataModelMap::purgeBookingCodeOfNonAlpha(const std::string &bkc)
{
  const boost::regex twoAlphas("[A-Z]{1,2}");

  boost::cmatch tokens;
  if(boost::regex_search(bkc.c_str(), tokens, twoAlphas))
    return tokens[0];
  return bkc;
}

unsigned int
DataModelMap::SDBMHash(const std::string& strValue)
{
  return (SDBMHash(strValue.c_str()));
}

unsigned int
DataModelMap::SDBMHash(const char* cptrValue)
{
  unsigned int hash = 0;

  if (!cptrValue)
    return hash;

  for (unsigned int i = 0; cptrValue[i] != '\0'; i++)
    hash = cptrValue[i] + (hash << 6) + (hash << 16) - hash;

  return (hash & 0x7FFFFFFF);
}

const DateTime
DataModelMap::convertDate(const char* inDate)
{
  return DateTime::convertDate(inDate);
}

const DateTime
DataModelMap::convertDateDDMMMYY(const char* inDate)
{
  if ((inDate == nullptr) || (strlen(inDate) == 0))
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT, "INVALID DATETIME");

  int year = 0;
  int month = 0;
  int day = 0;

  // Expect DDMMMYY
  const char* cSource = inDate;
  char tmpBuf[5];
  char* target = tmpBuf;

  // Get day
  for (int i = 0; i < 2; i++)
    *target++ = *cSource++;
  *target = '\0';
  day = atoi(tmpBuf);

  // Get month
  target = tmpBuf;
  for (int i = 0; i < 3; i++)
    *target++ = *cSource++;
  *target = '\0';
  if (!strcmp(tmpBuf, "JAN"))
    month = 1;
  else if (!strcmp(tmpBuf, "FEB"))
    month = 2;
  else if (!strcmp(tmpBuf, "MAR"))
    month = 3;
  else if (!strcmp(tmpBuf, "APR"))
    month = 4;
  else if (!strcmp(tmpBuf, "MAY"))
    month = 5;
  else if (!strcmp(tmpBuf, "JUN"))
    month = 6;
  else if (!strcmp(tmpBuf, "JUL"))
    month = 7;
  else if (!strcmp(tmpBuf, "AUG"))
    month = 8;
  else if (!strcmp(tmpBuf, "SEP"))
    month = 9;
  else if (!strcmp(tmpBuf, "OCT"))
    month = 10;
  else if (!strcmp(tmpBuf, "NOV"))
    month = 11;
  else if (!strcmp(tmpBuf, "DEC"))
    month = 12;

  // Get year
  target = tmpBuf;
  while (*cSource)
    *target++ = *cSource++;
  *target = '\0';
  // @todo assume base of 2000
  year = (atoi(tmpBuf)) + 2000;

  DateTime convertedTime(year, month, day);

  return (convertedTime);
}

std::string&
DataModelMap::toUpper(std::string& toConvert) const
{
  if (toConvert.empty())
    return toConvert;

  std::transform(toConvert.begin(), toConvert.end(), toConvert.begin(), (int (*)(int))toupper);

  return toConvert;
}

//--------------------------------------------------------------------------
// @method checkFuturePricingCurrency
//
// Description: If a future date is used for the pricing request
//              the nations currency must be checked to see if
//              a new currency must be used.
//
// @param PricingTrx         - pricingTrx pointer
// @param Itin*              - itinerary pointer
// @param PricingOptions*    - options pointer
//
// @return void
//--------------------------------------------------------------------------
void
DataModelMap::checkFuturePricingCurrency(PricingTrx* pricingTrx,
                                         Itin* itin,
                                         PricingOptions* options)
{

  if (pricingTrx)
  {
    if (pricingTrx->getRequest()->ticketingDT().isFutureDate())
    {
      LOG4CXX_DEBUG(logger, "PROCESSING FUTURE PRICING REQUEST");

      if (itin)
      {
        NationCode nationCode;
        CurrencyCode nationCurrency;
        bool rc = false;

        if (!pricingTrx->getRequest()->salePointOverride().empty())
        {
          const tse::Loc* loc =
              pricingTrx->dataHandle().getLoc(pricingTrx->getRequest()->salePointOverride(),
                                              pricingTrx->getRequest()->ticketingDT());

          if (loc)
            nationCode = loc->nation();
          else
            rc = false;
        }
        else
        {
          nationCode = pricingTrx->getRequest()->ticketingAgent()->agentLocation()->nation();

          if (!options->isMOverride())
          {
            const Customer* cust = (pricingTrx->getRequest()->ticketingAgent()->agentTJR());

            if (cust)
            {
              if (!(cust->defaultCur().empty()))
              {
                LOG4CXX_DEBUG(
                    logger,
                    "AGENCY HAS A TICKETING DEFAULT CURRENCY OVERRIDE: " << cust->defaultCur());

                if (options->currencyOverride() != cust->defaultCur())
                {
                  LOG4CXX_DEBUG(logger,
                                "RESETTING CURRENCY OVERRIDE TO TICKETING DEFAULT CURRENCY "
                                    << cust->defaultCur());
                  options->currencyOverride() = cust->defaultCur();
                }
                else
                  LOG4CXX_DEBUG(logger,
                                "CURRENCY OVERRIDE EQUALS TICKETING DEFAULT CURRENCY "
                                    << cust->defaultCur());

                return;
              }
            }
            else
              LOG4CXX_DEBUG(logger, "NO CUST RECORD");
          }
        }

        rc = CurrencyUtil::getNationCurrency(
            nationCode, nationCurrency, pricingTrx->getRequest()->ticketingDT());

        if (rc)
        {
          if (options)
          {
            if (!options->isMOverride())
            {
              if (nationCurrency != options->currencyOverride())
                options->currencyOverride() = nationCurrency;
            }
          }
        }
      }
    }
  }
}

void
DataModelMap::handleDynamicConfig(MemberMap& members, const xercesc::Attributes& attrs)
{
  DynamicConfigHandler handler(*_trx);
  if (!handler.check())
    return;

  DynamicConfigInput input;

  size_t numAtts = attrs.getLength();
  for (size_t i = 0; i < numAtts; ++i)
  {
    XMLChString xmlStr(attrs.getLocalName(i));
    XMLChString xmlValue(attrs.getValue(i));

    std::string attrName;
    xmlStr.parse(attrName);
    std::transform(attrName.begin(), attrName.end(), attrName.begin(), (int (*)(int))toupper);

    switch (members[SDBMHash(attrName)])
    {
    case 1: // Name - Configuration name
      input.setName(xmlValue.get<std::string>());
      break;
    case 2: // Value - Configuration value
      input.setValue(xmlValue.get<std::string>());
      break;
    case 3: // Substitute - Configuration name substitute enable
      input.setSubstitute(xmlValue.get<std::string>());
      break;
    case 4: // Optional - Configuration override is optional
      input.setOptional(xmlValue.get<std::string>());
      break;

    default:
      LOG4CXX_WARN(logger, "Attribute: " << xmlStr.c_str() << ", not mapped");
      break;
    }
  }

  handler.process(input);
}
} // tse namespace
