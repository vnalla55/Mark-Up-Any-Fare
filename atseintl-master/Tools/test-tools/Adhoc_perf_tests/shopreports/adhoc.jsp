<html>
<head>
<title>Shopping Adhoc Performance Test Reports</title>
<link rel="stylesheet" href="css/v2.css" type="text/css">
</head>

  <body bgcolor=white>
<br/>

<%@ page import="java.io.BufferedReader" %>
<%@ page import="java.io.File" %>
<%@ page import="java.io.FilenameFilter" %>
<%@ page import="java.io.FileReader" %>
<%@ page import="java.util.Arrays" %>
<%@ page import="java.util.Comparator" %>
<%!
class FileCompareByModifiedDate implements Comparator<File> {
  public int compare(File f1, File f2) {
    Long lm1 = f1.lastModified();
    Long lm2 = f2.lastModified();
    return lm2.compareTo(lm1);
  }
}

public File[] getFilesList(HttpServletRequest request) {
  String path = new File(request.getRealPath(request.getServletPath())).getParentFile().getAbsolutePath() + "/adhoc";
  File folder = new File(path);
  File[] testsList = folder.listFiles();
  Arrays.sort(testsList, new FileCompareByModifiedDate());
  return testsList;
}

public String[] getTitles(File[] testDirs) {
  String[] titles = new String[testDirs.length];
  for(int i = 0; i < testDirs.length; ++i) {
    titles[i] = "junk";
    if(!testDirs[i].isDirectory()) {
      continue;
    }
    File[] files = testDirs[i].listFiles(
      new FilenameFilter() {
        public boolean accept(File path, String name) {
          if( name != null ) {
            return name.endsWith(".html");
          }
          return false;
        }
      });
    if(files.length != 1) {
      titles[i] = "Something Is Wrong";
    }
    else {
      try {
        BufferedReader br = new BufferedReader(new FileReader(files[0]));
        String line;
        while ((line = br.readLine()) != null) {
          int tstart = line.indexOf("<title>");
          if( tstart > 0 ) {
            tstart += 7;
            int tend = line.indexOf("</title>");
            titles[i] = line.substring(tstart, tend);
            break;
          }
        }
        br.close();
      }
      catch(java.io.IOException exc) {
        titles[i] = "IO Exception";
      }
    }
  }
  return titles;
}
%>
<%
File[] listOfFiles = getFilesList(request);
String[] titles = getTitles(listOfFiles);
%>
<font face="arial" size="+1">
<table id=listTable slcolor='#BEC5DE' hlcolor='#BEC5DE' cellspacing="1" cellpadding="5" width="100%" bgcolor="gray" border="1">
<tr class="tableHeading">
<td>Adhoc Performance Test Title</td>
</tr>
<%
int j = 0;
for(int i = 0; i < listOfFiles.length; ++i) {
  if(listOfFiles[i].isDirectory()) {
    if( j % 2 == 0) {
%>
<tr class='evenTableRow'><td>
<%} else {%>
<tr class='oddTableRow'><td>
<%}%>
<%
String href = "adhoc/" + listOfFiles[i].getName() + "/" + listOfFiles[i].getName() + ".html";
out.print("<a href=\"" + href + "\">");
%>
<%= titles[i] %>
</td>
</tr>
<%
out.print("</a>");
  j++;
  }
}
%>
</table>
</font>
</body>
</html> 
