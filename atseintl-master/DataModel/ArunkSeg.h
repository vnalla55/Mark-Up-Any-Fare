//-------------------------------------------------------------------
//
//  File:        ArunkSeg.h
//  Created:     May 6, 2004
//  Authors:
//
//  Description: Itinerary arunk segment
//
//  Updates:
//          06/06/04 - VN - file created.
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

#pragma once

#include "Common/TseStringTypes.h"
#include "DataModel/TravelSeg.h"

namespace tse
{
class DataHandle;

class ArunkSeg : public TravelSeg
{
public:
  ArunkSeg();
  virtual ~ArunkSeg();

  virtual bool isArunk() const override { return true; }

  virtual ArunkSeg* toArunkSeg() override { return this; }
  virtual const ArunkSeg* toArunkSeg() const override { return this; }
  virtual ArunkSeg& toArunkSegRef() override { return *this; }
  virtual const ArunkSeg& toArunkSegRef() const override { return *this; }

  virtual ArunkSeg* clone(DataHandle& dh) const override;

  bool isRtwDynamicSupplementalArunk() const { return _rtwDynamicSupplementalArunk; }
  void setRtwDynamicSupplementalArunk(const bool value) { _rtwDynamicSupplementalArunk = value; }

private:
  bool _rtwDynamicSupplementalArunk;
};

} // tse namespace

