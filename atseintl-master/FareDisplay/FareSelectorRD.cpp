//-------------------------------------------------------------------
//
//  File:        FareSelectorRD.cpp
//  Created:     October 7, 2005
//  Authors:     Doug Batchelor
//
//  Updates:
//
//  Copyright Sabre 2003
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

#include "FareDisplay/FareSelectorRD.h"

#include "Common/FareDisplayUtil.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DBAccess/PaxTypeInfo.h"
#include "Diagnostic/DCFactoryFareDisplay.h"
#include "Diagnostic/Diag291CollectorFD.h"

namespace tse
{
// -------------------------------------------------------------------
//
// @MethodName  FareSelectorRD:setup()
// set handy pointers in this object
//
// @param trx - a reference to a FareDisplayTrx.
//
// @return bool
//
// -------------------------------------------------------------------------
bool
FareSelectorRD::setup(FareDisplayTrx& trx)
{
  FareSelector::setup(trx);

  _fdo = trx.getOptions();
  _request = trx.getRequest();

  _faresSelected = 0;
  _faresProcessed = 0;
  _faresMatched.clear();

  _wantDisc = _fdo->discountedValues().isNonPublishedFare();
  _wantFbr = _fdo->cat25Values().isNonPublishedFare();
  _wantNeg = _fdo->cat35Values().isNonPublishedFare();
  _wantConstr = _fdo->constructedAttributes().isConstructedFare();

  return true;
}

bool
FareSelectorRD::matchAddon(PaxTypeFare& ptFare, AddOnAttributes& addOn, bool isOrig)
{
  if (isOrig)
  {
    return (addOn.addonFootNote1() == ptFare.origAddonFootNote1() &&
            addOn.addonFootNote2() == ptFare.origAddonFootNote2() &&
            addOn.addonFareClass() == ptFare.origAddonFareClass() &&
            addOn.addonTariff() == ptFare.origAddonTariff() &&
            addOn.addonRouting() == ptFare.origAddonRouting() &&
            isEqual(addOn.addonAmount(), ptFare.origAddonAmount()) &&
            addOn.addonCurrency() == ptFare.origAddonCurrency() &&
            addOn.oWRT() == ptFare.origAddonOWRT());
  }
  else
  {
    return (addOn.addonFootNote1() == ptFare.destAddonFootNote1() &&
            addOn.addonFootNote2() == ptFare.destAddonFootNote2() &&
            addOn.addonFareClass() == ptFare.destAddonFareClass() &&
            addOn.addonTariff() == ptFare.destAddonTariff() &&
            addOn.addonRouting() == ptFare.destAddonRouting() &&
            isEqual(addOn.addonAmount(), ptFare.destAddonAmount()) &&
            addOn.addonCurrency() == ptFare.destAddonCurrency() &&
            addOn.oWRT() == ptFare.destAddonOWRT());
  }
}

bool
FareSelectorRD::matchPaxType(PaxTypeFare& ptFare)
{
  LOG4CXX_INFO(_logger, " Entered FareSelectorRD::matchPaxType()");
  //  if (ptFare.fareMarket()->isChildNeeded() && ptFare.paxType()->paxTypeInfo()->childInd() ==
  // 'Y')
  if (ptFare.fareMarket()->isChildNeeded() &&
      (ptFare.paxType()->paxTypeInfo()->isChild() ||
       ptFare.paxType()->paxTypeInfo()->childInd() == 'Y' ||
       ptFare.paxType()->paxTypeInfo()->psgGroupType() == "CH$"))
  {
    return true;
  }
  //  if (ptFare.fareMarket()->isInfantNeeded() && ptFare.paxType()->paxTypeInfo()->infantInd() ==
  // 'Y')
  if (ptFare.fareMarket()->isInfantNeeded() &&
      (ptFare.paxType()->paxTypeInfo()->isInfant() ||
       ptFare.paxType()->paxTypeInfo()->infantInd() == 'Y' ||
       ptFare.paxType()->paxTypeInfo()->psgGroupType() == "IN$"))
  {
    return true;
  }

  if (!ptFare.fareMarket()->isChildNeeded() && !ptFare.fareMarket()->isInfantNeeded() &&
      (ptFare.paxType()->paxTypeInfo()->isAdult() ||
       (!ptFare.paxType()->paxTypeInfo()->isChild() &&
        !ptFare.paxType()->paxTypeInfo()->isInfant())))
  {
    return true;
  }
  LOG4CXX_INFO(_logger,
               " Entered FareSelectorRD::matchPaxType() - FALSE -> CHD="
                   << ptFare.fareMarket()->isChildNeeded()
                   << " INF=" << ptFare.fareMarket()->isInfantNeeded());
  return false;
}

// -------------------------------------------------------------------
// @MethodName  FareSelectorRD::selectFares()
//
// Iterate through all PaxTypefares filtering out the all but
// the one and only fare for the short RD request.
//
// @param trx - a reference to a FareDisplayTrx.
//
// @return bool
// -------------------------------------------------------------------------
bool
FareSelectorRD::selectFares(FareDisplayTrx& trx)
{
  LOG4CXX_INFO(_logger, " Entered FareSelectorRD::selectFaresNew()");
  Diag291CollectorFD* dc291 = nullptr;

  if (trx.diagnostic().isActive() &&
      //     trx.getRequest()->diagnosticNumber() == DIAG_291_ID)
      trx.diagnostic().diagnosticType() == Diagnostic291)
  {
    dc291 = dynamic_cast<Diag291CollectorFD*>(DCFactoryFareDisplay::instance()->create(trx));
    if (dc291)
    {
      _wantDiag291 = true;
      dc291->enable(Diagnostic291);
      dc291->init(trx);
      dc291->printHeader();
    }
  }

  const PaxTypeFare* baseFare = nullptr;
  Itin* itin = trx.itin().front();

  std::vector<FareMarket*>::const_iterator fmItr = itin->fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmEnd = itin->fareMarket().end();
  for (; fmItr != fmEnd; fmItr++)
  {
    FareMarket& fareMarket = **fmItr;

    std::vector<PaxTypeFare*>::const_iterator ptfItr = fareMarket.allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::const_iterator ptfEnd = fareMarket.allPaxTypeFare().end();

    for (; ptfItr != ptfEnd; ptfItr++)
    {
      if ((*ptfItr) == nullptr || !(*ptfItr)->isValid())
        continue;

      _faresProcessed++;

      PaxTypeFare& ptFare = **ptfItr;
      if (ptFare.carrier() == INDUSTRY_CARRIER)
        getBookingCode(trx, ptFare);

      baseFare = ptFare.fareWithoutBase();
      LOG4CXX_DEBUG(_logger,
                    "Now considering FareClass " << baseFare->fareClass() << " FareBasisCode "
                                                 << ptFare.createFareBasisCodeFD(trx));

      Diag291CollectorFD::D291FailCode failCode = Diag291CollectorFD::FAIL_RD_MATCHED;

      if (baseFare->fareClass() != _fdo->fareClass())
      {
        failCode = Diag291CollectorFD::FAIL_RD_FARECLASS;
      }
      else if (!trx.isERD() && baseFare->linkNumber() != _fdo->linkNumber())
      {
        failCode = Diag291CollectorFD::FAIL_RD_LINK;
      }
      else if (!trx.isERD() && baseFare->sequenceNumber() != _fdo->sequenceNumber())
      {
        failCode = Diag291CollectorFD::FAIL_RD_SEQUENCE;
      }
      else if (!trx.isERD() && (!baseFare->createDate().isValid() ||
                                baseFare->createDate().dateToString(DDMMMYY, "") !=
                                    _fdo->createDate().dateToString(DDMMMYY, "") ||
                                baseFare->createDate().timeToSimpleString() != _fdo->createTime()))
      {
        failCode = Diag291CollectorFD::FAIL_RD_DATETIME;
      }
      else if (ptFare.createFareBasisCodeFD(trx) != _request->fareBasisCode())
      {
        failCode = Diag291CollectorFD::FAIL_RD_FAREBASIS;
      }
      else
      {
        //  -- Request calcFareAmount comes from PSS and will never have more than 9 digits
        //        - because FQ only sends 9 digits
        //  - and the fare-fareAmount may have more - because it is calculated by FareSelection as
        // for FQ
        //  - and is not truncated in any way.   One example had
        //        calcFareAmount as 160678.791 and
        //        fareAmount     as 160678.7911.        These do not match.
        //
        //  It appears that one way to make fare->fareAmount match calcFareAmount is to
        //  make a copy of fareAmount and process it in
        //  the same way we do when we send the value to PSS in the FareQutoe response.
        //  This was copied from FareDisplayResponseFormatter.cpp.
        std::ostringstream FAstr;
        FAstr.precision(9);
        FAstr << ptFare.fare()->fareAmount();
        MoneyAmount fareAdjustedFareAmount = atof(FAstr.str().c_str());
        NegPaxTypeFareRuleData* negRuleData = nullptr;

        if (!isEqual(_request->calcFareAmount(),
                     fareAdjustedFareAmount)) // checks to RD_EPSILON = 0.0001
        {
          failCode = Diag291CollectorFD::FAIL_RD_FAREAMOUNT;
        }
        else if (!_fdo->fareType().empty() && ptFare.fcaFareType() != _fdo->fareType())
        {
          failCode = Diag291CollectorFD::FAIL_RD_FARETYPE;
        }
        else if (ptFare.isDiscounted() != _wantDisc)
        {
          failCode = Diag291CollectorFD::FAIL_RD_DISCOUNTED;
        }
        else if (ptFare.isFareByRule() != _wantFbr)
        {
          failCode = Diag291CollectorFD::FAIL_RD_FBRULE;
        }
        else if (ptFare.isNegotiated() != _wantNeg)
        {
          failCode = Diag291CollectorFD::FAIL_RD_NEGOTIATED;
        }
        else if (ptFare.fare()->isConstructedFareInfoMissing() == _wantConstr)
        {
          failCode = Diag291CollectorFD::FAIL_RD_CONSTRUCTED;
        }
        else if (ptFare.hasCat35Filed() && ptFare.fareDisplayCat35Type() != _fdo->FDcat35Type())
        {
          failCode = Diag291CollectorFD::FAIL_RD_CAT35;
        }
        else if (ptFare.carrier() != _fdo->carrierCode())
        {
          failCode = Diag291CollectorFD::FAIL_RD_CARRIER;
        }
        else if (ptFare.currency() != _fdo->baseFareCurrency())
        {
          failCode = Diag291CollectorFD::FAIL_RD_CURRENCY;
        }
        else if (ptFare.fareTariff() != _fdo->fareTariff())
        {
          failCode = Diag291CollectorFD::FAIL_RD_TARIFF;
        }
        else if ((trx.isERD() && ptFare.vendor() != _fdo->vendorCode()) ||
                 (!trx.isERD() && baseFare->vendor() != _fdo->vendorCode()))
        {
          failCode = Diag291CollectorFD::FAIL_RD_VENDOR;
        }
        else if (!_fdo->ruleNumber().empty() && _fdo->ruleNumber() != ptFare.ruleNumber())
        {
          failCode = Diag291CollectorFD::FAIL_RD_RULENUMBER;
        }
        else if((!ptFare.hasCat35Filed() && !_fdo->getSourcePCCForCat35().empty())
            || (!ptFare.getAdjustedSellingCalcData() && !_fdo->getSourcePCCForASL().empty()))
        {
          failCode = Diag291CollectorFD::FAIL_RD_SOURCE_PCC;
        }
        else if (ptFare.hasCat35Filed() && (negRuleData = ptFare.getNegRuleData()) &&
                 _fdo->getSourcePCCForCat35() != negRuleData->sourcePseudoCity())
        {
          failCode = Diag291CollectorFD::FAIL_RD_SOURCE_PCC;
        }
        else if(ptFare.getAdjustedSellingCalcData()
            && _fdo->getSourcePCCForASL() != ptFare.getAdjustedSellingCalcData()->getSourcePcc())
        {
          failCode = Diag291CollectorFD::FAIL_RD_SOURCE_PCC;
        }
        else if (_wantConstr)
        {
          // Make the FareList Fares specifiedFareAmount be the same precision as that coming in
          // from the request.
          char tmpBuf[30];
          sprintf(tmpBuf, "%-.*f", 2, ptFare.specifiedFareAmount());
          MoneyAmount fareAdjustedFareAmount = atof(tmpBuf);

          if (_fdo->constructedAttributes().gateway1() != ptFare.gateway1())
          {
            failCode = Diag291CollectorFD::FAIL_RD_GATEWAY1;
          }
          else if (_fdo->constructedAttributes().gateway2() != ptFare.gateway2())
          {
            failCode = Diag291CollectorFD::FAIL_RD_GATEWAY2;
          }
          else if (_fdo->constructedAttributes().constructionType() != ptFare.constructionType())
          {
            failCode = Diag291CollectorFD::FAIL_RD_CONSTRUCTIONTYPE;
          }
          else if (!isEqual(_fdo->constructedAttributes().specifiedFareAmount(),
                            fareAdjustedFareAmount))
          {
            failCode = Diag291CollectorFD::FAIL_RD_SPECIFIED_AMOUNT;
          }
          else if (!isEqual(_fdo->constructedAttributes().constructedNucAmount(),
                            ptFare.constructedNucAmount()))
          {
            failCode = Diag291CollectorFD::FAIL_RD_CONSTR_NUC_AMOUNT;
          }
          else if (_fdo->origAttributes().isAddOn() &&
                   !matchAddon(ptFare, _fdo->origAttributes(), true))
          {
            failCode = Diag291CollectorFD::FAIL_RD_ORIGAO;
          }
          else if (_fdo->destAttributes().isAddOn() &&
                   !matchAddon(ptFare, _fdo->destAttributes(), false))
          {
            failCode = Diag291CollectorFD::FAIL_RD_DESTAO;
          }
        }
      }

      if (failCode == Diag291CollectorFD::FAIL_RD_MATCHED)
      {
        // Should not perform this match, if we are
        // merging similar looking fares
        // in long entries but have different
        // pax types. If the paxtypes are different
        // then they may have different itemNo. We want to
        // merge all these fares even in short request.
        // If we did this matching, we will invalidate
        // those paxtypes for which the rule item info
        // send back by PSS do not match
        if (!qualifyThisFare(trx, ptFare))
        {
          failCode = Diag291CollectorFD::FAIL_RD_QUALIFY;
        }
        else if (!(trx.isERD() && ptFare.fcasPaxType().empty()) && !matchPaxType(ptFare))
        {
          failCode = Diag291CollectorFD::FAIL_RD_PAXTYPE;
        }
        // Multi-airport fares may have different origin destination
        else if (!_request->fareOrigin().empty() &&
                 FareDisplayUtil::getFareOrigin(&ptFare) != _request->fareOrigin())
        {
          failCode = Diag291CollectorFD::FAIL_RD_ORIGIN;
        }
        else if (!_request->fareDestination().empty() &&
                 FareDisplayUtil::getFareDestination(&ptFare) != _request->fareDestination())
        {
          failCode = Diag291CollectorFD::FAIL_RD_DESTINATION;
        }
        // If a booking was sent as part of the RD request, validate that it matches
        // the booking code in the ptf. If the ptf does not have a booking code it'll
        // need to be retrieved to enable the validation check.
        // If a match is not made invalidate the fare.
        else if (!_request->bookingCode().empty())
        {
          if (ptFare.bookingCode().empty())
          {
            getBookingCode(trx, ptFare);
          }

          if ((trx.isERD() && ptFare.bookingCode()[0] != _request->bookingCode()[0]) ||
              (!trx.isERD() && ptFare.bookingCode() != _request->bookingCode()))
          {
            failCode = Diag291CollectorFD::FAIL_RD_BOOKINGCODE;
          }
        }
      }

      if (failCode == Diag291CollectorFD::FAIL_RD_MATCHED)
      {
        LOG4CXX_DEBUG(_logger,
                      "Selected Fare with fareclass " << baseFare->fareClass() << " FareBasisCode "
                                                      << ptFare.createFareBasisCodeFD(trx));
        _faresMatched.push_back(&ptFare);
      }
      else
      {
        ptFare.invalidateFare(PaxTypeFare::FD_Fare_Not_Selected_for_RD);
      }

      if (_wantDiag291)
      {
        if (ptFare.bookingCode().empty())
        {
          getBookingCode(trx, ptFare);
        }
        dc291->printFare(ptFare, failCode);
      }
    } // all PTFs
  } // all FMs

  // success if atease one fare selected
  LOG4CXX_DEBUG(_logger, "Number of fares selected: " << _faresMatched.size());
  if (_wantDiag291)
  {
    dc291->printFooter(_faresMatched.size(), _faresProcessed);
    dc291->flushMsg();

    return 0;
  }

  for (const auto elem : _faresMatched)
  {
    checkInhibitIndicator(trx, *elem);
  }

  return !_faresMatched.empty();
}
} //tse
