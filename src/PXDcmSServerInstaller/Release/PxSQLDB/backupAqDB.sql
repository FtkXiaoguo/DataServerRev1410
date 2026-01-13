USE master;
GO
 
IF EXISTS (select * from PxDcmDB.dbo.dbinfo) 
begin
	IF EXISTS (select * from PxDcmDB.dbo.InstanceView) 
	begin
	print ' --- do BACKUP DATABASE  PxDcmDB -- ' ;
	BACKUP DATABASE  PxDcmDB     TO DISK = 'C:\PxDmDBBack\PxDcmDB.dat' WITH INIT;
	end
	else
	print ' *** Warning: Cannot find Data for BACKUP DATABASE  PxDcmDB -- ' ;
end
else
print ' *** ERROR: Can not  BACKUP DATABASE  PxDcmDB -- ' ;
 

IF EXISTS (select * from PxDcmHistDB.dbo.Actions) 
begin
print ' --- do BACKUP DATABASE  PxDcmHistDB -- ' ;
BACKUP DATABASE  PxDcmHistDB TO DISK = 'C:\PxDmDBBack\PxDcmHistDB.dat' WITH INIT;
end
else
print ' *** ERROR: Can not  BACKUP DATABASE  PxDcmHistDB -- ' ;


