// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2007
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
#include "Taxes/LegacyTaxes/Category13.h"
#include "Taxes/LegacyTaxes/TaxDisplayCommonText.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/Common/TaxUtility.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Taxes/LegacyTaxes/LocationDescriptionUtil.h"

#include <sstream>
#include <algorithm>
#include <iterator>
#include <boost/lexical_cast.hpp>
#include <iostream>

using namespace tse;

const uint32_t Category13::NUMBER = 13;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category13::Category13
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category13::Category13() : _subCat1(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category13::~Category13
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

Category13::~Category13() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function Category13::build
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

void
Category13::build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem)
{
  Cabins cabinsNoCarrier;
  Cabins cabinsExclNoCarrier;

  Carriers carriers;

  CarrierCabins carrierCabins;
  CarrierCabins carrierExclCabins;

  getCarrierCabinInfo(trx,
                      taxDisplayItem,
                      cabinsNoCarrier,
                      cabinsExclNoCarrier,
                      carriers,
                      carrierCabins,
                      carrierExclCabins);

  std::string defaultCabins;

  if (cabinsNoCarrier.size())
  {
    std::ostringstream cabinsStream;
    std::copy(cabinsNoCarrier.begin(),
              cabinsNoCarrier.end(),
              std::ostream_iterator<std::string>(cabinsStream, ", "));

    defaultCabins =
        "* APPLICABLE TO THE FOLLOWING DEFAULT CABIN INVENTORY CLASS/ES:\n  " + cabinsStream.str();

    std::string::size_type replacepos = defaultCabins.rfind(", ");

    if (replacepos != std::string::npos)
      defaultCabins.replace(replacepos, 2, ".\n");
  }

  if (cabinsExclNoCarrier.size())
  {
    std::ostringstream cabinsStream;
    std::copy(cabinsExclNoCarrier.begin(),
              cabinsExclNoCarrier.end(),
              std::ostream_iterator<std::string>(cabinsStream, ", "));

    defaultCabins += "* NOT APPLICABLE TO THE FOLLOWING DEFAULT CABIN INVENTORY CLASS/ES:\n  " +
                     cabinsStream.str();

    std::string::size_type replacepos = defaultCabins.rfind(", ");

    if (replacepos != std::string::npos)
      defaultCabins.replace(replacepos, 2, ".\n");
  }

  // build _subCat1

  if (defaultCabins.size())
    _subCat1 = defaultCabins;

  if (carriers.size())
  {
    if (!cabinsNoCarrier.size() && !cabinsExclNoCarrier.size())
      _subCat1 = EMPTY_STRING();

    std::ostringstream carriersStream;
    std::copy(
        carriers.begin(), carriers.end(), std::ostream_iterator<CarrierCode>(carriersStream, ", "));

    _subCat1 += "  CARRIER/S WITH EXCEPTION DATA ";

    _subCat1 += carriersStream.str();

    std::string::size_type replacepos = _subCat1.rfind(", ");

    if (replacepos != std::string::npos)
      _subCat1.replace(replacepos, 2, " -");

    _subCat1 += " REFER TO SABRE ENTRY TXNHELP FOR SPECIFIC TAX CODE/CARRIER CABIN DATA.\n";
  }

  // build _subCat2

  std::string detailInfo =
      formatCabinDetailInfo(trx, taxDisplayItem, carrierCabins, carrierExclCabins);

  if (detailInfo.size())
  {
    if (defaultCabins.size())
    {
      _subCat2 = defaultCabins;
      _subCat2 += " \nTHE FOLOWING ARE EXCEPTIONS TO THE DEFAULT INVENTORY CLASSES:\n" + detailInfo;
    }
    else
    {
      _subCat2 = "THE FOLOWING ARE EXCEPTIONS TO THE DEFAULT INVENTORY CLASSES:\n" + detailInfo;
    }
  }

  _subCat3 = TaxDisplayCommonText::getCommonText(*taxDisplayItem.taxCodeReg(),
                                                 TaxDisplayCommonText::CABIN);

  if (_subCat1.empty() && _subCat2.empty() && _subCat3.empty())
  {
    _subCat1 = "     NO CABIN RESTRICTIONS APPLY.\n";
  }
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function formatCabinDetailLine
//
// Description: - Special Cabin/Carrier Entry
//                Allows the user to view tax code cabin data
//
// </PRE>
// ----------------------------------------------------------------------------
std::string
Category13::formatCabinDetailInfo(TaxTrx& taxTrx,
                                  TaxDisplayItem& taxDisplayItem,
                                  CarrierCabins& carrierCabins,
                                  CarrierCabins& carrierExclCabins)
{
  std::string cabinLine;

  for (std::vector<TaxCodeCabin*>::iterator cabinPtrs =
           taxDisplayItem.taxCodeReg()->cabins().begin();
       cabinPtrs < taxDisplayItem.taxCodeReg()->cabins().end();
       cabinPtrs++)
  {
    if ((*cabinPtrs)->carrier() == EMPTY_STRING())
      continue;

    if (!taxTrx.getRequest()->carrierCode().empty())
    {
      if ((*cabinPtrs)->carrier() != taxTrx.getRequest()->carrierCode())
        continue;
    }

    CarrierCabins::iterator i = carrierCabins.find((*cabinPtrs)->carrier());
    CarrierCabins::iterator j = carrierExclCabins.find((*cabinPtrs)->carrier());

    if (i != carrierCabins.end() || j != carrierExclCabins.end())
    {
      cabinLine +=
          "* APPLICABLE TO CABIN INVENTORY CLASSES FOR CARRIER " + (*cabinPtrs)->carrier() + "\n";

      if (i != carrierCabins.end())
      {
        Cabins c = i->second;

        std::ostringstream cabinsStream;
        std::copy(c.begin(), c.end(), std::ostream_iterator<std::string>(cabinsStream, ", "));

        cabinLine += "  " + cabinsStream.str();

        std::string::size_type replacepos = cabinLine.rfind(", ");

        if (replacepos != std::string::npos)
          cabinLine.replace(replacepos, 2, " ");

        cabinLine += "CLASSES OF SERVICE\n";

        carrierCabins.erase(i);
      }

      if (j != carrierExclCabins.end())
      {
        Cabins c = j->second;

        std::ostringstream cabinsStream;
        std::copy(c.begin(), c.end(), std::ostream_iterator<std::string>(cabinsStream, ", "));

        cabinLine += "  EXCEPT " + cabinsStream.str();

        std::string::size_type replacepos = cabinLine.rfind(", ");

        if (replacepos != std::string::npos)
          cabinLine.replace(replacepos, 2, " ");

        cabinLine += "CLASSES OF SERVICE\n";

        carrierExclCabins.erase(j);
      }
    }

    if (locInfo(taxTrx, cabinPtrs) || flightInfo(cabinPtrs))
    {

      cabinLine +=
          "* APPLICABLE TO CABIN INVENTORY CLASSES FOR CARRIER " + (*cabinPtrs)->carrier() + "\n";

      if (locInfo(taxTrx, cabinPtrs))
      {
        if ((*cabinPtrs)->exceptInd() == 'Y')
        {
          cabinLine += "  EXCEPT ";
        }
        else
        {
          cabinLine += "  ";
        }

        if ((*cabinPtrs)->classOfService() != EMPTY_STRING())
        {
          cabinLine += (*cabinPtrs)->classOfService() + " CLASS ";
        }

        if ((*cabinPtrs)->directionalInd() == BETWEEN)
        {
          cabinLine += "BETWEEN";
        }
        else if ((*cabinPtrs)->directionalInd() == FROM)
        {
          cabinLine += "FROM";
        }
        else if ((*cabinPtrs)->directionalInd() == WITHIN)
        {
          cabinLine += "WITHIN";
        }

        cabinLine += " " + LocationDescriptionUtil::description(
                               taxTrx,
                               static_cast<LocType>((*cabinPtrs)->loc1().locType()),
                               (*cabinPtrs)->loc1().loc()) +
                     " ";

        if ((*cabinPtrs)->directionalInd() != WITHIN &&
            LocationDescriptionUtil::description(
                taxTrx,
                static_cast<LocType>((*cabinPtrs)->loc2().locType()),
                (*cabinPtrs)->loc2().loc()) != EMPTY_STRING())
        {
          if ((*cabinPtrs)->directionalInd() == BETWEEN)
          {
            cabinLine += "AND";
          }
          else if ((*cabinPtrs)->directionalInd() == FROM)
          {
            cabinLine += "TO";
          }

          cabinLine += " " + LocationDescriptionUtil::description(
                                 taxTrx,
                                 static_cast<LocType>((*cabinPtrs)->loc2().locType()),
                                 (*cabinPtrs)->loc2().loc()) +
                       " ";
        }
      }

      if (flightInfo(cabinPtrs))
      {
        if (!locInfo(taxTrx, cabinPtrs))
        {
          cabinLine += "  ";
        }

        cabinLine += "ON FLIGHT " + boost::lexical_cast<std::string>((*cabinPtrs)->flight1());
        cabinLine += " TO " + boost::lexical_cast<std::string>((*cabinPtrs)->flight2()) + "\n";
      }
      else
      {
        std::string::size_type replacepos = cabinLine.rfind(" ");

        if (replacepos != std::string::npos)
          cabinLine.replace(replacepos, 1, "\n");

        continue;
      }
    }
  }
  return cabinLine;
}

bool
Category13::flightInfo(std::vector<TaxCodeCabin*>::iterator cabinPtrs)
{
  return ((*cabinPtrs)->flight1() != 0 && (*cabinPtrs)->flight2() != 0);
}

bool
Category13::locInfo(TaxTrx& trx, std::vector<TaxCodeCabin*>::iterator cabinPtrs)
{
  return (
      LocationDescriptionUtil::description(trx,
                                           static_cast<LocType>((*cabinPtrs)->loc1().locType()),
                                           (*cabinPtrs)->loc1().loc()) != EMPTY_STRING() &&
      (((*cabinPtrs)->directionalInd() == FROM || (*cabinPtrs)->directionalInd() == WITHIN) ||
       ((*cabinPtrs)->directionalInd() == BETWEEN &&
        LocationDescriptionUtil::description(trx,
                                             static_cast<LocType>((*cabinPtrs)->loc2().locType()),
                                             (*cabinPtrs)->loc2().loc()) != EMPTY_STRING())));
}

void
Category13::insertCabin(CarrierCabins& carrierCabins, std::string carrier, std::string cabin)
{

  Cabins c;
  CarrierCabins::iterator i = carrierCabins.find(carrier);
  if (i != carrierCabins.end())
  {
    c = i->second;
    c.insert(cabin);
    carrierCabins.erase(i);
  }
  else
  {
    c.insert(cabin);
  }

  carrierCabins.insert(std::make_pair(carrier, c));
}

void
Category13::getCarrierCabinInfo(TaxTrx& trx,
                                TaxDisplayItem& taxDisplayItem,
                                Cabins& cabinsNoCarrier,
                                Cabins& cabinsExclNoCarrier,
                                Carriers& carriers,
                                CarrierCabins& carrierCabins,
                                CarrierCabins& carrierExclCabins)
{

  for (std::vector<TaxCodeCabin*>::iterator cabinPtrs =
           taxDisplayItem.taxCodeReg()->cabins().begin();
       cabinPtrs < taxDisplayItem.taxCodeReg()->cabins().end();
       cabinPtrs++)
  {
    if ((*cabinPtrs)->carrier() != EMPTY_STRING())
    {
      carriers.insert((*cabinPtrs)->carrier());
    }

    if ((*cabinPtrs)->exceptInd() != 'Y')
    {
      if ((*cabinPtrs)->carrier() == EMPTY_STRING() &&
          (*cabinPtrs)->classOfService() != EMPTY_STRING() && (*cabinPtrs)->classOfService() != "*")
      {
        cabinsNoCarrier.insert((*cabinPtrs)->classOfService());
      }

      if (!locInfo(trx, cabinPtrs) && !flightInfo(cabinPtrs))
        insertCabin(carrierCabins, (*cabinPtrs)->carrier(), (*cabinPtrs)->classOfService());
    }
    else
    {
      if ((*cabinPtrs)->carrier() == EMPTY_STRING() &&
          (*cabinPtrs)->classOfService() != EMPTY_STRING() && (*cabinPtrs)->classOfService() != "*")
      {
        cabinsExclNoCarrier.insert((*cabinPtrs)->classOfService());
      }

      if (!locInfo(trx, cabinPtrs) && !flightInfo(cabinPtrs))
        insertCabin(carrierExclCabins, (*cabinPtrs)->carrier(), (*cabinPtrs)->classOfService());
    }
  }
}
