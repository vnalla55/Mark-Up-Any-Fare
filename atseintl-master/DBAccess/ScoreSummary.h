//-----------------------------------------------------------------------------------------------
//	   © 2004, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//	   and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//	   or transfer of this software/documentation, in any medium, or incorporation of this
//	   software/documentation into any system or publication, is strictly prohibited
//
//	  --------------------------------------------------------------------------------------------

#ifndef SCORESUMMARY_H
#define SCORESUMMARY_H

#include "DBAccess/Flattenizable.h"

namespace tse
{

class ScoreSummary
{
public:
  char sameCarrierInd106;
  char sameRuleTariffInd107;
  char sameFareInd108;

  int majorSetCnt;
  int dataSetNumber;
  int setWith106Cnt;
  int setWith107Cnt;
  int setWith108Cnt;

  bool operator==(const ScoreSummary& rhs) const
  {
    return ((sameCarrierInd106 == rhs.sameCarrierInd106) &&
            (sameRuleTariffInd107 == rhs.sameRuleTariffInd107) &&
            (sameFareInd108 == rhs.sameFareInd108));
  }

  static void dummyData(ScoreSummary& obj)
  {
    obj.sameCarrierInd106 = 'A';
    obj.sameRuleTariffInd107 = 'B';
    obj.sameFareInd108 = 'C';
  }

  void flattenize(Flattenizable::Archive& archive)
  {
    FLATTENIZE(archive, sameCarrierInd106);
    FLATTENIZE(archive, sameRuleTariffInd107);
    FLATTENIZE(archive, sameFareInd108);
  }

  ScoreSummary()
  {

    sameCarrierInd106 = 'X';
    sameRuleTariffInd107 = 'X';
    sameFareInd108 = 'X';
    majorSetCnt = 0;
    setWith106Cnt = 0;
    setWith107Cnt = 0;
    setWith108Cnt = 0;
    dataSetNumber = 0;
  }
};
}
#endif
