//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#ifndef FARE_CLASS_APP_DAO_H
#define FARE_CLASS_APP_DAO_H

#include "HashKey.h"
#include "DataAccessObject.h"
#include "TseTypes.h"

namespace tse
{
typedef HashKey<int> FareClassAppKey;
class FareClassAppInfo;
class DeleteList;

class FareClassAppDAO : public DataAccessObject<FareClassAppKey, std::vector<const FareClassAppInfo*>>
{
public:
    static FareClassAppDAO& instance();

    FareClassAppDAO(int cacheSize = 0, const std::string& cacheType="")
    {
    }
    const std::vector<const FareClassAppInfo*>& get(DeleteList& del,
                                                    const VendorCode& vendor,
                                                    const CarrierCode& carrier,
                                                    const TariffNumber& ruleTariff,
                                                    const RuleNumber& ruleNumber,
                                                    const FareClassCode& fareClass,
                                                    const DateTime& ticketDate);

    const std::string& cacheClass()
    {
        return _cacheClass;
    }

    virtual sfc::CompressedData*
      compress(const std::vector<const FareClassAppInfo*>* vect) const;

    virtual std::vector<const FareClassAppInfo*>*
      uncompress(const sfc::CompressedData& compressed) const;
protected:
    static std::string _name;
    static std::string _cacheClass;
    std::vector<const FareClassAppInfo*>* create(FareClassAppKey key);
    void destroy(FareClassAppKey key, std::vector<const FareClassAppInfo*>* recs);

    void load();
    void clear();
private:
    struct groupByKey;
    struct isNotApplicable;
    struct isNotApplicableForFD;
    static FareClassAppDAO* _instance;
};
} // namespace tse
#endif // FARE_CLASS_APP_DAO_H
