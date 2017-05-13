while (<>) 
{
  if ($_ =~ /DB_SOURCES/) 
  {
    $end = 0;
    while (<>) 
    {
      if ($_ !~ /\\$/) 
      {
        $end = 1;
      }
      s/^\s*//;
      s/\s*\\$//;
      if($_ !~ /DataHandle.cpp/)
      {
        print;
      }
      if ($end==1) 
      {
        exit (0) ;
      }
    }
  }
}
