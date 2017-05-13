//-------------------------------------------------------------------
//  Copyright Sabre 2016
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

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"
#include "DBAccess/Queries/QueryGetMaxPermittedAmountFare.h"

namespace tse
{
class DeleteList;
class Loc;
class Logger;
class MaxPermittedAmountFareInfo;

class MaxPermittedAmountFareDAO final
    : public DataAccessObject<MaxPermittedAmountFareKey, std::vector<MaxPermittedAmountFareInfo*>>
{
public:
  static MaxPermittedAmountFareDAO& instance();

  const std::vector<MaxPermittedAmountFareInfo*>& get(DeleteList& del,
                                                      const Loc& origin,
                                                      const Loc& dest,
                                                      const DateTime& date,
                                                      const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, MaxPermittedAmountFareKey& key) const override
  {
    return key.initialized = objectKey.getValue("ORIGINAIRPORT", key._a) &&
                             objectKey.getValue("ORIGINCITY", key._b) &&
                             objectKey.getValue("ORIGINNATION", key._c) &&
                             objectKey.getValue("DESTAIRPORT", key._d) &&
                             objectKey.getValue("DESTCITY", key._e) &&
                             objectKey.getValue("DESTNATION", key._f);
  }

  MaxPermittedAmountFareKey createKey(const MaxPermittedAmountFareInfo* info);

  void translateKey(const MaxPermittedAmountFareKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("ORIGINAIRPORT", key._a);
    objectKey.setValue("ORIGINCITY", key._b);
    objectKey.setValue("ORIGINNATION", key._c);
    objectKey.setValue("DESTAIRPORT", key._d);
    objectKey.setValue("DESTCITY", key._e);
    objectKey.setValue("DESTNATION", key._f);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MaxPermittedAmountFareDAO>;
  static DAOHelper<MaxPermittedAmountFareDAO> _helper;
  MaxPermittedAmountFareDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<MaxPermittedAmountFareKey, std::vector<MaxPermittedAmountFareInfo*>>(
          cacheSize, cacheType)
  {
  }
  std::vector<MaxPermittedAmountFareInfo*>* create(MaxPermittedAmountFareKey key) override;
  void
  destroy(MaxPermittedAmountFareKey key, std::vector<MaxPermittedAmountFareInfo*>* recs) override;

private:
  static MaxPermittedAmountFareDAO* _instance;
};

class MaxPermittedAmountFareHistoricalDAO
    : public HistoricalDataAccessObject<MaxPermittedAmountFareHistoricalKey,
                                        std::vector<MaxPermittedAmountFareInfo*>>
{
public:
  static MaxPermittedAmountFareHistoricalDAO& instance();
  const std::vector<MaxPermittedAmountFareInfo*>& get(DeleteList& del,
                                                      const Loc& origin,
                                                      const Loc& dest,
                                                      const DateTime& date,
                                                      const DateTime& ticketDate);

  bool
  translateKey(const ObjectKey& objectKey, MaxPermittedAmountFareHistoricalKey& key) const override
  {
    return key.initialized = checkKey(objectKey, key) && objectKey.getValue("STARTDATE", key._g) &&
                             objectKey.getValue("ENDDATE", key._h);
  }

  bool translateKey(const ObjectKey& objectKey,
                    MaxPermittedAmountFareHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._g, key._h, _cacheBy);

    return key.initialized = checkKey(objectKey, key);
  }

  void setKeyDateRange(MaxPermittedAmountFareHistoricalKey& key, const DateTime ticketDate) const
      override
  {
    DAOUtils::getDateRange(ticketDate, key._g, key._h, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<MaxPermittedAmountFareHistoricalDAO>;
  static DAOHelper<MaxPermittedAmountFareHistoricalDAO> _helper;
  MaxPermittedAmountFareHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "")
    : HistoricalDataAccessObject<MaxPermittedAmountFareHistoricalKey,
                                 std::vector<MaxPermittedAmountFareInfo*>>(cacheSize, cacheType)
  {
  }
  std::vector<MaxPermittedAmountFareInfo*>*
  create(MaxPermittedAmountFareHistoricalKey key) override;
  void destroy(MaxPermittedAmountFareHistoricalKey,
               std::vector<MaxPermittedAmountFareInfo*>* t) override;

private:
  bool checkKey(const ObjectKey& objectKey, MaxPermittedAmountFareHistoricalKey& key) const
  {
    return objectKey.getValue("ORIGINAIRPORT", key._a) &&
           objectKey.getValue("ORIGINCITY", key._b) && objectKey.getValue("ORIGINNATION", key._c) &&
           objectKey.getValue("DESTAIRPORT", key._d) && objectKey.getValue("DESTCITY", key._e) &&
           objectKey.getValue("DESTNATION", key._f);
  }

  static MaxPermittedAmountFareHistoricalDAO* _instance;
};
} // namespace tse
