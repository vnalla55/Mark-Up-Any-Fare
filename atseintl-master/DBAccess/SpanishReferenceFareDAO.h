#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/DAOHelper.h"
#include "DBAccess/DataAccessObject.h"
#include "DBAccess/HistoricalDataAccessObject.h"

namespace tse
{
class DeleteList;
class Logger;
class SpanishReferenceFareInfo;

typedef HashKey<CarrierCode, CarrierCode, LocCode, LocCode> SpanishReferenceFareKey;

class SpanishReferenceFareDAO
  : public DataAccessObject<SpanishReferenceFareKey, std::vector<SpanishReferenceFareInfo*> >
{
public:
  static SpanishReferenceFareDAO& instance();

  const std::vector<SpanishReferenceFareInfo*>&
  get (DeleteList& del, const CarrierCode& tktCarrier,
       const CarrierCode& fareCarrier, const LocCode& sourceLoc,
       const LocCode& destLoc, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SpanishReferenceFareKey& key) const override
  {
    return key.initialized =  objectKey.getValue("TKTCARRIER", key._a) &&
                              objectKey.getValue("FARECARRIER", key._b) &&
                              objectKey.getValue("ORIGINAIRPORT", key._c) &&
                              objectKey.getValue("DESTINATIONAIRPORT", key._d);
  }

  SpanishReferenceFareKey createKey(const SpanishReferenceFareInfo* info);

  void translateKey(const SpanishReferenceFareKey& key, ObjectKey& objectKey) const override
  {
    objectKey.setValue("TKTCARRIER", key._a);
    objectKey.setValue("FARECARRIER", key._b);
    objectKey.setValue("ORIGINAIRPORT", key._c);
    objectKey.setValue("DESTINATIONAIRPORT", key._d);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SpanishReferenceFareDAO>;
  static DAOHelper<SpanishReferenceFareDAO> _helper;
  SpanishReferenceFareDAO(int cacheSize = 0, const std::string& cacheType = "")
    : DataAccessObject<SpanishReferenceFareKey, std::vector<SpanishReferenceFareInfo*> >(
          cacheSize, cacheType)
  {
  }
  std::vector<SpanishReferenceFareInfo*>* create(SpanishReferenceFareKey key) override;
  void destroy(SpanishReferenceFareKey key, std::vector<SpanishReferenceFareInfo*>* recs) override;

private:
  static SpanishReferenceFareDAO* _instance;
};

typedef HashKey<CarrierCode, CarrierCode, LocCode, LocCode, DateTime, DateTime> SpanishReferenceFareHistoricalKey;

class SpanishReferenceFareHistoricalDAO
    : public HistoricalDataAccessObject<SpanishReferenceFareHistoricalKey, std::vector<SpanishReferenceFareInfo*> >
{
public:
  static SpanishReferenceFareHistoricalDAO& instance();
  const std::vector<SpanishReferenceFareInfo*>&
  get(DeleteList& del, const CarrierCode& tktCarrier, const CarrierCode& fareCarrier,
      const LocCode& sourceLoc, const LocCode& destLoc, const DateTime& date, const DateTime& ticketDate);

  bool translateKey(const ObjectKey& objectKey, SpanishReferenceFareHistoricalKey& key) const override
  {
    return key.initialized =  objectKey.getValue("TKTCARRIER", key._a) &&
                              objectKey.getValue("FARECARRIER", key._b) &&
                              objectKey.getValue("ORIGINAIRPORT", key._c) &&
                              objectKey.getValue("DESTINATIONAIRPORT", key._d) &&
                              objectKey.getValue("STARTDATE", key._e) &&
                              objectKey.getValue("ENDDATE", key._f);
  }

  bool translateKey(const ObjectKey& objectKey,
                    SpanishReferenceFareHistoricalKey& key,
                    const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);

    return key.initialized = objectKey.getValue("TKTCARRIER", key._a)
        && objectKey.getValue("FARECARRIER", key._b) && objectKey.getValue("ORIGINAIRPORT", key._c)
        && objectKey.getValue("DESTINATIONAIRPORT", key._d);
  }

  void setKeyDateRange(SpanishReferenceFareHistoricalKey& key, const DateTime ticketDate) const override
  {
    DAOUtils::getDateRange(ticketDate, key._e, key._f, _cacheBy);
  }

  const std::string& cacheClass() override { return _cacheClass; }

protected:
  static std::string _name;
  static std::string _cacheClass;
  friend class DAOHelper<SpanishReferenceFareHistoricalDAO>;
  static DAOHelper<SpanishReferenceFareHistoricalDAO> _helper;
  SpanishReferenceFareHistoricalDAO(int cacheSize = 0, const std::string& cacheType = "") :
      HistoricalDataAccessObject<SpanishReferenceFareHistoricalKey,
          std::vector<SpanishReferenceFareInfo*> >(cacheSize, cacheType)
  {
  }
  std::vector<SpanishReferenceFareInfo*>* create(SpanishReferenceFareHistoricalKey key) override;
  void destroy(SpanishReferenceFareHistoricalKey, std::vector<SpanishReferenceFareInfo*>* t) override;

private:
  static SpanishReferenceFareHistoricalDAO* _instance;
};
} // namespace tse

