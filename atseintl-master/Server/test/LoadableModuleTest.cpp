#include "test/include/CppUnitHelperMacros.h"

#include "Server/LoadableModule.h"
#include "Server/LoadableModuleMethods.h"

#include "Server/test/MockModule.h"

using namespace tse;

class LocalModule : public Module
{
public:
  LocalModule(const std::string& name, TseServer& srv) : Module(name, srv) {}
  virtual ~LocalModule() {}

  virtual bool initialize(int, char**) { return true; }
  virtual void postInitialize() {}
  virtual void preShutdown() {}
};

static LoadableModuleRegister<Module, LocalModule>
_("libLocalModule.so");

class LoadableModuleTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE(LoadableModuleTest);
  CPPUNIT_TEST(testPositive);
  CPPUNIT_TEST(testNegative);
  CPPUNIT_TEST(testLocal);
  CPPUNIT_TEST(testCopy);
  CPPUNIT_TEST_SUITE_END();

public:
  void testPositive()
  {
    TseServer* srv = 0;
    const int nargs = 1;
    const char* args[nargs + 1] = { "--argument" };
    bool result;

    LoadableModule<Module> module;

    result = module.load("libMockModule.so", *srv, "Mock", nargs, (char**)args);
    MockModule* mockModule = static_cast<MockModule*>(module.instance());

    CPPUNIT_ASSERT(result == true);
    CPPUNIT_ASSERT(module.instance() != 0);
    CPPUNIT_ASSERT(mockModule != 0);
    CPPUNIT_ASSERT_EQUAL(std::string("Mock"), module.instance()->getName());
    CPPUNIT_ASSERT(!&module.instance()->getServer());
    CPPUNIT_ASSERT_EQUAL((size_t)1, mockModule->args().size());
    CPPUNIT_ASSERT_EQUAL(std::string("--argument"), mockModule->args()[0]);

    result = module.postLoad("libMockModule.so", *srv, "Mock");
    CPPUNIT_ASSERT(result == true);
    CPPUNIT_ASSERT(mockModule->isPostInitialized());

    module.preUnload();
    CPPUNIT_ASSERT(mockModule->isPreShutdown());

    module.unload();
    CPPUNIT_ASSERT(!module.instance());
  }

  void testNegative()
  {
    TseServer* srv = 0;
    LoadableModule<Module> module;

    bool result = module.load("libNonExistent.so", *srv, "Mock", 0, 0);

    CPPUNIT_ASSERT(result == false);
    CPPUNIT_ASSERT(module.instance() == 0);
  }

  void testLocal()
  {
    TseServer* srv = 0;
    LoadableModule<Module> module;

    bool result = module.load("libLocalModule.so", *srv, "Local", 0, 0);

    LocalModule* localModule = dynamic_cast<LocalModule*>(module.instance());
    CPPUNIT_ASSERT(result == true);
    CPPUNIT_ASSERT(module.instance() != 0);
    CPPUNIT_ASSERT(localModule != 0);
  }

  void testCopy()
  {
    TseServer* srv = 0;
    LoadableModule<Module> module1, module3;

    CPPUNIT_ASSERT(!module3.instance());

    module1.load("libLocalModule.so", *srv, "Local", 0, 0);
    LoadableModule<Module> module2(module1);
    module3 = module2;

    CPPUNIT_ASSERT_EQUAL(module1.instance(), module2.instance());
    CPPUNIT_ASSERT_EQUAL(module1.instance(), module3.instance());
  }
};

CPPUNIT_TEST_SUITE_REGISTRATION(LoadableModuleTest);
