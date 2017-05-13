// -------------------------------------------------------------------
//
//  Copyright (C) Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
// -------------------------------------------------------------------

#include <Common/ProgramOptionsParser.h>

#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>
#include <boost/range/algorithm/equal.hpp>

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <gtest/gtest.h>

#include <test/include/GtestHelperMacros.h>


namespace tse
{

struct ChangableParams
{
  ChangableParams();
  void setWorkingDirectory();
  void readReadyFileFromEnvironmentVariable();

  // Name of sentinel file to be written when the server is ready to take requests
  std::string readyFile;

  // Server port, to override the configured value
  uint16_t ipport;

  // Application Console port, to override the configured value
  uint16_t acport;

  // BigIP device
  std::string bigip;

  // BigIP pool name
  std::string poolname;

  // System type
  std::string systype;

  // Connection limit, to override the configured value
  uint16_t connlimit;

  // Connection limit basis, to override the configured value
  std::string connlimitBasis;

  // Application Console group name, to override the configured value
  std::string groupname;

  // Save the current working directory since the daemon might change it later
  std::string workingDir;

  bool isDaemon;

  std::string config;
  std::vector<std::string> defines;
};

class ProgramOptionsParserTest: public ::testing::Test
{
public:
  ProgramOptionsParserTest()
    : programParser(outputStreamForTests, false, "")
  {
  }

  void readArguments(int argc, char* argv[])
  {
    programParser.parseArguments(argc, argv);
    programParser.readVariablesFromArguments(params.isDaemon,
                                             params.config,
                                             params.defines,
                                             params.readyFile,
                                             params.ipport,
                                             params.bigip,
                                             params.poolname,
                                             params.systype,
                                             params.acport,
                                             params.connlimit,
                                             params.connlimitBasis,
                                             params.groupname);
  }

  void assertCompare(const ChangableParams& expectedParams)
  {
    ASSERT_EQ(expectedParams.isDaemon, params.isDaemon);
    ASSERT_EQ(expectedParams.config, params.config);
    ASSERT_EQ(expectedParams.readyFile, params.readyFile);
    ASSERT_EQ(expectedParams.ipport, params.ipport);
    ASSERT_EQ(expectedParams.bigip, params.bigip);
    ASSERT_EQ(expectedParams.poolname, params.poolname);
    ASSERT_EQ(expectedParams.systype, params.systype);
    ASSERT_EQ(expectedParams.acport, params.acport);
    ASSERT_EQ(expectedParams.connlimit, params.connlimit);
    ASSERT_EQ(expectedParams.connlimitBasis, params.connlimitBasis);
    ASSERT_EQ(expectedParams.groupname, params.groupname);

    ASSERT_EQ(expectedParams.defines.size(), params.defines.size());
    ASSERT_TRUE(boost::range::equal(expectedParams.defines, params.defines));
  }

  void checkIfStreamContainTheText(const char* text)
  {
    const std::string& output = outputStreamForTests.str();
    ASSERT_FALSE(output.empty());

    bool textFound = (std::string::npos != output.find(text));
    ASSERT_TRUE(textFound);
  }

protected:
  std::ostringstream outputStreamForTests;

  ProgramOptionsParser programParser;
  ChangableParams params;
};

TEST_F(ProgramOptionsParserTest, emptyArgumentList)
{
  const ChangableParams oryginalParams;

  const int argc = 1;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");

  readArguments(argc, argv);

  assertCompare(oryginalParams);
  ASSERT_TRUE(outputStreamForTests.str().empty());
}

TEST_F(ProgramOptionsParserTest, checkHelp)
{
  ChangableParams expectedParams;

  const int argc = 2;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>("-h");

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("help message");
}

TEST_F(ProgramOptionsParserTest, checkHelpLongSingleDashSyntax)
{
  ChangableParams expectedParams;

  const int argc = 2;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>("-help");

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("help message");
}

TEST_F(ProgramOptionsParserTest, isDeamonChangedShortSyntax)
{
  ChangableParams expectedParams;
  expectedParams.isDaemon = true;

  const int argc = 2;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>("-d");

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("Daemon mode");
}

TEST_F(ProgramOptionsParserTest, isDeamonChangedLongSyntax)
{
  ChangableParams expectedParams;
  expectedParams.isDaemon = true;

  const int argc = 2;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>("--daemon");

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("Daemon mode");
}

TEST_F(ProgramOptionsParserTest, isDeamonChangedLongSingleDashSyntax)
{
  ChangableParams expectedParams;
  expectedParams.isDaemon = true;

  const int argc = 2;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>("-daemon");

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("Daemon mode");
}

TEST_F(ProgramOptionsParserTest, acportDeliveredInArgumentsList)
{
  ChangableParams expectedParams;
  const char* acport = "5001";
  expectedParams.acport = boost::lexical_cast<uint16_t>(acport);

  const int argc = 3;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>("--acport");
  argv[2] = const_cast<char*>(acport);

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("APPLICATION_CONSOLE.PORT");
}

TEST_F(ProgramOptionsParserTest, multipleDefinesDeliveredInArgumentsList)
{
  const char defines[][40] = { "group.name1=value2", "group.name2=value2", "group.name5=30",
                               "grup.s=7", "TO_MAN.IOR_FILE=tseserver.shopping.ior" };
  const unsigned NUMBER_OF_DEFINES = sizeof(defines) / sizeof(*defines);

  ChangableParams expectedParams;
  expectedParams.defines.insert(expectedParams.defines.end(), defines, defines+NUMBER_OF_DEFINES);

  const int argc = 1 + NUMBER_OF_DEFINES*2;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");

  for(unsigned u=1; u<argc; u+=2)
  {
    argv[u]   = const_cast<char*>("-D");
    argv[u+1] = const_cast<char*>(defines[u/2]);
  }

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("Define");
}

TEST_F(ProgramOptionsParserTest, manyNotOrderedArgumentsInArgumentsList)
{
  const char* ipport = "53601";

  ChangableParams expectedParams;
  expectedParams.bigip   = "unspecified-device";
  expectedParams.config  = "./tseserver.acms.cfg";
  expectedParams.ipport  = boost::lexical_cast<uint16_t>(ipport);
  expectedParams.systype = "TestHybrid";
  expectedParams.groupname = "Shopping-Sandbox";

  const std::string& appconsolegroup_argument = "--appconsolegroup=" + expectedParams.groupname;

  const int argc = 10;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>("--systype");
  argv[2] = const_cast<char*>(expectedParams.systype.c_str());
  argv[3] = const_cast<char*>("-c");
  argv[4] = const_cast<char*>(expectedParams.config.c_str());
  argv[5] = const_cast<char*>("--ipport");
  argv[6] = const_cast<char*>(ipport);
  argv[7] = const_cast<char*>("--bigip");
  argv[8] = const_cast<char*>(expectedParams.bigip.c_str());
  argv[9] = const_cast<char*>(appconsolegroup_argument.c_str());

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("SERVER_SOCKET_ADP.BIGIP");
  checkIfStreamContainTheText("Config file");
  checkIfStreamContainTheText("SERVER_SOCKET_ADP.PORT");
  checkIfStreamContainTheText("SERVER_SOCKET_ADP.SYSTYPE");
  checkIfStreamContainTheText("APPLICATION_CONSOLE.GROUPNAME");
}

TEST_F(ProgramOptionsParserTest, multipleWaysOfDeliveringArgumentsInArgumentList)
{
  ChangableParams expectedParams;
  expectedParams.bigip          = "unspecified-device";
  expectedParams.config         = "./tseserver.acms.cfg";
  expectedParams.connlimitBasis = "member";
  expectedParams.readyFile      = "./.SHOPPING.1.ready";

  const std::string& connlimitBasis_argument = "--connlimitBasis=" + expectedParams.connlimitBasis;
  const std::string& readyFile_argument      = "-r" + expectedParams.readyFile;

  const int argc = 7;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>("--bigip");
  argv[2] = const_cast<char*>(expectedParams.bigip.c_str());
  argv[3] = const_cast<char*>("-c");
  argv[4] = const_cast<char*>(expectedParams.config.c_str());
  argv[5] = const_cast<char*>(connlimitBasis_argument.c_str());
  argv[6] = const_cast<char*>(readyFile_argument.c_str());

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("SERVER_SOCKET_ADP.BIGIP");
  checkIfStreamContainTheText("Config file");
  checkIfStreamContainTheText("SERVER_SOCKET_ADP.CONNLIMIT_BASIS");
  checkIfStreamContainTheText("Ready file");
}

TEST_F(ProgramOptionsParserTest, wrongArgumentsDeliveredInArgumentList)
{
  ChangableParams expectedParams;
  expectedParams.config         = "./tseserver.acms.cfg";
  expectedParams.readyFile      = "./.SHOPPING.1.ready";

  std::string notAcceptableArgument1("--baseline=atsev2.2015.01.ag");
  std::string notAcceptableArgument2_name("--appconsoleport");
  std::string notAcceptableArgument2_value("5001");

  const int argc = 8;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>("--config");
  argv[2] = const_cast<char*>(expectedParams.config.c_str());
  argv[3] = const_cast<char*>(notAcceptableArgument1.c_str());
  argv[4] = const_cast<char*>("--readyfile");
  argv[5] = const_cast<char*>(expectedParams.readyFile.c_str());
  argv[6] = const_cast<char*>(notAcceptableArgument2_name.c_str());
  argv[7] = const_cast<char*>(notAcceptableArgument2_value.c_str());

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("Config file");
  checkIfStreamContainTheText("Ready file");
}

TEST_F(ProgramOptionsParserTest, lifeArguments)
{
  using std::string;
  using boost::lexical_cast;

  const string argumentDefine("TO_MAN.IOR_FILE=tseserver.shopping.ior");

  ChangableParams expectedParams;
  expectedParams.config = "./tseserver.acms.cfg";
  expectedParams.readyFile = "./.SHOPPING.1.ready";
  expectedParams.connlimit = 70;
  expectedParams.ipport = 53601;
  expectedParams.bigip = "unspecified-device";
  expectedParams.poolname = "shopmip.daily_pool%d3pindislb11.sabre.com";
  expectedParams.systype = "TestHybrid";
  expectedParams.acport = 500;
  expectedParams.connlimitBasis = "member";
  expectedParams.defines.push_back(argumentDefine);
  expectedParams.groupname = "ShoppingMIP-Daily";

  const string& argumentConfig          = "-c " + expectedParams.config;
  const string& argumentReadyFile       = "-readyfile=" + expectedParams.readyFile;
  const string& argumentConnlimit       = "-connlimit="
                                           + lexical_cast<string>(expectedParams.connlimit);
  const string& argumentIpport          = "-ipport=" + lexical_cast<string>(expectedParams.ipport);
  const string& argumentBigip           = "-bigip=" + expectedParams.bigip;
  const string& argumentPoolname        = "-poolname=" + expectedParams.poolname;
  const string& argumentSystype         = "-systype=" + expectedParams.systype;
  const string& argumentAcport          = "-acport=" + lexical_cast<string>(expectedParams.acport);
  const string& argumentConnlimitBasis  = "-connlimitBasis=" + expectedParams.connlimitBasis;
  const string& argumentDefines         = "-D " + argumentDefine;
  const string& argumentAppconsolegroup = "-appconsolegroup=" + expectedParams.groupname;

  const int argc = 19;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>("-baseline=atsev2.2015.01.ag");
  argv[2] = const_cast<char*>("-adminapp=shopping");
  argv[3] = const_cast<char*>("-adminservice=SHOPPING");
  argv[4] = const_cast<char*>("-admininstance=1");
  argv[5] = const_cast<char*>(argumentAppconsolegroup.c_str());
  argv[6] = const_cast<char*>(argumentReadyFile.c_str());
  argv[7] = const_cast<char*>("-appconsoleport=5001");
  argv[8] = const_cast<char*>("-appconsoleportfile=./.SHOPPING.1.appconsoleport");
  argv[9] = const_cast<char*>(argumentIpport.c_str());
  argv[10] = const_cast<char*>("-ipportfile=./.SHOPPING.1.ipport");
  argv[11] = const_cast<char*>(argumentBigip.c_str());
  argv[12] = const_cast<char*>(argumentPoolname.c_str());
  argv[13] = const_cast<char*>(argumentSystype.c_str());
  argv[14] = const_cast<char*>(argumentConnlimit.c_str());
  argv[15] = const_cast<char*>(argumentConnlimitBasis.c_str());
  argv[16] = const_cast<char*>(argumentDefines.c_str());
  argv[17] = const_cast<char*>(argumentConfig.c_str());
  argv[18] = const_cast<char*>(argumentAcport.c_str());

  readArguments(argc, argv);

  assertCompare(expectedParams);
}

TEST_F(ProgramOptionsParserTest, multipleDefines)
{
  ChangableParams expectedParams;
  const char* acport = "5001";
  expectedParams.acport = boost::lexical_cast<uint16_t>(acport);

  expectedParams.defines.push_back("TO_MAN.IOR_FILE=tseserver.shoppingis.ior");
  expectedParams.defines.push_back("TSE_SERVER.OVERRIDE_CFGS=tseserver.cfg.user");

  expectedParams.config = "./tseserver.acms.cfg";

  std::vector<std::string> argumentsDefines(expectedParams.defines.size());
  for(unsigned i=0; i<expectedParams.defines.size(); ++i)
    argumentsDefines[i] = "-D " + expectedParams.defines[i];

  const std::string& argumentConfig = "-c " + expectedParams.config;
  std::string argumentAcport = "--acport=";
  argumentAcport += acport;

  const int argc = 5;
  char* argv[argc];
  argv[0] = const_cast<char*>("./server.sh");
  argv[1] = const_cast<char*>(argumentsDefines[0].c_str());
  argv[2] = const_cast<char*>(argumentConfig.c_str());
  argv[3] = const_cast<char*>(argumentsDefines[1].c_str());
  argv[4] = const_cast<char*>(argumentAcport.c_str());

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("APPLICATION_CONSOLE.PORT");
  checkIfStreamContainTheText("Config file");
  checkIfStreamContainTheText("Define");
}

TEST_F(ProgramOptionsParserTest, someArgumentsMultipleTimes)
{
  ChangableParams expectedParams;
  const char* acport = "5001";
  expectedParams.acport = boost::lexical_cast<uint16_t>(acport);

  expectedParams.defines.push_back("TO_MAN.IOR_FILE=tseserver.shoppingis.ior");
  expectedParams.defines.push_back("TSE_SERVER.OVERRIDE_CFGS=tseserver.cfg.user");
  expectedParams.defines.push_back("CRICITAL_DEFINE=inavailable");

  expectedParams.config = "./tseserver.acms.cfg";

  std::vector<std::string> argumentsDefines(expectedParams.defines.size());
  for(unsigned i=0; i<expectedParams.defines.size(); ++i)
    argumentsDefines[i] = "-D " + expectedParams.defines[i];

  const std::string& argumentConfig = "-c " + expectedParams.config;
  const std::string& argumentAcport = std::string("--acport=") + acport;

  const int argc = 14;
  char* argv[argc];
  argv[0]  = const_cast<char*>("./server.sh");
  argv[1]  = const_cast<char*>("-a 1111");
  argv[2]  = const_cast<char*>("--acport=8888");
  argv[3]  = const_cast<char*>("-acport=13");
  argv[4]  = const_cast<char*>("-a 2341");
  argv[5]  = const_cast<char*>("-c configfile.cfg");
  argv[6]  = const_cast<char*>("-config=justfile.txt");
  argv[7]  = const_cast<char*>(argumentsDefines[0].c_str());
  argv[8]  = const_cast<char*>("-a 204");
  argv[9]  = const_cast<char*>(argumentsDefines[1].c_str());
  argv[10] = const_cast<char*>("--config=./tobehonest_notconfigfile.not.conf");
  argv[11] = const_cast<char*>(argumentAcport.c_str());
  argv[12] = const_cast<char*>(argumentConfig.c_str());
  argv[13] = const_cast<char*>(argumentsDefines[2].c_str());

  readArguments(argc, argv);

  assertCompare(expectedParams);
  checkIfStreamContainTheText("APPLICATION_CONSOLE.PORT");
  checkIfStreamContainTheText("Config file");
  checkIfStreamContainTheText("Define");

}

/*************************************************************************************************/
ChangableParams::ChangableParams()
{
  ipport = 0;
  acport = 0;
  connlimit = 0;
  isDaemon = false;

  std::string config = "tseserver.acms.cfg";

  setWorkingDirectory();
  readReadyFileFromEnvironmentVariable();
}

void ChangableParams::setWorkingDirectory()
{
  using namespace boost::filesystem;
  try
  {
    path currentPath = current_path();
    workingDir = currentPath.string();
  }
  catch(const filesystem_error& e)
  {
    std::cerr << "Unable to get the current working directory, because: " << e.what() << std::endl;
    exit(EXIT_FAILURE);
  }
  catch(...)
  {
    std::cerr << "Unable to get the current working directory\n";
    exit(EXIT_FAILURE);
  }
}

void ChangableParams::readReadyFileFromEnvironmentVariable()
{
  // TODO: Ask if it should be as command line argument?
  // Get the readyfile from the environment (may be overridden later
  // by command line arguments)
  char* envString = getenv("ATSE_READYFILE");
  if (NULL != envString)
    readyFile = envString;
}

} //namespace tse
