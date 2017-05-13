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

#include "DataModel/MileageTrx.h"
#include "Service/Service.h"


namespace tse
{
class Trx;
class TseServer;
class MileageService;
class MileageRoute;
class MileageRouteItem;
class DataHandle;
class GDPrompt;
class ConfigMan;

class MileageService final : public Service
{
  friend class MileageServiceTest;

public:
  MileageService(const std::string& name, tse::TseServer& server);

  /**
   * Ititialize the object
   *
   * @param argc number of parameters
   * @param argv parameters
   *
   * @return true if successful, false otherwise
   */

  bool initialize(int argc = 0, char* argv[] = nullptr) override;

  /**
   * Process a transaction object
   *
   * @param trx    transaction to process
   *
   * @return true if successful, false otherwise
   */

  virtual bool process(MileageTrx& trx) override;

private:
  ConfigMan& _config;

  virtual bool getTPM(MileageRouteItem&, DataHandle&, GDPrompt*&) const;
  virtual bool getTPM(MileageRouteItem&, DataHandle&) const;
  virtual bool getSouthAtlantic(MileageRoute&) const;
  virtual bool getTPD(MileageRoute&) const;

  bool processTPD(MileageRoute&) const;
  bool processEqualization(MileageRoute&) const;
};
} // end namespace tse

