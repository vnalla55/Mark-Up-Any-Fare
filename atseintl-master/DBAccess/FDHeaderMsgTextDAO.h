//----------------------------------------------------------------------------
//  File: FDHeaderMsgTextDAO.h
//
//  Author: Partha Kumar Chakraborti
//
//  Copyright Sabre 2005
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "Common/TseEnums.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/DeleteList.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "FareDisplay/FDHeaderMsgText.h"

#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
typedef HashKey<uint64_t> FDHeaderMsgTextKey;

class FDHeaderMsgTextDAO
    : public DataAccessObject<FDHeaderMsgTextKey, std::vector<const std::string*> >
{
public:
  static FDHeaderMsgTextDAO& instance();
  std::vector<const std::string*>&
  get(DeleteList& del, const uint64_t& itemNo, const DateTime& ticketDate);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FDHeaderMsgTextDAO>;

  static DAOHelper<FDHeaderMsgTextDAO> _helper;

  FDHeaderMsgTextDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<FDHeaderMsgTextKey, std::vector<const std::string*> >(cacheSize, cacheType)
  {
  }

  std::vector<const std::string*>* create(FDHeaderMsgTextKey key) override;

  void destroy(FDHeaderMsgTextKey key, std::vector<const std::string*>* t) override;

private:
  struct isEffective;
  static FDHeaderMsgTextDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

// --------------------------------------------------
// Historical DAO: FDHeaderMsgTextHistoricalDAO
// --------------------------------------------------
class FDHeaderMsgTextHistoricalDAO
    : public HistoricalDataAccessObject<FDHeaderMsgTextKey, std::vector<const std::string*> >
{
public:
  static FDHeaderMsgTextHistoricalDAO& instance();
  std::vector<const std::string*>&
  get(DeleteList& del, const uint64_t& itemNo, const DateTime& ticketDate);

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;

  friend class DAOHelper<FDHeaderMsgTextHistoricalDAO>;

  static DAOHelper<FDHeaderMsgTextHistoricalDAO> _helper;

  FDHeaderMsgTextHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<FDHeaderMsgTextKey, std::vector<const std::string*> >(cacheSize,
                                                                                       cacheType)
  {
  }

  std::vector<const std::string*>* create(FDHeaderMsgTextKey key) override;

  void destroy(FDHeaderMsgTextKey key, std::vector<const std::string*>* t) override;

private:
  struct isEffective;
  static FDHeaderMsgTextHistoricalDAO* _instance;
  static log4cxx::LoggerPtr _logger;
};

} // End of namespace tse
