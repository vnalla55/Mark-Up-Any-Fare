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
#include <stdexcept>
#include <cppunit/extensions/HelperMacros.h>
#include <set>
#include <boost/iostreams/device/file.hpp>
#include <boost/iostreams/stream.hpp>
#include <istream>

#include "TestServer/Facades/TaxStringTestProcessor.h"
#include "TestServer/Server/TestServerRequestRecorder.h"

namespace io = boost::iostreams;
using namespace std;
using namespace tax;

namespace tax
{

class RequestValidator
{
public:
  std::string request;
  std::string response;
  std::string taxStringProcessorResponse;

  bool isValid()
  {
    TaxStringTestProcessor taxStringProcessor;
    taxStringProcessor.processString(request);
    taxStringProcessorResponse = TestServerRequestRecorder::removeBadChar(taxStringProcessor.getResponseMessage());

    return (response == taxStringProcessorResponse);
  }
  void logErrorInfo()
  {
    if (response.length() != taxStringProcessorResponse.length())
    {
      std::cout << "Difference in length: Expected length" << response.length()
                << " but current value is " << taxStringProcessorResponse.length() << std::endl;
    }
    size_t length = response.length();
    if (length > taxStringProcessorResponse.length())
      length = taxStringProcessorResponse.length();

    for (size_t i = 0; i < length; i++)
    {
      if (response[i] != taxStringProcessorResponse[i])
      {
        std::cout << "First Difference on Character:" << i << " Expected:" << response[i]
                  << " but value is:" << taxStringProcessorResponse[i] << std::endl;
        break;
      }
    }
    std::cout << "Expected:\n" << response << "\nReceived:\n" << taxStringProcessorResponse << "\nRequest:\n"
              << request << std::endl;
    ;
  }
};

class ComponentTests : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(ComponentTests);

  CPPUNIT_TEST(TaxRequestResponseMath);

  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
  }

  void tearDown()
  {
  }

  void TaxRequestResponseMath()
  {
    std::string filename("TestServer/Server/test/ComponentTests.xml");
    boost::iostreams::stream<boost::iostreams::file_source> file(filename.c_str());
    std::string line;
    // std::getline(file, line);
    bool finalResult = true;
    std::cout << std::endl;
    while (std::getline(file, line))
    {
      RequestValidator rv;
      std::getline(file, rv.request);
      std::getline(file, rv.response);
      bool result = rv.isValid();
      if (!result)
      {
        finalResult = false;
        std::cout << "Response ERROR:" << line << std::endl;
        rv.logErrorInfo();
      }
      else
      {
        std::cout << "Response OK:" << line << std::endl;
      }
    }

    file.close();
    std::cout << "Component Test Result";
    CPPUNIT_ASSERT(finalResult);
  }

private:
};

CPPUNIT_TEST_SUITE_REGISTRATION(ComponentTests);
} // namespace tax
