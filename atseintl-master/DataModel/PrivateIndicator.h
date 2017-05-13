//-------------------------------------------------------------------
//
//  File:        PrivateIndicator.h
//  Created:     April 7, 2005
//  Authors:     Tony Lam
//  Description: This class will process Private Indicator for Private
//               fare.  There are 4 different Private Indicators.
//               PILLOW -> filed without Account Code or Corporate ID.
//                         filed with Cat 15 Security;
//                         Cat 25 flat or percent off discount, with
//                           Cat 15 or 35.
//                         Cat 35 selling fare with Display Type L or T, not
//                           marked up.
//                         When fare is marked up, then change to a slash.
//               SLASH  -> filed via Cat 35 and marked up via NFMU tool or FMS
//                           marked up tool and no Account Code or Corporate
//                           ID.
//               X      -> Ticketing ineligible Cat 35.
//               STAR   -> filed with a Corporate ID or an Account Code.
//                         marked up fare via Cat 35 and has a Corp ID or
//                         Account Code.
//
//  Updates:
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

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <string>

namespace tse
{
class PaxTypeFare;

class PrivateIndicator
{
public:
  static const std::string BLANK_INDICATOR;
  static constexpr char YES_INDICATOR =
      'Y'; // this value defines the mode date not in last 24 hours Indicator
  static const std::string PILLOW;
  static const std::string SLASH;
  static const std::string TICKETING_INELIGIBLE;
  static const std::string STAR;

  static void privateIndicatorOld(const PaxTypeFare& ptf,
                               std::string& privateInd,
                               bool setToBlank = true,
                               bool isFQ = false);

  static void privateIndicator(const PaxTypeFare& ptf,
                                 std::string& privateInd,
                                 bool setToBlank = true,
                                 bool isFQ = false);

  enum IndicatorSeq
  { NotPrivate = 0,
    CorpIDSeq,
    Cat35Seq,
    XTktC35,
    Private };

  static constexpr Indicator IND_NOTPRIVATE = ' ';
  static constexpr Indicator IND_CORPID = '*';
  static constexpr Indicator IND_CAT35 = '/';
  static constexpr Indicator IND_XTKTC35 = 'X';
  static constexpr Indicator IND_PRIVATE = '@';

  static uint16_t getPrivateFareIndicatorOld(const PaxTypeFare& paxTypeFare, bool isFQ = false);
  static uint16_t getPrivateFareIndicator(const PaxTypeFare& paxTypeFare, bool isFQ = false);

  static const uint16_t NUM_IND_TYPE = 5;
  static void resolvePrivateFareIndOld(uint16_t& targetIndSeq, uint16_t nextIndSeq);
  static void resolvePrivateFareInd(uint16_t& targetIndSeq, uint16_t nextIndSeq);

  static Indicator privateFareIndicator(uint16_t privateIndSeq)
  {
    if (privateIndSeq < NUM_IND_TYPE)
      return _indicators[privateIndSeq];
    else
      return _indicators[NotPrivate];
  }
  static std::string const& privateFareIndicatorStr(uint16_t privateIndSeq)
  {
    if (privateIndSeq < NUM_IND_TYPE)
      return _indStr[privateIndSeq];
    else
      return _indStr[NotPrivate];
  }

private:
  static const Indicator _indicators[NUM_IND_TYPE];
  static const std::string _indStr[NUM_IND_TYPE];
};

} // namespace tse

