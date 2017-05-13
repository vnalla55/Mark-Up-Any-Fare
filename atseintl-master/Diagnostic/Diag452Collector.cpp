//-----------------------------------------------------------------------------
//
//  File:     Diag452Collector.cpp
//
//  Author :  Slawek Machowicz
//
//  Copyright Sabre 2009
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "Diagnostic/Diag452Collector.h"

#include "DBAccess/TPMExclusion.h"
#include "Routing/SouthAtlanticTPMExclusion.h"

using namespace std;

namespace tse
{
namespace
{
const string DIAG452FIELDS[] = {
  "MATCHED",             "CRS",                "GOV CXR",           "SEQ NO",      "NOT APPL TO YY",
  "ONLINE SERVICE ONLY", "DIR FARE COMPONENT", "FC LOC 1",          "FC LOC 2",    "GI",
  "SR 1 APPL",           "SR 1 LOC 1",         "SR 1 LOC 2",        "SR 2 APPL",   "SR 2 LOC 1",
  "SR 2 LOC 2",          "VIA POINT RES",      "CSR ONLINE ON GOV", "SURFACE PER", "CREATE DATE",
  "FIRST SALE DATE",     "LAST SALE DATE",     "EXPIRE DATE",       "SR 1",        "SR 2",
  "MULTIHOST"
};

const int LINE_LENGTH = 63;
const int COLON_POS = 20;
}

// displays header of diagnostic
void
Diag452Collector::printHeader()
{
  if (_active)
  {
    *this << "*******************  TPM EXCLUSION TABLE  *********************\n";
  }
}

// displays footer of diagnostic
void
Diag452Collector::printFooter()
{
  if (_active)
  {
    if (_separatorNeeded)
      printLine();

    *this << "TPM EXCLUSION IS ";
    if (!_matched)
      *this << "NOT ";
    *this << "APPLIED\n";

    printLine();
  }
}

// prints one record from database
Diag452Collector&
  Diag452Collector::operator << (const TPMExclusion& tpmExclusion)
{
  if (_active)
  {
    printLineTitle(SouthAtlanticTPMExclusion::FAILED_GOV_CXR);
    *this << tpmExclusion.carrier() << "\n";

    printCrsMultihost(tpmExclusion);

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_SEQ_NO);
    *((DiagCollector*)this) << (tpmExclusion.seqNo()) << "\n";

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_NOT_APPL_TO_YY);
    *this << tpmExclusion.notApplToYY() << "\n";

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_ONLINE_SERVICE_ONLY);
    *this << tpmExclusion.onlineSrvOnly() << "\n";

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_DIR_FARE_COMPONENT);
    printDirectionality(tpmExclusion.directionality());
    *this << "\n";

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_FC_LOC_1);
    decodeLoc(tpmExclusion.loc1type(), tpmExclusion.loc1());

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_FC_LOC_2);
    decodeLoc(tpmExclusion.loc2type(), tpmExclusion.loc2());

    globalDirectionToStr(_gd, tpmExclusion.globalDir());
    printLineTitle(SouthAtlanticTPMExclusion::FAILED_GI);
    *this << _gd << "\n";

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_SR_1_APPL);
    *this << tpmExclusion.sec1Appl() << "\n";

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_SR_1_LOC_1);
    decodeLoc(tpmExclusion.sec1Loc1Type(), tpmExclusion.sec1Loc1());

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_SR_1_LOC_2);
    decodeLoc(tpmExclusion.sec1Loc2Type(), tpmExclusion.sec1Loc2());

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_SR_2_APPL);
    *this << tpmExclusion.sec2Appl() << "\n";

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_SR_2_LOC_1);
    decodeLoc(tpmExclusion.sec2Loc1Type(), tpmExclusion.sec2Loc1());

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_SR_2_LOC_2);
    decodeLoc(tpmExclusion.sec2Loc2Type(), tpmExclusion.sec2Loc2());

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_VIA_POINT_RES);
    *this << tpmExclusion.viaPointRest() << "\n";

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_CSR_ONLINE_ON_GOV);
    *this << tpmExclusion.consecMustBeOnGovCxr() << "\n";

    printLineTitle(SouthAtlanticTPMExclusion::FAILED_SURFACE_PER);
    *this << tpmExclusion.surfacePermitted() << "\n";

    printDates(tpmExclusion);
    _separatorNeeded = true;
  }
  return *this;
}

// displays result of matching and separator
Diag452Collector&
  Diag452Collector::operator << (SouthAtlanticTPMExclusion::TPMExclusionFailCode failCode)
{
  if (_active)
  {
    *this << "VALIDATION RESULT:   ";

    if (failCode == SouthAtlanticTPMExclusion::MATCHED)
      _matched = true;
    else
      *this << "FAIL - ";
    *this << DIAG452FIELDS[failCode] << '\n';

    printLine();
    _separatorNeeded = false;
  }
  return *this;
}

void
Diag452Collector::printFareMarketHeader(const MileageRoute& mRoute)
{
  if (_active)
  {
    _ticketingDT = mRoute.ticketingDT();
    displayMileageRouteItems(mRoute.mileageRouteItems());
    globalDirectionToStr(_gd, mRoute.globalDirection());
    *this << "     /CXR-" << mRoute.governingCarrier() << "/GI-" << _gd;
    *this << " DIR-" << (mRoute.isOutbound() ? "OUT" : "IN") << "\n";
    printLine();
  }
}

// displays global direction in current line
void
Diag452Collector::displayMileageRouteItems(const MileageRouteItems& mrItems)
{
  std::vector<MileageRouteItem>::const_iterator t = mrItems.begin();
  std::vector<MileageRouteItem>::const_iterator end = mrItems.end();

  if (t < end)
  {
    *this << SouthAtlanticTPMExclusion::multiCityOrig(*t);

    while (t < end)
    {
      if (t->isSurface())
      {
        *this << " // ";
      }
      else
      {
        *this << '-' << t->segmentCarrier() << '-';
      }
      *this << SouthAtlanticTPMExclusion::multiCityDest(*t);
      t++;
    }
  }
}

void
Diag452Collector::printLineTitle(SouthAtlanticTPMExclusion::TPMExclusionFailCode failCode)
{
  *this << DIAG452FIELDS[failCode] << setw(COLON_POS - DIAG452FIELDS[failCode].length()) << ':';
}

void
Diag452Collector::printDirectionality(Directionality dir)
{
  switch (dir)
  {
  case FROM:
    *this << "FROM";
    break;
  case BETWEEN:
    *this << "BETWEEN";
    break;
  default:
    break;
  }
}

void
Diag452Collector::printDates(const TPMExclusion& tpmExclusion)
{
  printLineTitle(SouthAtlanticTPMExclusion::FAILED_CREATE_DATE);
  *this << tpmExclusion.createDate().dateToString(DDMMMYYYY, "") << "\n";

  printLineTitle(SouthAtlanticTPMExclusion::FAILED_FIRST_SALE_DATE);
  *this << tpmExclusion.effDate().dateToString(DDMMMYYYY, "") << "\n";

  printLineTitle(SouthAtlanticTPMExclusion::FAILED_LAST_SALE_DATE);
  *this << tpmExclusion.discDate().dateToString(DDMMMYYYY, "") << "\n";

  printLineTitle(SouthAtlanticTPMExclusion::FAILED_EXPIRE_DATE);
  *this << tpmExclusion.expireDate().dateToString(DDMMMYYYY, "") << "\n";
}

void
Diag452Collector::printCrsMultihost(const TPMExclusion& tpmExclusion)
{
  if (rootDiag()->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL")
  {
    printLineTitle(SouthAtlanticTPMExclusion::FAILED_CRS);
    if (tpmExclusion.userApplType() == CRS_USER_APPL)
      *this << tpmExclusion.userAppl();
    *this << '\n';
    printLineTitle(SouthAtlanticTPMExclusion::FAILED_MULTIHOST);
    if (tpmExclusion.userApplType() == MULTIHOST_USER_APPL)
      *this << tpmExclusion.userAppl();
    *this << '\n';
  }
}

// decodes Location
void
Diag452Collector::decodeLoc(LocTypeCode locType, const LocCode& locCode, int& pos)
{
  const ZoneInfo* zoneInfo = nullptr;
  static const string area("AREA ");
  static const string subarea("SUBAREA ");

  switch (locType)
  {
  case IATA_AREA:
    printTextWithNewLine(area, pos);
    printTextWithNewLine(locCode, pos);
    break;
  case SUBAREA:
    printTextWithNewLine(subarea, pos);
    printTextWithNewLine(locCode, pos);
    break;
  case MARKET:
  case NATION:
  case STATE_PROVINCE:
    printTextWithNewLine(locCode, pos);
    break;
  case ZONE:
    zoneInfo = getZoneInfo(locCode);
    if (zoneInfo)
      printZone(*zoneInfo, pos);
    break;
  default:
    break;
  }
}

void
Diag452Collector::decodeLoc(LocTypeCode locType, const LocCode& locCode)
{
  int pos = COLON_POS;
  decodeLoc(locType, locCode, pos);
  *this << "\n";
}

void
Diag452Collector::printZone(const ZoneInfo& zone, int& pos)
{
  static const string separator(", ");
  static const string zoneTitle("ZONE ");
  bool first = true;

  std::vector<std::vector<ZoneInfo::ZoneSeg> >::const_iterator i = zone.sets().begin();
  std::vector<std::vector<ZoneInfo::ZoneSeg> >::const_iterator ie = zone.sets().end();
  std::vector<ZoneInfo::ZoneSeg>::const_iterator j;

  // if any records is excluded print only short info
  for (i = zone.sets().begin(); i != ie; ++i)
  {
    j = i->begin();
    for (; j != i->end(); ++j)
    {
      if (j->inclExclInd() == 'E')
      {
        printTextWithNewLine(zoneTitle, pos);
        printTextWithNewLine(zone.zone(), pos);
        return;
      }
    }
  }

  for (i = zone.sets().begin(); i != ie; ++i)
  {
    j = i->begin();
    for (; j != i->end(); ++j)
    {
      if (!first)
      {
        printTextWithNewLine(separator, pos);
      }
      decodeLoc(j->locType(), j->loc(), pos);
      first = false;
    }
  }
}

void
Diag452Collector::printTextWithNewLine(const std::string& text, int& pos)
{
  const static string newLineString = "\n          ";

  if (pos + (int)text.length() > LINE_LENGTH)
  {
    *this << newLineString;
    pos = 10;
  }
  *this << text;
  pos += text.length();
}

const ZoneInfo*
Diag452Collector::getZoneInfo(const LocCode& locCode)
{
  Zone zone(locCode);
  LocUtil::padZoneNo(zone);
  return _trx->dataHandle().getZone(SABRE_USER, zone, MANUAL, _ticketingDT);
}
}
