USE PxDcmDB
GO

-- new tables or new initializations -------------

IF NOT EXISTS (select * from PxDcmDB.dbo.dbinfo  
	Where  MajorVersion=1 and MinorVersion=6 and BuildVersion=1 and BuildMinorVersion=0 )
begin	
	-- quit script
	RAISERROR ('Not a 1.6.1.0. database, bypass patching', 20, 1) with log

end
GO


print 'start to do 1.6.1.0 patch'

--Drop table LocalAEGroupAssignment
--GO


DECLARE @host_ip VARCHAR(15)
declare @cmd varchar(200)
declare @temp varchar(255)
create table #ip(iptext varchar(255))

begin
   set @cmd = 'ping -n 1 ' + rtrim(host_name())
   insert #ip exec master..xp_cmdshell @cmd
   select @host_ip = ISNULL(substring(iptext,(charindex('[',iptext)+1),
			(charindex(']',iptext)-(charindex('[',iptext)+1))),'')
   from #ip
   where charindex('[',iptext)>0
end
drop table #ip
SET @host_ip = rtrim(@host_ip)

-- get host name  and it's IP
DECLARE @pcname VARCHAR(128)
SELECT @pcname = rtrim(host_name())
DECLARE @AETitle1 VARCHAR(16)

SET @AETitle1 = 'AqNET_Local_00'
INSERT INTO LocalAE VALUES('AqNET local import AE', @AETitle1, @pcname, @host_ip, 105,1, 2, 'Default local import AE title')
GO


ALTER TABLE AqNetOption ADD Display VARCHAR(80) default ''
GO

IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.AqNetOption WHERE KeyStr = 'EnableAuditTrail' )  
	INSERT INTO PxDcmDB.dbo.AqNetOption (KeyStr, ValueStr, Display) 
		VALUES( 'EnableAuditTrail', '0', 'Enalbe audit trail')

Update PxDcmDB.dbo.AqNetOption SET Display='Enalbe audit trail' WHERE KeyStr = 'EnableAuditTrail' 

IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.AqNetOption WHERE KeyStr = 'RequiredFreeSpaceOnDriveC' ) 
	INSERT INTO PxDcmDB.dbo.AqNetOption (KeyStr, ValueStr, Display) 
		VALUES( 'RequiredFreeSpaceOnDriveC', '1000', 'Required free space on drive C')
 
Update PxDcmDB.dbo.AqNetOption SET Display='Required free space on drive C' 
	WHERE KeyStr = 'RequiredFreeSpaceOnDriveC'

INSERT INTO PxDcmDB.dbo.AqNetOption (KeyStr, ValueStr, Display) 
	VALUES( 'EnableSSO', '0', 'Enable single sign on')
 
	
INSERT INTO PxDcmDB.dbo.AqNetOption (KeyStr, ValueStr, Display) 
	VALUES( 'OneInstanceTopDBSize', '350', 'Instance record average top size in bytes for fragment warning')


CREATE UNIQUE INDEX AqNetOption__Key_index on AqNetOption (KeyStr)
CREATE UNIQUE INDEX AqNetOption__Display_index on AqNetOption (Display)
GO


-- starting 1.7 FilmingPattern table is added. Here is for fresh install or update from versions before 1.7
IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'FilmingPattern'))
BEGIN 
print 'create FilmingPattern table'
CREATE TABLE FilmingPattern
(
	TagFilterID int not null FOREIGN KEY REFERENCES TagFilter(ID) ON DELETE CASCADE,
	PrinterID int not null FOREIGN KEY REFERENCES Printer(ID) ON DELETE CASCADE,
	Options int not null, -- delete original series after print? 0=no, 1=yes
	SkipN int, -- >=0
	DisplayMode varchar(16) -- eg. 2x2, 2x4
	CONSTRAINT FilmingPattern_Unique UNIQUE (TagFilterID, PrinterID)
)
END

-- table changes -----------

-- CharacterSet is multi value can be 1024, we save up to  256

ALTER TABLE StudyLevel ALTER COLUMN CharacterSet VARCHAR(256)


-- Three new columns added in seriesLevel table in release 1.7
-- also existing in CreateAQNetDB.sql.
-- print 'Add seriesDate, sereisTime and accessTime column'
IF NOT EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'SeriesDate' AND Table_Name ='SeriesLevel') 
BEGIN 
	--- add two more columns
	ALTER TABLE SeriesLevel ADD SeriesDate VARCHAR(10) DEFAULT ('')  
	ALTER TABLE SeriesLevel ADD SeriesTime VARCHAR(16) DEFAULT ('')
END


IF NOT EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'AccessTime' AND Table_Name ='StudyLevel') 
BEGIN 
	ALTER TABLE StudyLevel ADD AccessTime datetime DEFAULT GETDATE()
END

GO


Update StudyLevel set AccessTime=(select max(modifyTime) from serieslevel group by studylevelID 
	having studylevelID=StudyLevel.studylevelID)
	Where  AccessTime is Null

IF EXISTS (SELECT * FROM sysindexes WHERE name='AuxSOPUID_index')
	DROP INDEX PrivateData.AuxSOPUID_index

ALTER TABLE PrivateData ADD ProcessName varchar(64) DEFAULT ('')
ALTER TABLE PrivateData ADD ProcessType varchar(64) DEFAULT ('')
GO


CREATE UNIQUE INDEX AuxSOPUID_index on PrivateData (AuxSOPUID, Type, Name, Subtype)
GO


USE PxDcmHistDB	
-- drop the view to allow rename SID
ALTER TABLE SeriesAttribute DROP column Description
GO

ALTER TABLE SeriesAttribute ADD ProcessName varchar(64)
ALTER TABLE SeriesAttribute ADD ProcessType varchar(64)
ALTER TABLE SeriesAttribute ADD Description VARCHAR(256)
GO

-- changes for SSO ------------
USE PxDcmDB	
GO


-- SSO group can be very long, we set it to 64
ALTER TABLE UserGroup ALTER COLUMN Name VARCHAR(64)

-- convert all internal department to department 'AQNet'
update UserGroup SET DepartmentId=(
Select top 1 DepartmentID from Department d join Organization o on  
d.OrganizationId = o.OrganizationId 
where o.Name='AQNet' AND d.Name='AQNet')
delete Department where Name <> 'AQNet'

delete Organization where Name <> 'AQNet'
delete Department where Name <> 'AQNet'
GO



-- change department to DomainT
EXEC sp_rename 'Department.DepartmentID', 'DomainID', 'COLUMN'
ALTER TABLE Department DROP CONSTRAINT	Department_Unique
ALTER TABLE Department ADD Type	INT DEFAULT 0
ALTER TABLE Department WITH NOCHECK ADD CONSTRAINT DomainT_Name_Unique UNIQUE  NONCLUSTERED ([Name])
GO

Update Department SET Type=0
EXEC sp_rename 'Department', 'DomainT'
GO


-- update Usergroup
EXEC sp_rename 'UserGroup.DepartmentID', 'DomainID', 'COLUMN'
GO
declare @uqname varchar(50)
declare @SQL nvarchar(1024)
select top 1 @uqname=name from sysobjects where name like 'UQ__UserGroup%'
--print @uqname
set @SQL = 'ALTER TABLE UserGroup DROP CONSTRAINT ' + @uqname
--print @SQL
exec sp_executesql @SQL 
GO
ALTER TABLE UserGroup ADD SID VARBINARY(128) default 0x00
ALTER TABLE UserGroup ADD CONSTRAINT UserGroup_Unique UNIQUE(Name, DomainID)
GO

print 'create RightsName talbe'
CREATE TABLE  RightsName
(
	NameID 	INT IDENTITY(1,1) PRIMARY KEY,
	Name 		VARCHAR(64) NOT NULL
)
CREATE UNIQUE INDEX RightsName__index on RightsName (Name)

print 'create RightsValue talbe'
CREATE TABLE  RightsValue
(
	ValueID 	INT IDENTITY(1,1) PRIMARY KEY,
	RValue 		VARCHAR(800) NOT NULL
)
CREATE UNIQUE INDEX RightsValue__index on RightsValue (RValue)

INSERT INTO RightsName VALUES ('AccessAllData')
INSERT INTO RightsValue VALUES ('1')

print 'create UserGroupRights talbe'
CREATE TABLE  UserGroupRights
(
	GroupID 		INT not null REFERENCES UserGroup(UserGroupID) ON DELETE CASCADE,
	RightsKeyID 	INT not null REFERENCES RightsName(NameID) ON DELETE NO ACTION,
	RightsValueID	INT not null REFERENCES RightsValue(ValueID) ON DELETE NO ACTION,
  	CONSTRAINT UserGroupRights_Unique PRIMARY KEY CLUSTERED (GroupID, RightsKeyID)
)

INSERT INTO UserGroupRights (GroupID, RightsKeyID, RightsValueID) 
SELECT UserGroupID, NameID, ValueID FROM UserGroup, RightsName, RightsValue
	Where UserGroup.Name='Administrators' or UserGroup.Name='shared'
GO

-- update useraccount
declare @uqname varchar(50)
declare @SQL nvarchar(1024)
select top 1 @uqname=name from sysobjects where name like 'UQ__UserAccount%'
--print @uqname
set @SQL = 'ALTER TABLE UserAccount DROP CONSTRAINT ' + @uqname
--print @SQL
exec sp_executesql @SQL 
GO

IF EXISTS (SELECT * FROM sysindexes WHERE name='UserAccount__index')
	DROP INDEX UserAccount.UserAccount__index

ALTER TABLE UserAccount ADD DomainID INT REFERENCES DomainT(DomainID) ON DELETE NO ACTION
GO

Update UserAccount Set DomainID = (SELECT top 1 DomainID FROM DomainT Where Name = 'AQNet')
GO

CREATE UNIQUE INDEX UserAccount__index on UserAccount (Username, DomainID)
GO


USE PxDcmHistDB	
-- drop the view to allow rename SID
IF EXISTS (select * from information_schema.tables 
	where table_name ='UserObject' and  table_type = 'VIEW')
   DROP VIEW UserObject
GO
EXEC sp_rename 'AqObject.SID', 'DomainName', 'COLUMN'
GO


Update AqObjectType Set TypeName='Domain', ViewName='DomainObject', Description='AqNET Domain object'
Where TypeName='Department'
GO

-- verify and update DB to patched version
USE PxDcmDB
if	( EXISTS (select * from LocalAE Where AETitle='AqNET_Local_00') 
	and EXISTS (select Display from AqNetOption)
	and EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'FilmingPattern'))
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'SeriesDate' AND Table_Name ='SeriesLevel') 
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'AccessTime' AND Table_Name ='StudyLevel') 
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'ProcessName' AND Table_Name ='PrivateData') 
	and EXISTS (SELECT * FROM sysindexes WHERE name='AuxSOPUID_index')
	and EXISTS (SELECT * FROM sysindexes WHERE name='UserAccount__index')
	and EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'DomainT'))
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'DomainID' AND Table_Name ='UserGroup') 
	and EXISTS (SELECT * FROM UserGroupRights)
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'DomainID' AND Table_Name ='UserAccount') 
	)

	begin
	Update PxDcmDB.dbo.dbinfo Set MajorVersion=1, MinorVersion=7, BuildVersion=0,BuildMinorVersion=0 
	print 'success on patching database version from 1.6.1.0 to 1.7.0.0'
	end

else
	begin
	RAISERROR ('failed on verify, abort to set version to 1.7.0.0', 20, 1) with log	
	end
GO
