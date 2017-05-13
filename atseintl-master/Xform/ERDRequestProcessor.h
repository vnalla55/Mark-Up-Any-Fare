//-------------------------------------------------------------------
//
//  File:        ERDRequestProcessor.h
//  Created:     October 26, 2008
//  Authors:     Konrad Koch
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

#pragma once

#include "DataModel/FareDisplayTrx.h"

#include <algorithm>

namespace tse
{
class ERDFltSeg;

class ERDRequestProcessor
{
public:
  //--------------------------------------------------------------------------
  // @function ERDRequestProcessor::ERDRequestProcessor
  //
  // Description: constructor
  //
  // @param fareDisplayTrx - reference to a valid FD transaction object
  //--------------------------------------------------------------------------
  ERDRequestProcessor(FareDisplayTrx& fareDisplayTrx);

  //--------------------------------------------------------------------------
  // @function ERDRequestProcessor::~ERDRequestProcessor
  //
  // Description: destructor
  //--------------------------------------------------------------------------
  virtual ~ERDRequestProcessor();

  //--------------------------------------------------------------------------
  // @function ERDRequestProcessor::process
  //
  // Description: Fills FD trx according to user input
  //
  // @param erdFareInfos - reference to list of ERD infos
  // @return void
  //--------------------------------------------------------------------------
  bool process();

  //--------------------------------------------------------------------------
  // @function ERDRequestProcessor::prepareDTS
  //
  // Description: Prepares content of DTS tag
  //
  // @param content - decoded and decompressed content
  // @return void
  //--------------------------------------------------------------------------
  bool prepareDTS(std::string& content);

private:
  FareDisplayTrx& _trx;
  std::vector<ERDFareComp*> _fareComponents;
  std::vector<ERDFareComp*> _originalFareComponents;
  bool _useInternationalRounding;

  bool multiplePaxTypesLeft() const;
  ERDFltSeg* findSegment(uint16_t number) const;
  bool checkSegments(std::vector<uint16_t>& requestedSNs) const;
  void removeSurfaceSegments(std::vector<uint16_t>& requestedSNs) const;
  bool filterByFareBasisCode(const FareClassCode& requestedFBC);
  bool filterBySegmentNumbers(const std::vector<uint16_t>& requestedSNs);
  bool filterByPaxTypeCode(const PaxTypeCode& requestedPTC);
  bool filterByPaxTypeNumber(uint16_t requestedPTN);
  void filterByChildTypes();
  bool validateFareComponents(bool validateForSegments = false);
  bool ifFareBasisExists(const FareClassCode& requestedFBC) const;
  bool ifFareBasisWithoutTktDesExists(const FareClassCode& requestedFBC) const;
  void buildErrorMessage(const std::string& message,
                         bool warning = true,
                         bool trailingMsg = false) const;
  void buildTrailingMessage() const;
  void buildPtcErrorMessage() const;
  bool validateRDErrors() const;
  bool validatePassenger();
  bool validateSegments(const FareClassCode requestedFareBasis);
  bool storeResults() const;
  void completeFareBreaks();
  void calculateInternationalRoundingValue();
};
}

