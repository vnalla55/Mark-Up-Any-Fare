//-------------------------------------------------------------------
//
//  File:        DisplayDocumentFactory.cpp
//  Authors:     Mike Carroll
//  Created:     July 24, 2005
//  Description: Factory to create our display documents
//
//  Copyright Sabre 2005
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

#include "FareDisplay/Templates/DisplayDocumentFactory.h"

#include "Common/FareDisplayUtil.h"
#include "Common/NonFatalErrorResponseException.h"
#include "DataModel/FareDisplayOptions.h"
#include "FareDisplay/InclusionCodeConsts.h"
#include "FareDisplay/Templates/AddOnFaresSection.h"
#include "FareDisplay/Templates/AlternateCarriersSection.h"
#include "FareDisplay/Templates/CurrencyConversionSection.h"
#include "FareDisplay/Templates/DiagElementField.h"
#include "FareDisplay/Templates/DiagFaresSection.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/FaresHeaderSection.h"
#include "FareDisplay/Templates/FaresSection.h"
#include "FareDisplay/Templates/FaresSectionFilter.h"
#include "FareDisplay/Templates/FBSection.h"
#include "FareDisplay/Templates/FlightCountSection.h"
#include "FareDisplay/Templates/FQCurrencyConversionSection.h"
#include "FareDisplay/Templates/FTCurrencyConversionSection.h"
#include "FareDisplay/Templates/FTSection.h"
#include "FareDisplay/Templates/HeaderMsgDiagSection.h"
#include "FareDisplay/Templates/HeaderMsgSection.h"
#include "FareDisplay/Templates/ICDisplayDocument.h"
#include "FareDisplay/Templates/IntlConstructionSection.h"
#include "FareDisplay/Templates/MPDisplayDocument.h"
#include "FareDisplay/Templates/MPHeaderSection.h"
#include "FareDisplay/Templates/MPSection.h"
#include "FareDisplay/Templates/MultiACCITrailerSection.h"
#include "FareDisplay/Templates/MultiTransportDiagSection.h"
#include "FareDisplay/Templates/NoFaresSection.h"
#include "FareDisplay/Templates/NoMarketMPSection.h"
#include "FareDisplay/Templates/OrigDestSection.h"
#include "FareDisplay/Templates/RBSection.h"
#include "FareDisplay/Templates/RDSection.h"
#include "FareDisplay/Templates/RoutingSection.h"
#include "FareDisplay/Templates/UserInputSection.h"
#include "FareDisplay/Templates/UserPrefSection.h"

#include <string>

namespace tse
{
Document&
DisplayDocumentFactory::getDisplayDocument(FareDisplayTrx& trx,
                                           const FareDisplayPref& prefRec,
                                           FareDispTemplate& templateRec,
                                           std::vector<FareDispTemplateSeg*>& templateSegRecs)
{
  // Check diagnostic document requirement first
  // 195, 214, 215 and 854 should create non diagnostic documents
  if (needDiagDisplayDocument(trx))
  {
    DiagDisplayDocument& document = trx.dataHandle().safe_create<DiagDisplayDocument>();

    switch (trx.getRequest()->diagnosticNumber())
    {
    case DIAG_200_ID:
      buildDiag200DisplayDocument(trx, document);
      break;
    case DIAG_201_ID:
      buildDiag201DisplayDocument(trx, document);
      break;
    case DIAG_202_ID:
      buildDiag202DisplayDocument(trx, document);
      break;
    case DIAG_203_ID:
      buildDiag203DisplayDocument(trx, document);
      break;
    case DIAG_205_ID:
      buildDiag205DisplayDocument(trx, document);
      break;
    case DIAG_206_ID:
      buildDiag203DisplayDocument(trx, document);
      break;
    case DIAG_211_ID:
      buildDiag211DisplayDocument(trx, document);
      break;
    case DIAG_213_ID:
      buildDiag213DisplayDocument(trx, document);
      break;
    case DIAG_216_ID:
      buildDiag216DisplayDocument(trx, document);
      break;
    case DIAG_217_ID:
      buildDiag217DisplayDocument(trx, document);
      break;
    case DIAG_218_ID:
      buildDiag218DisplayDocument(trx, document);
      break;
    case DIAG_220_ID:
      buildDiag220DisplayDocument(trx, document);
      break;
    default:
      break;
    }
    return document;
  }
  if ((trx.getRequest()->requestType() == FARE_DISPLAY_REQUEST ||
       trx.allPaxTypeFare().size() > 1) &&
      trx.getRequest()->requestType() != FARE_BOOKINGCODE_REQUEST &&
      trx.getRequest()->requestType() != FARE_MILEAGE_REQUEST)
  {
    FQDisplayDocument& document =
        trx.dataHandle().safe_create<FQDisplayDocument>(&prefRec, &templateSegRecs);
    buildFQDisplayDocument(trx, document);
    return document;
  }
  else if (trx.isRD())
  {
    if (trx.getOptions()->FBDisplay() == TRUE_INDICATOR)
    {
      FBDisplayDocument& document = trx.dataHandle().safe_create<FBDisplayDocument>();
      buildFBDisplayDocument(trx, document);
      return document;
    }
    else if (trx.getRequest()->requestedInclusionCode() == ADDON_FARES)
    {
      ICDisplayDocument& document = trx.dataHandle().safe_create<ICDisplayDocument>();
      buildICDisplayDocument(trx, document);
      return document;
    }
    else
    {
      RDDisplayDocument& document =
          trx.dataHandle().safe_create<RDDisplayDocument>(&templateSegRecs);
      buildRDDisplayDocument(trx, document);
      return document;
    }
  }
  else if (trx.getRequest()->requestType() == FARE_TAX_REQUEST)
  {
    FTDisplayDocument& document = trx.dataHandle().safe_create<FTDisplayDocument>();
    buildFTDisplayDocument(trx, document);
    return document;
  }
  else if (trx.getRequest()->requestType() == FARE_MILEAGE_REQUEST)
  {
    MPDisplayDocument& document = trx.dataHandle().safe_create<MPDisplayDocument>(&templateSegRecs);
    buildMPDisplayDocument(trx, document);
    return document;
  }
  else if (trx.getRequest()->requestType() == FARE_BOOKINGCODE_REQUEST)
  {
    RBDisplayDocument& document = trx.dataHandle().safe_create<RBDisplayDocument>();
    buildRBDisplayDocument(trx, document);
    return document;
  }
  else if (trx.getRequest()->requestType() == TAX_CODE_REQUEST)
  {
    TXDisplayDocument& document = trx.dataHandle().safe_create<TXDisplayDocument>();
    buildTXDisplayDocument(trx, document);
    return document;
  }
  else // PFC_REQUEST
  {
    PXDisplayDocument& document = trx.dataHandle().safe_create<PXDisplayDocument>();
    buildPXDisplayDocument(trx, document);
    return document;
  }
}

Document&
DisplayDocumentFactory::getDisplayDocument(FareDisplayTrx& trx)
{
  MPDisplayDocument& document = trx.dataHandle().safe_create<MPDisplayDocument>();
  buildMPDisplayDocument(trx, document);
  return document;
}

void
DisplayDocumentFactory::buildFQDisplayDocument(FareDisplayTrx& trx, FQDisplayDocument& document)
{
  // No Fares for Shopper entry

  if (trx.allPaxTypeFare().empty() && trx.isShopperRequest())
  {
    NoFaresSection* noFaresSection = &trx.dataHandle().safe_create<NoFaresSection>(trx);
    document.sections().push_back(noFaresSection);
    return;
  }

  // ORIG DEST section
  OrigDestSection* origDestSection = &trx.dataHandle().safe_create<OrigDestSection>(trx);
  document.sections().push_back(origDestSection);

  // Flight counts section
  if (trx.hasScheduleCountInfo())
  {
    FlightCountSection* flightCountSection = &trx.dataHandle().safe_create<FlightCountSection>(trx);
    document.sections().push_back(flightCountSection);
  }
  else if (!trx.isShopperRequest() && trx.getOptions()->singleCarrierSvcSched() == 'A')
  {
    // Alternate carriers
    AlternateCarriersSection* altCarriersSection =
        &trx.dataHandle().safe_create<AlternateCarriersSection>(trx);
    document.sections().push_back(altCarriersSection);
  }

  // Header message section
  HeaderMsgSection* headerMsgSection = &trx.dataHandle().safe_create<HeaderMsgSection>(trx);
  document.sections().push_back(headerMsgSection);

  // Currency conversion section
  FQCurrencyConversionSection* fqCurrencyConversionSection =
      &trx.dataHandle().safe_create<FQCurrencyConversionSection>(trx);
  document.sections().push_back(fqCurrencyConversionSection);

  if (trx.getRequest()->inclusionCode() == ADDON_FARES)
  {
    // AddOn Fares section
    AddOnFaresSection* faresSection =
        &trx.dataHandle().safe_create<AddOnFaresSection>(trx, document.templateSegRecs());
    buildFareRowElements(trx, *faresSection, document);
    document.sections().push_back(faresSection);
  }
  else
  {
    // Fares section header
    FaresHeaderSection* faresHeaderSection =
        &trx.dataHandle().safe_create<FaresHeaderSection>(trx, document.templateSegRecs());

    // Fares section filter
    FaresSectionFilter* faresSectionFilter;
    trx.dataHandle().get(faresSectionFilter);
    faresSectionFilter->initialize(trx);

    // Fares section
    FaresSection* faresSection = &trx.dataHandle().safe_create<FaresSection>(
        trx, *faresHeaderSection, *faresSectionFilter, document.templateSegRecs());
    buildFareRowElements(trx, *faresSection, document);
    document.sections().push_back(faresSection);
  }

  const int tmplateID = (document.templateSegRecs() && !document.templateSegRecs()->empty()
      && document.templateSegRecs()->front()) ? document.templateSegRecs()->front()->templateID() : 0;

  // Routings section
  if (document.prefs().showRoutings() == YES_INDICATOR && tmplateID != RETAILER_CATEGORY)
  {
    RoutingSection* routingSection = &trx.dataHandle().safe_create<RoutingSection>(trx);
    document.sections().push_back(routingSection);
  }

  if (trx.getRequest()->isMultiAccCorpId())
  {
    MultiACCITrailerSection* trailerSection =
        &trx.dataHandle().safe_create<MultiACCITrailerSection>(trx);
    document.sections().push_back(trailerSection);
  }
}

void
DisplayDocumentFactory::buildRDDisplayDocument(FareDisplayTrx& trx, RDDisplayDocument& document)
{
  // Only if data available
  if (trx.allPaxTypeFare().empty())
  {
    handleFareBasisNotFoundError(trx);
  }

  // Check if any RTG diagnostic info to display
  if (trx.isDiagnosticRequest())
  {
    std::string& rtgDiagInfo = trx.fdResponse()->rtgDiagInfo();
    if (!rtgDiagInfo.empty())
    {
      trx.response() << SPACE << std::endl << rtgDiagInfo << std::endl << SPACE << std::endl;
    }
  }
  // Fares section header
  FaresHeaderSection* faresHeaderSection =
      &trx.dataHandle().safe_create<FaresHeaderSection>(trx, document.templateSegRecs());
  document.sections().push_back(faresHeaderSection);

  // Fares section
  FaresSection* faresSection =
      &trx.dataHandle().safe_create<FaresSection>(trx, document.templateSegRecs());
  buildFareRowElements(trx, *faresSection, document);
  document.sections().push_back(faresSection);

  // RD section
  RDSection* rdSection = &trx.dataHandle().safe_create<RDSection>(trx);
  document.sections().push_back(rdSection);

  if (trx.allPaxTypeFare().size() > 1 || trx.getOptions()->isRoutingDisplay() == TRUE_INDICATOR)
  {
    RoutingSection* rtgSection = &trx.dataHandle().safe_create<RoutingSection>(trx);
    document.sections().push_back(rtgSection);
  }
}

void
DisplayDocumentFactory::buildFBDisplayDocument(FareDisplayTrx& trx, FBDisplayDocument& document)
{
  // Only if data available
  if (trx.allPaxTypeFare().empty())
  {
    handleFareBasisNotFoundError(trx);
  }

  // FBSection carriers
  FBSection* fbSection = &trx.dataHandle().safe_create<FBSection>(trx);
  document.sections().push_back(fbSection);
}

void
DisplayDocumentFactory::buildFTDisplayDocument(FareDisplayTrx& trx, FTDisplayDocument& document)
{
  // Currency conversion section
  FTCurrencyConversionSection* ftCurrencyConversionSection =
      &trx.dataHandle().safe_create<FTCurrencyConversionSection>(trx);
  document.sections().push_back(ftCurrencyConversionSection);

  FTSection* ftSection = &trx.dataHandle().safe_create<FTSection>(trx);
  document.sections().push_back(ftSection);
}

void
DisplayDocumentFactory::buildMPDisplayDocument(FareDisplayTrx& trx, MPDisplayDocument& document)
{
  MPType mpType(FareDisplayUtil::determineMPType(trx));
  if (mpType == SHORT_MP)
  {
    // ORIG DEST section
    OrigDestSection* origDestSection = &trx.dataHandle().safe_create<OrigDestSection>(trx);
    document.sections().push_back(origDestSection);

    // Fares section header
    FaresHeaderSection* faresHeaderSection =
        &trx.dataHandle().safe_create<FaresHeaderSection>(trx, document.templateSegRecs());

    // Fares section filter
    FaresSectionFilter* faresSectionFilter;
    trx.dataHandle().get(faresSectionFilter);
    faresSectionFilter->initialize(trx);

    // Fares section
    FaresSection* faresSection = &trx.dataHandle().safe_create<FaresSection>(
        trx, *faresHeaderSection, *faresSectionFilter, document.templateSegRecs());
    buildFareRowElements(trx, *faresSection, document);
    document.sections().push_back(faresSection);
  }
  else if (mpType == LONG_MP)
  {
    MPHeaderSection* mpHeaderSection = &trx.dataHandle().safe_create<MPHeaderSection>(trx);
    document.sections().push_back(mpHeaderSection);
  }

  // MP section
  if (mpType == NO_MARKET_MP)
  {
    NoMarketMPSection* mpSection = &trx.dataHandle().safe_create<NoMarketMPSection>(trx);
    document.sections().push_back(mpSection);
  }
  else
  {
    MPSection* mpSection = &trx.dataHandle().safe_create<MPSection>(trx);
    document.sections().push_back(mpSection);
  }
}

void
DisplayDocumentFactory::buildRBDisplayDocument(FareDisplayTrx& trx, RBDisplayDocument& document)
{
  RBSection* rbSection = &trx.dataHandle().safe_create<RBSection>(trx);
  document.sections().push_back(rbSection);
}

void
DisplayDocumentFactory::buildICDisplayDocument(FareDisplayTrx& trx, ICDisplayDocument& document)
{
  IntlConstructionSection* icSection = &trx.dataHandle().safe_create<IntlConstructionSection>(trx);
  document.sections().push_back(icSection);
}

void
DisplayDocumentFactory::buildTXDisplayDocument(FareDisplayTrx& trx, TXDisplayDocument& document)
{
  trx.response() << "RECEIVED REQUEST.  TX RESPONSE UNDER CONSTRUCTION" << std::endl;
}

void
DisplayDocumentFactory::buildPXDisplayDocument(FareDisplayTrx& trx, PXDisplayDocument& document)
{
  trx.response() << "RECEIVED REQUEST.  PX RESPONSE UNDER CONSTRUCTION" << std::endl;
}

void
DisplayDocumentFactory::buildDiag200DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // ORIG DEST section
  OrigDestSection* origDestSection = &trx.dataHandle().safe_create<OrigDestSection>(trx);
  document.sections().push_back(origDestSection);

  // Header message section
  HeaderMsgSection* headerMsgSection = &trx.dataHandle().safe_create<HeaderMsgSection>(trx);
  document.sections().push_back(headerMsgSection);

  // Diag Fares section
  DiagFaresSection* faresSection = &trx.dataHandle().safe_create<DiagFaresSection>(trx);
  document.sections().push_back(faresSection);

  DiagElementField* elementField;
  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(LINE_NUMBER), 1, 3, JustificationType(RIGHT));
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(FARE_BASIS_TKT_DSG), // Element type
                           6, // Element data start pos
                           15, // Element data field size
                           JustificationType(LEFT), // Field justification
                           6, // Header text pos
                           "FARE BASIS"); // Header text
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_1_IND), 21, 2, JustificationType(LEFT), 21, "01");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_2_IND), 24, 2, JustificationType(LEFT), 24, "02");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_3_IND), 27, 2, JustificationType(LEFT), 27, "03");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_5_IND), 30, 2, JustificationType(LEFT), 30, "05");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_6_IND), 33, 2, JustificationType(LEFT), 33, "06");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_7_IND), 36, 2, JustificationType(LEFT), 36, "07");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_11_IND), 39, 2, JustificationType(LEFT), 39, "11");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_14_IND), 42, 2, JustificationType(LEFT), 42, "14");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_15_IND), 45, 2, JustificationType(LEFT), 45, "15");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_16_IND), 48, 2, JustificationType(LEFT), 48, "16");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(CAT_23_IND), 51, 2, JustificationType(LEFT), 51, "23");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(PAX_TYPE), 54, 3, JustificationType(LEFT), 54, "PAX");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(FARE_CLASS), 58, 3, JustificationType(LEFT), 58, "TYP");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(FAIL_CODE), 62, 2, JustificationType(LEFT), 62, "FC");
  faresSection->columnFields().push_back(elementField);
}

void
DisplayDocumentFactory::buildDiag201DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // ORIG DEST section
  OrigDestSection* origDestSection = &trx.dataHandle().safe_create<OrigDestSection>(trx);
  document.sections().push_back(origDestSection);

  // Header message section
  HeaderMsgSection* headerMsgSection = &trx.dataHandle().safe_create<HeaderMsgSection>(trx);
  document.sections().push_back(headerMsgSection);

  // Diag Fares section
  DiagFaresSection* faresSection = &trx.dataHandle().safe_create<DiagFaresSection>(trx);
  document.sections().push_back(faresSection);

  DiagElementField* elementField;
  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(LINE_NUMBER), 1, 3, JustificationType(RIGHT));
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(FARE_BASIS_TKT_DSG), 7, 16, JustificationType(LEFT), 7, "FARE BASIS");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(PAX_TYPE), 23, 3, JustificationType(LEFT), 23, "PAX");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(BASE_FARE), 31, 9, JustificationType(RIGHT), 31, "BASE FARE");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(US_TAX), 43, 8, JustificationType(RIGHT), 43, "US TAXES");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(TOTAL_FARE), 53, 10, JustificationType(LEFT), 53, "TOTAL FARE");
  faresSection->columnFields().push_back(elementField);
}

void
DisplayDocumentFactory::buildDiag202DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // ORIG DEST section
  OrigDestSection* origDestSection = &trx.dataHandle().safe_create<OrigDestSection>(trx);
  document.sections().push_back(origDestSection);

  // Header message section
  HeaderMsgSection* headerMsgSection = &trx.dataHandle().safe_create<HeaderMsgSection>(trx);
  document.sections().push_back(headerMsgSection);

  // Diag Fares section
  DiagFaresSection* faresSection = &trx.dataHandle().safe_create<DiagFaresSection>(trx);
  document.sections().push_back(faresSection);

  DiagElementField* elementField;
  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(LINE_NUMBER), 1, 3, JustificationType(RIGHT));
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(FARE_BASIS_TKT_DSG), 7, 16, JustificationType(LEFT), 7, "FARE BASIS");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(JOURNEY_TYPE), 24, 1, JustificationType(LEFT));
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(BASE_FARE), 29, 9, JustificationType(RIGHT), 29, "BASE FARE");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(SURCHARGES), 41, 10, JustificationType(RIGHT), 41, "SURCHARGES");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(TOTAL_FARE), 53, 10, JustificationType(RIGHT), 53, "TOTAL FARE");
  faresSection->columnFields().push_back(elementField);
}

void
DisplayDocumentFactory::buildDiag203DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // ORIG DEST section
  OrigDestSection* origDestSection = &trx.dataHandle().safe_create<OrigDestSection>(trx);
  document.sections().push_back(origDestSection);

  // Header message section
  HeaderMsgSection* headerMsgSection = &trx.dataHandle().safe_create<HeaderMsgSection>(trx);
  document.sections().push_back(headerMsgSection);

  // Diag Fares section
  DiagFaresSection* faresSection = &trx.dataHandle().safe_create<DiagFaresSection>(trx);
  document.sections().push_back(faresSection);

  DiagElementField* elementField;
  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(LINE_NUMBER), 1, 3, JustificationType(RIGHT));
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(FARE_BASIS_TKT_DSG), 7, 16, JustificationType(LEFT), 7, "FARE BASIS");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(BASE_FARE), 24, 9, JustificationType(RIGHT), 24, "BASE FARE");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(PAX_TYPE), 36, 3, JustificationType(LEFT), 36, "PAX");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(FARE_TYPE_CODE), 44, 3, JustificationType(LEFT), 41, "FARE TYPE");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(DISPLAY_TYPE), 57, 3, JustificationType(LEFT), 52, "DISPLAY TYPE");
  faresSection->columnFields().push_back(elementField);
}

void
DisplayDocumentFactory::buildDiag216DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // USER PREF section
  UserPrefSection* userPrefSection = &trx.dataHandle().safe_create<UserPrefSection>(trx);
  document.sections().push_back(userPrefSection);
}

void
DisplayDocumentFactory::buildDiag205DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // ORIG DEST section
  OrigDestSection* origDestSection = &trx.dataHandle().safe_create<OrigDestSection>(trx);
  document.sections().push_back(origDestSection);

  // Header message section
  HeaderMsgSection* headerMsgSection = &trx.dataHandle().safe_create<HeaderMsgSection>(trx);
  document.sections().push_back(headerMsgSection);

  // Diag Fares section
  DiagFaresSection* faresSection = &trx.dataHandle().safe_create<DiagFaresSection>(trx);
  document.sections().push_back(faresSection);

  DiagElementField* elementField;
  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(LINE_NUMBER), 1, 3, JustificationType(RIGHT));
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(FARE_BASIS_TKT_DSG), 7, 16, JustificationType(LEFT), 7, "FARE BASIS");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(FARE_CURRENCY_CODE), 26, 3, JustificationType(LEFT));
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(ORIGINAL_FARE), 31, 9, JustificationType(RIGHT), 27, "ORIGINAL FARE");
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(DiagFieldElement(DISPLAY_CURRENCY_CODE), 43, 3, JustificationType(LEFT));
  faresSection->columnFields().push_back(elementField);

  trx.dataHandle().get(elementField);
  elementField->initialize(
      DiagFieldElement(CONVERTED_FARE), 48, 14, JustificationType(RIGHT), 48, "CONVERTED FARE");
  faresSection->columnFields().push_back(elementField);
}

void
DisplayDocumentFactory::buildDiag218DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // ACTUAL PAX TYPE section
  ActualPaxTypeSection* _actualPaxTypeSection =
      &trx.dataHandle().safe_create<ActualPaxTypeSection>(trx);
  document.sections().push_back(_actualPaxTypeSection);
}

void
DisplayDocumentFactory::buildDiag220DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // USER INPUT section
  UserInputSection* _userInputSection = &trx.dataHandle().safe_create<UserInputSection>(trx);
  document.sections().push_back(_userInputSection);
}

void
DisplayDocumentFactory::buildDiag211DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // Header messages diag section
  HeaderMsgDiagSection* headerMsgDiag = &trx.dataHandle().safe_create<HeaderMsgDiagSection>(trx);
  document.sections().push_back(headerMsgDiag);
}

void
DisplayDocumentFactory::buildDiag213DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // Grouping preferences section
  GroupingPreferenceSection* groupingPrefSection =
      &trx.dataHandle().safe_create<GroupingPreferenceSection>(trx);
  document.sections().push_back(groupingPrefSection);
}

void
DisplayDocumentFactory::buildDiag217DisplayDocument(FareDisplayTrx& trx,
                                                    DiagDisplayDocument& document)
{
  // MultiTransport section
  MultiTransportDiagSection* multiTransportDiag =
      &trx.dataHandle().safe_create<MultiTransportDiagSection>(trx);
  document.sections().push_back(multiTransportDiag);
}

void
DisplayDocumentFactory::buildFareRowElements(FareDisplayTrx& trx,
                                             FaresSection& faresSection,
                                             Document& document)
{

  if (faresSection.templateSegRecs().empty())
    return;

  std::vector<FareDispTemplateSeg*>::const_iterator iter = faresSection.templateSegRecs().begin();
  std::vector<FareDispTemplateSeg*>::const_iterator iterEnd = faresSection.templateSegRecs().end();

  JustificationType justifyType;
  for (; iter != iterEnd; iter++)
  {
    ElementField* elementField;
    trx.dataHandle().get(elementField);

    if ((*iter)->elementJustify() == ' ')
      justifyType = JustificationType(LEFT);
    else if ((*iter)->elementJustify() == JustificationType(LOGIC))
      justifyType = JustificationType(LEFT);
    else
      justifyType = JustificationType(RIGHT);
    elementField->initialize(FieldColumnElement((*iter)->columnElement()),
                             (*iter)->elementStart(),
                             (*iter)->elementLength(),
                             justifyType);
    faresSection.columnFields().push_back(elementField);
  }
}

void
DisplayDocumentFactory::buildFareRowElements(FareDisplayTrx& trx,
                                             AddOnFaresSection& faresSection,
                                             Document& document)
{
  if (faresSection.templateSegRecs().empty())
    return;

  std::vector<FareDispTemplateSeg*>::const_iterator iter = faresSection.templateSegRecs().begin();
  std::vector<FareDispTemplateSeg*>::const_iterator iterEnd = faresSection.templateSegRecs().end();

  JustificationType justifyType;
  for (; iter != iterEnd; iter++)
  {
    ElementField* elementField;
    trx.dataHandle().get(elementField);

    if ((*iter)->elementJustify() == ' ')
      justifyType = JustificationType(LEFT);
    else if ((*iter)->elementJustify() == JustificationType(LOGIC))
      justifyType = JustificationType(LEFT);
    else
      justifyType = JustificationType(RIGHT);
    elementField->initialize(FieldColumnElement((*iter)->columnElement()),
                             (*iter)->elementStart(),
                             (*iter)->elementLength(),
                             justifyType);
    faresSection.columnFields().push_back(elementField);
  }
}

bool
DisplayDocumentFactory::needDiagDisplayDocument(FareDisplayTrx& trx)
{
  int16_t diagNumber = trx.getRequest()->diagnosticNumber();
  return (trx.isFDDiagnosticRequest() && diagNumber != DIAG_195_ID && diagNumber != DIAG_214_ID &&
          diagNumber != DIAG_215_ID && diagNumber != DIAG_854_ID);
}

void
DisplayDocumentFactory::handleFareBasisNotFoundError(FareDisplayTrx& trx)
{
  if (trx.isERD())
    throw NonFatalErrorResponseException(ErrorResponseException::PRCRD_FARE_BASIS_NOT_FOUND,
                                         "$FARE BASIS NOT FOUND FOR CITYPAIR/DATE$");
  else if (trx.isLongRD())
    throw NonFatalErrorResponseException(ErrorResponseException::LONGRD_FARE_BASIS_NOT_FOUND,
                                         "$FARE BASIS NOT FOUND FOR CITYPAIR/DATE$");
  else if (trx.isShortRD())
    throw NonFatalErrorResponseException(ErrorResponseException::SHORTRD_FARE_BASIS_NOT_FOUND,
                                         "$FARE BASIS NOT FOUND FOR CITYPAIR/DATE$");
  else
    throw NonFatalErrorResponseException(ErrorResponseException::FARE_BASIS_NOT_FOUND,
                                         "$FARE BASIS NOT FOUND FOR CITYPAIR/DATE$");
}
} // tse namespace
