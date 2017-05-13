//-------------------------------------------------------------------
//
//  File:        FareSelectorRD.h
//  Created:     October 7, 2005
//  Authors:     Doug Batchelor
//
//  Updates:
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

#include "FareDisplay/FareSelector.h"

#include <log4cxx/helpers/objectptr.h>

namespace tse
{
class FareDisplayTrx;
class FareDisplayOptions;
class FareDisplayRequest;
class AddOnAttributes;

enum FailCode
{
  D291_LINK = 1,
  D291_SEQUENCE,
  D291_DATE,
  D291_FARECLASS,
  D291_FAREBASIS, // 5
  D291_FAREAMOUNT,
  D291_FARETYPE,
  D291_DISCOUNTED,
  D291_FBRULE,
  D291_NEGOTIATED, // 10
  D291_CONSTRUCTEDMISSINGINFO,
  D291_CAT35,
  D291_CARRIER,
  D291_CURRENCY,
  D291_VENDOR, // 15
  D291_TARIFF,
  D291_RULENUMBER,
  D291_GATEWAYS,
  D291_CONSTRUCTIONTYPE,
  D291_CONSTRUCTEDFARES, // 20
  D291_ORIGAO,
  D291_DESTAO,
  D291_QUALIFY,
  D291_PAXTYPE,
  D291_BOOKINGCODE, // 25
  D291_ORIGDEST,
  D291_LAST
};

//   @class FareDisplayRD
//
//   Description:
//   Retrieves The RD Fare
class FareSelectorRD : public FareSelector
{
  friend class FareSelectorTest;

public:
  virtual bool setup(FareDisplayTrx& trx) override;
  bool selectFares(FareDisplayTrx& trx) override;

  bool checkFare(FareDisplayTrx& trx,
                 FareDisplayOptions& fdo,
                 FareDisplayRequest& request,
                 PaxTypeFare& ptFare);

  bool matchAddon(PaxTypeFare& ptFare, AddOnAttributes& addOn, bool isOrig);

  bool matchPaxType(PaxTypeFare& ptFare);

private:
  FareDisplayOptions* _fdo = nullptr;
  FareDisplayRequest* _request = nullptr;

  uint16_t _faresSelected = 0;
  uint16_t _faresProcessed = 0;

  bool _wantDisc = false;
  bool _wantFbr = false;
  bool _wantNeg = false;
  bool _wantConstr = false;
  bool _wantDiag291 = false;
  std::vector<PaxTypeFare*> _faresMatched;
};

} // namespace tse

