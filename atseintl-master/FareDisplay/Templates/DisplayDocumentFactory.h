//-------------------------------------------------------------------
//
//  File:        DisplayDocumentFactory.h
//  Authors:     Mike Carroll
//  Created:     July 24, 2005
//  Description: This factory class creates DisplayDocument objects
//
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

#pragma once

#include "Common/Singleton.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DBAccess/FareDisplayPref.h"
#include "DBAccess/FareDispTemplate.h"
#include "DBAccess/FareDispTemplateSeg.h"
#include "FareDisplay/Templates/ActualPaxTypeSection.h"
#include "FareDisplay/Templates/AddOnFaresSection.h"
#include "FareDisplay/Templates/DiagDisplayDocument.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/FaresSection.h"
#include "FareDisplay/Templates/FBDisplayDocument.h"
#include "FareDisplay/Templates/FQDisplayDocument.h"
#include "FareDisplay/Templates/FTDisplayDocument.h"
#include "FareDisplay/Templates/GroupingPreferenceSection.h"
#include "FareDisplay/Templates/ICDisplayDocument.h"
#include "FareDisplay/Templates/MPDisplayDocument.h"
#include "FareDisplay/Templates/PXDisplayDocument.h"
#include "FareDisplay/Templates/RBDisplayDocument.h"
#include "FareDisplay/Templates/RDDisplayDocument.h"
#include "FareDisplay/Templates/TXDisplayDocument.h"
#include "FareDisplay/Templates/UserInputSection.h"
#include "FareDisplay/Templates/UserPrefSection.h"

namespace tse
{

class DisplayDocumentFactory
{
  friend class DisplayDocumentFactoryTest;

protected:
  DisplayDocumentFactory() {}

  DisplayDocumentFactory(const DisplayDocumentFactory&) = delete;
  DisplayDocumentFactory& operator=(const DisplayDocumentFactory&) = delete;

  friend class tse::Singleton<DisplayDocumentFactory>;

public:
  Document& getDisplayDocument(FareDisplayTrx& trx,
                               const FareDisplayPref& prefRec,
                               FareDispTemplate& templateRec,
                               std::vector<FareDispTemplateSeg*>& templateSegRecs);

  Document& getDisplayDocument(FareDisplayTrx&);

private:
  void buildFQDisplayDocument(FareDisplayTrx& trx, FQDisplayDocument& document);
  void buildRDDisplayDocument(FareDisplayTrx& trx, RDDisplayDocument& document);
  void buildFBDisplayDocument(FareDisplayTrx& trx, FBDisplayDocument& document);
  void buildFTDisplayDocument(FareDisplayTrx& trx, FTDisplayDocument& document);
  void buildMPDisplayDocument(FareDisplayTrx& trx, MPDisplayDocument& document);
  void buildRBDisplayDocument(FareDisplayTrx& trx, RBDisplayDocument& document);
  void buildTXDisplayDocument(FareDisplayTrx& trx, TXDisplayDocument& document);
  void buildPXDisplayDocument(FareDisplayTrx& trx, PXDisplayDocument& document);
  void buildDiag200DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);
  void buildDiag201DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);
  void buildDiag202DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);
  void buildDiag203DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);
  void buildDiag205DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);
  void buildDiag211DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);
  void buildDiag213DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);
  void buildDiag216DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);
  void buildDiag217DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);
  void buildDiag218DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);
  void buildDiag220DisplayDocument(FareDisplayTrx& trx, DiagDisplayDocument& document);

  void buildFareRowElements(FareDisplayTrx& trx, FaresSection& faresSection, Document& document);

  void
  buildFareRowElements(FareDisplayTrx& trx, AddOnFaresSection& faresSection, Document& document);
  void buildICDisplayDocument(FareDisplayTrx& trx, ICDisplayDocument&);

  bool needDiagDisplayDocument(FareDisplayTrx& trx);

  void handleFareBasisNotFoundError(FareDisplayTrx& trx);
};
} // namespace tse
