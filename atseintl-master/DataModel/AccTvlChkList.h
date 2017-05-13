//-------------------------------------------------------------------
//
//  File:        AccTvlChkList.h
//  Authors:
//
//  Description:
//
//  Copyright Sabre 2006
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

#include "Common/SmallBitSet.h"

namespace tse
{

class AccTvlChkList
{
public:
  AccTvlChkList() : _chkFlags(AccTvl_Default) {}

  enum AccTvlChkFlag
  {
    AccTvl_Default = 0x00,
    AccTvl_ChkSameCpmt = 0x01,
    AccTvl_ChkNumPsg = 0x02,
    AccTvl_ChkSameRule = 0x04,
    AccTvl_ChkBkgCode = 0x08,
    AccTvl_PsgNegAppl = 0x10,
    AccTvl_TktGuaranteed = 0x20
  };

  typedef SmallBitSet<uint8_t, AccTvlChkFlag> AccTvlChkFlags;

  void setChkSameCpmt(bool isSet = true) { _chkFlags.set(AccTvl_ChkSameCpmt, isSet); }
  bool isChkSameCpmt() const { return _chkFlags.isSet(AccTvl_ChkSameCpmt); }

  void setChkNumPsg(bool isSet = true) { _chkFlags.set(AccTvl_ChkNumPsg, isSet); }
  bool isChkNumPsg() const { return _chkFlags.isSet(AccTvl_ChkNumPsg); }

  void setChkSameRule(bool isSet = true) { _chkFlags.set(AccTvl_ChkSameCpmt, isSet); }
  bool isChkSameRule() const { return _chkFlags.isSet(AccTvl_ChkSameRule); }

  void setPsgNegAppl(bool isSet = true) { _chkFlags.set(AccTvl_PsgNegAppl, isSet); }
  bool isPsgNegAppl() const { return _chkFlags.isSet(AccTvl_PsgNegAppl); }

  void setChkBkgCode(bool isSet = true) { _chkFlags.set(AccTvl_ChkBkgCode, isSet); }
  bool isChkBkgCode() const { return _chkFlags.isSet(AccTvl_ChkBkgCode); }

  void setTktGuaranteed(bool isSet = true) { _chkFlags.set(AccTvl_TktGuaranteed, isSet); }
  bool isTktGuaranteed() const { return _chkFlags.isSet(AccTvl_TktGuaranteed); }

  void set(uint8_t& value) { _chkFlags.set((AccTvlChkFlag)value); }
  const uint8_t& value() const { return _chkFlags.value(); }

private:
  AccTvlChkFlags _chkFlags;
};

} // tse namespace

