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
#include "DBAccess/ETicketPseudoInfo.h"
#include "DBAccess/HashKey.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
typedef HashKey<PseudoCityCode, CarrierCode> ETicketPseudoKey;

class ETicketPseudoDAO : public DataAccessObject<ETicketPseudoKey, std::vector<ETicketPseudoInfo*> >
{
public:
  static ETicketPseudoDAO& instance();

  const std::vector<ETicketPseudoInfo*>&
  get(DeleteList& del, const PseudoCityCode& agentPcc, const CarrierCode& carrier);

  log4cxx::LoggerPtr& getLogger() override { return _logger; }

  ETicketPseudoKey createKey(const ETicketPseudoInfo* info);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<ETicketPseudoDAO>;

  static DAOHelper<ETicketPseudoDAO> _helper;

  ETicketPseudoDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<ETicketPseudoKey, std::vector<ETicketPseudoInfo*> >(cacheSize, cacheType)
  {
  }

  std::vector<ETicketPseudoInfo*>* create(ETicketPseudoKey key) override;

  void destroy(ETicketPseudoKey key, std::vector<ETicketPseudoInfo*>* recs) override;

  void load() override;

private:
  static ETicketPseudoDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};
} // namespace tse
