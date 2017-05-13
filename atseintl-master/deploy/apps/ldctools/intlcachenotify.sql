DELETE FROM INTLCACHENOTIFY
WHERE APPLICATIONNAME = 'JWTEST'
;
INSERT INTO INTLCACHENOTIFY
   ( priority,createdate,entitytype,applicationname,keystring )
VALUES
   ( 0, str2jts('==date==.000000'), '===cache==' , 'LDCTEST', 'C|==key==\n^'     )
;
