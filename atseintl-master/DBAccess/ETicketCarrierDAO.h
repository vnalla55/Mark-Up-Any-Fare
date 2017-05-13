//------------------------------------------------------------------------------
// Copyright 2006, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//------------------------------------------------------------------------------
//
#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/ETicketCarrierInfo.h"
#include "DBAccess/HashKey.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class DeleteList;

typedef HashKey<PseudoCityCode, CarrierCode> ETicketCarrierKey;

class ETicketCarrierDAO
    : public DataAccessObject<ETicketCarrierKey, std::vector<ETicketCarrierInfo*> >
{
public:
  static ETicketCarrierDAO& instance();

  const std::vector<ETicketCarrierInfo*>&
  get(DeleteList& del, const PseudoCityCode& agentPcc, const CarrierCode& carrier);

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  ETicketCarrierKey createKey(const ETicketCarrierInfo* info);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<ETicketCarrierDAO>;

  static DAOHelper<ETicketCarrierDAO> _helper;

  ETicketCarrierDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ETicketCarrierKey, std::vector<ETicketCarrierInfo*> >(cacheSize, cacheType)
  {
  }

  std::vector<ETicketCarrierInfo*>* create(ETicketCarrierKey key) override;

  void destroy(ETicketCarrierKey key, std::vector<ETicketCarrierInfo*>* recs) override;

  void load() override;

private:
  static ETicketCarrierDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
