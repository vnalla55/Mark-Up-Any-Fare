#pragma once

#include "Currency/FCCommand.h"

#include <vector>

namespace tse
{
class NUCInfo;
class CurrencyTrx;

namespace
{
const unsigned CURRENCY_COLUMN_WIDTH = 9;
const unsigned CARRIER_COLUMN_WIDTH = 4;
const unsigned NUCRATE_COLUMN_WIDTH = 18;
const unsigned EFFSALE_COLUMN_WIDTH = 14;
const unsigned LASTSALE_COLUMN_WIDTH = 13;

const char* const COLUMN_NAME_CURRENCY = "CURRENCY";
const char* const COLUMN_NAME_CARRIER = ""; // just a placeholder
const char* const COLUMN_NAME_NUCRATE = "       NUC RATE";
const char* const COLUMN_NAME_EFFSALE = "EFF SALE";
const char* const COLUMN_NAME_LASTSALE = "LAST SALE-TKT";

const CarrierCode EMPTY_CARRIER = "";
const CurrencyCode EMPTY_CURRENCY = "";
}

class FCDisplay : public FCCommand
{
public:
  FCDisplay(CurrencyTrx& _trx) : FCCommand(_trx) {}
  void process() override;

  enum DateRange
  { UNSPECIFIED_DATE_RANGE = 0,
    INVALID_DATE_RANGE,
    CURRENT,
    CURRENT_TO_INFINITY,
    FROM_DATE_TO_INFINITY };

  enum CurrenciesRange
  { UNSPECIFIED_CURRENCIES_RANGE = 0,
    INVALID_CURRENCIES_RANGE,
    DISPLAY_ALL,
    DISPLAY_SINGLE };

protected:
  virtual void show_response(const std::vector<NUCInfo*>& nucs);
  virtual void determine_ranges(DateRange& date_range, CurrenciesRange& currencies_range);
  virtual void show_nuc_list(const std::vector<NUCInfo*>& nucs);
  virtual void append_output_line(NUCInfo& nuc);
  virtual void show_column_names();
  virtual void sort_nuc_list(const std::vector<NUCInfo*>& vec_);

private:
  const std::vector<NUCInfo*>&
  get_nuc_info_for_multiple_currencies(const DateTime& start_date, const DateTime& end_date);
  const std::vector<NUCInfo*>& get_nuc_info(const DateTime& start_date,
                                            const DateTime& end_date,
                                            const CurrencyCode& currency,
                                            CurrenciesRange curr_range);
  const std::vector<NUCInfo*>& get_nuc_info_for_single_currency(const DateTime& start_date,
                                                                const DateTime& end_date,
                                                                const CurrencyCode& currency);

  std::vector<NUCInfo*> nuc_vector; // to hold transaction-time data
};
}
