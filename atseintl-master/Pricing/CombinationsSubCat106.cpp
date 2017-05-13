//-------------------------------------------------------------------
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "Pricing/Combinations.h"
#include "Pricing/CombinationsSubCat106.h"

#include "DBAccess/CarrierCombination.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/OpenJawRestriction.h"

namespace tse
{

log4cxx::LoggerPtr
CombinationsSubCat::_logger(log4cxx::Logger::getLogger("atseintl.Pricing.CombinationsSubCat106"));

bool
CombinationsSubCat106::match()
{
  LOG4CXX_INFO(_logger, "Entered: CombinationsSubCat106.match ()");

  // char ret = IDLE;
  char ret = PASS;

  std::vector<CarrierCombination*>::const_iterator iter;
  std::vector<CarrierCombination*>::const_iterator iterEnd;

  const std::vector<CarrierCombination*>& carrierCombVector =
      _trx.dataHandle().getCarrierCombination(_vendor, _itemNo);

  if (UNLIKELY(carrierCombVector.empty()))
  {
    LOG4CXX_INFO(_logger, " carrierCombList.empty().......");
    displayDiagError();
    return false;
  }

  DiagCollectorGuard dcg1(_diag, _diagnostic);
  if (UNLIKELY(_diag.isActive()))
  {
    _diag << std::endl << "  ------";
    displayDiagItem();
  }

  size_t numOfFU = _components.size();

  for (size_t fuCount = 0; fuCount < numOfFU; ++fuCount)
  {
    if (!_components.needAllPassSameMajorItem())
    {
      _components[fuCount]._passMinor = false;
      if (_components[fuCount]._passMajor)
        continue;
    }
    else if (UNLIKELY(_components[fuCount]._passMinor))
    {
      if (_diag.isActive())
      {
        _diag << " PASSED MINOR ";
        if (_components[fuCount].getSubCat(Combinations::m106) == Combinations::MATCH)
          _diag << "MATCH106\n";
        else
          _diag << "NOT MATCH106\n";
      }

      continue;
    }
    if (_components[fuCount]._targetFareUsage == _components[fuCount]._currentFareUsage)
    {
      continue;
    }

    FareUsage& targetFU = *_components[fuCount]._targetFareUsage;
    PaxTypeFare* targetFare = targetFU.paxTypeFare();
    const CombinabilityRuleInfo* pTargetCat10 = targetFU.rec2Cat10();

    targetFare = Combinations::validateFareCat10overrideCat25(_fu.paxTypeFare(), targetFare, &_prU);
    if (targetFare != targetFU.paxTypeFare() && targetFare->rec2Cat10())
      pTargetCat10 = targetFare->rec2Cat10();

    const bool targetFUIsIndustry = targetFare->fare()->isIndustry();

    if (UNLIKELY(_diag.isActive()))
    {
      _diag << "  COMBINING WITH FARE " << targetFare->createFareBasis(nullptr);

      if (targetFUIsIndustry)
      {
        _diag << " INDUSTRY FARE GOV-CARRIER: "
              << targetFare->fareMarket()->governingCarrier();
      }
      _diag << std::endl;
    }

    if (UNLIKELY(!pTargetCat10))
    {
      LOG4CXX_INFO(_logger, "processCarrierRestriction: NO REC 2 CAT 10");
      if (_diag.isActive())
        _diag << " TARGET FU NO REC2 CAT 10\n";

      return false;
    }

    ret = Combinations::IDLE;

    iter = carrierCombVector.begin();
    iterEnd = carrierCombVector.end();

    for (; iter != iterEnd; ++iter)
    {
      CarrierCombination* pCarrierRestrictions = *iter;

      //---------------------------------------------------------------
      //----- determine if negative 106 ---------
      //---------------------------------------------------------------
      //----- pCarrierRestrictions->carrierCombAppl contain
      //----- blank = both domestic and international allowed
      //-----   N   = both domestic and international not allowed
      //-----   1   = US/CA domestic only allowed
      //-----                    (does not allow international)
      //-----   2   = US/CA domestic not allowed
      //-----                    (international is allowed)
      //-----   3   = US/CA domestic joints allowed
      //-----   4   = US/CA domestic joints allowed
      //----- note: allowed means allowed to combine with restriction
      //---------------------------------------------------------------

      if (UNLIKELY(_diag.isActive()))
      {
        _diag << "   CARRIER COMB APPL - " << pCarrierRestrictions->carrierCombAppl() << std::endl;
      }

      // IF it is joint fair,  invalidate it since joint fares are not allowed.
      if (UNLIKELY(pCarrierRestrictions->carrierCombAppl() == '3' ||
          pCarrierRestrictions->carrierCombAppl() == '4'))
      {
        ret = NO_MATCH;
      }
      else
      {
        if (pCarrierRestrictions->carrierCombAppl() == 'N' ||
            (pCarrierRestrictions->carrierCombAppl() == '2' &&
             _prU.geoTravelType() != GeoTravelType::International))
        {
          _negativeApplication = true;
        }

        std::vector<CarrierCode>& cxrVec = pCarrierRestrictions->carriers();
        std::vector<CarrierCode>::iterator ccIter;
        std::vector<CarrierCode>::iterator ccIterEnd;
        CarrierCode carrier;

        //----- if positive & no CXR & between & have GEO -----
        //-----------------------------------------------------
        if (UNLIKELY(!cxrVec.empty() && pCarrierRestrictions->carrierCombAppl() == ' ' &&
            (cxrVec[0].empty() || cxrVec[0] == Combinations::MATCH_ANY_CARRIER) &&
            pCarrierRestrictions->tvlSecConstInd() == 'B' &&
            pCarrierRestrictions->loc1Type() != ' '))
        {
          ret = Combinations::MATCH;
          //--- skip carrier check ---
          //--- will continue validate TSI in next routine ---
        }
        else if (UNLIKELY(cxrVec.empty()))
        {
          ret = Combinations::MATCH;
        }
        //----- check carrier(s) ------------------------------
        //-----------------------------------------------------
        else
        {
          ccIter = cxrVec.begin();
          ccIterEnd = cxrVec.end();

          if (UNLIKELY(_diag.isActive()))
            _diag << "   CARRIER -";

          bool carrierExist = false;
          for (; ccIter != ccIterEnd; ccIter++)
          {
            carrier = *ccIter;
            if (!carrier.empty())
            {
              carrierExist = true;
            }
            else if (carrierExist)
            {
              // this carrier is empty and we did find one before
              break;
            }

            if (UNLIKELY(_diag.isActive()))
              _diag << " " << carrier;

            //----- if no more carrier or wildcard -----
            if (carrier.empty() || carrier == Combinations::MATCH_ANY_CARRIER ||
                carrier == DOLLAR_CARRIER)
            {
              ret = Combinations::MATCH;
              break;
            }

            if (carrier == pTargetCat10->carrierCode() &&
                pTargetCat10->carrierCode() != JOINT_CARRIER &&
                pCarrierRestrictions->carrierCombAppl() != '3')
            {
              ret = Combinations::MATCH;
              break;
            }

            if (UNLIKELY(pTargetCat10->carrierCode() == ANY_CARRIER))
            {
              if (carrier == _fu.paxTypeFare()->fareMarket()->governingCarrier())
              {
                ret = Combinations::MATCH;
                break;
              }
            }

            // If targetFU is industry, carrier validation
            // should be against its governingCarrer
            if (targetFUIsIndustry &&
                carrier == targetFare->fareMarket()->governingCarrier())
            {
              ret = Combinations::MATCH;
              break;
            }

          } // for carrierCount
          _diag << std::endl;

        } // if check carrier (s)

        //----- if carrier match data, check TRVL -----------
        if (ret == Combinations::MATCH)
        {
          //----- check over water restrictions --------
          //--------------------------------------------
          //----- pCarrierRestrictions->tvlSecOverWaterInd contain
          //----- blank = not apply
          //-----   X   = overwater (between continental US/CA and
          //-----          Alasaka/Hawaii/points in the Caribbean;
          //-----          between Alaska and Hawaii)

          //----- check over water restrictions --------
          if (UNLIKELY(pCarrierRestrictions->tvlSecOverwaterInd() == Combinations::RESTRICTION_APPLIES))
          {
            if (_diag.isActive())
            {
              _diag << "   TRAVEL SECTOR OVERWATER IND - "
                    << pCarrierRestrictions->tvlSecOverwaterInd() << std::endl;
            }

            const Loc* tOrig = targetFare->fareMarket()->origin();
            const Loc* tDest = targetFare->fareMarket()->destination();

            if (LocUtil::isOverWater(*tOrig, *tDest))
            {
              ret = Combinations::MATCH;
            }
          }

          //----- check international restriction ------
          //--------------------------------------------
          //----- pCarrierRestrictions->tvlSecIntlInd contain
          //----- blank = not apply
          //-----   X   = international (between continental US/CA and
          //-----          any point outside US/CA or between any two
          //-----          countries other than US/CA)

          //----- check international restriction ------
          if (pCarrierRestrictions->tvlSecIntlInd() == Combinations::RESTRICTION_APPLIES)
          {
            if (_diag.isActive())
            {
              _diag << "   TRAVEL SECTOR INTERNATIONAL IND - "
                    << pCarrierRestrictions->tvlSecIntlInd() << std::endl;
            }

            if (targetFare->isInternational())
            {
              ret = Combinations::MATCH;
            }
          }

          //----- check TSI/Geographic restriction -----
          //--------------------------------------------
          if (LIKELY(pCarrierRestrictions->tvlSectSi() == 0))
          {
            if (UNLIKELY(_diag.isActive()))
            {
              _diag << "   TRAVEL SEGMENT IND - " << pCarrierRestrictions->tvlSectSi() << std::endl;
            }

            matchGEO(ret, *pCarrierRestrictions, *targetFare, *pTargetCat10);
          }

          if (ret == Combinations::MATCH)
            break;
        } // end if MATCH
        else if (UNLIKELY(ret == Combinations::ABORT))
          break;
      }
    } // for iter

    //--- check ret ---
    if (UNLIKELY(ret == Combinations::ABORT))
    {
      return false;
    }
    else if (ret == Combinations::MATCH)
    {
      if (UNLIKELY(_diag.isActive()))
        _diag << "     MATCH" << std::endl;
      _components[fuCount].getSubCat(Combinations::m106) = Combinations::MATCH;
      ret = Combinations::MATCH;
    }
    else
    {
      if (UNLIKELY(_diag.isActive()))
        _diag << "     NO MATCH" << std::endl;
      _components[fuCount].getSubCat(Combinations::m106) = Combinations::NO_MATCH;
      ret = NO_MATCH;
    }
  } // for fuCount

  return (ret != Combinations::ABORT);
}

void
CombinationsSubCat106::matchGEO(char& ret,
                                const CarrierCombination& cxrComb,
                                const PaxTypeFare& targetFare,
                                const CombinabilityRuleInfo& targetCat10)
{
  if (!cxrComb.loc1().empty())
  {
    const tse::LocCode& orig = targetFare.fareMarket()->boardMultiCity();
    const tse::LocCode& dest = targetFare.fareMarket()->offMultiCity();
    const VendorCode& vendor = targetCat10.vendorCode();

    displayDiag(cxrComb, orig, dest);

    if (cxrComb.loc2().empty())
    {
      if (isInLoc(orig, cxrComb.loc1Type(), cxrComb.loc1(), vendor) ||
          isInLoc(dest, cxrComb.loc1Type(), cxrComb.loc1(), vendor))
        ret = Combinations::MATCH;
      else
        ret = Combinations::NO_MATCH;

      return;
    }

    if (isInLoc(orig, cxrComb.loc1Type(), cxrComb.loc1(), vendor))
    {
      if (isInLoc(dest, cxrComb.loc2Type(), cxrComb.loc2(), vendor))
      {
        ret = Combinations::MATCH;
        return;
      }
    }

    // performance: don't try to match orig-loc2 if succesfully matched orig-loc1
    else if (isInLoc(orig, cxrComb.loc2Type(), cxrComb.loc2(), vendor) &&
             isInLoc(dest, cxrComb.loc1Type(), cxrComb.loc1(), vendor))
    {
      ret = Combinations::MATCH;
      return;
    }

    ret = Combinations::NO_MATCH;
  }
}

bool
CombinationsSubCat106::isInLoc(const LocCode& validatingLoc,
                               const Indicator restrictionLocType,
                               const LocCode& restrictionLoc,
                               const VendorCode& validatingVendor) const
{
  return LocUtil::isInLoc(validatingLoc,
                          restrictionLocType,
                          restrictionLoc,
                          validatingVendor,
                          RESERVED,
                          GeoTravelType::International,
                          LocUtil::OTHER,
                          _trx.getRequest()->ticketingDT());
}

void
CombinationsSubCat106::displayDiag(const CarrierCombination& cxrComb,
                                   const LocCode& orig,
                                   const LocCode& dest)
{
  if (UNLIKELY(_diag.isActive()))
  {
    _diag << "     VALIDATE - " << cxrComb.tvlSecConstInd() << std::endl << "              "
          << "  LOC 1: " << cxrComb.loc1Type() << " - " << cxrComb.loc1()
          << "  LOC 2 : " << cxrComb.loc2Type() << " - " << cxrComb.loc2() << std::endl
          << "              "
          << "  ORG: " << orig << "  DES: " << dest << std::endl;
  }
}
}
