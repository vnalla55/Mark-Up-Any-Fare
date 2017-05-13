// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include <tr1/functional>

#include <boost/format.hpp>
#include <boost/spirit/include/qi_numeric.hpp>

#include "DataModel/PricingTrx.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Diagnostic/Diagnostic.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/PaxTypeCodeInfo.h"
#include "DBAccess/SectorDetailInfo.h"
#include "DBAccess/ServiceBaggageInfo.h"
#include "DBAccess/TaxCodeTextInfo.h"
#include "DBAccess/TaxReportingRecordInfo.h"
#include "DBAccess/TaxRulesRecord.h"
#include "Taxes/LegacyFacades/DaoDiagnostics.h"

namespace tse
{

const char* DaoDiagnostics::PARAM_TABLE_NAME = "TB";

DaoDiagnostics::DaoDiagnostics() {}

DaoDiagnostics::~DaoDiagnostics() {}

namespace DaoDiag
{

class DaoTable
{
public:
  template <class T>
  static void printTableDiagnostic(DiagCollector& diagCollector, const PricingTrx& trx)
  {
    printTableName<T>(diagCollector);
    T::print(diagCollector, trx);
  }

protected:
  template <class R, class T>
  static void printEntryData(DiagCollector& diagCollector, T getData)
  {
    if (nullptr != getData)
    {
      const R& data = getData();
      for(typename R::value_type elem : data)
      {
        diagCollector << "\n-----------\n";
        elem->print(diagCollector);
      }
    }
  }

  template <class P>
  static typename P::valueType getParam(DiagCollector& diagCollector, const PricingTrx& trx)
  {
    const std::string& param = trx.diagnostic().diagParamMapItem(P::name());
    if (!param.empty())
    {
      return P::getParam(param);
    }
    std::string errorMsg;
    errorMsg.append("Error: ").append(P::name()).append(" param was not specified\n");
    throw std::invalid_argument(errorMsg);
  }

  template <class P1, class P2, class R, class F>
  static void printDiagnosticData(DiagCollector& diagCollector, const PricingTrx& trx, F fun)
  {
    try
    {
      typename P1::valueType param1 = getParam<P1>(diagCollector, trx);
      typename P2::valueType param2 = getParam<P2>(diagCollector, trx);

      std::tr1::function<R(void)> getData = std::tr1::bind(fun, &trx.dataHandle(), param1, param2);
      printEntryData<R>(diagCollector, getData);
    }
    catch (const std::exception& e)
    {
      diagCollector << e.what();
      return;
    }
  }

private:
  DaoTable();
  ~DaoTable();
  DaoTable(const DaoTable&);
  DaoTable& operator==(const DaoTable&);

  template <class T>
  static void printTableName(DiagCollector& diagCollector)
  {
    diagCollector << boost::format("\n ---- %-s TABLE %|58t| ----\n") % T::name()
                  << "***************************************************************\n";
  }
};

template <typename T>
class Param
{
public:
  typedef T valueType;

  static valueType getParam(const std::string& s) { return valueType(s); }
};

class ParamVendor : public Param<VendorCode>
{
public:
  static const char* name()
  {
    return "VE";
  };
};

class ParamItemNo
{
public:
  typedef int valueType;
  static const char* name()
  {
    return "NR";
  };
  static valueType getParam(const std::string& s)
  {
    int itemNr = std::numeric_limits<int>::max();
    boost::spirit::qi::parse(s.begin(), s.end(), boost::spirit::qi::int_, itemNr);
    return itemNr;
  }
};

class ParamNation : public Param<NationCode>
{
public:
  static const char* name()
  {
    return "NA";
  };
};

class ParamTaxPointTag
{
public:
  typedef Indicator valueType;
  static const char* name()
  {
    return "TP";
  };
  static valueType getParam(const std::string& s)
  {
    return s.empty() ? Indicator('\0') : Indicator(s[0]);
  }
};

class ParamCarrierCode : public Param<CarrierCode>
{
public:
  static const char* name()
  {
    return "CC";
  };
};

class ParamTaxCode : public Param<TaxCode>
{
public:
  static const char* name()
  {
    return "TC";
  };
};

class ParamTaxType : public Param<TaxType>
{
public:
  static const char* name()
  {
    return "TT";
  };
};

class TablePassengerTypeCode : public DaoTable
{
public:
  static const char* name()
  {
    return "PASSENGERTYPECODE";
  };
  static void print(DiagCollector& diagCollector, const PricingTrx& trx)
  {
    printDiagnosticData<ParamVendor, ParamItemNo, std::vector<const PaxTypeCodeInfo*> >(
        diagCollector, trx, &DataHandle::getPaxTypeCode);
  }
};

class TableServiceBaggage : public DaoTable
{
public:
  static const char* name()
  {
    return "SERVICEBAGGAGE";
  };
  static void print(DiagCollector& diagCollector, const PricingTrx& trx)
  {
    printDiagnosticData<ParamVendor, ParamItemNo, std::vector<const ServiceBaggageInfo*> >(
        diagCollector, trx, &DataHandle::getServiceBaggage);
  }
};

class TableSectorDetail : public DaoTable
{
public:
  static const char* name()
  {
    return "SECTORDETAIL";
  };
  static void print(DiagCollector& diagCollector, const PricingTrx& trx)
  {
    printDiagnosticData<ParamVendor, ParamItemNo, std::vector<const SectorDetailInfo*> >(
        diagCollector, trx, &DataHandle::getSectorDetail);
  }
};

class TableTaxCodeText : public DaoTable
{
public:
  static const char* name()
  {
    return "TAXCODETEXT";
  };
  static void print(DiagCollector& diagCollector, const PricingTrx& trx)
  {
    printDiagnosticData<ParamVendor, ParamItemNo, std::vector<const TaxCodeTextInfo*> >(
        diagCollector, trx, &DataHandle::getTaxCodeText);
  }
};

class TableTaxRulesRecord : public DaoTable
{
public:
  static const char* name()
  {
    return "TAXRULESRECORD";
  };
  static void print(DiagCollector& diagCollector, const PricingTrx& trx)
  {
    try
    {
      NationCode param1 = getParam<ParamNation>(diagCollector, trx);
      Indicator param2 = getParam<ParamTaxPointTag>(diagCollector, trx);
      typedef const std::vector<const TaxRulesRecord*> RetType;
      std::tr1::function<RetType(void)> getData = std::tr1::bind(
          &DataHandle::getTaxRulesRecord, &trx.dataHandle(), param1, param2, trx.ticketingDate());
      printEntryData<RetType>(diagCollector, getData);
    }
    catch (const std::exception& e)
    {
      diagCollector << e.what();
      return;
    }
  }
};

class TableTaxReportingRecord : public DaoTable
{
public:
  static const char* name()
  {
    return "TAXREPORTINGRECORD";
  };
  static void print(DiagCollector& diagCollector, const PricingTrx& trx)
  {
    try
    {
      VendorCode param1 = getParam<ParamVendor>(diagCollector, trx);
      NationCode param2 = getParam<ParamNation>(diagCollector, trx);
      CarrierCode param3 = getParam<ParamCarrierCode>(diagCollector, trx);
      TaxCode param4 = getParam<ParamTaxCode>(diagCollector, trx);
      TaxType param5 = getParam<ParamTaxType>(diagCollector, trx);

      typedef const std::vector<const TaxReportingRecordInfo*> RetType;
      std::tr1::function<RetType(void)> getData = std::tr1::bind(&DataHandle::getTaxReportingRecord,
                                                                 &trx.dataHandle(),
                                                                 param1,
                                                                 param2,
                                                                 param3,
                                                                 param4,
                                                                 param5,
                                                                 trx.ticketingDate());
      printEntryData<RetType>(diagCollector, getData);
    }
    catch (const std::exception& e)
    {
      diagCollector << e.what();
      return;
    }
  }
};

} // DaoDiag

void
DaoDiagnostics::printDBDiagnostic(PricingTrx& trx)
{
  DCFactory* factory = DCFactory::instance();
  DiagCollector& diagCollector = *(factory->create(trx));
  diagCollector.trx() = &trx;
  diagCollector.enable(Diagnostic831);
  diagCollector << "\n"
                   "***************************************************************"
                   "\n"
                   " ----               DAO DIAG ACTIVATED                    ----"
                   "\n"
                   "***************************************************************"
                   "\n";
  using namespace tse::DaoDiag;
  const std::string& tableName = trx.diagnostic().diagParamMapItem(PARAM_TABLE_NAME);
  if (!tableName.empty())
  {

    if (tableName == TablePassengerTypeCode::name())
    {
      DaoTable::printTableDiagnostic<TablePassengerTypeCode>(diagCollector, trx);
    }
    else if (tableName == TableServiceBaggage::name())
    {
      DaoTable::printTableDiagnostic<TableServiceBaggage>(diagCollector, trx);
    }
    else if (tableName == TableSectorDetail::name())
    {
      DaoTable::printTableDiagnostic<TableSectorDetail>(diagCollector, trx);
    }
    else if (tableName == TableTaxRulesRecord::name())
    {
      DaoTable::printTableDiagnostic<TableTaxRulesRecord>(diagCollector, trx);
    }
    else if (tableName == TableTaxReportingRecord::name())
    {
      DaoTable::printTableDiagnostic<TableTaxReportingRecord>(diagCollector, trx);
    }
    else if (tableName == TableTaxCodeText::name())
    {
      DaoTable::printTableDiagnostic<TableTaxCodeText>(diagCollector, trx);
    }
    else
    {
      diagCollector << "There is no diagnostic for table = " << tableName;
    }
  }
  diagCollector.flushMsg();
}
} // tse
