/* Copyright TeraRecon, 2006. Authors Gang Li, Srinivasan Viswanathan */
USE PxDcmDB

IF NOT EXISTS (select * from PxDcmDB.dbo.dbinfo  
	Where  MajorVersion=1 and MinorVersion=7 and BuildVersion=2 and BuildMinorVersion=0 )
begin	
	-- quit script
	RAISERROR ('Not a 1.7.2.0. database, bypass patching', 20, 1) with log
	return 
end

print 'start to do 1.7.2.0 patch'

print 'patch InstanceLevel to add ScanOptions column in PxDcmDB'
ALTER TABLE InstanceLevel ADD ScanOptions VARCHAR(16)

print 'patch SeriesLevel to add Manufacturer column in PxDcmDB'
ALTER TABLE SeriesLevel ADD Manufacturer VARCHAR(32) NULL



-- verify and update DB to patched version
USE PxDcmDB
if	( 
	EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'ScanOptions' AND Table_Name ='InstanceLevel') 
	AND EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'Manufacturer' AND Table_Name ='SeriesLevel') 
	)

	begin
	Update PxDcmDB.dbo.dbinfo Set MajorVersion=1, MinorVersion=8, BuildVersion=0,BuildMinorVersion=0 
	print 'success on patching database version from 1.7.2.0 to 1.8.0.0'
	end

else
	begin
	RAISERROR ('failed on verify, abort to set version to 1.8.0.0', 20, 1) with log	
	end
