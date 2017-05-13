<html>
<head>
<title>Shopping Continuous Performance Test Reports</title>
<link rel="stylesheet" href="css/v2.css" type="text/css">
</head>

<body bgcolor=white>
<br><center><h1>Shopping Continuous Performance Tests</h1></center>
<br>

<%@ page import="java.io.BufferedReader" %>
<%@ page import="java.io.File" %>
<%@ page import="java.io.FilenameFilter" %>
<%@ page import="java.io.FileReader" %>
<%@ page import="java.util.Arrays" %>
<%@ page import="java.util.Comparator" %>
<%!
class CPTRow {
  public CPTRow() {
  }
  public void setDate(String d) { this.date = d; }
  public String getDate() { return this.date; }
  public void setISURL(String url) { this.isURL = url; }
  public String getISURL() { return this.isURL; }
  public void setISTitle(String title) { this.isTitle = title; }
  public String getISTitle() { return this.isTitle; }
  public void setMIPURL(String url) { this.mipURL = url; }
  public String getMIPURL() { return this.mipURL; }
  public void setMIPTitle(String title) { this.mipTitle = title; }
  public String getMIPTitle() { return this.mipTitle; }

  private String date;
  private String isURL;
  private String isTitle;
  private String mipURL;
  private String mipTitle;
}

class FileCompareByModifiedDate implements Comparator<File> {
  public int compare(File f1, File f2) {
    Long lm1 = f1.lastModified();
    Long lm2 = f2.lastModified();
    return lm2.compareTo(lm1);
  }
}

public File[] getFilesList(HttpServletRequest request) {
  String path = new File(request.getRealPath(request.getServletPath())).getParentFile().getAbsolutePath() + "/cpt";
  File folder = new File(path);
  File[] testsList = folder.listFiles();
  Arrays.sort(testsList, new FileCompareByModifiedDate());
  return testsList;
}

public String getTitleSuffix(File dir) {
  StringBuffer buf = new StringBuffer();;
  File[] files = dir.listFiles(
    new FilenameFilter() {
      public boolean accept(File path, String name) {
        if( name != null ) {
          return name.endsWith(".html");
        }
        return false;
      }
    });
  if(files.length != 1) {
     buf.append("Something Is Wrong");
  }
  else {
    try {
      BufferedReader br = new BufferedReader(new FileReader(files[0]));
      String line;
      while ((line = br.readLine()) != null) {
        int tstart = line.indexOf("<td>atsev2");
        if( tstart >= 0 ) {
          tstart += 4;
          int tend = line.indexOf("</td>");
          if( buf.length() > 0 )
            buf.append(" Vs ");
          buf.append(line.substring(tstart, tend));
        }
      }
      br.close();
    }
    catch(java.io.IOException exc) {
      buf.append("IO Exception");
    }
  }
  return buf.toString();
}

public CPTRow buildCPTRow(File dateDir) {
  CPTRow cptRow = new CPTRow();
  cptRow.setDate(dateDir.getName());

  File[] dateFiles = dateDir.listFiles();
  for(int i = 0; i < dateFiles.length; ++i) {
    File f = dateFiles[i];
    if(!f.isDirectory()) {
      continue;
    }
    if( f.getName().endsWith("is") ) {
      cptRow.setISURL("cpt/" + dateDir.getName() + "/" + f.getName() + "/" + f.getName() + ".html");
      StringBuffer title = new StringBuffer(f.getName().startsWith("daily") ? " Daily" : " Weekly");
      title.append(" IS ");
      title.append(" - ").append( getTitleSuffix(f) );
      cptRow.setISTitle(title.toString());
    }
    else if( f.getName().endsWith("mip") ) {
      cptRow.setMIPURL("cpt/" + dateDir.getName() + "/" + f.getName() + "/" + f.getName() + ".html");
      StringBuffer title = new StringBuffer(f.getName().startsWith("daily") ? " Daily" : " Weekly");
      title.append(" MIP");
      title.append(" - ").append( getTitleSuffix(f) );
      cptRow.setMIPTitle(title.toString());
    }
  }
  return cptRow;
}

public CPTRow[] buildCPTRows(File[] testDirs) {
  CPTRow[] cptRows = new CPTRow[testDirs.length];
  for(int i = 0; i < testDirs.length; ++i) {
    cptRows[i] = null;
    if(!testDirs[i].isDirectory()) {
      continue;
    }
    cptRows[i] = buildCPTRow(testDirs[i]);
  }
  return cptRows;
}
%>
<%
File[] listOfFiles = getFilesList(request);
CPTRow[] cptRows = buildCPTRows(listOfFiles);
%>
<font face="arial" >
<table id=listTable slcolor='#BEC5DE' hlcolor='#BEC5DE' cellspacing="0" cellpadding="3" bgcolor="gray" width="100%" border="1">
<tr class="tableHeading">
<td width="10%">Date</td>
<td width="45%">IS</td>
<td width="45%">MIP</td>
</tr>
<%
int j = 0;
for(int i = 0; i < cptRows.length; ++i) {
  if(cptRows[i] != null) {
    if( j % 2 == 0) {
%>
<tr class='evenTableRow'>
<%} else {%>
<tr class='oddTableRow'>
<%}%>
<td><%=cptRows[i].getDate()%></td>
<td> <% out.print("<a href=\"" + cptRows[i].getISURL() + "\">"); %> <%= cptRows[i].getISTitle() %> </td>
<td> <% out.print("<a href=\"" + cptRows[i].getMIPURL() + "\">"); %> <%= cptRows[i].getMIPTitle() %> </td>
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
