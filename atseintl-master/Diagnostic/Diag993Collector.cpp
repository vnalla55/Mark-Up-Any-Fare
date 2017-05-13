//----------------------------------------------------------------------------
//  File:        Diag993Collector.cpp
//
//
//  Copyright Sabre 2010
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

#include "Diagnostic/Diag993Collector.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/FareMarketUtil.h"
#include "Common/FareTypeMatrixUtil.h"
#include "Common/Money.h"
#include "Common/ShoppingUtil.h"
#include "Common/TNBrands/ItinBranding.h"
#include "Common/TNBrands/TNBrandsFunctions.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FlexFares/Types.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "Pricing/MergedFareMarket.h"
#include "Util/FlatSet.h"


#include <algorithm>
#include <iomanip>
#include <vector>

namespace tse
{

namespace
{
const std::string SEPARATOR = "- -- - ---- ------------ --- --- - - -------- --- --- --- --------";

const std::string SEPARATORAWARD =
    "- -- - ---- ------------ --- - - -------- --- -------- --- --- --------";

const std::string SEPARATORSURCHARGES =
    "- -- - ---- ------------ --- --- - - -------- --- -------- --------";

class PrintUtil
{
  tse::Diag993Collector& _dc;

public:
  PrintUtil(tse::Diag993Collector& dc) : _dc(dc) {}

  void operator()(const TravelSeg* segment)
  {

    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(segment);
    if (airSeg)
    {
      _dc << " " << airSeg->carrier();
      _dc << " " << airSeg->flightNumber();
    }
  }

  void operator()(const BookingCode& bkg) { _dc << bkg << " "; }
};

Vector<const PaxTypeFare*>
getCat12OrYqyrFares(const PrecalculatedTaxes& precalculatedTax)
{
  FlatSet<const PaxTypeFare*> fares;

  for (const auto amount : precalculatedTax)
  {
    if (!amount.second.has(PrecalculatedTaxesAmount::CAT12) &&
        !amount.second.has(PrecalculatedTaxesAmount::YQYR))
      continue;

    fares.insert(amount.first);
  }

  return fares.steal_container();
}

struct FareComparatorByAmmount
{
  bool operator()(const PaxTypeFare* firstFare, const PaxTypeFare* secondFare) const
  {
    return firstFare->nucFareAmount() < secondFare->nucFareAmount();
  }
};

} // unnamed namespace

void
Diag993Collector::printNewItin(const Itin& itin)
{
  writeSeparator(DIVIDER);
  if (itin.itinNum() != INVALID_INT_INDEX)
  {
    *this << "ITIN: " << itin.itinNum() << "\n";
  }
}

void
Diag993Collector::writeBrandInfo(uint16_t itinIndex,
                                 uint16_t brandIndex)
{
  const PricingTrx& trx = pricingTrx();

  Diag993Collector& dc = *this;

  dc << "ITIN INDEX: " << itinIndex << "  BRAND ID: ";

  if(trx.getRequest()->isBrandedFaresRequest())
    dc << (trx.validBrands().empty() ? "N/A" : (trx.validBrands()[brandIndex]));
  else
    dc << trx.getRequest()->brandId(brandIndex);

  dc << std::endl;

  if (trx.getRequest()->isBrandedFaresRequest())
    return;

  const std::vector<BookingCode>& brandedBKC(trx.getRequest()->brandedFareBookingCode(brandIndex));

  dc << "BRANDED BKC: ";
  std::for_each(brandedBKC.begin(), brandedBKC.end(), PrintUtil(dc));
  dc << "\n";
}

void
Diag993Collector::writeBrandCombinationInfo(const std::string& brands)
{
  *this << "BRAND COMBINATION: " << brands << std::endl;
}

void Diag993Collector::printMergerHeader()
{
  *this << "\nFARE MARKET MERGER STAGE:\n";
}

void Diag993Collector::printMergerFooter()
{
  *this << "\nEND FARE MARKET MERGER STAGE\n";
}

void Diag993Collector::displayInputFareMarkets(const std::vector<FareMarket*>& allGovCxrFM)
{
  *this << "\nFARE MARKETS BEING MERGED:" << std::endl;

  for (FareMarket* fm : allGovCxrFM)
  {
    TSE_ASSERT(fm != nullptr);

    *this << fm->origin()->loc() << "-" << fm->governingCarrier()
          << "-" << fm->destination()->loc() << std::endl;
  }
  *this << std::endl;
}

void
Diag993Collector::displaySoldoutStatus(IbfErrorMessage status)
{
  *this << "MERGED SOLDOUT STATUS: " << ShoppingUtil::getIbfErrorCode(status) << std::endl;
}

void
Diag993Collector::writeUseAnyBrand(const MergedFareMarket& mfm,
                                   const BrandCode& origBrand)
{
  *this << "MERGED FARE MARKET " << mfm.origin()->loc() << "-" << mfm.destination()->loc()
    << " WILL ALLOW ANY BRAND (WAS: " << origBrand << ")" << std::endl;
}

void
Diag993Collector::displayCbsX()
{
  *this << "CBS=X TRIGGERED\n";
}

void
Diag993Collector::displayValidFareFound(const FareClassCode& fareClass, const BrandCode& brandCode)
{
  *this << "FARE " << fareClass << " MATCHES BRAND " << brandCode << std::endl;
}

void
Diag993Collector::displayNoValidFaresForBrand(const BrandCode& brandCode)
{
  *this << "NO FARES FILED FOR BRAND " << brandCode << std::endl;
}

void
Diag993Collector::displayCbsNotAllowedForLeg()
{
  *this << "CBS NOT ALLOWED ON THIS LEG\n";
}

void
Diag993Collector::printMFM(const MergedFareMarket& mergedFareMarket)
{
    *this << mergedFareMarket << std::endl;
}

void
Diag993Collector::printMergedFareMarketVect(const std::vector<MergedFareMarket*>& mergedFareMarketVect,
                                            flexFares::GroupId groupId,
                                            const skipper::BrandingOptionSpace& brandingOptionSpace,
                                            const skipper::ItinBranding& itinBranding)
{
  if (mergedFareMarketVect.empty())
    *this << "NO VALID FARE MARKET" << std::endl << std::endl << std::endl;

  for (auto mergedFareMarket : mergedFareMarketVect)
  {
    printMergedFareMarket(*mergedFareMarket, groupId, brandingOptionSpace, itinBranding);
    *this << std::endl;
  }
}

void
Diag993Collector::printMergedFareMarket(const MergedFareMarket& mergedFareMarket,
                                        flexFares::GroupId groupId,
                                        const skipper::BrandingOptionSpace& brandingOptionSpace,
                                        const skipper::ItinBranding& itinBranding)
{
  for (auto fareMarket : mergedFareMarket.mergedFareMarket())
  {
    printFareMarket(*fareMarket, groupId, brandingOptionSpace, itinBranding);
    *this << std::endl;
  }
}

void
Diag993Collector::printFareMarket(const FareMarket& fareMarket,
                                  flexFares::GroupId groupId,
                                  const skipper::BrandingOptionSpace& brandingOptionSpace,
                                  const skipper::ItinBranding& itinBranding)
{
  if (!_active)
  {
    return;
  }

  // If we dont have travel segments, we count output this line
  if (fareMarket.travelSeg().empty())
  {
    return;
  }

  Diag993Collector& dc = *this;
  dc << " \n" << FareMarketUtil::getFullDisplayString(fareMarket) << "\n ";
  // print segment info
  std::for_each(fareMarket.travelSeg().begin(), fareMarket.travelSeg().end(), PrintUtil(dc));
  dc << "\n \n";

  dc << "BRAND CODE :";
  std::map<Direction, BrandCode> brands =
      itinBranding.getBrandCodeForMarket(fareMarket, brandingOptionSpace);
  for (const auto& b: brands)
    dc << " " << b.second << " (" << directionToIndicator(b.first) << ")";
  dc << "\nGEOTRAVELTYPE : ";
  dc << DiagnosticUtil::geoTravelTypeToString(fareMarket.geoTravelType());
  dc << "\n \n";

  if (pricingTrx().isFlexFare())
  {
    dc << "FLEX FARES GROUP ID: " << groupId << "\n";
    dc << "TOTAL PAXTYPEFARES : " << fareMarket.allPaxTypeFare().size() << "\n";
  }

  const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();
  if (!paxTypeCortegeVec.empty())
  {
    std::vector<PaxTypeBucket>::const_iterator ptcIt = paxTypeCortegeVec.begin();
    std::vector<PaxTypeBucket>::const_iterator ptcEnd = paxTypeCortegeVec.end();
    for (; ptcIt != ptcEnd; ++ptcIt)
    {
      const PaxTypeBucket& cortege = *ptcIt;
      printPaxTypeBucket(fareMarket, cortege);
    }
  }
  else
  {
    dc << " \n";
    dc << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
       << fareMarket.destination()->description() << '\n';
  }

  dc << SEPARATOR;
  if (_application)
    dc << "-";
  dc << "\n \n";

  return;
}

Diag993Collector & Diag993Collector::operator<<(const MergedFareMarket& mfm)
{
  std::vector<FareMarket*>::const_iterator fmIt = mfm.mergedFareMarket().begin();
  for (; fmIt != mfm.mergedFareMarket().end(); ++fmIt)
  {
    *this << **fmIt << std::endl;
  }

  return *this;
}

void
Diag993Collector::writeSeparator(SeparatorType st)
{
  if (_active)
  {
    std::string strLine;
    if (_application)
      strLine = "-";

    if (st == FARE_HEADER)
    {
      std::string strLine1, strLine2;
      if (_application)
      {
        strLine1 = "F";
        strLine2 = "A";
      }

      *this << "  GI V RULE  FARE BASIS  CAB TRF O O      AMT CUR FAR PAX   CNV   " << strLine1
            << "\n"
            << "                             NUM R I              TPE TPE   AMT   " << strLine2
            << "\n" << SEPARATOR << "\n";
    }
    else if (st == SURCHARGES_HEADER)
    {
      *this << "  GI V RULE  FARE BASIS  CAB TRF O O      AMT CUR      SUR     YQYR"
            << "\n"
            << "                             NUM R I                           "
            << "\n" << SEPARATORSURCHARGES << "\n";
    }
    else if (st == DIVIDER)
    {
      *this << "--------------------------------------------------------------" << strLine << "\n";
    }
  }
}

void
Diag993Collector::writePtfPrefix(const PaxTypeFare& paxFare)
{
  Diag993Collector& dc = *this;

  dc.setf(std::ios::left, std::ios::adjustfield);

  dc << std::setw(2) << cnvFlags(paxFare);

  std::string gd;
  globalDirectionToStr(gd, paxFare.fare()->globalDirection());

  dc << std::setw(3) << gd << std::setw(2) << Vendor::displayChar(paxFare.vendor()) << std::setw(5)
     << paxFare.ruleNumber();

  std::string fareBasis = paxFare.createFareBasis(pricingTrx(), false);
  if (fareBasis.size() > 12)
    fareBasis = fareBasis.substr(0, 12) + "*"; // Cross-of-lorraine?

  dc << std::setw(13) << fareBasis << std::setw(4);

  if (TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(pricingTrx()))
    dc  << paxFare.cabin().getClassAlphaNumAnswer();
  else
    dc << paxFare.cabin().getClassAlphaNum();

  dc << std::setw(4) << paxFare.tcrRuleTariff();

  dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxFare);

  if (paxFare.directionality() == FROM)
    dc << std::setw(2) << "O";
  else if (paxFare.directionality() == TO)
    dc << std::setw(2) << "I";
  else
    dc << "  ";
}

Diag993Collector&
Diag993Collector::operator<<(const PaxTypeFare& paxFare)
{
  if (_active)
  {
    Diag993Collector& dc = *this;

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc.writePtfPrefix(paxFare);

    if (paxFare.invalidFareCurrency())
    {
      dc << " missing nuc rate for " << paxFare.currency() << '\n';
      return *this; // return so that invalid parts of the paxFare.g are not attempted.
    }
    else // farecurrency is valid
    {
      dc << std::setw(8) << Money(paxFare.fareAmount(), paxFare.currency()) << " ";

      if (pricingTrx().awardRequest())
      {
        dc.setf(std::ios::right);
        dc << std::setw(8) << paxFare.mileage();
      }

      if (!paxFare.isFareClassAppMissing())
      {
        dc << std::setw(4) << paxFare.fcaFareType();
      }
      else
      {
        dc << "UNK";
      }

      if (!paxFare.isFareClassAppSegMissing())
      {
        if (paxFare.fcasPaxType().empty())
          dc << "*** ";
        else
          dc << std::setw(4) << paxFare.fcasPaxType();
      }
      else
      {
        dc << "UNK";
      }

      dc.setf(std::ios::right, std::ios::adjustfield);
      dc.setf(std::ios::fixed, std::ios::floatfield);
      dc.precision(2);
      dc << std::setw(8) << paxFare.nucFareAmount();
    }

    if (_application)
    {
      FareMarket::FareRetrievalFlags flag = paxFare.retrievalFlag();
      dc << FareMarket::fareRetrievalFlagToStr(flag);
    }

    dc << '\n';

    if (Vendor::displayChar(paxFare.vendor()) == '*')
    {
      dc << "     " << paxFare.vendor() << '\n';
    }

    if (pricingTrx().isPbbRequest())
    {
      std::vector<BrandCode> brandCodes;
      paxFare.getValidBrands(pricingTrx(), brandCodes);
      dc << "     VALID BRANDS: ";
      for( const auto& brandCode : brandCodes)
      {
        dc << brandCode << " ";
      }
      dc << "\n";
    }
  }

  return *this;
}

Diag993Collector&
Diag993Collector::operator<<(const FareMarket& fareMarket)
{
  if (!_active)
  {
    return *this;
  }

  // If we dont have travel segments, we count output this line
  if (fareMarket.travelSeg().empty())
  {
    return *this;
  }

  Diag993Collector& dc = *this;
  dc << " \n" << FareMarketUtil::getFullDisplayString(fareMarket) << "\n ";
  // print segment info
  std::for_each(fareMarket.travelSeg().begin(), fareMarket.travelSeg().end(), PrintUtil(dc));
  dc << "\n \n";

  dc << "GEOTRAVELTYPE : ";
  dc << DiagnosticUtil::geoTravelTypeToString(fareMarket.geoTravelType());
  dc << "\n \n";

  if (pricingTrx().isFlexFare())
  {
    dc << "FLEX FARES GROUP ID: " << _groupId << "\n";
    dc << "TOTAL PAXTYPEFARES : " << fareMarket.allPaxTypeFare().size() << "\n";
  }

  const std::vector<PaxTypeBucket>& paxTypeCortegeVec = fareMarket.paxTypeCortege();
  if (!paxTypeCortegeVec.empty())
  {
    std::vector<PaxTypeBucket>::const_iterator ptcIt = paxTypeCortegeVec.begin();
    std::vector<PaxTypeBucket>::const_iterator ptcEnd = paxTypeCortegeVec.end();
    for (; ptcIt != ptcEnd; ++ptcIt)
    {
      const PaxTypeBucket& cortege = *ptcIt;
      printPaxTypeBucket(fareMarket, cortege);
    }
  }
  else
  {
    dc << " \n";
    dc << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->description() << '-'
       << fareMarket.destination()->description() << '\n';
  }

  dc << SEPARATOR;
  if (_application)
    dc << "-";
  dc << "\n \n";

  return *this;
}

void
Diag993Collector::writePtfAllTaxes(const FareMarket& fareMarket,
                                   const PaxTypeFare& paxTypeFare,
                                   const PrecalculatedTaxes& taxes)
{
  Diag993Collector& dc(*this);

  const MoneyAmount surchargesAmount =
      taxes.getAmount(PrecalculatedTaxesAmount::CAT12, paxTypeFare);
  const MoneyAmount yqyrAmount = taxes.getAmount(PrecalculatedTaxesAmount::YQYR, paxTypeFare);

  if (surchargesAmount == INVALID_AMOUNT &&
      yqyrAmount == INVALID_AMOUNT)
    return;

  dc.writePtfPrefix(paxTypeFare);
  if (paxTypeFare.invalidFareCurrency())
  {
    dc << " missing nuc rate for " << paxTypeFare.currency() << "\n";
    return;
  }

  dc << std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << " ";

  dc.setf(std::ios::right, std::ios::adjustfield);
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.precision(2);

  if (surchargesAmount != INVALID_AMOUNT)
    dc << std::setw(8) << surchargesAmount << " ";
  else
    dc << "         ";

  if (yqyrAmount != INVALID_AMOUNT)
    dc << std::setw(8) << yqyrAmount << " ";
  else
    dc << "         ";

  dc << "\n";
}

void
Diag993Collector::printPaxTypeBucket(const FareMarket& fareMarket, const PaxTypeBucket& cortege)
{
  Diag993Collector& dc = *this;
  const std::vector<PaxTypeFare*>& paxFareVec = cortege.paxTypeFare();
  if (paxFareVec.empty())
  {
    dc << " \n";
    dc << "NO FARES FOUND FOR MARKET : " << fareMarket.origin()->loc() << '-'
       << fareMarket.destination()->loc()
       << ". REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';

    return;
  }

  dc << " \n";
  dc << "REQUESTED PAXTYPE : " << cortege.requestedPaxType()->paxType() << '\n';
  dc << " INBOUND CURRENCY : " << cortege.inboundCurrency() << '\n';
  dc << "OUTBOUND CURRENCY : " << cortege.outboundCurrency() << '\n';

  if (fareMarket.breakIndicator())
  {
    dc << "N/A FOR PRICING\n";
  }
  else
  {
    if (!fareMarket.fareBasisCode().empty())
    {
      dc << "FARE CLASS " << fareMarket.fareBasisCode() << " FOR PRICING\n";
    }
  }

  dc << " \n";

  if (fareMarket.retrievalDate() != DateTime::emptyDate())
  {
    dc << "RETRIEVAL DATE: " << fareMarket.retrievalDate().toIsoExtendedString() << "\n";
  }

  dc << " \n";

  if (canPrintFares())
  {
    writeSeparator(FARE_HEADER);

    std::vector<PaxTypeFare*>::const_iterator ptfIt = paxFareVec.begin();
    std::vector<PaxTypeFare*>::const_iterator ptfEnd = paxFareVec.end();
    for (; ptfIt != ptfEnd; ++ptfIt)
    {
      PaxTypeFare& paxFare = **ptfIt;
      dc << paxFare;
    }
  }

  if (canPrintPrecalculatedTaxes())
    printPrecalculatedTaxes(fareMarket, cortege.cxrPrecalculatedTaxes());

  dc.flushMsg();
}

void
Diag993Collector::printPrecalculatedTaxes(const FareMarket& fareMarket,
                                          const CxrPrecalculatedTaxes& cxrTaxes)
{
  Diag993Collector& dc = *this;
  dc << " \n \n";

  if (cxrTaxes.empty())
  {
    dc << "  ERROR: Surcharges and YQYR not computed\n";
    return;
  }

  const MoneyAmount min(std::numeric_limits<MoneyAmount>::max());
  MoneyAmount cat12(0), yqyr(0), xf(0), minCat12(min), minYqyr(min), minXf(min);

  for (auto& i : cxrTaxes)
  {
    dc << "CALCULATED SURCHARGES AND YQYR";
    if (!i.first.empty())
      dc << " FOR VALIDATING CARRIER " << i.first;
    dc << ":\n \n";

    dc.writeSeparator(SURCHARGES_HEADER);

    const PrecalculatedTaxes& taxes = i.second;

    Vector<const PaxTypeFare*> fares = getCat12OrYqyrFares(taxes);
    std::sort(fares.begin(), fares.end(), FareComparatorByAmmount());

    for (const PaxTypeFare* fare : fares)
      writePtfAllTaxes(fareMarket, *fare, taxes);

    dc << SEPARATORSURCHARGES << "\n";

    cat12 = taxes.getDefaultAmount(PrecalculatedTaxesAmount::CAT12);
    if (cat12 < minCat12)
      minCat12 = cat12;

    yqyr = taxes.getDefaultAmount(PrecalculatedTaxesAmount::YQYR);
    if (yqyr < minYqyr)
      minYqyr = yqyr;

    xf = taxes.getDefaultAmount(PrecalculatedTaxesAmount::XF);
    if (xf < minXf)
      minXf = xf;

    dc << " \nSURCHARGES AMOUNT USED FOR ESTIMATION: " << cat12
       << "\nYQYR TAX AMOUNT USED FOR ESTIMATION: " << yqyr
       << "\nXF TAX AMOUNT USED FOR ESTIMATION: " << xf << "\n \n";
  }

  dc << " \nLOWER BOUND USED IN INITIAL FARE PATH CONSTRUCTION:"
     << " \n  SURCHARGES AMOUNT: " << minCat12
     << "\n  YQYR TAX AMOUNT: " << minYqyr
     << "\n  XF TAX AMOUNT: " << minXf
     << "\n  TOTAL AMOUNT: " << minCat12 + minYqyr + minXf << "\n \n";
}

bool
Diag993Collector::canPrintPrecalculatedTaxes() const
{
  return pricingTrx().diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL" ||
         pricingTrx().diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "TAX";
}

bool
Diag993Collector::canPrintFares() const
{
  return pricingTrx().diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) != "TAX";
}

PricingTrx&
Diag993Collector::pricingTrx()
{
  PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(trx());
  TSE_ASSERT(pricingTrx);
  return *pricingTrx;
}

const PricingTrx&
Diag993Collector::pricingTrx() const
{
  const PricingTrx* pricingTrx = dynamic_cast<const PricingTrx*>(trx());
  TSE_ASSERT(pricingTrx);
  return *pricingTrx;
}

void
Diag993Collector::displayFailedFlexFaresHeader(const FareMarket& fm, flexFares::GroupId groupId)
{
  if (!pricingTrx().isFlexFare() ||
      !pricingTrx().diagnostic().diagParamMapItemPresent(Diagnostic::DISPLAY_DETAIL) ||
      "FAILEDPTFS" != pricingTrx().diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL))
    return;

  Diag993Collector& dc = *this;
  _groupId = groupId;

  dc << "\n\nFAILED FLEX FARES FOR " << fm.origin()->loc() << "-" << fm.governingCarrier() << "-"
     << fm.destination()->loc() << "\nGROUPID: " << _groupId
     << "\nTOTAL PTFs BEFORE VALIDATION: " << fm.allPaxTypeFare().size() << "\n";
  dc << "- ---- ----------- --- - - -------- --- ----\n"
     << "V RULE FARE BASIS  TRF O O    AMT   CUR FARE\n"
     << "                   NUM R I              TYPE\n"
     << "- ---- ----------- --- - - -------- --- ----\n";
}

void
Diag993Collector::displayFailedFlexFaresPTF(const PaxTypeFare* ptf)
{
  if (!pricingTrx().isFlexFare() ||
      !pricingTrx().diagnostic().diagParamMapItemPresent(Diagnostic::DISPLAY_DETAIL) ||
      "FAILEDPTFS" != pricingTrx().diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL))
    return;

  Diag993Collector& dc = *this;

  dc << std::setw(1) << Vendor::displayChar(ptf->vendor()) << std::setw(5) << ptf->ruleNumber()
     << std::setw(12) << ptf->createFareBasis(pricingTrx(), false) << std::setw(4)
     << ptf->tcrRuleTariff();

  dc << std::setw(2) << DiagnosticUtil::getOwrtChar(*ptf);

  if (ptf->directionality() == FROM)
    dc << std::setw(2) << "O";
  else if (ptf->directionality() == TO)
    dc << std::setw(2) << "I";
  else
    dc << std::setw(2) << " ";

  dc << std::setw(9) << Money(ptf->fareAmount(), ptf->currency()) << std::setw(5)
     << ptf->fcaFareType();

  dc << "\n";

  dc.flushMsg();
}

void
Diag993Collector::displayPassedFlexFareCount(const FareMarket& fm)
{
  if (!pricingTrx().isFlexFare() ||
      !pricingTrx().diagnostic().diagParamMapItemPresent(Diagnostic::DISPLAY_DETAIL) ||
      "FAILEDPTFS" != pricingTrx().diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL))
    return;

  Diag993Collector& dc = *this;
  uint16_t totalSize = 0;
  std::vector<PaxTypeBucket>::const_iterator toCortIt = fm.paxTypeCortege().begin();
  std::vector<PaxTypeBucket>::const_iterator toCortItEnd = fm.paxTypeCortege().end();

  for (; toCortIt != toCortItEnd; ++toCortIt)
  {
    const PaxTypeBucket& toCortege = *toCortIt;
    totalSize += toCortege.paxTypeFare().size();
  }

  dc << "\nTOTAL PTFs PASSED VALIDATION: " << totalSize << "\n";
}

void
Diag993Collector::printAllFareMarkets(ShoppingTrx& trx)
{
  uint32_t legIndex = 0;
  for (ShoppingTrx::Leg& leg : trx.legs())
  {
    ItinIndex& index = leg.carrierIndex();
    const bool aso = leg.stopOverLegFlag();
    const ItinIndex::ItinIndexIterator end = aso ? index.endAcrossStopOverRow() : index.endRow();

    *this << "LEG " << legIndex << (aso ? " (ASO LEG)" : "") << std::endl << std::endl;

    for (const auto& m : index.root())
    {
      const ItinIndex::Key cxrKey = m.first;

      ItinIndex::ItinCell* const cell =
          ShoppingUtil::retrieveDirectItin(index, cxrKey, ItinIndex::CHECK_NOTHING);

      TSE_ASSERT(cell);
      Itin* const directItin = cell->second;

      TSE_ASSERT(directItin);
      if (directItin->fareMarket().empty())
        continue;

      const CarrierCode cxr = directItin->fareMarket().front()->governingCarrier();
      *this << "CARRIER " << cxr << std::endl << std::endl;

      for (FareMarket* fareMarket : directItin->fareMarket())
        *this << *fareMarket << std::endl;
    }

    ++legIndex;
  }
}
}
