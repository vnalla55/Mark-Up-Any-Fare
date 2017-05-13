//----------------------------------------------------------------------------
//
//        File: FCConvert.h
// Description: FCConvert class converts currencies
//		to NUC and NUC to currencies,
//		and adds formatted output to
//		the response
//		provides processing for entries:
//			FC$
//
//     Created: 10/11/07
//     Authors: Tomasz Karczewski
//
//  Copyright Sabre 2007
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

#include "Currency/FCCommand.h"
#include "DataModel/CurrencyTrx.h"

#include <string>

namespace tse
{
static const unsigned FCC_CURRENCY_CLMN_WIDTH = 4;
static const unsigned FCC_VALUE_CLMN_WIDTH = 13;
static const unsigned FCC_RNDRULE_CLMN_WIDTH = 27;

static const double NUC_PSEUDO_ROUNDING_FACTOR = 0.1;
static const CurrencyCode NUC_CODE = "NUC";

static const std::string TRUNCATED_INFO_STRING = "TRUNCATED";

enum ConversionType
{ NUC_TO_CURRENCY = 0,
  CURRENCY_TO_NUC,
  INVALID };

enum RoundingType
{ RT_INTERNATIONAL = 0,
  RT_DOMESTIC = 1,
  RT_OTHER_TAXES = 2 };

static std::string ROUNDING_TYPE_STRINGS[] = { "INTERNATIONAL FARES", "DOMESTIC FARES",
                                               "OTHER/TAXES" };

static std::string ROUNDING_STRINGS[] = { "ROUNDED UP TO NEXT", "ROUNDED TO NEAREST",
                                          "ROUNDED DOWN TO", "NO ROUNDING" };

class FCConvert : public FCCommand
{
public:
  FCConvert(CurrencyTrx& trx);
  void process() override;

protected:
  virtual double convert_to_currency(double nuc_rate, double quantity);
  virtual double convert_to_NUC(double nuc_rate, double quantity);
  virtual double
  apply_rounding(double value, double rounding_factor, RoundingRule rule, unsigned noDec = 0);
  virtual double
  get_truncated_for_rounding(double value, double rounding_factor, unsigned& tfr_nodec);
  virtual std::string format_rounding(const CurrencyCode& currency,
                                      double converted_value,
                                      double rounding_factor,
                                      RoundingRule rrule,
                                      unsigned curr_decimals,
                                      RoundingType type);
  virtual const std::string format_header(const NUCInfo& nuc_info, const DateTime& dte);
  virtual std::string format_currency_display(double value, unsigned curr_no_dec, int max_len = -1);
  virtual std::string
  format_truncated_line(const CurrencyCode& shown_code, double val, double rounding_factor);
  virtual void validate_data();

  virtual std::string get_additional_header_message();

private:
  void show_response();
  std::string get_rounding_rule_string(RoundingRule rule, double rounding_factor);
  const std::string& get_rounding_type_string(RoundingType type);
  void perform_conversion();
  std::string create_response();

  void get_conversion_data();

  void get_db_data(bool get_roundings);
  void get_nuc_rate();
  void get_nodec_and_roundings_for_currency(bool get_roundings);

  // transaction - time private data
  CurrencyCode currency;
  CurrencyNoDec currency_nodec;
  double sourceVal;
  double nucRate;
  NUCInfo* currency_nuc;
  double convertedVal;
  ConversionType conversionType;
  DateTime trxDate;

  RoundingRule itlRoundingRule;
  RoundingRule domRoundingRule;
  RoundingRule taxRoundingRule;

  double itlRoundingFactor;
  double domRoundingFactor;
  double taxRoundingFactor;
};

inline FCConvert::FCConvert(CurrencyTrx& trx)
  : FCCommand(trx),
    currency_nodec(0),
    sourceVal(-1),
    nucRate(-1),
    currency_nuc(nullptr),
    convertedVal(-1),
    conversionType(INVALID),
    itlRoundingRule(NONE),
    domRoundingRule(NONE),
    taxRoundingRule(NONE),
    itlRoundingFactor(-1),
    domRoundingFactor(-1),
    taxRoundingFactor(-1)
{
}
}
