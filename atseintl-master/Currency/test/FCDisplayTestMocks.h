#include <vector>
#include <string>
#include <exception>
#define protected public
#include "DBAccess/DataHandle.h"
#include "DBAccess/NUCInfo.h"
#include "DBAccess/Currency.h"
#undef protected

namespace tse
{

// mocks
namespace
{
// test data - entries for multiple carriers, multiple dates, multiple currencies, multiple rates &
// multiple nucFactors
const char* test_nuc_data[][9] = {
  //	curr   carrier	expireDate		    effDate			  nucFact    rndF nucNd rndNd rRule
  { "PLN", "", "1990-11-07 23:13:44.000", "1980-11-07 23:13:44.000", "1.7134532", "0", "7", "0",
    "UP" },
  { "PLN", "", "1995-11-07 23:13:44.000", "1990-11-07 23:13:44.000", "2.713453", "0", "6", "0",
    "DOWN" },
  { "PLN", "", "1999-11-07 23:13:44.000", "1995-11-07 23:13:44.000", "3", "0", "0", "0", "" },
  { "PLN", "", "2004-11-07 23:13:44.000", "1999-11-07 23:13:44.000", "4.4532", "0", "4", "0", "" },
  { "PLN", "", "2007-01-07 23:13:44.000", "2004-11-07 23:13:44.000", "5.7134532", "0", "7", "0",
    "" },
  { "PLN", "AA", "2007-01-07 23:13:44.000", "2004-11-07 23:13:44.000", "5.9434532", "0", "7", "0",
    "" },
  { "PLN", "", "2012-11-07 23:13:44.000", "2007-01-07 23:13:44.000", "7.98234", "0", "5", "0", "" },
  { "PLN", "AA", "2012-11-07 23:13:44.000", "2007-01-07 23:13:44.000", "7", "0", "0", "0", "" },
  { "PLN", "5T", "2012-11-07 23:13:44.000", "2007-01-07 23:13:44.000", "7", "0", "0", "0", "" },
  { "PLN", "", "2018-11-07 23:13:44.000", "2012-11-07 23:13:44.000", "7", "0", "0", "0", "" },
  { "PLN", "", "9999-12-31 23:13:44.000", "2018-11-07 23:13:44.000", "9.7132", "0", "4", "0", "" },
  { "GBP", "", "1990-11-07 23:13:44.000", "1980-11-07 23:13:44.000", "0.17134532", "0", "8", "0",
    "UP" },
  { "GBP", "", "1995-11-07 23:13:44.000", "1990-11-07 23:13:44.000", "0.2713453", "0", "7", "0",
    "DOWN" },
  { "GBP", "", "1999-11-07 23:13:44.000", "1995-11-07 23:13:44.000", "0.3", "0", "1", "0", "" },
  { "GBP", "", "2004-11-07 23:13:44.000", "1999-11-07 23:13:44.000", "0.44532", "0", "5", "0", "" },
  { "GBP", "", "2007-01-07 23:13:44.000", "2004-11-07 23:13:44.000", "0.57134532", "0", "8", "0",
    "" },
  { "GBP", "AA", "2007-01-07 23:13:44.000", "2004-11-07 23:13:44.000", "0.59434532", "0", "8", "0",
    "" },
  { "GBP", "", "2012-11-07 23:13:44.000", "2007-01-07 23:13:44.000", "0.712345", "0", "6", "0",
    "" },
  { "GBP", "AA", "2012-11-07 23:13:44.000", "2007-01-07 23:13:44.000", "0.7", "0", "1", "0", "" },
  { "GBP", "5T", "2012-11-07 23:13:44.000", "2007-01-07 23:13:44.000", "0.7", "0", "1", "0", "" },
  { "GBP", "", "2018-11-07 23:13:44.000", "2012-11-07 23:13:44.000", "0.7", "0", "1", "0", "" },
  { "GBP", "", "9999-12-31 23:13:44.000", "2018-11-07 23:13:44.000", "0.97132", "0", "5", "0", "" }
};

std::vector<NUCInfo*>* nuc_data_vector;

struct NUCTestData
{
public:
  NUCTestData() { init_data(); }

  static std::vector<NUCInfo*>* get_all_nuc_info()
  {
    std::vector<NUCInfo*>* to_ret = new std::vector<NUCInfo*>;
    for (unsigned i = 0; i < nuc_data_vector->size(); i++)
      to_ret->push_back((*nuc_data_vector)[i]);
    random_shuffle(to_ret->begin(), to_ret->end());
    return to_ret;
  }

  void init_data()
  {
    nuc_data_vector = new std::vector<NUCInfo*>;
    for (unsigned i = 0; i < sizeof(test_nuc_data) / sizeof(test_nuc_data[0]); i++)
    {
      // cout << "\nadding element\n" ;
      std::string eff_date_str = test_nuc_data[i][3];
      std::string exp_date_str = test_nuc_data[i][2];
      NUCInfo& nuc = *new NUCInfo;
      nuc._cur = test_nuc_data[i][0];
      nuc._carrier = test_nuc_data[i][1];
      nuc._expireDate = DateTime(exp_date_str);
      nuc._effDate = DateTime(eff_date_str);
      nuc._nucFactor = strtod(test_nuc_data[i][4], NULL);
      nuc._roundingFactor = atoi(test_nuc_data[i][5]);
      nuc._nucFactorNodec = atoi(test_nuc_data[i][6]);
      nuc._roundingFactorNodec = atoi(test_nuc_data[i][7]);
      nuc._roundingRule = 0 == strncmp(test_nuc_data[i][8], "UP", 2)
                              ? UP
                              : 0 == strncmp(test_nuc_data[i][8], "DOWN", 4) ? DOWN : NONE;

      nuc_data_vector->push_back(&nuc);
    }

    random_shuffle(nuc_data_vector->begin(), nuc_data_vector->end()); // we shouldn't count on the
                                                                      // fact that DB data could be
                                                                      // sorted
  }
};

static NUCTestData __init_test_data__;

struct pred1
{
  bool fut;
  CurrencyCode cc;
  DateTime dt;
  pred1(const CurrencyCode& cc_, const DateTime& dt_) : cc(cc_), dt(dt_) {}

  bool operator()(NUCInfo* nuc)
  {
    bool to_ret = nuc->_cur == cc && nuc->_effDate <= dt && nuc->_expireDate > dt;
    // cout << (to_ret ? "\nto_ret=TRUE\n":"\nto_ret=FALSE\n");
    return to_ret;
  }
};

struct pred2
{
  CurrencyCode cc;
  DateTime start_dt;
  DateTime end_dt;
  pred2(const CurrencyCode& cc_, const DateTime& dt_, const DateTime& enddt_)
    : cc(cc_), start_dt(dt_), end_dt(enddt_)
  {
  }

  bool operator()(NUCInfo* nuc)
  {
    return !(nuc->_cur == cc && nuc->_expireDate > start_dt && nuc->_effDate <= end_dt);
  }
};
}

class MockDataHandle
{
public:
  MockDataHandle() {}

  NUCInfo*
  getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
  {
    pred1 pred(currency, date);
    // cout << "GETNUCFIRST\n******************\n";
    for (unsigned i = 0; i < nuc_data_vector->size(); i++)
    {
      if (pred((*nuc_data_vector)[i]))
        return (*nuc_data_vector)[i];
    }
    return NULL;
  }

  const std::vector<NUCInfo*>&
  getNUCAllCarriers(const CurrencyCode& currency,
                    const DateTime& start_date,
                    const DateTime& end_date = boost::date_time::pos_infin)
  {
    static std::vector<NUCInfo*> toret;
    toret.clear();

    pred2 pred(currency, start_date, end_date);

    for (unsigned i = 0; i < nuc_data_vector->size(); i++)
    {
      if (!pred((*nuc_data_vector)[i]))
        toret.push_back((*nuc_data_vector)[i]);
    }

    std::random_shuffle(toret.begin(), toret.end());
    return toret;
  }

  const std::vector<CurrencyCode>& getAllCurrencies()
  {
    static std::vector<CurrencyCode> to_ret;
    static bool initialized = false;
    if (!initialized)
    {
      to_ret.push_back("PLN");
      to_ret.push_back("GBP");
      initialized = true;
    }

    return to_ret;
  }

  /*    // gets data for all carriers; enables future rates
   virtual std::vector<NUCInfo*>&
   getNUCbalhblah(const CurrencyCode& currency,
   const DateTime& start_date,
   bool get_future_rates)
   {
   //cout << "MockDataHandle::getNUC()\t";
   vector<NUCInfo*>* toret = new vector<NUCInfo*>;
   pred1 pred(currency,start_date,get_future_rates);
   //	  remove_copy_if(nuc_data_vector->begin(),nuc_data_vector->end(),(*toret).begin(),pred); //
   coredumps for some reason
   for (unsigned i=0;i<nuc_data_vector->size();i++)
   {
   if (!pred((*nuc_data_vector)[i])) toret->push_back((*nuc_data_vector)[i]) ;
   }
   //cout << "returning vector of size: " << toret->size() << "\n";
   random_shuffle(toret->begin() , toret->end());
   return * toret ;
   }*/
};

} // tse
