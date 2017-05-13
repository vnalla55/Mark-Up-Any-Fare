//-----------------------------------------------------------------------------
//
//  File:     Diag212CollectorFD.C
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "Diagnostic/Diag212CollectorFD.h"

#include "Common/Assert.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FDSuppressFare.h"
#include "DBAccess/Nation.h"

using namespace std;

namespace tse
{
void
Diag212CollectorFD::init(FareDisplayTrx& trx)
{
  _isMainPcc = false;
  _isAgntPcc = false;
  _isTjrGrNo = false;
  _fDataMatch = false;
  _fFinishProc = false;
  _fMatchFound = PS_MATCH_NOT_FOUND;
  _fHeaderDisplayed = false;
  _agentPcc = trx.getRequest()->ticketingAgent()->tvlAgencyPCC();
  _mainPcc = trx.getRequest()->ticketingAgent()->mainTvlAgencyPCC();
  _tjrGroup = trx.getRequest()->ticketingAgent()->tjrGroup();
  _fdTrx = &trx;
}

void
Diag212CollectorFD::initParam(Diagnostic& root)
{
  _fdTrx = dynamic_cast<FareDisplayTrx*>(trx());

  TSE_ASSERT(_fdTrx);

  _isMainPcc = false;
  _isAgntPcc = false;
  _isTjrGrNo = false;
  _fDataMatch = false;
  _fFinishProc = false;
  _fMatchFound = PS_MATCH_NOT_FOUND;
  _fHeaderDisplayed = false;

  _agentPcc = _fdTrx->getRequest()->ticketingAgent()->tvlAgencyPCC();
  _mainPcc = _fdTrx->getRequest()->ticketingAgent()->mainTvlAgencyPCC();
  _tjrGroup = _fdTrx->getRequest()->ticketingAgent()->tjrGroup();
}
// ----------------------------------------------------------------------------
void
Diag212CollectorFD::printHeader()
{
  if (!_active)
    return;

  ostringstream header;
  header << std::setfill('*') << std::setw(45) << "  FARE DISPLAY SUPPRESSION  " << std::setw(17)
         << '*';
  header << "\n"
         << "TABLES-SPTFSU                                 FAREDISPSUPPRESS"
         << "\n";
  *this << "\n" << header.str() << "\n";

  if (_agentPcc.empty() && _mainPcc.empty() && (_tjrGroup == 0))
  {
    *this << "DIAGNOSTIC 212 ONLY VALID FOR PCC, HOME PCC OR TJR GROUP NO."
          << "\n";
    _fFinishProc = true;
    return;
  }

  if (_isMainPcc || _isAgntPcc || _isTjrGrNo)
    return;

  *this << "NO RECORD EXISTS FOR THIS PSEUDO CITY CODE"
        << "\n";
  _fFinishProc = true;
}
// ----------------------------------------------------------------------------
void
Diag212CollectorFD::printFooter()
{
  if (!_active)
    return;

  ostringstream footer;
  footer << std::setfill('*') << std::setw(62) << '*';

  *this << footer.str() << "\n";
  *this << "                    END DIAGNOSTIC INFO"
        << "\n"
        << "      NODE: ";

  char hostName[1024];

  if (::gethostname(hostName, 1023) < 0)
  {
    // cout << "hostName: '" << hostName << "'";
    *this << "HOST NAME ERROR"
          << "\n";
  }
  else
  {
    string aString(hostName);
    std::transform(aString.begin(), aString.end(), aString.begin(), (int (*)(int))toupper);
    *this << aString << "\n";
  }
  *this << footer.str() << "\n";
}
// ----------------------------------------------------------------------------
void
Diag212CollectorFD::printAgentPccSuppFares(const std::vector<const FDSuppressFare*>& fares,
                                           bool fMatch)
{
  if (_isMainPcc)
    printSuppresFares(SFS_AGENTPCC, fares, fMatch);
}
// ----------------------------------------------------------------------------
void
Diag212CollectorFD::printMainPccSuppFares(const std::vector<const FDSuppressFare*>& fares,
                                          bool fMatch)
{
  if (_isAgntPcc)
    printSuppresFares(SFS_MAINPCC, fares, fMatch);
}
// ----------------------------------------------------------------------------
void
Diag212CollectorFD::printTjrGroupNoSuppFares(const std::vector<const FDSuppressFare*>& fares,
                                             bool fMatch)
{
  if (_isTjrGrNo)
    printSuppresFares(SFS_TJRGROUPNO, fares, fMatch);
}
// ----------------------------------------------------------------------------
void
Diag212CollectorFD::printSuppresFares(RecordSource src,
                                      const std::vector<const FDSuppressFare*>& fares,
                                      bool fMatch)
{
  if (!_active || _fFinishProc)
    return;

  addSuppressFares(fares);
  // check if match was done
  if (_fMatchFound > PS_MATCH_NOT_FOUND)
  {
    _fMatchFound = PS_NOT_PROCESSED;
    _fDataMatch = true;
  }
  else
  {
    _fMatchFound = PS_MATCH_FOUND;
    _fDataMatch = fMatch;
  }
  printHeader(src);
  printSuppressFares();
  if (!_fDataMatch)
    *this << setw(62) << " "
          << "\n"
          << "*** DATA DID NOT APPLY TO REQUEST ***"
          << "\n";
}

// -------------------------------------------------------------------
// Diag212CollectorFD::addSuppressFares()
//
// @param fares Fares, which will be stored in memory.
// -------------------------------------------------------------------
void
Diag212CollectorFD::addSuppressFares(const std::vector<const FDSuppressFare*>& fares)
{
  if (fares.empty())
  {
    return;
  }

  map<SuppressFareMarket, set<CarrierCode> >* fareMap = &_multiCarrierSuppressFares;
  SuppressFareMarket marketKey;

  std::vector<const FDSuppressFare*>::const_iterator fareIter = fares.begin();
  std::vector<const FDSuppressFare*>::const_iterator fareIterEnd = fares.end();

  for (; fareIter != fareIterEnd; fareIter++)
  {
    switch ((*fareIter)->fareDisplayType())
    {
    case 'M':
    case ' ':
    case '\0':
      fareMap = &_multiCarrierSuppressFares;
      break;
    case 'S':
      fareMap = &_singleCarrierSuppressFares;
      break;
    default:
      continue;
    }

    marketKey.loc1 = (*fareIter)->loc1();
    marketKey.loc2 = (*fareIter)->loc2();
    switch ((*fareIter)->directionality())
    {
    case FROM:
      marketKey.directionality = MD_FROM;
      break;
    case WITHIN:
      marketKey.directionality = MD_WITHIN;
      break;
    case BETWEEN:
      marketKey.directionality = marketKey.loc1.loc().empty() ? MD_WORLDWIDE : MD_BETWEEN;
      break;
    default:
      continue;
    }

    map<SuppressFareMarket, set<CarrierCode> >::iterator elem = fareMap->find(marketKey);

    if (elem != fareMap->end()) // The same market found, only add a carrier.
    {
      (*elem).second.insert((*fareIter)->carrier());
    }
    else // create new fare
    {
      set<CarrierCode> carriers;
      carriers.insert((*fareIter)->carrier());
      fareMap->insert(pair<SuppressFareMarket, set<CarrierCode> >(marketKey, carriers));
    }
  }
}
// -------------------------------------------------------------------
// Diag212CollectorFD::printHeader()
//
// @param level - define level on which match was done
// @param state - define state of matching
// @param f1Separator - is 1st separator line displayed
// print all suppression records
// -------------------------------------------------------------------
void
Diag212CollectorFD::printHeader(RecordSource level)
{
  ostringstream separator;
  separator << std::setfill('*') << std::setw(62) << '*';
  if (_fHeaderDisplayed)
    *this << separator.str() << "\n";

  switch (_fMatchFound)
  {
  case PS_MATCH_NOT_FOUND:
    *this << "THE FOLLOWING RECORD NOT MATCH"
          << "\n";
    break;
  case PS_MATCH_FOUND:
    *this << "THE FOLLOWING RECORD WAS PROCESSED"
          << "\n";
    break;
  case PS_NOT_PROCESSED:
    *this << "THE FOLLOWING RECORD WAS NOT PROCESSED DUE TO PREVIOUS MATCH"
          << "\n";
    break;
  };

  switch (level)
  {
  case SFS_AGENTPCC:
    *this << "PSEUDO CITY CODE MATCHED: " << _agentPcc << "\n";
    break;
  case SFS_MAINPCC:
    *this << "HOME PSEUDO CITY CODE MATCHED: " << _mainPcc << "\n";
    break;
  case SFS_TJRGROUPNO:
    *this << "TJR GROUP NUMBER MATCHED: " << _tjrGroup << "\n";
    break;
  }
  *this << separator.str() << "\n";
  _fHeaderDisplayed = true;
}

// -------------------------------------------------------------------
// Diag212CollectorFD::printSuppressFares()
//
// @param fNewLine should display new line separator in begin
// print all suppression records
// -------------------------------------------------------------------
void
Diag212CollectorFD::printSuppressFares()
{
  ostringstream separator;
  separator << std::setfill('-') << std::setw(62) << '-';
  map<SuppressFareMarket, set<CarrierCode> >::const_iterator mfib =
      _multiCarrierSuppressFares.begin();
  map<SuppressFareMarket, set<CarrierCode> >::const_iterator mfie =
      _multiCarrierSuppressFares.end();
  bool bFirstLine = true;
  if (_multiCarrierSuppressFares.size())
  {
    for (; mfib != mfie; mfib++)
    {
      if (bFirstLine)
        bFirstLine = false;
      else
        *this << separator.str() << "\n";
      *this << "MULTI CARRIER DISPLAY"
            << "\n"
            << "SUPPRESS CARRIERS:";
      printSuppressFare(mfib->first, mfib->second);
    }
  }
  else
  {
    *this << separator.str() << "\n";
    *this << "MULTI CARRIER DISPLAY"
          << "\n"
          << "SUPPRESS CARRIERS: NONE"
          << "\n";
  }

  map<SuppressFareMarket, set<CarrierCode> >::const_iterator sfib =
      _singleCarrierSuppressFares.begin();
  map<SuppressFareMarket, set<CarrierCode> >::const_iterator sfie =
      _singleCarrierSuppressFares.end();
  if (_singleCarrierSuppressFares.size())
  {
    for (; sfib != sfie; sfib++)
    {
      *this << separator.str() << "\n";
      *this << "SINGLE CARRIER DISPLAY"
            << "\n"
            << "SUPPRESS CARRIERS:";
      printSuppressFare(sfib->first, sfib->second);
    }
  }
  else
  {
    *this << separator.str() << "\n";
    *this << "SINGLE CARRIER DISPLAY"
          << "\n"
          << "SUPPRESS CARRIERS: NONE"
          << "\n";
  }
  _multiCarrierSuppressFares.clear();
  _singleCarrierSuppressFares.clear();
}
// -------------------------------------------------------------------
// Diag212CollectorFD::printSuppressFares()
//
// @param market Market of fare.
//
// @param carriers Carriers which will be enumerated.
// -------------------------------------------------------------------
void
Diag212CollectorFD::printSuppressFare(const SuppressFareMarket& market,
                                      const set<CarrierCode>& carriers)
{
  if (carriers.empty())
  {
    *this << " NONE"
          << "\n";
    return;
  }

  std::set<CarrierCode>::const_iterator carrierIter = carriers.begin();
  std::set<CarrierCode>::const_iterator carrierIterEnd = carriers.end();
  int xPos = 18;

  for (; carrierIter != carrierIterEnd; carrierIter++)
  {
    *this << ' ' << *carrierIter;
    xPos += carrierIter->size() + 1;

    if (xPos > 58)
    {
      *this << "\n";
      xPos = 0;
    }
  }

  *this << "\n";

  switch (market.directionality)
  {
  case MD_WORLDWIDE:
    *this << "WORLDWIDE"
          << "\n";
    return;
  case MD_BETWEEN:
    *this << "BETWEEN ";
    break;
  case MD_FROM:
    *this << "FROM ";
    break;
  case MD_WITHIN:
    *this << "WITHIN ";
    break;
  default:
    break;
  }

  printLoc(market.loc1);

  switch (market.directionality)
  {
  case MD_BETWEEN:
    *this << " AND ";
    break;
  case MD_FROM:
    *this << " TO ";
    break;
  case MD_WITHIN:
    *this << "\n";
    return;
  default:
    break;
  }

  printLoc(market.loc2);
  *this << "\n";
}

// -------------------------------------------------------------------
// Diag212CollectorFD::printLoc()
//
// @param loc Loc which will be printed.
// -------------------------------------------------------------------
void
Diag212CollectorFD::printLoc(const LocKey& loc)
{
  const Nation* nation = nullptr;
  StateCode state;

  switch (loc.locType())
  {
  case 'A':
    *this << "AREA " << loc.loc();
    break;
  case '*':
    *this << "SUBAREA " << loc.loc();
    break;
  case 'C':
    *this << loc.loc();
    break;
  case 'N':
    if (!_fdTrx)
      break;
    nation = _fdTrx->dataHandle().getNation(loc.loc(), _fdTrx->travelDate());
    *this << (nation == nullptr ? loc.loc().data() : nation->description().data());
    break;
  case 'S':
    if (!_fdTrx)
      break;
    nation = _fdTrx->dataHandle().getNation(loc.loc().substr(0, 2), _fdTrx->travelDate());
    state = loc.loc().substr(2, 2);
    *this << state << ", ";
    *this << (nation == nullptr ? loc.loc().data() : nation->description()).data();
    break;
  case 'Z':
    *this << "ZONE " << loc.loc();
    break;
  case ' ':
    *this << "ANYWHERE";
  default:
    break;
  }
}
}
