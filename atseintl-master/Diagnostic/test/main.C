#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/Test.h>
#include <cppunit/TestSuite.h>
#include <vector>

using namespace CppUnit;

int main(int argc, char** argv)
{

    TextUi::TestRunner runner;
    TestFactoryRegistry& registry = CppUnit::TestFactoryRegistry::getRegistry();
    Test* test = registry.makeTest();
    if (argc > 1 )
    {
        TestSuite* suite = dynamic_cast<TestSuite*>(test);
        if (suite)
        {
            std::vector<Test *> tests = suite->getTests();
            for (int i=0; i<tests.size(); i++)
            {
                runner.addTest(tests[i]);
            } 
            for (int i=1; i<argc; i++)
            {
                runner.run(argv[i], false, true, false);
            }
        }
    }
    else
    {
	runner.addTest(test);
        runner.run("", false, true, false);
    }
    return 0;
}
