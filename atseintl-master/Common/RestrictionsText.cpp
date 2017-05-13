//----------------------------------------------------------------------------
//  Copyright Sabre 2009
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
#include "Common/RestrictionsText.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Routing.h"
#include "DBAccess/RoutingRestriction.h"
#include "Routing/RoutingConsts.h"

namespace tse
{

namespace
{
const std::string OR = "OR";
}

//--------------------------------------------------------
//  RESTRICTION 1
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE VIA CITY
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST NOT BE VIA CITY
//--------------------------------------------------------
void
RestrictionsText::restriction1(const RoutingRestriction& restriction, const bool& indent)
{
  std::ostringstream ostr;
  checkIndent(indent);
  if (restriction.negViaAppl() == PERMITTED)
  {
    ostr << "RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED";
  }
  else if (restriction.negViaAppl() == BLANK)
  {
    ostr << "RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK";
  }
  else
  {
    ostr << "TRAVEL BETWEEN " << restriction.market1() << " AND " << restriction.market2()
         << mustBeText(restriction.negViaAppl()) << " VIA " << restriction.viaMarket();
  }
  finalizeRestrictionFunction(ostr);
}

void
RestrictionsText::restriction1(RtgRestCI i, RtgRestCI e, const bool& indent)
{
  std::ostringstream ostr;
  checkIndent(indent);
  RoutingRestriction& restriction(**i);
  if (restriction.negViaAppl() == PERMITTED)
  {
    ostr << "RESTRICTION IGNORED - APPLICATION INDICATOR IS PERMITTED";
  }
  else if (restriction.negViaAppl() == BLANK)
  {
    ostr << "RESTRICTION IGNORED - APPLICATION INDICATOR IS BLANK";
  }
  else
  {
    ostr << "TRAVEL BETWEEN " << restriction.market1() << " AND " << restriction.market2()
         << mustBeText(restriction.negViaAppl()) << " VIA " << restriction.viaMarket()
         << displayCities(++i, e, OR);
  }
  finalizeRestrictionFunction(ostr);
}

//--------------------------------------------------------
//  RESTRICTION 2
//  TRAVEL MUST BE VIA CITY
//  TRAVEL MUST NOT BE VIA CITY
//--------------------------------------------------------
void
RestrictionsText::restriction2(const RoutingRestriction& restriction, const bool& indent)
{
  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "TRAVEL" << mustBeText(restriction.negViaAppl()) << " VIA " << restriction.viaMarket();
  finalizeRestrictionFunction(ostr);
}

void
RestrictionsText::restriction2(RtgRestCI i, RtgRestCI e, const bool& indent)
{
  std::ostringstream ostr;
  checkIndent(indent);
  RoutingRestriction& restriction(**i);
  ostr << "TRAVEL" << mustBeText(restriction.negViaAppl()) << " VIA " << restriction.viaMarket()
       << displayCities(++i, e, OR);
  finalizeRestrictionFunction(ostr);
}

//--------------------------------------------------------
//  RESTRICTION 3
//  TRAVEL MUST BE NONSTOP
//  TRAVEL MUST NOT BE NONSTOP
//  TRAVEL MUST BE DIRECT
//  TRAVEL MUST NOT BE DIRECT
//  TRAVEL MUST BE NONSTOP OR DIRECT
//  TRAVEL MUST NOT BE NONSTOP OR DIRECT
//--------------------------------------------------------
void
RestrictionsText::restriction3(const RoutingRestriction& restriction, const bool& indent)
{
  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "TRAVEL" << mustBeText(restriction.negViaAppl())
       << nonStopText(restriction.nonStopDirectInd());
  finalizeRestrictionFunction(ostr);
}

//--------------------------------------------------------
//  RESTRICTION 4
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE NONSTOP
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST NOT BE NONSTOP
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE DIRECT
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST NOT BE DIRECT
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST BE NONSTOP OR DIRECT
//  TRAVEL BETWEEN CITY1 AND CITY2 MUST NOT BE NONSTOP OR DIRECT
//--------------------------------------------------------
void
RestrictionsText::restriction4(const RoutingRestriction& restriction, const bool& indent)
{
  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "TRAVEL BETWEEN " << restriction.market1() << " AND " << restriction.market2()
       << mustBeText(restriction.negViaAppl()) << nonStopText(restriction.nonStopDirectInd());
  finalizeRestrictionFunction(ostr);
}

//--------------------------------------------------------
//  RESTRICTION 5
//  TRAVEL TO/FROM CITY1 MUST BE VIA CITY3
//  TRAVEL TO/FROM CITY1 MUST NOT BE VIA CITY3
//--------------------------------------------------------
void
RestrictionsText::restriction5(const RoutingRestriction& restriction, const bool& indent)
{
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);

  ostr << "TRAVEL TO/FROM " << restriction.market1() << mustBeText(restriction.negViaAppl())
       << " VIA " << restriction.viaMarket();
  finalizeRestrictionFunction(ostr);
}

void
RestrictionsText::restriction5(RtgRestCI i, RtgRestCI e, const bool& indent)
{
  RoutingRestriction& restriction(**i);
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);

  ostr << "TRAVEL TO/FROM " << restriction.market1() << mustBeText(restriction.negViaAppl())
       << " VIA " << restriction.viaMarket() << displayCities(++i, e, OR);
  finalizeRestrictionFunction(ostr);
}

//--------------------------------------------------------
//  RESTRICTION 6
//  TRAVEL TO/FROM CITY1 MUST BE NONSTOP
//  TRAVEL TO/FROM CITY1 MUST NOT BE NONSTOP
//  TRAVEL TO/FROM CITY1 MUST BE DIRECT
//  TRAVEL TO/FROM CITY1 MUST NOT BE DIRECT
//  TRAVEL TO/FROM CITY1 MUST BE NONSTOP OR DIRECT
//  TRAVEL TO/FROM CITY1 MUST NOT BE NONSTOP OR DIRECT
//--------------------------------------------------------
void
RestrictionsText::restriction6(const RoutingRestriction& restriction, const bool& indent)
{
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);

  ostr << "TRAVEL TO/FROM " << restriction.market1() << mustBeText(restriction.negViaAppl())
       << nonStopText(restriction.nonStopDirectInd());
  finalizeRestrictionFunction(ostr);
}

//----------------------------------------------------------
//  RESTRICTION 7
//  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 REQUIRED
//  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 PERMITTED
//  BETWEEN CITY1 AND CITY2 STOPOVER IN CITY3 NOT PERMITTED
//----------------------------------------------------------
void
RestrictionsText::restriction7(const RoutingRestriction& restriction, const bool& indent)
{
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);

  ostr << "BETWEEN " << restriction.market1() << " AND " << restriction.market2() << " STOPOVER IN "
       << restriction.viaMarket() << permReqText(restriction.negViaAppl());
  finalizeRestrictionFunction(ostr);
}

void
RestrictionsText::restriction7(RtgRestCI i, RtgRestCI e, const bool& indent)
{
  RoutingRestriction& restriction(**i);
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);

  ostr << "BETWEEN " << restriction.market1() << " AND " << restriction.market2() << " STOPOVER IN "
       << restriction.viaMarket() << displayCities(++i, e, OR)
       << permReqText(restriction.negViaAppl());
  finalizeRestrictionFunction(ostr);
}

void
RestrictionsText::restriction8FMR(const FareDisplayTrx& trx,
                                  const RoutingRestriction& restriction,
                                  const bool& indent)
{
  if (!isRtwOrCt(trx))
    return;

  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "MUST NOT EXCEED MPM " << restriction.mpm();
  finalizeRestrictionFunction(ostr);
}

//----------------------------------------------------------
//  RESTRICTION 8
//  STOPOVER IN CITY3 REQUIRED
//  STOPOVER IN CITY3 NOT PERMITTED
//  STOPOVER IN CITY3 PERMITTED
//----------------------------------------------------------
void
RestrictionsText::restriction8(const RoutingRestriction& restriction, const bool& indent)
{
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "STOPOVER IN " << restriction.viaMarket() << permReqText(restriction.negViaAppl());
  finalizeRestrictionFunction(ostr);
}

void
RestrictionsText::restriction9FMR(const FareDisplayTrx& trx, const bool& indent)
{
  if (!isRtwOrCt(trx))
    return;

  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "TRAVEL IS NOT PERMITTED VIA THE FARE ORIGIN";
  finalizeRestrictionFunction(ostr);
}

//-------------------------------------------------------------------
//  RESTRICTION 10
//  CHANGE OF AIRCRAFT IN CITY3 REQUIRED
//  CHANGE OF AIRCRAFT IN CITY3 PERMITTED
//  CHANGE OF AIRCRAFT IN CITY3 NOT PERMITTED
//-------------------------------------------------------------------
void
RestrictionsText::restriction10(const RoutingRestriction& restriction, const bool& indent)
{
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "CHANGE OF AIRCRAFT IN " << restriction.viaMarket()
       << permReqText(restriction.negViaAppl());
  finalizeRestrictionFunction(ostr);
}

void
RestrictionsText::restriction10(RtgRestCI i, RtgRestCI e, const bool& indent)
{
  RoutingRestriction& restriction(**i);
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "CHANGE OF AIRCRAFT IN " << restriction.viaMarket() << displayCities(++i, e, OR)
       << permReqText(restriction.negViaAppl());
  finalizeRestrictionFunction(ostr);
}

//-------------------------------------------------------------------
//  RESTRICTION 11
//  BETWEEN CITY1 AND CITY2 AIR SECTOR REQUIRED
//  BETWEEN CITY1 AND CITY2 AIR SECTOR PERMITTED
//  BETWEEN CITY1 AND CITY2 AIR SECTOR NOT PERMITTED
//  BETWEEN CITY1 AND CITY2 SURFACE SECTOR REQUIRED
//  BETWEEN CITY1 AND CITY2 SURFACE SECTOR PERMITTED
//  BETWEEN CITY1 AND CITY2 SURFACE SECTOR NOT PERMITTED
//-------------------------------------------------------------------
void
RestrictionsText::restriction11(const RoutingRestriction& restriction, const bool& indent)
{
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "BETWEEN " << restriction.market1() << " AND " << restriction.market2();
  if (restriction.airSurfaceInd() == AIR)
    ostr << " AIR SECTOR";
  else if (restriction.airSurfaceInd() == SURFACE)
    ostr << " SURFACE SECTOR";
  else
    ostr << " AIR OR SURFACE SECTOR";
  ostr << permReqText(restriction.negViaAppl());
  finalizeRestrictionFunction(ostr);
}

//-------------------------------------------------------------------
//  RESTRICTION 12
//  MAXIMUM PERMITTED MILEAGE TO/FROM CITY1
//-------------------------------------------------------------------
void
RestrictionsText::restriction12(const RoutingRestriction& restriction, const bool& indent)
{
  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "MAXIMUM PERMITTED MILEAGE "
       << "TO/FROM " << restriction.market1();
  finalizeRestrictionFunction(ostr);
}

//-------------------------------------------------------------------
//  RESTRICTION 16
//  MILEAGE SYSTEM APPLIES ORIGIN TO DESTINATION
//-------------------------------------------------------------------
void
RestrictionsText::restriction16(const RoutingRestriction& restriction, const bool& indent)
{
  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "MILEAGE SYSTEM APPLIES ORIGIN TO DESTINATION";
  finalizeRestrictionFunction(ostr);
}

//-------------------------------------------------------------------
//  RESTRICTION 17
//  CARRIER LISTING ONLY
//-------------------------------------------------------------------
void
RestrictionsText::restriction17(const RoutingRestriction& restriction, const bool& indent)
{
  std::ostringstream ostr;
  checkIndent(indent);
  if (restriction.viaCarrier().empty())
    ostr << "CARRIER LISTING ONLY";
  else
    ostr << restriction.viaCarrier() << " ONLY";
  finalizeRestrictionFunction(ostr);
}

void
RestrictionsText::restriction17(RtgRestCI i, RtgRestCI e, const bool& indent)
{
  RoutingRestriction& restriction(**i);
  std::ostringstream ostr;
  checkIndent(indent);
  if (restriction.viaCarrier().empty())
    ostr << "CARRIER LISTING ONLY";
  else
    ostr << restriction.viaCarrier() << displayCarriers(++i, e, OR) << " ONLY";
  finalizeRestrictionFunction(ostr);
}

//-------------------------------------------------------------------
//  RESTRICTION 21
//  WHEN ORIGIN IS CTY AND DEST IS CTY TRAVEL MUST BE VIA CITY3
//  WHEN ORIGIN IS CTY AND DEST IS CTY TRAVEL MUST NOT BE VIA CITY3
//  123456789012345678901234567890123456789012345678901234567890123
//-------------------------------------------------------------------
void
RestrictionsText::restriction21(const RoutingRestriction& restriction, const bool& indent)
{
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "WHEN ORIGIN IS " << restriction.market1() << " AND DEST IS " << restriction.market2()
       << " TRAVEL" << mustBeText(restriction.negViaAppl()) << " VIA " << restriction.viaMarket();
  finalizeRestrictionFunction(ostr);
}

void
RestrictionsText::restriction21(RtgRestCI i, RtgRestCI e, const bool& indent)
{
  RoutingRestriction& restriction(**i);
  if (restriction.negViaAppl() == BLANK)
    return;

  std::ostringstream ostr;
  checkIndent(indent);
  ostr << "WHEN ORIGIN IS " << restriction.market1() << " AND DEST IS " << restriction.market2()
       << " TRAVEL" << mustBeText(restriction.negViaAppl()) << " VIA " << restriction.viaMarket()
       << displayCities(++i, e, OR);
  finalizeRestrictionFunction(ostr);
}
void
RestrictionsText::checkIndent(bool indent)
{
  if (indent)
    _oss << "       ";
  else
    _oss << " ";
}
std::string
RestrictionsText::displayCities(const RtgRestCI& it, const RtgRestCI& ie, const std::string& conj)
{
  std::ostringstream ret;
  for (RtgRestCI i = it; i != ie; i++)
    ret << " " << conj << " " << (*i)->viaMarket();
  return ret.str();
}
std::string
RestrictionsText::displayCarriers(const RtgRestCI& it, const RtgRestCI& ie, const std::string& conj)
{
  std::ostringstream ret;
  for (RtgRestCI i = it; i != ie; i++)
    ret << " " << conj << " " << (*i)->viaCarrier();
  return ret.str();
}
std::string
RestrictionsText::permReqText(Indicator negViaAppl)
{
  if (negViaAppl == PERMITTED)
    return " PERMITTED";
  else if (negViaAppl == REQUIRED)
    return " REQUIRED";
  return " NOT PERMITTED";
}
std::string
RestrictionsText::mustBeText(Indicator negViaAppl)
{
  if (negViaAppl == PERMITTED || negViaAppl == REQUIRED)
    return " MUST BE";
  return " MUST NOT BE";
}
std::string
RestrictionsText::nonStopText(Indicator nonStopDirectInd)
{
  if (nonStopDirectInd == NONSTOP)
    return " NONSTOP";
  else if (nonStopDirectInd == DIRECT)
    return " DIRECT";
  return " NONSTOP OR DIRECT";
}
void
RestrictionsText::finalizeRestrictionFunction(std::ostringstream& ostr)
{
  const size_t MAX_LINE_LENGTH = 57; // 63 - left margin(7) + 1(space)
  size_t lineLen = ostr.str().length();
  // if no split is needed, do nothing
  if (!_lineWrap || lineLen <= MAX_LINE_LENGTH)
  {
    _oss << ostr.str() << std::endl;
  }
  else
  {
    const std::string& str(ostr.str());
    std::string splitStr = " "; // no terminal chars or line breakdown, so just break on space
    std::string::const_iterator it = str.begin();
    std::string::const_iterator ie = it + MAX_LINE_LENGTH;
    std::string::const_iterator i = it;
    while (i != str.end())
    {
      if (ie == str.end()) // if last line, then wohole string
        i = str.end();
      else // else find substring
        i = std::find_end(it, ie, splitStr.begin(), splitStr.end());
      // put result to stream
      _oss << str.substr(it - str.begin(), i - it) << std::endl;
      if (i != str.end()) // if not last substring, then add left margin spaces, and set iteratirs
      {
        _oss << "       ";
        it = i + 1;
        ie = (size_t)(str.end() - it) > MAX_LINE_LENGTH ? (it + MAX_LINE_LENGTH) : str.end();
      }
    }
  }
}

bool
RestrictionsText::isRtwOrCt(const FareDisplayTrx& trx)
{
  return trx.offMultiCity() == trx.boardMultiCity();
}

} // namespace
