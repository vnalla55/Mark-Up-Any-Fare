//-------------------------------------------------------------------
//
//  File:        HeaderMsgDiagSection.cpp
//  Author:      Konrad Koch
//  Created:     December 13, 2007
//  Description: Diagnostic for header messages in FQ
//
//  Copyright Sabre 2007
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

#include "FareDisplay/FDHeaderMsgController.h"

#include "Common/Logger.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FDHeaderMsg.h"
#include "DBAccess/Nation.h"
#include "DBAccess/State.h"
#include "FareDisplay/Templates/HeaderMsgDiagSection.h"

namespace tse
{
namespace
{
static const char SINGLE_CARRIER = 'S';
static const char MULTI_CARRIER = 'M';
static const std::string NOT_AVAILABLE = "N/A";
}

static Logger
logger("atseintl.FareDisplay.Templates.HeaderMsgDiagSection");

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  HeaderMsgDiagSection::buildDisplay()
//
// Gets all header messages valid for requested FQ
// and builds diagnostic output
//
// @return  void
//
// </PRE>
// -------------------------------------------------------------------
void
HeaderMsgDiagSection::buildDisplay()
{
  LOG4CXX_INFO(logger, "Entering HeaderMsgDiagSection::buildDisplay()");

  FDHeaderMsgController headerMsgController(_trx, _trx.preferredCarriers());

  // Get header messages for current transaction.
  std::vector<const FDHeaderMsg*> headerMsgs = headerMsgController.getFilteredHeaderMsgs();

  // Check if we got empty list.
  if (headerMsgs.empty())
  {
    LOG4CXX_INFO(logger, "No header messages!");
    // It'll display message "Diag 211 returned no data"
    return;
  }

  // Setup default output formating options.
  _trx.response() << std::left << std::setfill(' ');

  // Display diagnostic header message.
  _trx.response() << "FARE DISPLAY HEADER     MAX 5 MESSAGES DISPLAY ON SCREEN" << std::endl;
  _trx.response() << "TABLES-SPTFH/SPTFHS     FAREDISPLAYHDRMSG/FAREDISPLAYHDRMSGSEG" << std::endl;

  // Main loop. Iterate through all returned messages.
  std::vector<const FDHeaderMsg*>::iterator iterator = headerMsgs.begin();
  std::vector<const FDHeaderMsg*>::iterator end = headerMsgs.end();
  unsigned int msgCount = 1;

  while (iterator != end)
  {
    const FDHeaderMsg* msg = *iterator;

    // Just to be sure if we don't get null pointer.
    if (!msg)
    {
      LOG4CXX_ERROR(logger, "Null message received!");
      ++iterator;
      continue;
    }

    // Check if message count is greater than five
    if (msgCount > 5)
    {
      // Separator line
      _trx.response() << std::setfill('*') << std::setw(62) << "" << std::endl;

      // Display info messasge
      _trx.response() << "THE FOLLOWING MESSAGES DO NOT DISPLAY ON THE FQ SCREEN" << std::endl;
    }

    // Separator line
    _trx.response() << std::setfill('*') << std::setw(62) << "" << std::endl;

    // Message counter
    _trx.response() << std::setfill(' ') << msgCount << ". ";

    // User line
    buildUserLine(msg);

    // Message line
    _trx.response() << msg->text() << std::endl;

    // Sequence line
    _trx.response() << "SEQ NO - " << std::setfill(' ') << std::setw(9) << msg->seqNo()
                    << " MESSAGE NO - " << msg->msgItemNo() << std::endl;

    // FQ display type line
    buildFDTypeLine(msg);

    // Market line
    buildMarketLine(msg);

    // Sales validity line
    buildSaleValidityLine(msg);

    // Global direction line
    buildGlobalDirLine(msg);

    // Routing line
    buildRoutingLine(msg);

    // Start position line
    buildStartPositionLine(msg);

    // Create date line
    _trx.response() << "CREATE DATE - " << std::setw(15) << formatDate(msg->createDate());
    _trx.response() << " EXPIRE DATE - " << formatDate(msg->expireDate()) << std::endl;

    // Travel dates line
    _trx.response() << "TRAVEL EFFECTIVE - " << std::setw(10) << formatDate(msg->effDate());
    _trx.response() << " TRAVEL DISCONTINUE - " << formatDate(msg->discDate()) << std::endl;

    // Next message
    ++iterator;
    ++msgCount;
  }

  // Separator line
  _trx.response() << std::setfill('*') << std::setw(62) << "" << std::endl;
  _trx.response() << std::setfill(' ') << std::setw(21) << ""
                  << "END DIAGNOSTIC INFO" << std::endl;

  // Node information line
  _trx.response() << std::setw(6) << ""
                  << "NODE: ";

  char hostName[1024];

  if (::gethostname(hostName, 1023) < 0)
  {
    _trx.response() << "HOST NAME ERROR" << std::endl;
  }
  else
  {
    std::string text(hostName);
    // Make hostname uppercase
    std::transform(text.begin(), text.end(), text.begin(), (int (*)(int))toupper);
    _trx.response() << text << std::endl;
  }
  // Separator line
  _trx.response() << std::setfill('*') << std::setw(62) << "" << std::endl;

  LOG4CXX_INFO(logger, "Leaving HeaderMsgDiagSection::buildDisplay()");
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  HeaderMsgDiagSection::buildUserLine()
//
// Builds and renders user line
//
// @param msg pointer to FDHeaderMsg object
//
// @return  void
//
// </PRE>
// -------------------------------------------------------------------
void
HeaderMsgDiagSection::buildUserLine(const FDHeaderMsg* msg)
{
  if (msg->userApplType() == NO_PARAM)
  {
    if (msg->pseudoCityType() == PCCTYPE_BRANCH)
      _trx.response() << "PCC";
    else if (msg->pseudoCityType() == PCCTYPE_HOME)
      _trx.response() << "HOME PCC";

    _trx.response() << " USER: " << msg->pseudoCityCode();
  }
  else
  {
    if (msg->userApplType() == CRS_USER_APPL)
      _trx.response() << "CRS";
    else if (msg->userApplType() == MULTIHOST_USER_APPL)
      _trx.response() << "MULTIHOST";

    _trx.response() << " USER: " << msg->userAppl();
  }
  _trx.response() << std::endl;
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  HeaderMsgDiagSection::buildFDTypeLine()
//
// Builds and renders fare display type line
//
// @param msg pointer to FDHeaderMsg object
//
// @return  void
//
// </PRE>
// -------------------------------------------------------------------
void
HeaderMsgDiagSection::buildFDTypeLine(const FDHeaderMsg* msg)
{
  std::string text = "VALID ON ";
  bool displayCarrier = false;

  switch (msg->fareDisplayType())
  {
  case SINGLE_CARRIER:
    text += "SINGLE";
    displayCarrier = true;
    break;

  case MULTI_CARRIER:
    text += "MULTI";
    break;

  case BLANK:
    text += "SINGLE AND MULTI";
    break;

  default:
    LOG4CXX_ERROR(logger, "Invalid fare display type!");
  }

  text += " CARRIER DISPLAYS";
  _trx.response() << std::setw(46) << text << " CARRIER - ";

  if (displayCarrier && !msg->carrier().empty())
    _trx.response() << msg->carrier();
  else
    _trx.response() << NOT_AVAILABLE;

  _trx.response() << std::endl;
}

void
HeaderMsgDiagSection::buildMarketLine(const FDHeaderMsg* msg)
{
  _trx.response() << "APPLIES ";

  // Special case where both locations are type NONE
  if (msg->loc1().locType() == LOCTYPE_NONE && msg->loc2().locType() == LOCTYPE_NONE)
  {
    _trx.response() << "ANYWHERE";
  }
  else
  {
    if (msg->exceptLoc() == 'Y')
      _trx.response() << "ANYWHERE EXCEPT ";

    bool displaySecondLocation = true;

    switch (msg->directionality())
    {
    case BETWEEN:
      _trx.response() << "BETWEEN";
      break;

    case FROM:
      _trx.response() << "FROM";
      break;

    case WITHIN:
      _trx.response() << "WITHIN";
      displaySecondLocation = false;
      break;

    case BOTH:
      _trx.response() << "WORLDWIDE";
      displaySecondLocation = false;
      break;

    default:
      LOG4CXX_ERROR(logger, "Invalid directionality!");
    }

    _trx.response() << " " << formatLocation(msg->loc1().locType(), msg->loc1().loc());

    if (displaySecondLocation)
    {
      switch (msg->directionality())
      {
      case BETWEEN:
        _trx.response() << " AND";
        break;

      case FROM:
        _trx.response() << " TO";
        break;

      default:
        LOG4CXX_ERROR(logger, "Invalid directionality!");
      }

      _trx.response() << " " << formatLocation(msg->loc2().locType(), msg->loc2().loc());
    }

    if (msg->insideBufferZone() == 'Y')
      _trx.response() << " INSIDE BUFFER ZONE";
    else if (msg->outsideBufferZone() == 'Y')
      _trx.response() << " OUTSIDE BUFFER ZONE";
  }
  _trx.response() << std::endl;
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  HeaderMsgDiagSection::buildSaleValidityLine()
//
// Builds and renders sale validity line
//
// @param msg pointer to FDHeaderMsg object
//
// @return  void
//
// </PRE>
// -------------------------------------------------------------------
void
HeaderMsgDiagSection::buildSaleValidityLine(const FDHeaderMsg* msg)
{
  _trx.response() << "VALID WHEN SOLD";

  if (msg->posLocType() == 0 || msg->posLocType() == LOCTYPE_NONE)
  {
    _trx.response() << " ANYWHERE";
  }
  else
  {
    if (msg->exceptPosLoc() == 'Y')
    {
      _trx.response() << " ANYWHERE EXCEPT";
    }
    _trx.response() << " IN " << formatLocation(msg->posLocType(), msg->posLoc());
  }
  _trx.response() << std::endl;
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  HeaderMsgDiagSection::buildGlobalDirLine()
//
// Builds and renders global directionality line
//
// @param msg pointer to FDHeaderMsg object
//
// @return  void
//
// </PRE>
// -------------------------------------------------------------------
void
HeaderMsgDiagSection::buildGlobalDirLine(const FDHeaderMsg* msg)
{
  // Get global direction indicator
  std::string globalDir;

  // Convert global dir indicator to string
  globalDirectionToStr(globalDir, msg->globalDir());

  if (globalDir.empty())
    globalDir = NOT_AVAILABLE;

  // Get inclusion code
  std::string inclCode = msg->inclusionCode();

  if (inclCode.empty())
    inclCode = NOT_AVAILABLE;

  // Get currency
  std::string currency = msg->cur();

  if (msg->cur().empty())
  {
    currency = NOT_AVAILABLE;
  }
  else
  {
    if (msg->exceptCur() == 'Y')
      currency = "X " + currency;
  }

  // Build line
  _trx.response() << "GI - " << std::setw(18) << globalDir;
  _trx.response() << " INCL CODE - " << std::setw(12) << inclCode;
  _trx.response() << " CURR - " << currency << std::endl;
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  HeaderMsgDiagSection::buildRoutingLine()
//
// Builds and renders routing line
//
// @param msg pointer to FDHeaderMsg object
//
// @return  void
//
// </PRE>
// -------------------------------------------------------------------
void
HeaderMsgDiagSection::buildRoutingLine(const FDHeaderMsg* msg)
{
  // Get routing tariff
  std::string routingTariff;

  if (msg->routingTariff() == 0)
    routingTariff = NOT_AVAILABLE;
  else
  {
    // Convert routing tariff number to string
    std::ostringstream oss;

    oss << msg->routingTariff();
    routingTariff = oss.str();
  }
  // Get routing range
  std::string routingRange1 = msg->routing1();
  bool routingRangeEmpty = true;

  if (routingRange1.empty())
    routingRange1 = NOT_AVAILABLE;
  else
    routingRangeEmpty = false;

  // Get routing range2
  std::string routingRange2 = msg->routing2();

  if (routingRange2.empty())
    routingRange2 = NOT_AVAILABLE;
  else
    routingRangeEmpty = false;

  // Build line
  _trx.response() << "ROUTING TAR - " << std::setw(15) << routingTariff << " ROUTING RANGE - ";

  if (routingRangeEmpty)
    _trx.response() << NOT_AVAILABLE;
  else
    _trx.response() << std::setfill('0') << std::setw(3) << routingRange1 << " - " << routingRange2
                    << std::setfill(' ');

  _trx.response() << std::endl;
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  HeaderMsgDiagSection::buildStartPositionLine()
//
// Builds and renders start position line
//
// @param msg pointer to FDHeaderMsg object
//
// @return  void
//
// </PRE>
// -------------------------------------------------------------------
void
HeaderMsgDiagSection::buildStartPositionLine(const FDHeaderMsg* msg)
{
  std::string startPoint = msg->startPoint();
  std::string justify;

  if (startPoint.empty())
  {
    startPoint = NOT_AVAILABLE;
    justify = NOT_AVAILABLE;
  }
  else
  {
    int value = 0;
    std::istringstream iss(startPoint);

    // Convert string to int
    iss >> value;

    if (value)
    {
      justify = NOT_AVAILABLE;
    }
    else
    {
      justify = startPoint;
      startPoint = NOT_AVAILABLE;
    }
  }

  _trx.response() << "START IN POSITION " << std::setw(11) << startPoint;
  _trx.response() << " JUSTIFY - " << justify << std::endl;
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  HeaderMsgDiagSection::formatLocation()
//
// Translates location type and code to string
//
// @param locType type of location
// @param locCode location code
//
// @return string representation of location
//
// </PRE>
// -------------------------------------------------------------------
std::string
HeaderMsgDiagSection::formatLocation(const Indicator& locType, const LocCode& locCode)
{
  std::string text;

  switch (locType)
  {
  case LOCTYPE_AREA:
    text = "AREA " + locCode;
    break;

  case LOCTYPE_SUBAREA:
    text = "SUBAREA " + locCode;
    break;

  case LOCTYPE_CITY:
    text += locCode;
    break;

  case LOCTYPE_NATION:
  {
    // Special requirement for space purposes (abbreviate United States to U.S.)
    if (locCode == NATION_US)
    {
      text = "U.S.";
    }
    else
    {
      const Nation* nation = _trx.dataHandle().getNation(locCode, _trx.travelDate());

      if (nation)
        text = nation->description();
      else
        text = NOT_AVAILABLE;
    }
  }
  break;

  case LOCTYPE_STATE:
  {
    // State is coded NNSS (NN - nation, SS - state ex.:USTX)
    NationCode nationCode(locCode.substr(0, 2));
    StateCode stateCode(locCode.substr(2, 2));

    const State* state = _trx.dataHandle().getState(nationCode, stateCode, _trx.travelDate());

    if (state)
      text = state->description();
    else
      text = NOT_AVAILABLE;
  }
  break;

  case LOCTYPE_ZONE:
    text = "ZONE " + locCode;
    break;

  case LOCTYPE_NONE:
    text = "ANYWHERE";
    break;

  default:
    text = NOT_AVAILABLE;
  }
  return text;
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  HeaderMsgDiagSection::formatDate()
//
// Formats date accoring to diagnostic requirements
//
// @param date date to format
//
// @return string representation of date
//
// </PRE>
// -------------------------------------------------------------------
std::string
HeaderMsgDiagSection::formatDate(const DateTime& date)
{
  return date.isInfinity() ? "INFINITY" : date.dateToString(DDMMMYYYY, nullptr);
}
} // tse namespace
