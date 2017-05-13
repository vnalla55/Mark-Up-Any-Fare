//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#pragma once

#include "TseTypes.h"
#include "Flattenizable.h"
#include "CompressedDataUtils.h"

namespace tse
{
  class OptionalServicesSeg
  {
  public:
    OptionalServicesSeg()
      : _segNo(0)
      , _category(0)
      , _ruleTariff(0)
    {
    }

    uint16_t&                 segNo() { return _segNo; }
    const uint16_t&           segNo() const { return _segNo; }

    uint16_t&                 category() { return _category; }
    const uint16_t&           category() const { return _category; }

    uint16_t&                 ruleTariff() { return _ruleTariff; }
    const uint16_t&           ruleTariff() const { return _ruleTariff; }

    RuleNumber&               rule() { return _rule; }
    const RuleNumber&         rule() const { return _rule; }

    bool operator==(const OptionalServicesSeg &second) const
    {
      return
        (_segNo             == second._segNo) &&
        (_category          == second._category) &&
        (_ruleTariff        == second._ruleTariff) &&
        (_rule              == second._rule);
    }

    void flattenize(Flattenizable::Archive & archive)
    {
      FLATTENIZE(archive, _segNo);
      FLATTENIZE(archive, _category);
      FLATTENIZE(archive, _ruleTariff);
      FLATTENIZE(archive, _rule);
    }

    static void dummyData(OptionalServicesSeg& obj)
    {
      obj._segNo          = 1;
      obj._category       = 2;
      obj._ruleTariff     = 3;
      obj._rule           = "ABCD";
    }

    WBuffer &write (WBuffer &os) const
    {
      return convert(os, this);
    }

    RBuffer &read (RBuffer &is)
    {
      return convert(is, this);
    }
  protected:
    uint16_t              _segNo;
    uint16_t              _category;
    uint16_t              _ruleTariff;
    RuleNumber            _rule;

  private:
    template <typename B, typename T> static B& convert (B& buffer,
                                                         T ptr)
    {
      return buffer
             & ptr->_segNo
             & ptr->_category
             & ptr->_ruleTariff
             & ptr->_rule;
    }
  };
}
