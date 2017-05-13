//////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//
//  File:           FareSelectorQualifierChain.h
//
//  Description:    Build/hold QualifierChain for
//                  FareSelector classes.
//
//  Updates:
//
//  Copyright Sabre 2006
//
//  The copyright to the computer program(s) herein
//  is the property of Sabre.
//  The program(s) may be used and/or copied only with
//  the written permission of Sabre or in accordance
//  with the terms and conditions stipulated in the
//  agreement/contract under which the program(s)
//  have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseStringTypes.h"
#include "DataModel/PaxTypeFare.h"
#include "FareDisplay/Qualifier.h"
#include "FareDisplay/QualifierChain.h"


namespace tse
{
class Qualifier;

class FareSelectorQualifierChain : public QualifierChain
{
public:
  FareSelectorQualifierChain();
  virtual ~FareSelectorQualifierChain() {}

  bool buildChain(FareDisplayTrx& trx, bool useAll) override;
  inline bool demolishChain() override
  {
    if (_itsQualifier != nullptr)
    {
      // This curious chain destruction takes advantage of the fact
      // that all qualifier objects in the chain are held by the
      // local dataHandle (_dh)  and will be destructed when this
      // object goes out of scope.
      _itsQualifier = nullptr;
    }
    return true;
  }

  const PaxTypeFare::FareDisplayState
  qualifyPaxTypeFare(FareDisplayTrx& trx, const PaxTypeFare& ptFare) const override;

  int size() const override
  {
    if (_itsQualifier == nullptr)
      return 0;
    else
    {
      int cntr = 0;
      Qualifier* q = _itsQualifier;
      while (q != nullptr)
      {
        cntr++;
        q = q->successor();
      }
      return cntr;
    }
  }

  template <typename T>
  Qualifier* setSuccessor(Qualifier* prev, FareDisplayTrx& trx)
  {
    T* successor;
    _dh.get(successor);
    bool qualifierNotSet = (_itsQualifier == nullptr);
    if (successor->setup(trx))
    {
      if (prev)
      {
        prev->successor() = successor;
        if (qualifierNotSet)
          _itsQualifier = prev;
      }
      if (qualifierNotSet)
        _itsQualifier = successor;
      return successor;
    }
    else
    {
      return prev;
    }
  }

private:
  Qualifier* _itsQualifier;
  DataHandle _dh;
};

} // namespace tse

