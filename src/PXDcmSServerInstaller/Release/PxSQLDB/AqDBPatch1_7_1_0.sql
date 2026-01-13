/* Copyright TeraRecon, 2006. Authors Gang Li, Srinivasan Viswanathan */
USE PxDcmDB

IF NOT EXISTS (select * from PxDcmDB.dbo.dbinfo  
	Where  MajorVersion=1 and MinorVersion=7 and BuildVersion=1 and BuildMinorVersion=0 )
begin	
	-- quit script
	RAISERROR ('Not a 1.7.1.0. database, bypass patching', 20, 1) with log
	return 
end

print 'start to do 1.7.1.0 patch'

if not EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'ImageDate' AND Table_Name ='InstanceLevel') 
begin
	print 'patch InstanceLevel for adding ImageDate, ImageTime, WasLossyCompressed columns'
	ALTER TABLE InstanceLevel ADD ImageDate varchar(10) NULL
	ALTER TABLE InstanceLevel ADD ImageTime varchar(16) NULL
	ALTER TABLE InstanceLevel ADD WasLossyCompressed tinyint NULL -- find the possible values and create the allowable values contraint
end

-- delete PxDcmDB2 and PxDcmDB3 2010/03/15 By K.Ko

-- verify and update DB to patched version
USE PxDcmDB
if	( EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'ImageDate' AND Table_Name ='InstanceLevel') 
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'ImageTime' AND Table_Name ='InstanceLevel') 
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'WasLossyCompressed' AND Table_Name ='InstanceLevel')
	)

	begin
	Update PxDcmDB.dbo.dbinfo Set MajorVersion=1, MinorVersion=7, BuildVersion=2,BuildMinorVersion=0 
	print 'success on patching database version from 1.7.1.0 to 1.7.2.0'
	end

else
	begin
	RAISERROR ('failed on verify, abort to set version to 1.7.2.0', 20, 1) with log	
	end
