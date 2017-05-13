#!/bin/sh

JAVA_HOME=/opt/atseintl/3rdParty/bin/java
CLASSPATH=${CLASSPATH}:/opt/support/scripts/shopping/chartjars/chartmaker.jar
CLASSPATH=${CLASSPATH}:/opt/support/scripts/shopping/chartjars/jcommon-1.0.15.jar
CLASSPATH=${CLASSPATH}:/opt/support/scripts/shopping/chartjars/iText-2.1.3.jar
CLASSPATH=${CLASSPATH}:/opt/support/scripts/shopping/chartjars/jfreechart-1.0.12.jar
export CLASSPATH

${JAVA_HOME}/bin/java -cp ${CLASSPATH} com.sabre.cpt.ChartMaker $*
