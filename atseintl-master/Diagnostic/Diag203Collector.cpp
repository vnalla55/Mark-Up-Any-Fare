//----------------------------------------------------------------------------
//  File:        Diag203Collector.C
//  Authors:     Quan Ta
//  Created:     Mar 2007
//
//  Description: Diagnostic 203 - Fare Type Pricing table info
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2007
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

#include "Diagnostic/Diag203Collector.h"

#include "Common/FareCalcUtil.h"
#include "DataModel/PricingOptions.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareTypeQualifier.h"
#include "FareCalc/FareCalcConsts.h"
#include "FareCalc/FcStream.h"
#include "Fares/FareTypeMatcher.h"

#include <iomanip>

namespace tse
{
void
Diag203Collector::process(PricingTrx& trx)
{
  if (!_active)
    return;

  FareCalcUtil::getUserAppl(trx, *trx.getRequest()->ticketingAgent(), _userApplType, _userAppl);
  _qualifier = FareTypeMatcher::getFtPricingQualifier(*trx.getOptions());

  displayHeading(trx);

  int count = 0;
  if (trx.getOptions()->isFareFamilyType())
  {
    FareTypeMatcher ftMatch(*_trx);
    count += ftMatch.fareTypeQualifier().size();

    if (count > 0)
      (*this) << ftMatch.fareTypeQualifier();
  }
#ifdef FT_DEV
#else
  else
  {
    static char all_qualifier[][2] = { "2", "3", "1" }; // NL, EX, IT

    // This is not part of the Fare Type pricing requirement, I am adding
    // this feature to enable us to see all fare type pricing item filed
    // with the plain diag 203 (without T/xx) entry

    for (int i = 0, n = sizeof(all_qualifier) / sizeof(all_qualifier[0]); i < n; i++)
    {
      _qualifier = all_qualifier[i];
      const std::vector<FareTypeQualifier*>& ftqList =
          trx.dataHandle().getFareTypeQualifier(_userApplType, _userAppl, _qualifier);

      count += ftqList.size();
      (*this) << ftqList;
    }

    // Only get the default (blank), if none is coded for the CRS
    if (!_userAppl.empty() && count == 0)
    {
      for (int i = 0, n = sizeof(all_qualifier) / sizeof(all_qualifier[0]); i < n; i++)
      {
        _qualifier = all_qualifier[i];
        const std::vector<FareTypeQualifier*>& ftqList =
            trx.dataHandle().getFareTypeQualifier(' ', "", _qualifier);

        count += ftqList.size();
        (*this) << ftqList;
      }
    }
  }
#endif

  if (count == 0)
  {
    (*this) << "NO MATCH FOUND\n";
  }
}

void
Diag203Collector::displayHeading(PricingTrx& trx)
{
  _trx = &trx;

  if (_userApplType == 0)
  {
    FareCalcUtil::getUserAppl(trx, *trx.getRequest()->ticketingAgent(), _userApplType, _userAppl);
    _qualifier = FareTypeMatcher::getFtPricingQualifier(*trx.getOptions());
  }

  if (trx.getOptions()->isFareFamilyType())
  {
    (*this) << "***************************************************************\n"
            << "* FARE TYPE PRICING: " << _userApplType << '/' << _userAppl << '/' << _qualifier
            << '\n' << "***************************************************************\n";
  }
  else
  {
    (*this) << "***************************************************************\n"
            << "* FARE TYPE PRICING\n"
            << "***************************************************************\n";
  }
}

std::string
Diag203Collector::showFtqInfo(const FareTypeQualifier& ftq)
{
  std::ostringstream os;

  if (_userApplType == 0)
  {
    FareCalcUtil::getUserAppl(
        *_trx, *_trx->getRequest()->ticketingAgent(), _userApplType, _userAppl);
    _qualifier = FareTypeMatcher::getFtPricingQualifier(*_trx->getOptions());
  }

  os << "USER TYPE:" << std::setw(6) << _userApplType << "   JRNY DOM: " << ftq.journeyTypeDom()
     << "   PU DOM: " << ftq.pricingUnitDom()
     << "   CREATE: " << ftq.createDate().dateToString(MMDDYYYY, "/") << '\n'
     << "USER APPL:" << std::setw(6) << _userAppl << "       INTL: " << ftq.journeyTypeIntl()
     << "     INTL: " << ftq.pricingUnitIntl()
     << "   EXPIRE: " << ftq.expireDate().dateToString(MMDDYYYY, "/") << '\n'
     << "FT QUALIFIER: " << std::setw(2)
     << (_qualifier == "1" ? "IT" : (_qualifier == "2" ? "NL" : (_qualifier == "3" ? "EX" : "NL")))
     << "        EOE: " << ftq.journeyTypeEoe();

  return os.str();
}

std::string
Diag203Collector::showFtqPsgInfo(const FareTypeQualifier& ftq)
{
  std::ostringstream os;
  os << "   PSG TYPE:";

  const std::set<PaxTypeCode>& psgType = ftq.psgType();
  for (const auto elem : psgType)
  {
    os << " " << elem;
  }
  return os.str();
}

std::string
Diag203Collector::showFtqMsgInfo(const FareTypeQualifier& ftq)
{
  std::ostringstream os;

  FareCalc::FcStream os_fareType(_trx, FareCalcConsts::FCL_MAX_LINE_LENGTH, 12);
  FareCalc::FcStream os_req(_trx, FareCalcConsts::FCL_MAX_LINE_LENGTH, 12);
  FareCalc::FcStream os_grpMsg(_trx, FareCalcConsts::FCL_MAX_LINE_LENGTH, 12);
  FareCalc::FcStream os_itMsg(_trx, FareCalcConsts::FCL_MAX_LINE_LENGTH, 12);

  os_fareType << "  FARE TYPE:";
  os_req << " FT REQ IND:";
  os_grpMsg << "GRP MSG IND:";
  os_itMsg << " IT MSG IND:";

  // TODO: any sorting preference?

  int maxLength = FareCalcConsts::FCL_MAX_LINE_LENGTH - 6;
  const std::map<PaxTypeCode, FareTypeQualMsg>& qualMsg = ftq.qualifierMgs();
  for (const auto& elem : qualMsg)
  {
    if (os_fareType.lineLength() > maxLength)
    {
      os << os_fareType.str() << '\n' << os_req.str() << '\n' << os_grpMsg.str() << '\n'
         << os_itMsg.str() << "\n  \n";

      os_fareType.clear();
      os_req.clear();
      os_grpMsg.clear();
      os_itMsg.clear();

      os_fareType << "  FARE TYPE:";
      os_req << " FT REQ IND:";
      os_grpMsg << "GRP MSG IND:";
      os_itMsg << " IT MSG IND:";
    }

    const PaxTypeCode& fareType = elem.first;
    const FareTypeQualMsg& qualMsg = elem.second;
    int width = fareType.size();

    os_fareType << std::setw(width + 1) << fareType;
    os_req << std::setw(width) << qualMsg.fareTypeReqInd() << ' ';
    os_grpMsg << std::setw(width) << qualMsg.groupTrailerMsgInd() << ' ';
    os_itMsg << std::setw(width) << qualMsg.itTrailerMsgInd() << ' ';
  }

  if (!os_fareType.str().empty())
  {
    os << os_fareType.str() << '\n' << os_req.str() << '\n' << os_grpMsg.str() << '\n'
       << os_itMsg.str() << '\n';
  }

  return os.str();
}

Diag203Collector&
Diag203Collector::operator << (const FareTypeQualifier& ftq)
{
  std::ostringstream os;

  os << showFtqInfo(ftq) << "\n  \n" << showFtqPsgInfo(ftq) << "\n  \n" << showFtqMsgInfo(ftq);

  (*this) << os.str() << std::endl;

  return *this;
}

Diag203Collector&
Diag203Collector::operator << (const std::vector<FareTypeQualifier*>& ftqList)
{
  if (!ftqList.empty())
  {
    for (const auto elem : ftqList)
    {
      (*this) << *elem << "---------------------------------------------------------------\n";
    }
  }
  return *this;
}
}
