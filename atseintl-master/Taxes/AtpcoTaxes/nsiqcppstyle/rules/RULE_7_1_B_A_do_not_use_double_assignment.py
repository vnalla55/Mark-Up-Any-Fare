"""
Do not use double assignment.

if it's shown... this rule reports a violation.

== Violation ==

    k = t = 1;      <== Violation. double assingments are used.
    void a() {
        b = c = 2;  <== Violation. double assingments are used. 
    }
    
== Good ==

    k = 1;  <== OK
    t = 1; 
    void a() {
        b = 2;
        c = 2;   
    }

"""
from nsiqcppstyle_rulehelper import  *
from nsiqcppstyle_reporter import *
from nsiqcppstyle_rulemanager import *

def RunRule(lexer, line, lineno) :
    if (Search(r"[a-zA-Z0-9]+\s*=\s*[a-zA-Z0-9]+\s*=", line)) :
        nsiqcppstyle_reporter.Error(DummyToken(lexer.filename, line, lineno, 0), __name__, "Do not use double assignment in a same line")
        
ruleManager.AddLineRule(RunRule)

###########################################################################################
# Unit Test
###########################################################################################

from nsiqunittest.nsiqcppstyle_unittestbase import *
class testRule(nct):
    def setUpRule(self):
        ruleManager.AddLineRule(RunRule)   
    def test1(self):
        self.Analyze("thisfile.c","""
void Hello() {
   int k = a = 2;
}
""")
        assert CheckErrorContent(__name__)
    def test2(self):
        self.Analyze("thisfile.c","""
int k = c = 2;
void Hello() {
}
""")
        assert CheckErrorContent(__name__)
    def test3(self):
        self.Analyze("thisfile.c","""
int k = 2; int j = 2;
void Hello() {
}
""")
        assert not CheckErrorContent(__name__)
    