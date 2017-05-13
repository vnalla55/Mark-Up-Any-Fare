//-----------------------------------------------------------------------------
//
//  File:     Diag291CollectorFD.C
//
//  Author :  Slawek Machowicz
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
#include "Diagnostic/Diag291CollectorFD.h"

#include "Common/Assert.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFareRuleData.h"

namespace tse
{
namespace
{
const std::string DIAG291FIELDS[Diag291CollectorFD::FAIL_RD_LAST + 1] = {
  // general fields
  "FARE CLASS",        "LINK NUMBER",      "SEQUENCE NUMBER",        "DATE/TIME",
  "FAREBASIS CODE",    "FARE AMOUNT",      "FARE TYPE",              "REQUESTED DISCOUNT",
  "REQUESTED CAT25",   "REQUESTED NEG",    "SOURCE PCC",             "REQUESTED CONSTRUCTED",  "CAT 35 TYPE",
  "CARRIER",           "CURRENCY",         "VENDOR",                 "TARIFF",
  "RULE NUMBER",       "BOOKING CODE",     "FARE ORIGIN",            "FARE DESTINATION",
  // construction fields
  "GATEWAY 1",         "GATEWAY 2",        "ORIGIN ADD ON",          "DEST ADD ON",
  "CONSTRUCTION TYPE", "SPECIFIED AMOUNT", "CONSTRUCTED NUC AMOUNT",
  // qualify
  "QUALIFY",           "PAX TYPE",
  // positive
  "FARE MATCHED",
  // other
  "UNKNOWN"
};
}

void
Diag291CollectorFD::init(FareDisplayTrx& trx)
{
  _fdTrx = &trx;

  _fdo = trx.getOptions();
  _request = trx.getRequest();

  _wantDisc = _fdo->discountedValues().isNonPublishedFare();
  _wantFbr = _fdo->cat25Values().isNonPublishedFare();
  _wantNeg = _fdo->cat35Values().isNonPublishedFare();
  _wantConstr = _fdo->constructedAttributes().isConstructedFare();
}

void
Diag291CollectorFD::initParam(Diagnostic& root)
{
  _fdTrx = dynamic_cast<FareDisplayTrx*>(trx());

  TSE_ASSERT(_fdTrx);

  _fdo = _fdTrx->getOptions();
  _request = _fdTrx->getRequest();

  _wantDisc = _fdo->discountedValues().isNonPublishedFare();
  _wantFbr = _fdo->cat25Values().isNonPublishedFare();
  _wantNeg = _fdo->cat35Values().isNonPublishedFare();
  _wantConstr = _fdo->constructedAttributes().isConstructedFare();
}

// Diag291CollectorFD::printHeader()
// Dispalys beginning of diag 291
void
Diag291CollectorFD::printHeader()
{
  if (!_active)
    return;

  *this << std::setfill('*') << std::setw(62) << '*' << std::endl;
  *this << "RULE DISPLAY FARE SELECTION DIAGNOSTIC 291" << std::endl;
  *this << std::setw(62) << '*' << std::endl;
  *this << "REQUEST INFORMATION:" << std::setfill(' ') << std::endl;

  uint16_t last = (_wantConstr ? FAIL_RD_LAST_CONSTRUCTED : FAIL_RD_LAST_GENERAL) + 1;
  for (uint16_t counter = FAIL_RD_FIRST; counter < last; counter++)
  {
    *this << DIAG291FIELDS[counter]
          << std::setw(25 - static_cast<int>(DIAG291FIELDS[counter].length())) << ' ';

    switch (counter)
    {
    case FAIL_RD_LINK:
      *this << _fdo->linkNumber();
      break;
    case FAIL_RD_SEQUENCE:
      *this << _fdo->sequenceNumber();
      break;
    case FAIL_RD_DATETIME:
      *this << _fdo->createDate().dateToString(DDMMMYY, "") << " " << _fdo->createTime();
      break;
    case FAIL_RD_FARECLASS:
      *this << _fdo->fareClass();
      break;
    case FAIL_RD_FAREBASIS:
      *this << _request->fareBasisCode();
      break;
    case FAIL_RD_FAREAMOUNT:
      *this << _request->calcFareAmount();
      break;
    case FAIL_RD_FARETYPE:
      *this << _fdo->fareType();
      break;
    case FAIL_RD_DISCOUNTED:
      *this << (_wantDisc ? 'Y' : 'N');
      break;
    case FAIL_RD_FBRULE:
      *this << (_wantFbr ? 'Y' : 'N');
      break;
    case FAIL_RD_NEGOTIATED:
      *this << (_wantNeg ? 'Y' : 'N');
      break;
    case FAIL_RD_SOURCE_PCC:
      *this << "N/A";
      break;
    case FAIL_RD_CONSTRUCTED:
      *this << (_wantConstr ? 'Y' : 'N');
      break;
    case FAIL_RD_CAT35:
      *this << _fdo->FDcat35Type();
      break;
    case FAIL_RD_CARRIER:
      *this << _fdo->carrierCode();
      break;
    case FAIL_RD_CURRENCY:
      *this << _fdo->baseFareCurrency();
      break;
    case FAIL_RD_VENDOR:
      *this << _fdo->vendorCode();
      break;
    case FAIL_RD_TARIFF:
      *this << _fdo->fareTariff();
      break;
    case FAIL_RD_RULENUMBER:
      *this << _fdo->ruleNumber();
      break;
    case FAIL_RD_BOOKINGCODE:
      *this << _request->bookingCode();
      break;
    case FAIL_RD_ORIGIN:
      *this << _request->fareOrigin();
      break;
    case FAIL_RD_DESTINATION:
      *this << _request->fareDestination();
      break;
    case FAIL_RD_GATEWAY1:
      *this << _fdo->constructedAttributes().gateway1();
      break;
    case FAIL_RD_GATEWAY2:
      *this << _fdo->constructedAttributes().gateway2();
      break;
    case FAIL_RD_ORIGAO:
      *this << _fdo->origAttributes().addonFareClass();
      break;
    case FAIL_RD_DESTAO:
      *this << _fdo->destAttributes().addonFareClass();
      break;
    case FAIL_RD_CONSTRUCTIONTYPE:
      *this << _fdo->constructedAttributes().constructionType();
      break;
    case FAIL_RD_SPECIFIED_AMOUNT:
      *this << _fdo->constructedAttributes().specifiedFareAmount();
      break;
    case FAIL_RD_CONSTR_NUC_AMOUNT:
      *this << _fdo->constructedAttributes().constructedNucAmount();
      break;
    default:
      *this << DIAG291FIELDS[FAIL_RD_LAST]; // Unknown
      break;
    }
    *this << std::endl;
  }
}

// Diag291CollectorFD::printFooter()
// Dispalys summary of diag 291
void
Diag291CollectorFD::printFooter(size_t faresFound, int faresProcessed)
{
  if (!_active)
    return;

  *this << std::setfill('*') << std::setw(62) << '*' << std::endl << "RULE DISPLAY MATCHED "
        << faresFound << " FARES OF TOTAL " << faresProcessed << std::endl << std::setfill('*')
        << std::setw(62) << '*' << std::endl;
}

// Diag291CollectorFD::printFare()
// Dispalys processed fare in diag 291
void
Diag291CollectorFD::printFare(PaxTypeFare& ptFare, D291FailCode failCode)
{
  if (!_active)
    return;

  if (rootDiag()->diagParamMap().empty())
  {
    if (failCode == FAIL_RD_FARECLASS || failCode == FAIL_RD_LINK || failCode == FAIL_RD_SEQUENCE ||
        failCode == FAIL_RD_DATETIME || failCode == FAIL_RD_FAREBASIS)
    {
      return;
    }
  }
  else
  {
    if (!rootDiag()->shouldDisplay(ptFare))
    {
      return;
    }
  }

  const PaxTypeFare* baseFare = ptFare.fareWithoutBase();

  *this << std::setfill('*') << std::setw(62) << '*' << std::endl;

  if (failCode >= FAIL_RD_FIRST && failCode < FAIL_RD_LAST)
  {
    *this << DIAG291FIELDS[failCode];
  }
  else
  {
    *this << DIAG291FIELDS[FAIL_RD_LAST];
  }

  if (failCode == FAIL_RD_MATCHED)
  {
    *this << std::endl << std::setfill('*') << std::setw(62) << '*' << std::endl;
    return;
  }

  *this << " MISMATCH" << std::endl;
  *this << std::setfill('*') << std::setw(62) << '*' << std::endl;
  *this << "PROCESSED FARE INFORMATION:" << std::setfill(' ') << std::endl;

  uint16_t last =
      ((_wantConstr && !ptFare.fare()->isConstructedFareInfoMissing()) ? FAIL_RD_LAST_CONSTRUCTED
                                                                       : FAIL_RD_LAST_GENERAL) +
      1;
  for (uint16_t counter = FAIL_RD_FIRST; counter < last; ++counter)
  {
    *this << DIAG291FIELDS[counter]
          << std::setw(25 - static_cast<int>(DIAG291FIELDS[counter].length())) << ' ';

    switch (counter)
    {
    case FAIL_RD_LINK:
      *this << baseFare->linkNumber();
      break;
    case FAIL_RD_SEQUENCE:
      *this << baseFare->sequenceNumber();
      break;
    case FAIL_RD_DATETIME:
      *this << baseFare->createDate().dateToString(DDMMMYY, "") << " "
            << baseFare->createDate().timeToSimpleString();
      break;
    case FAIL_RD_FARECLASS:
      *this << baseFare->fareWithoutBase()->fareClass();
      break;
    case FAIL_RD_FAREBASIS:
      *this << ptFare.createFareBasisCodeFD(*_fdTrx);
      break;
    case FAIL_RD_FAREAMOUNT:
      *this << ptFare.fare()->fareAmount();
      break;
    case FAIL_RD_FARETYPE:
      *this << ptFare.fcaFareType();
      break;
    case FAIL_RD_DISCOUNTED:
      *this << (ptFare.isDiscounted() ? 'Y' : 'N');
      break;
    case FAIL_RD_FBRULE:
      *this << (ptFare.isFareByRule() ? 'Y' : 'N');
      break;
    case FAIL_RD_NEGOTIATED:
      *this << (ptFare.isNegotiated() ? 'Y' : 'N');
      break;
    case FAIL_RD_SOURCE_PCC:
      {
        if (ptFare.hasCat35Filed())
        {
          const NegPaxTypeFareRuleData* negRuleData = ptFare.getNegRuleData();
          if (negRuleData && negRuleData->fareRetailerRuleId())
            *this << "ID:" << negRuleData->fareRetailerRuleId() << " PCC:" << negRuleData->sourcePseudoCity();
        }
        else if(ptFare.getAdjustedSellingCalcData())
        {
          *this << "ASL ID:" << ptFare.getAdjustedSellingCalcData()->getFareRetailerRuleId() << " PCC:"
              << ptFare.getAdjustedSellingCalcData()->getSourcePcc();
        }
      }
      break;
    case FAIL_RD_CONSTRUCTED:
      *this << (ptFare.fare()->isConstructedFareInfoMissing() ? 'N' : 'Y');
      break;
    case FAIL_RD_CAT35:
      *this << ptFare.fareDisplayCat35Type();
      break;
    case FAIL_RD_CARRIER:
      *this << ptFare.carrier();
      break;
    case FAIL_RD_CURRENCY:
      *this << ptFare.currency();
      break;
    case FAIL_RD_VENDOR:
      *this << (_fdTrx->isERD() ? ptFare.vendor() : baseFare->vendor());
      break;
    case FAIL_RD_TARIFF:
      *this << (_fdTrx->isERD() ? ptFare.fareTariff() : baseFare->fareTariff());
      break;
    case FAIL_RD_RULENUMBER:
      *this << ptFare.ruleNumber();
      break;
    case FAIL_RD_BOOKINGCODE:
      *this << ptFare.bookingCode();
      break;
    case FAIL_RD_ORIGIN:
      *this << FareDisplayUtil::getFareOrigin(&ptFare);
      break;
    case FAIL_RD_DESTINATION:
      *this << FareDisplayUtil::getFareDestination(&ptFare);
      break;
    case FAIL_RD_GATEWAY1:
      *this << ptFare.gateway1();
      break;
    case FAIL_RD_GATEWAY2:
      *this << ptFare.gateway2();
      break;
    case FAIL_RD_ORIGAO:
      *this << ptFare.origAddonFareClass();
      break;
    case FAIL_RD_DESTAO:
      *this << ptFare.destAddonFareClass();
      break;
    case FAIL_RD_CONSTRUCTIONTYPE:
      *this << ptFare.constructionType();
      break;
    case FAIL_RD_SPECIFIED_AMOUNT:
      *this << ptFare.specifiedFareAmount();
      break;
    case FAIL_RD_CONSTR_NUC_AMOUNT:
      *this << ptFare.constructedNucAmount();
      break;
    default:
      *this << DIAG291FIELDS[FAIL_RD_LAST]; // Unknown
      break;
    }
    *this << std::endl;
  }
}
} // namespace
