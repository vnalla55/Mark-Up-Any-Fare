This is backup scripts using to adhoc performance tests.

In runsoak directory are store scripts to make tests. Description how to use it is available here: http://atsei.dev.sabre.com/ATSEIWiki/Wiki.jsp?page=SOPerformanceTesting. Runsoak use v2 ACMS scripts to compare configs from tested nodes. Them are stored in runsoak/Config. In runsoak.pl are hardcoded path to files, so if you want to change place from you run script remember to set paths again.

In shopreports are store backup .jsp, .html and reports files. This files should be placed in http serwer public directory.

runsoak require config file. Here is example what config should contain:

ID=main.07.at.vs.mip_opt.06.bb.2nd.Try
Title=MIP Optimization CPU Comparison - Main 07.at Vs MipOpt 06.bb 2nd Try
ControlServer=picli309
TestServer=picli310
Port=53601
AppConsolePort=5001
ControlBaseline=atsev2.2012.07.at
TestBaseline=atsev2.2012.06.bb_mip-opt
MaxDuration=4200
InstrDbServer=dbshli-scc11.sabre.com
Filters=tst