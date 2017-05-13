//----------------------------------------------------------------
//
//  File:	RuleProcessingData.h
//  Authors:	Andrew Ahmad
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
//-------------------------------------------------------------------

#pragma once

namespace tse
{

class StopoversInfoWrapper;
class TransfersInfoWrapper;

class RuleProcessingData
{
public:
  RuleProcessingData() : _soInfoWrapper(nullptr), _trInfoWrapper(nullptr) {}

  StopoversInfoWrapper* soInfoWrapper() { return _soInfoWrapper; }
  void soInfoWrapper(StopoversInfoWrapper* soInfoWrapper) { _soInfoWrapper = soInfoWrapper; }

  TransfersInfoWrapper* trInfoWrapper() { return _trInfoWrapper; }
  void trInfoWrapper(TransfersInfoWrapper* trInfoWrapper) { _trInfoWrapper = trInfoWrapper; }

private:
  StopoversInfoWrapper* _soInfoWrapper;
  TransfersInfoWrapper* _trInfoWrapper;
};

}; // namespace tse

