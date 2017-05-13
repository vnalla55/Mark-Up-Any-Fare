//-------------------------------------------------------------------------------
// Copyright 2004, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//
#ifndef FARE_DAO_H
#define FARE_DAO_H

#include "HashKey.h"
#include "DataAccessObject.h"
#include "TseTypes.h"

namespace tse
{
typedef HashKey<int> FareInfoKey;
class FareInfo;
class DeleteList;

class FareDAO : public DataAccessObject<FareInfoKey, std::vector<const FareInfo*>>
{
public:

    static FareDAO& instance();

    const std::vector<const FareInfo*>& get(DeleteList& del,
                                            const LocCode& market1,
                                            const LocCode& market2,
                                            const CarrierCode& cxr,
                                            const DateTime& startDate,
                                            const DateTime& endDate,
                                            const DateTime& ticketDate,
                                            bool fareDisplay = false);

    const std::vector<const FareInfo*>& get(DeleteList& del,
                                           const LocCode& market1,
                                           const LocCode& market2,
                                           const CarrierCode& cxr,
                                           const VendorCode& vendor,
                                           const DateTime& ticketDate);

    void loadFaresForMarket(const LocCode& market1, const LocCode& market2,
                            const std::vector<CarrierCode>& cxr);

    bool isRuleInFareMarket(const LocCode& market1,
                            const LocCode& market2,
                            const CarrierCode& cxr,
                            const RuleNumber ruleNumber);

    const std::string& cacheClass()
    {
        return _cacheClass;
    }

protected:

    static std::string _name;
    static std::string _cacheClass;
public:
    FareDAO()
    {
    }

  virtual sfc::CompressedData *
    compress (const std::vector<const FareInfo*> *vect) const;

  virtual std::vector<const FareInfo*> *
    uncompress (const sfc::CompressedData &compressed) const;

    std::vector<const FareInfo*>* create(FareInfoKey key);

    void destroy(FareInfoKey key, std::vector<const FareInfo*>* t);

private:
    struct isEffective;
    struct isVendor;
    static FareDAO* _instance;
};

} // namespace tse
#endif // FARE_DAO_H

