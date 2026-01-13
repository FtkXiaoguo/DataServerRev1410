/* Copyright TeraRecon, 2003. Authors Gang Li */
USE PxDcmDB
GO

-- special patch for beta 1.7.1.0
IF EXISTS (select * from PxDcmDB.dbo.dbinfo  
	Where  MajorVersion=1 and MinorVersion=7 and BuildVersion=1 and BuildMinorVersion=0 )
begin	

	if not EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'PKey' AND Table_Name ='PrivateDataReference') 
	begin
		print 'patch PrivateDataReference for adding PRIMARY KEY'
		-- add primary key for remember sequence
		ALTER TABLE PrivateDataReference ADD PKey INT NOT NULL IDENTITY
		ALTER TABLE PrivateDataReference ADD CONSTRAINT PK_PrivateDataReference PRIMARY KEY (PKey ) 
	end

end

-- new tables or new initializations -------------

IF NOT EXISTS (select * from PxDcmDB.dbo.dbinfo  
	Where  MajorVersion=1 and MinorVersion=7 and BuildVersion=0 and BuildMinorVersion=0 )
begin	
	-- quit script
	RAISERROR ('Not a 1.7.0.0. database, bypass patching', 20, 1) with log

end
GO


print 'start to do 1.7.0.0 patch'

IF EXISTS (SELECT * FROM sysindexes WHERE name='AuxSOPUID_index')
	DROP INDEX PrivateData.AuxSOPUID_index

ALTER TABLE PrivateData ADD VolumesHash  varchar(64) DEFAULT ('') WITH VALUES
ALTER TABLE PrivateData ADD ParameterHash varchar(64) DEFAULT ('') WITH VALUES

GO

CREATE UNIQUE INDEX AuxSOPUID_index on PrivateData (AuxSOPUID, Type, Name, Subtype, ProcessName, ProcessType, VolumesHash, ParameterHash)
GO

IF EXISTS (SELECT * FROM sysindexes WHERE name='PrivateDataReference_Unique')
begin
	ALTER TABLE PrivateDataReference NOCHECK CONSTRAINT PrivateDataReference_Unique
	ALTER TABLE PrivateDataReference DROP CONSTRAINT PrivateDataReference_Unique
end

ALTER TABLE PrivateDataReference ADD VolumeID  varchar(64) DEFAULT ('') WITH VALUES

GO

ALTER TABLE PrivateDataReference ADD 
	CONSTRAINT PrivateDataReference_Unique UNIQUE( PrivateDataID, AuxRefSeriesUID, VolumeID)
GO

ALTER TABLE PrivateDataReference CHECK CONSTRAINT PrivateDataReference_Unique
GO

-- add primary key for remember sequence
ALTER TABLE PrivateDataReference ADD PKey INT NOT NULL IDENTITY
ALTER TABLE PrivateDataReference ADD CONSTRAINT PK_PrivateDataReference PRIMARY KEY (PKey ) 

GO

--Drop table LocalAEGroupAssignment
--GO

CREATE UNIQUE INDEX ComparatorOp__index on Comparator (Op)
CREATE UNIQUE INDEX ComparatorOpString__index on Comparator (OpString)
CREATE UNIQUE INDEX DicomTagTagString__index on DicomTag (TagString)

CREATE TABLE DataClassifyPattern
(
	ID INT IDENTITY(1,1) PRIMARY KEY,
	TagFilterID INT NOT NULL UNIQUE REFERENCES TagFilter(ID) ON DELETE CASCADE,
	TypeTag BIGINT NOT NULL REFERENCES DicomTag(Tag) ON DELETE CASCADE,
	Description  VARCHAR(128)
)


CREATE TABLE DataProcessJob
(
	ID INT IDENTITY(1,1) PRIMARY KEY,
	JobName	VARCHAR(64)  UNIQUE,
	Description  VARCHAR(128)
)

CREATE TABLE DataProcessor
(
	ID INT IDENTITY(1,1) PRIMARY KEY,
	ProcessName	VARCHAR(128)  UNIQUE,
	Handler	VARCHAR(128)  UNIQUE,
	Description  VARCHAR(128)
)


CREATE TABLE DataJobProcessList
(
	ID INT IDENTITY(1,1) PRIMARY KEY,
	JobID INT NOT NULL REFERENCES DataProcessJob(ID) ON DELETE CASCADE,
	Processor INT NOT NULL REFERENCES DataProcessor(ID) ON DELETE CASCADE,
	JobOrder	INT NOT NULL,
	CONSTRAINT DataJobProcessList_Unique UNIQUE (JobID, Processor,JobOrder)
)


CREATE TABLE DataProcessPattern
(
	ID INT IDENTITY(1,1) PRIMARY KEY,
	TagFilterID INT NOT NULL UNIQUE REFERENCES TagFilter(ID) ON DELETE CASCADE,
	Job INT NOT NULL REFERENCES DataProcessJob(ID) ON DELETE CASCADE,
	Description  VARCHAR(128)
)
GO

-- verify and update DB to patched version
USE PxDcmDB
if	( EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'DataClassifyPattern'))
	and EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'DataProcessPattern') )
	and EXISTS (SELECT * FROM sysindexes WHERE name='AuxSOPUID_index')
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'ParameterHash' AND Table_Name ='PrivateData') 
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'VolumeID' AND Table_Name ='PrivateDataReference') 
	and EXISTS (SELECT * FROM sysindexes WHERE name='PrivateDataReference_Unique')
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'PKey' AND Table_Name ='PrivateDataReference') 
	)

	begin
	Update PxDcmDB.dbo.dbinfo Set MajorVersion=1, MinorVersion=7, BuildVersion=1,BuildMinorVersion=0 
	print 'success on patching database version from 1.7.0.0 to 1.7.1.0'
	end

else
	begin
	RAISERROR ('failed on verify, abort to set version to 1.7.1.0', 20, 1) with log	
	end
GO
