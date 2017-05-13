//-----------------------------------------------------------------------------
//
//  File:     Diag207CollectorFD.h
//
//  Author :  Adam Szalajko
//
//  Copyright Sabre 2008
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------
#include "Diagnostic/Diag207CollectorFD.h"

#include "Common/TseUtil.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FareDispCldInfPsgType.h"
#include "DBAccess/FareDispInclDsplType.h"
#include "DBAccess/FareDispInclFareType.h"
#include "DBAccess/FareDispInclRuleTrf.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/FareDisplayWeb.h"
#include "DBAccess/FareDispRec1PsgType.h"
#include "DBAccess/FareDispRec8PsgType.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Rules/RuleUtil.h"

using namespace std;
namespace tse
{
// ------------------------------------------------------------------------
void
Diag207CollectorFD::displayInclusionCode(FareDisplayTrx& trx, FareDisplayInclCd* inlcusionCode)
{
  if (!_active)
    return;

  int lineLength = 61;
  _fareDisplayInclCd = inlcusionCode;
  *this << "\n";

  if (_fareDisplayInclCd != nullptr)
  {
    _inclCode = _fareDisplayInclCd->inclusionCode();
    _description = _fareDisplayInclCd->description();
    _dtAft = _fareDisplayInclCd->displTypeAndOrFareType();
    _ftApt = _fareDisplayInclCd->fareTypeAndOrPsgType();
    _dtApt = _fareDisplayInclCd->displTypeAndOrPsgType();
    _exceptFareType = _fareDisplayInclCd->exceptFareType();
    _exceptPsgType = _fareDisplayInclCd->exceptPsgType();

    displayInclCode(trx);
    displayRuleTrf(trx);
    *this << "\n";
    displayRelation(trx);
    *this << "\n";
    displayFareTypes(trx);
    displayDispTypes(trx);
    *this << "\n";

    displayRec1PsgTypes(trx);
    *this << "\n";

    displayChdPsgTypes(trx);
    *this << "\n";

    displayInfPsgTypes(trx);
    *this << "\n";
    if (trx.getRequest()->diagnosticNumber() == DIAG_209_ID)
    {
      displayRec8PsgTypes(trx);
    }
  }
  else
  {
    *this << " NO INCLUSION CODE FOUND : USE DEFAULT"
          << "\n";
  }
  addFooter(lineLength);
}
// ----------------------------------------------------------------------------
void
Diag207CollectorFD::displayWebInclusionCode(FareDisplayTrx& trx)
{
  if (!_active)
    return;

  int lineLength = 62;
  *this << "\n";

  displayWebFares(trx);
  addFooter(lineLength);
}

void
Diag207CollectorFD::displayInclCode(const FareDisplayTrx& trx)
{
  *this << " ****************  INCLUSION CODE DIAGNOSTIC  *************** "
        << "\n";
  *this << _inclCode << "    " << _description << "\n";
  *this << "-----------------------------------------------------"
        << "\n";
}

void
Diag207CollectorFD::displayRuleTrf(const FareDisplayTrx& trx)
{
  *this << "RULETARIFF NUMBER : ";

  if (_fareDisplayInclCd != nullptr)
  {
    const std::vector<FareDispInclRuleTrf*>& ruleTariffRecs =
        trx.dataHandle().getFareDispInclRuleTrf(_fareDisplayInclCd->userApplType(),
                                                _fareDisplayInclCd->userAppl(),
                                                _fareDisplayInclCd->pseudoCityType(),
                                                _fareDisplayInclCd->pseudoCity(),
                                                _fareDisplayInclCd->inclusionCode());
    if (ruleTariffRecs.empty())
    {
      *this << "NO RESTRICTIONS"
            << "\n";
      return;
    }

    int i;
    std::vector<FareDispInclRuleTrf*>::const_iterator iter = ruleTariffRecs.begin();
    std::vector<FareDispInclRuleTrf*>::const_iterator iterEnd = ruleTariffRecs.end();
    int _sizeLeft = ruleTariffRecs.size();
    while (_sizeLeft > 0)
    {
      if (iter != ruleTariffRecs.begin())
      {
        *this << "                    ";
      }
      if (_sizeLeft < 3)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          int _ruleTrf = (*iter)->ruleTariff();
          *this << (*iter)->vendorCode() << "--" << _ruleTrf << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        *this << "\n";
      }
      else
      {
        for (i = 0; i < 3; i++)
        {
          int _ruleTrf = (*iter)->ruleTariff();
          *this << (*iter)->vendorCode() << "--" << _ruleTrf << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        *this << "\n";
        _sizeLeft = _sizeLeft - 3;
      }
    } // End While loop

    *this << "\n";
    if (!ruleTariffRecs.empty())
    {
      *this << "RELATIONSHIP BETWEEN RULETARIFF AND OTHER TABLES IS ALWAYS AND"
            << "\n";
    }
  }
  else
  {
    *this << "\n";
  }
}

void
Diag207CollectorFD::displayRelation(const FareDisplayTrx& trx)
{
  *this << "RELATIONSHIP BETWEEN TABLES: ";

  if ((_dtAft == 'A') && (_ftApt == 'A') && (_dtApt == 'A'))
  {
    *this << "DT AND FT AND PT"
          << "\n";
  }
  else if ((_dtAft == 'O') && (_ftApt == 'O') && (_dtApt == 'O'))
  {
    *this << "DT OR FT OR PT"
          << "\n";
  }
  else if ((_dtAft == 'O') && (_ftApt == 'A') && (_dtApt == 'O'))
  {
    *this << "DT OR /FT AND PT/"
          << "\n";
  }
  else if ((_dtAft == 'A') && (_ftApt == 'O') && (_dtApt == 'A'))
  {
    *this << "DT AND /FT OR PT/"
          << "\n";
  }
  else if ((_dtAft == 'O') && (_ftApt == 'A') && (_dtApt == 'A'))
  {
    *this << "/DT OR FT/ AND PT"
          << "\n";
  }
  else if ((_dtAft == 'A') && (_ftApt == 'O') && (_dtApt == 'O'))
  {
    *this << "/DT AND FT/ OR PT"
          << "\n";
  }
  else if ((_dtAft == 'O') && (_ftApt == ' ') && (_dtApt == ' '))
  {
    *this << "DT OR FT"
          << "\n";
  }
  else if ((_dtAft == 'A') && (_ftApt == ' ') && (_dtApt == ' '))
  {
    *this << "DT AND FT"
          << "\n";
  }
  else if ((_dtAft == ' ') && (_ftApt == 'A') && (_dtApt == ' '))
  {
    *this << "FT AND PT"
          << "\n";
  }
  else if ((_dtAft == ' ') && (_ftApt == 'O') && (_dtApt == ' '))
  {
    *this << "FT OR PT"
          << "\n";
  }
  else if ((_dtAft == ' ') && (_ftApt == ' ') && (_dtApt == 'A'))
  {
    *this << "DT AND PT"
          << "\n";
  }
  else if ((_dtAft == ' ') && (_ftApt == ' ') && (_dtApt == 'O'))
  {
    *this << "DT OR PT"
          << "\n";
  }
  else
  {
    *this << "\n";
  }
}

void
Diag207CollectorFD::displayFareTypes(const FareDisplayTrx& trx)
{
  int i;

  if (_exceptFareType == 'Y')
  {
    *this << "ANY FARE TYPE EXCEPT: ";
  }
  else
  {
    *this << "FARE TYPES: ";
  }
  if (_fareDisplayInclCd != nullptr)
  {
    const std::vector<FareDispInclFareType*>& fareTypes =
        trx.dataHandle().getFareDispInclFareType(_fareDisplayInclCd->userApplType(),
                                                 _fareDisplayInclCd->userAppl(),
                                                 _fareDisplayInclCd->pseudoCityType(),
                                                 _fareDisplayInclCd->pseudoCity(),
                                                 _fareDisplayInclCd->inclusionCode());
    if (fareTypes.empty())
    {
      *this << "NO RESTRICTIONS"
            << "\n";
      return;
    }
    std::vector<FareDispInclFareType*>::const_iterator iter = fareTypes.begin();
    std::vector<FareDispInclFareType*>::const_iterator iterEnd = fareTypes.end();
    int _sizeLeft = fareTypes.size();

    while (_sizeLeft > 0)
    {
      if (iter != fareTypes.begin())
      {
        if (_exceptFareType == 'Y')
        {
          *this << "                     ";
        }
        else
        {
          *this << "            ";
        }
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          FareType _fareT = (*iter)->fareType();
          *this << _fareT << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        *this << "\n";
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          FareType _fareT = (*iter)->fareType();
          *this << _fareT << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        *this << "\n";
        _sizeLeft = _sizeLeft - 10;
      }
    }
  }
  else
  {
    *this << "\n";
  }
}

void
Diag207CollectorFD::displayDispTypes(const FareDisplayTrx& trx)
{
  *this << "DISPLAY TYPES   : ";

  if (_fareDisplayInclCd != nullptr)
  {
    uint16_t i;
    const std::vector<FareDispInclDsplType*>& displayTypes =
        trx.dataHandle().getFareDispInclDsplType(_fareDisplayInclCd->userApplType(),
                                                 _fareDisplayInclCd->userAppl(),
                                                 _fareDisplayInclCd->pseudoCityType(),
                                                 _fareDisplayInclCd->pseudoCity(),
                                                 _fareDisplayInclCd->inclusionCode());
    if (displayTypes.empty())
    {
      *this << "NO RESTRICTION"
            << "\n";
      return;
    }
    std::vector<FareDispInclDsplType*>::const_iterator iter = displayTypes.begin();
    std::vector<FareDispInclDsplType*>::const_iterator iterEnd = displayTypes.end();
    uint16_t _sizeLeft = displayTypes.size();

    while (_sizeLeft > 0)
    {
      if (iter != displayTypes.begin())
      {
        *this << "               ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          Indicator _dispType = (*iter)->displayCatType();
          *this << _dispType << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        *this << "\n";
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          Indicator _dispType = (*iter)->displayCatType();
          *this << _dispType << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        *this << "\n";
        _sizeLeft = _sizeLeft - 10;
      }
    }
  }
  else
  {
    *this << "\n";
  }
}

void
Diag207CollectorFD::displayRec1PsgTypes(const FareDisplayTrx& trx)
{
  if (_exceptPsgType == 'Y')
  {
    *this << "ANY PSG TYPE EXCEPT REC 1 PSG TYPES: ";
  }
  else
  {
    *this << "ADULT PSG TYPES : ";
  }

  if (_fareDisplayInclCd != nullptr)
  {
    const std::vector<FareDispRec1PsgType*>& rec1PsgTypes =
        trx.dataHandle().getFareDispRec1PsgType(_fareDisplayInclCd->userApplType(),
                                                _fareDisplayInclCd->userAppl(),
                                                _fareDisplayInclCd->pseudoCityType(),
                                                _fareDisplayInclCd->pseudoCity(),
                                                _fareDisplayInclCd->inclusionCode());
    if (rec1PsgTypes.empty())
    {
      *this << "NO RESTRICTION"
            << "\n";
      return;
    }

    std::vector<FareDispRec1PsgType*>::const_iterator iter = rec1PsgTypes.begin();
    std::vector<FareDispRec1PsgType*>::const_iterator iterEnd = rec1PsgTypes.end();
    int _sizeLeft = rec1PsgTypes.size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != rec1PsgTypes.begin())
      {
        if (_exceptPsgType == 'Y')
        {
          *this << "                                     ";
        }
        else
        {
          *this << "                 ";
        }
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          PaxTypeCode _psgType = (*iter)->psgType();
          *this << _psgType << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        *this << "\n";
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          PaxTypeCode _psgType = (*iter)->psgType();
          *this << _psgType << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        *this << "\n";
        _sizeLeft = _sizeLeft - 10;
      }
    }
  }
  else
  {
    *this << "\n";
  }
}

void
Diag207CollectorFD::displayRec8PsgTypes(const FareDisplayTrx& trx)
{
  *this << "\n";
  *this << "REC 8 PSG TYPES : ";

  if (_fareDisplayInclCd != nullptr)
  {
    const std::vector<FareDispRec8PsgType*>& rec8PsgTypes =
        trx.dataHandle().getFareDispRec8PsgType(_fareDisplayInclCd->userApplType(),
                                                _fareDisplayInclCd->userAppl(),
                                                _fareDisplayInclCd->pseudoCityType(),
                                                _fareDisplayInclCd->pseudoCity(),
                                                _fareDisplayInclCd->inclusionCode());

    std::vector<FareDispRec8PsgType*>::const_iterator iter = rec8PsgTypes.begin();
    std::vector<FareDispRec8PsgType*>::const_iterator iterEnd = rec8PsgTypes.end();
    int _sizeLeft = rec8PsgTypes.size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != rec8PsgTypes.begin())
      {
        *this << "                 ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          PaxTypeCode _psgType = (*iter)->psgType();
          *this << _psgType << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        *this << "\n";
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          PaxTypeCode _psgType = (*iter)->psgType();
          *this << _psgType << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        *this << "\n";
        _sizeLeft = _sizeLeft - 10;
      }
    }
  }
  else
  {
    *this << "\n";
  }
}

void
Diag207CollectorFD::displayChdPsgTypes(const FareDisplayTrx& trx)
{
  *this << "CHILD PSG TYPES : ";

  if (_fareDisplayInclCd != nullptr)
  {
    const std::vector<FareDispCldInfPsgType*>& cldPsgTypes =
        trx.dataHandle().getFareDispCldInfPsgType(_fareDisplayInclCd->userApplType(),
                                                  _fareDisplayInclCd->userAppl(),
                                                  _fareDisplayInclCd->pseudoCityType(),
                                                  _fareDisplayInclCd->pseudoCity(),
                                                  _fareDisplayInclCd->inclusionCode(),
                                                  'C');
    if (cldPsgTypes.empty())
    {

      *this << "NOT FILED"
            << "\n";
      return;
    }
    std::vector<FareDispCldInfPsgType*>::const_iterator iter = cldPsgTypes.begin();
    std::vector<FareDispCldInfPsgType*>::const_iterator iterEnd = cldPsgTypes.end();
    int _sizeLeft = cldPsgTypes.size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != cldPsgTypes.begin())
      {
        *this << "                 ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          PaxTypeCode _psgType = (*iter)->psgType();
          *this << _psgType << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        *this << "\n";
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          PaxTypeCode _psgType = (*iter)->psgType();
          *this << _psgType << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        *this << "\n";
        _sizeLeft = _sizeLeft - 10;
      }
    }
  }
  else
  {
    *this << "\n";
  }
}

void
Diag207CollectorFD::displayInfPsgTypes(const FareDisplayTrx& trx)
{
  *this << "INFANT PSG TYPES: ";

  if (_fareDisplayInclCd != nullptr)
  {
    const std::vector<FareDispCldInfPsgType*>& infPsgTypes =
        trx.dataHandle().getFareDispCldInfPsgType(_fareDisplayInclCd->userApplType(),
                                                  _fareDisplayInclCd->userAppl(),
                                                  _fareDisplayInclCd->pseudoCityType(),
                                                  _fareDisplayInclCd->pseudoCity(),
                                                  _fareDisplayInclCd->inclusionCode(),
                                                  'I');
    if (infPsgTypes.empty())
    {
      *this << "NOT FILED"
            << "\n";
      return;
    }

    std::vector<FareDispCldInfPsgType*>::const_iterator iter = infPsgTypes.begin();
    std::vector<FareDispCldInfPsgType*>::const_iterator iterEnd = infPsgTypes.end();
    int _sizeLeft = infPsgTypes.size();
    int i;

    while (_sizeLeft > 0)
    {
      if (iter != infPsgTypes.begin())
      {
        *this << "                  ";
      }
      if (_sizeLeft < 10)
      {
        for (i = 0; i != _sizeLeft; i++)
        {
          PaxTypeCode _psgType = (*iter)->psgType();
          *this << _psgType << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        _sizeLeft = 0; // we are done
        *this << "\n";
      }
      else
      {
        for (i = 0; i < 10; i++)
        {
          PaxTypeCode _psgType = (*iter)->psgType();
          *this << _psgType << " ";
          if (iter != iterEnd)
          {
            iter++;
          }
        }
        *this << "\n";
        _sizeLeft = _sizeLeft - 10;
      }
    }
  }
  else
  {
    *this << "\n";
  }
}

void
Diag207CollectorFD::displayWebFares(const FareDisplayTrx& trx)
{
  *this << "\n";

  *this << "*******************  WEB FARES DIAGNOSTIC  *******************"
        << "\n";

  bool fFirstLine = true;

  const CarrierCode carrier = *trx.preferredCarriers().begin();

  const std::vector<FareDisplayWeb*>& fareDisplayWeb =
      trx.dataHandle().getFareDisplayWebForCxr(carrier);

  if (fareDisplayWeb.empty())
    return;

  std::vector<FareDisplayWeb*>::const_iterator iter = fareDisplayWeb.begin();
  std::vector<FareDisplayWeb*>::const_iterator iterEnd = fareDisplayWeb.end();

  std::set<PaxTypeCode> webPaxTypes;
  FareDisplayWeb tmpWeb;

  for (tmpWeb = **iter; iter != iterEnd; iter++)
  {
    if ((**iter) == tmpWeb)
    {
      if ((*iter)->paxType() != "")
        webPaxTypes.insert((*iter)->paxType());
      continue;
    }
    displayWebFare(trx, tmpWeb, webPaxTypes, fFirstLine);
    tmpWeb = **iter;
    if ((*iter)->paxType() != "")
      webPaxTypes.insert((*iter)->paxType());
  }
  displayWebFare(trx, tmpWeb, webPaxTypes, fFirstLine);
}
void
Diag207CollectorFD::displayWebFare(const FareDisplayTrx& trx,
                                   FareDisplayWeb& web,
                                   std::set<PaxTypeCode>& paxTypeCodes,
                                   bool& fIsFirstLine)
{
  int i;
  std::ostringstream str;
  TariffCode tariffCode;

  if (!fIsFirstLine)
    addStarLine(62);
  fIsFirstLine = false;

  *this << "TABLES-SPTFW/SPTFWS           FAREDISPLAYWEB/FAREDISPLAYWEBSEG"
        << "\n";
  str << "CARRIER-" << web.carrier();
  addText(str, 15);
  str << "RULE TARIFF-" << std::setw(3) << std::setiosflags(ios::left) << web.ruleTariff();

  if (getRuleTariffDescription(trx, web.ruleTariff(), web.vendor(), web.carrier(), tariffCode))
  {
    str << "/" << std::setiosflags(ios::left) << tariffCode;
  }

  addText(str, 29);
  str << "RULE NUMBER-" << web.rule();
  addText(str, 17, true);
  str << "RECORD 1 DISPLAY TYPE-" << web.displayInd();
  addText(str, 29);
  str << "SOURCE-" << web.vendor();
  addText(str, 31, true);
  str << "FARE CLASS-" << web.fareClass();
  addText(str, 25);
  str << "RECORD 1 TICKET DESIGNATOR-" << web.tktDesignator();
  addText(str, 37, true);
  str << "PASSENGER TYPE-";
  i = 4;

  std::set<PaxTypeCode>::iterator segIter = paxTypeCodes.begin();
  std::set<PaxTypeCode>::iterator segIterEnd = paxTypeCodes.end();

  for (; segIter != segIterEnd; segIter++)
  {
    str << std::setw(3) << (*segIter);
    if (++i >= 16)
    {
      addText(str, 63, true);
      i = 0;
    }
    else
      str << " ";
  }
  if (i)
    addText(str, 63, true);
  paxTypeCodes.clear();
}

// -------------------------------------------------------------------
// <PRE>
//
// @MethodName  Diag207CollectorFD:::addText()
//
// Adds a new string to the response
//
// @param str string stream which will be added to the response
// @param size width of the field
// @param fNewLine if true, new line is added and the end of string
//
// @return  void.
//
// </PRE>
// -------------------------------------------------------------------------

void
Diag207CollectorFD::addText(std::ostringstream& str, int size, bool fNewLine)
{
  std::string retStr = str.str();

  *this << std::setw(size) << std::setiosflags(ios::left) << retStr;

  if (fNewLine)
    *this << "\n";

  str.str("");
}

void
Diag207CollectorFD::addFooter(int lineLength)
{
  addStarLine(lineLength);
  *this << "                     END DIAGNOSTIC INFO"
        << "\n";
  *this << "      NODE: ";
  char hostName[1024];
  if (::gethostname(hostName, 1023) < 0)
  {
    cout << "hostName: '" << hostName << "'";
    *this << "HOST NAME ERROR"
          << "\n";
  }
  else
  {
    string aString(hostName);
    std::transform(aString.begin(), aString.end(), aString.begin(), (int (*)(int))toupper);
    *this << aString << "\n";
  }
  addStarLine(lineLength);
}
void
Diag207CollectorFD::addStarLine(int LineLength)
{
  *this << setfill('*') << setw(LineLength) << "" << setfill(' ') << "\n";
}
bool
Diag207CollectorFD::getRuleTariffDescription(const FareDisplayTrx& trx,
                                             const TariffNumber& ruleTariff,
                                             const VendorCode& vendor,
                                             const CarrierCode& carrier,
                                             TariffCode& ruleTariffCode)
{
  ruleTariffCode = EMPTY_STRING();

  // -----------------------------------------------
  // Retrieve TariffCrossRef Info
  // -----------------------------------------------
  // first check if rule is INTERNATIONAL
  const std::vector<TariffCrossRefInfo*> tariffDescInfoList =
      trx.dataHandle().getTariffXRefByRuleTariff(
          vendor, carrier, INTERNATIONAL, ruleTariff, TseUtil::getTravelDate(trx.travelSeg()));
  if (!tariffDescInfoList.empty())
  {
    ruleTariffCode = (*tariffDescInfoList.begin())->ruleTariffCode();
    return true;
  }

  // If not INTERNATIONAL, then check if DOMESTIC
  std::vector<TariffCrossRefInfo*> tariffDescInfoListDom =
      trx.dataHandle().getTariffXRefByRuleTariff(
          vendor, carrier, DOMESTIC, ruleTariff, TseUtil::getTravelDate(trx.travelSeg()));

  if (!tariffDescInfoListDom.empty())
  {
    ruleTariffCode = (*tariffDescInfoListDom.begin())->ruleTariffCode();
    return true;
  }
  return false;
}
}
