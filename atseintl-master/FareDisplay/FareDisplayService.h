//----------------------------------------------------------------------------
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Service/Service.h"

#include <set>
#include <vector>

namespace tse
{
class ConfigMan;
class FareDisplayTrx;
class NonPublishedValues;
class PaxTypeFare;
class RuleItemInfo;
class TseServer;

class FareDisplayService : public Service
{
  friend class FareDisplayServiceTest;
  friend class FDService_merge;

public:
  FareDisplayService(const std::string& sname, TseServer& tseServer);

  bool initialize(int argc = 0, char* argv[] = nullptr) override;

  virtual bool process(FareDisplayTrx& trx) override;

private:
  ConfigMan& _config;
  bool applyGroupingAndSorting(FareDisplayTrx& trx);
  void filterScheduleCount(FareDisplayTrx& trx);

  enum RequestType
  { FQ_REQUEST,
    AD_REQUEST,
    RD_REQUEST,
    RB_REQUEST,
    FT_REQUEST,
    MP_REQUEST,
    DIAGNOSTIC_REQUEST };

private:

  bool processFQ(FareDisplayTrx& trx);
  bool processFT(FareDisplayTrx& trx);
  bool processRD(FareDisplayTrx& trx);
  bool processAD(FareDisplayTrx& trx);
  bool processRB(FareDisplayTrx& trx);
  bool processMP(FareDisplayTrx&);
  bool processSDS(FareDisplayTrx&);
  bool processDiagnostic(FareDisplayTrx& trx);
  RequestType getRequestType(FareDisplayTrx& trx);
  void processRequest(FareDisplayTrx& trx, FareDisplayService::RequestType& reqType);
  void display(FareDisplayTrx& trx);
  void convertCurrency(FareDisplayTrx& trx);
  void setupDisplayCurrency(FareDisplayTrx& trx);
  void checkPaxTypes(FareDisplayTrx& trx);
  void eliminateDuplicateFares(FareDisplayTrx& trx);
  void checkPrivateIndicator(FareDisplayTrx& trx);
  void checkItemNo(FareDisplayTrx& trx);
  void removePTF(FareDisplayTrx& trx, std::vector<PaxTypeFare*>::iterator& ptfItr);
  bool
  matchNPValues(PaxTypeFare& ptf, NonPublishedValues& nonPubValues, const RuleItemInfo* itemInfo);
  virtual void mergeFares(FareDisplayTrx& trx, const bool needAllFares = false);
};
}
