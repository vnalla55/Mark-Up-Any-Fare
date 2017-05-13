//-------------------------------------------------------------------
//
//  Created: 2013
//
//  Copyright Sabre 2004
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
#include "DataModel/NegPaxTypeFareRuleData.h"

namespace tse
{

NegPaxTypeFareRuleData*
NegPaxTypeFareRuleData::clone(DataHandle& dataHandle) const
{
  NegPaxTypeFareRuleData* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  copyTo(*cloneObj);

  return cloneObj;
}

void
NegPaxTypeFareRuleData::copyTo(NegPaxTypeFareRuleData& cloneObj) const
{
  // Clone base
  PaxTypeFareRuleData::copyTo(cloneObj);

  cloneObj._netAmount = _netAmount;
  cloneObj._nucNetAmount = _nucNetAmount;
  cloneObj._cat35Level = _cat35Level;
  cloneObj._ticketingIndicator = _ticketingIndicator; // TODO: remove? dup of secRec?
  cloneObj._calculatedNegCurrency = _calculatedNegCurrency;
  cloneObj._creatorPCC = _creatorPCC;
  cloneObj._calcInd = _calcInd;
  cloneObj._percent = _percent;
  cloneObj._ruleAmt = _ruleAmt;
  cloneObj._frrId = _frrId;
  cloneObj._frrSeqNo = _frrSeqNo;
  cloneObj._frrSourcePcc = _frrSourcePcc;
  cloneObj._updateInd = _updateInd;
  cloneObj._redistributeInd = _redistributeInd;
  cloneObj._sellInd = _sellInd;
  cloneObj._ticketInd = _ticketInd;
  cloneObj._validatingCxr = _validatingCxr;

  cloneObj._negFareRestExt = _negFareRestExt;
  cloneObj._negFareRestExtSeq = _negFareRestExtSeq;
  cloneObj._fareProperties = _fareProperties;
  cloneObj._valueCodeAlgorithm = _valueCodeAlgorithm;
  cloneObj._printOption = _printOption;
  cloneObj._isT979FareIndInRange = _isT979FareIndInRange;
  cloneObj._cat25Responsive = _cat25Responsive;
  cloneObj._fareRetailerCode = _fareRetailerCode;
}

} // tse namespace
