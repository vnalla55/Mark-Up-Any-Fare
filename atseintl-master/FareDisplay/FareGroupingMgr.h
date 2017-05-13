//-------------------------------------------------------------------
//  File: FareGroupingMgr.h
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


#include <string>

namespace tse
{
class FareDisplayTrx;

/**
*   @class FareGroupingMgr
*
*   Description:
*   FareGroupingMgr provides the interface to group Fares.
*/

class FareGroupingMgr
{

public:
  virtual ~FareGroupingMgr() = default;
  /** Interface to group Fares*/
  virtual bool groupFares(FareDisplayTrx& trx) = 0;

  /**
   * replace constructor with factory : creates concrete class for Fare Grouping.
   * */
  static FareGroupingMgr* create(FareDisplayTrx& trx);

protected:
  /**
   * Verifies if it is necessary to generate routing sequence. Routing Sequence is generated
   * only if it is an Internation Market or Same City Pair Request.
   */
  bool isSequenceTranslationRequired(FareDisplayTrx&);
  /**
   * process the basic steps for Fare group/sort.
   */
  void processGrouping(FareDisplayTrx& trx);
};
}
