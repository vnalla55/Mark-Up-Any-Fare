//----------------------------------------------------------------------------
//
//        File: FCException.h
// Description: exceptions and error messages
//		used by FC commands
//
//     Created: 15/11/07
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

#include "Common/Logger.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"

#include <stdexcept>

namespace fc_exceptions
{
// error messages, logged to greenscreen
static std::string INVALID_CURRENCY_STRING = "INVALID CURRENCY";
static std::string DATA_NOT_ON_FILE_STRING = "DATA NOT ON FILE";

static std::string PROCESSING_ERROR_STRING = "PROCESSING ERROR";
static std::string UNKNOWN_ERROR_STRING = "UNKNOWN ERROR";
static std::string UNKNOWN_REQUEST_STRING = "UNKNOWN REQUEST";

static std::string INVALID_DECIMALS_STRING = "INVALID DECIMALS";

static std::string REFER_TO_FCHELP = " - REFER TO FCHELP";

static std::string BEYOND_MAX_HIST_DATE = "BEYOND MAXIMUM HISTORICAL DATE - ";
static std::string FIELD_ERROR_STRING = "INVALID FIELD LIMIT";

static std::string FORMAT_ERROR_STRING = "FORMAT";

class RecordDoesntExist : public std::runtime_error
{
public:
  virtual ~RecordDoesntExist() throw() {}
  RecordDoesntExist(const std::string& msg,
                    const tse::CurrencyCode& curr,
                    log4cxx::LoggerPtr& logger)
    : std::runtime_error(msg), currency(curr)
  {
    LOG4CXX_ERROR(logger, msg);
  }
  tse::CurrencyCode currency;
};

class NoNucRecord : public RecordDoesntExist
{
public:
  NoNucRecord(const std::string& msg, const tse::CurrencyCode& curr, log4cxx::LoggerPtr& logger)
    : RecordDoesntExist(msg, curr, logger)
  {
  }
};

class NoCurrencyRecord : public RecordDoesntExist
{
public:
  NoCurrencyRecord(const std::string& msg,
                   const tse::CurrencyCode& curr,
                   log4cxx::LoggerPtr& logger)
    : RecordDoesntExist(msg, curr, logger)
  {
  }
};

// error that should be logged to greenscreen
class OtherUserError : public std::runtime_error
{
public:
  OtherUserError(const std::string& msg, log4cxx::LoggerPtr& logger) : std::runtime_error(msg)
  {
    LOG4CXX_ERROR(logger, msg);
  }
};

class FormatError : public OtherUserError
{
public:
  FormatError(log4cxx::LoggerPtr& logger) : OtherUserError(FORMAT_ERROR_STRING, logger) {}
};

class WrongDateEntered : public OtherUserError
{
public:
  WrongDateEntered(const std::string& msg, log4cxx::LoggerPtr& logger) : OtherUserError(msg, logger)
  {
  }
};

class WrongConvertedValue : public OtherUserError
{
public:
  WrongConvertedValue(const std::string& msg, log4cxx::LoggerPtr& logger)
    : OtherUserError(msg, logger)
  {
  }
};

class WrongDecimalsForCurrency : public OtherUserError
{
public:
  WrongDecimalsForCurrency(const std::string& msg, log4cxx::LoggerPtr& logger)
    : OtherUserError(msg, logger)
  {
  }
};
}
