//----------------------------------------------------------------------------
//  Copyright Sabre 2011
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#ifndef TEST_LOGGER_H
#define TEST_LOGGER_H

#include <log4cxx/helpers/objectptr.h>
#include <log4cxx/consoleappender.h>
#include <log4cxx/writerappender.h>
#include <log4cxx/simplelayout.h>
#include <log4cxx/helpers/bytearrayoutputstream.h>
#include <log4cxx/helpers/outputstreamwriter.h>

class LoggerLevelSetter
{
public:
  explicit LoggerLevelSetter(const char* name, log4cxx::LevelPtr level)
    : _logger(log4cxx::Logger::getLogger(name)), _prevLevel(_logger->getLevel())
  {
    _logger->setLevel(level);
  }

  LoggerLevelSetter(log4cxx::LoggerPtr logger, log4cxx::LevelPtr level)
    : _logger(logger), _prevLevel(_logger->getLevel())
  {
    _logger->setLevel(level);
  }

  ~LoggerLevelSetter() { _logger->setLevel(_prevLevel); }

  log4cxx::LoggerPtr get() { return _logger; }

protected:
  log4cxx::LoggerPtr _logger;
  log4cxx::LevelPtr _prevLevel;
};

class LoggerGetOff : public LoggerLevelSetter
{
public:
  explicit LoggerGetOff(const char* name) : LoggerLevelSetter(name, log4cxx::Level::getOff()) {}
};

class RootLoggerGetOff : public LoggerLevelSetter
{
public:
  RootLoggerGetOff() : LoggerLevelSetter(log4cxx::Logger::getRootLogger(), log4cxx::Level::getOff())
  {
  }
};

class ConsoleLogger : protected LoggerLevelSetter
{
public:
  explicit ConsoleLogger(const char* name) : LoggerLevelSetter(name, log4cxx::Level::getAll())
  {
    _logger->removeAllAppenders();
    _logger->addAppender(new log4cxx::ConsoleAppender(new log4cxx::SimpleLayout));
  }

  ~ConsoleLogger() { _logger->removeAllAppenders(); }

  using LoggerLevelSetter::get;
};

class TestLogger : protected LoggerLevelSetter
{
public:
  explicit TestLogger(const char* name)
    : LoggerLevelSetter(name, log4cxx::Level::getAll()),
      _stream(new log4cxx::helpers::ByteArrayOutputStream)
  {
    init();
  }

  explicit TestLogger(log4cxx::LoggerPtr logger, log4cxx::LevelPtr level)
    : LoggerLevelSetter(logger, level), _stream(new log4cxx::helpers::ByteArrayOutputStream)
  {
    init();
  }

  ~TestLogger() { _logger->removeAllAppenders(); }

  using LoggerLevelSetter::get;

  std::string str()
  {
    std::vector<unsigned char> array = _stream->toByteArray();
    return std::string(array.begin(), array.end());
  }

protected:
  log4cxx::helpers::ByteArrayOutputStreamPtr _stream;

private:
  void init()
  {
    _logger->removeAllAppenders();

    log4cxx::helpers::OutputStreamPtr str(_stream);
    log4cxx::WriterAppenderPtr app = new log4cxx::WriterAppender;
    app->setWriter(new log4cxx::helpers::OutputStreamWriter(str));
    app->setLayout(new log4cxx::SimpleLayout);

    _logger->addAppender(app);
  }
};

#endif
