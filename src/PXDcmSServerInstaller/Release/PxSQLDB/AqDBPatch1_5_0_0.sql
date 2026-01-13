USE PxDcmDB
GO

-- new tables or new initializations -------------

IF NOT EXISTS (select * from PxDcmDB.dbo.dbinfo  
	Where  MajorVersion=1 and MinorVersion=5 and BuildVersion=0 and BuildMinorVersion=0 )
begin	
	-- quit script
	RAISERROR ('Not a 1.5.0.0. database, bypass patching', 20, 1) with log

end
GO


-- patch database first
print 'start to do 1.5.0.0 patch'

USE PxDcmHistDB	-- Create database PxDcmHistDB

ALTER DATABASE PxDcmHistDB SET ARITHABORT ON 

-- setup AqSystemObject table and stored procedures

IF NOT EXISTS (select * from information_schema.tables where table_name ='AqObjectType' and table_type = 'BASE TABLE')
begin   

-- look up table for object type
print 'create AqObjectType'
CREATE TABLE AqObjectType
(
  ID		INT IDENTITY(0,1) PRIMARY KEY,
  TypeName 	VARCHAR(64) not null,
  ViewName VARCHAR(64) not null default '',	-- name of object view 
  Description 	VARCHAR(256)  not null default ''
)
CREATE UNIQUE INDEX TypeName_index on AqObjectType(TypeName)


-- a table to make sure no one can delete the system object type
-- to keep the type not deleted, do following inserting
-- Insert AqObjectTypeHolder Select ID from AqObjectType Where TypeName='YourObjectType'
print 'create AqObjectTypeHolder table'
CREATE TABLE AqObjectTypeHolder
(
	ID INT not null REFERENCES AqObjectType(ID) ON DELETE NO ACTION
)

-- make null object for dfefault
--SET IDENTITY_INSERT AqObjectType ON 

--INSERT INTO AqObjectType (ID, TypeName, ViewName, StartID, EndID, Description) 
--	VALUES (0, '_NULL_', 'AqSystemObject', 'a null object just for link key')
--SET IDENTITY_INSERT AqObjectType OFF 
--Insert AqObjectTypeHolder Select ID from AqObjectType Where TypeName='_NULL_'
end

GO


--begin of MakeAqObjectType
print 'create procedure MakeAqObjectType'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeAqObjectType' AND type = 'P')
   DROP PROCEDURE MakeAqObjectType
GO

CREATE PROCEDURE MakeAqObjectType( 
	@TypeName 	VARCHAR(64),
	@ViewName	VARCHAR(64),
	@Description VARCHAR(256)
 )

AS
BEGIN
	SET NOCOUNT ON

	IF NOT EXISTS ( SELECT * FROM dbo.AqObjectType WHERE TypeName=@TypeName )
	begin
		INSERT INTO dbo.AqObjectType(
		TypeName,
		ViewName,
		Description
		) 
		VALUES (
		@TypeName,
		@ViewName,
		@Description
		)
	-- put a record in AqObjectTypeHolder to hold this record from deleting
	Insert AqObjectTypeHolder Select ID FROM dbo.AqObjectType WHERE TypeName=@TypeName
	end

	
END
--end of MakeAqObjectType
GO



IF NOT EXISTS (select * from information_schema.tables where table_name ='AqObject' and table_type = 'BASE TABLE')
begin   

-- initial system object first

EXEC MakeAqObjectType '_Null_',			'AqObject', 'AqNET system null object'
EXEC MakeAqObjectType 'ImageServer',	'AqObject', 'AqNET image server object'
EXEC MakeAqObjectType 'DICOMServer',	'AqObject', 'AqNET DICOM server object'
EXEC MakeAqObjectType 'DatabaseServer',	'AqObject', 'AqNET Database server object'
EXEC MakeAqObjectType 'StorageServer',	'AqObject', 'AqNET storage server object'
EXEC MakeAqObjectType 'WebServer',		'AqObject', 'AqNET Web server object'
EXEC MakeAqObjectType 'ProcessServer',	'AqObject', 'AqNET process server object'
EXEC MakeAqObjectType 'Client',			'AqObject', 'AqNET client object'
EXEC MakeAqObjectType 'LocalAE',		'AEObject', 'AqNET Local AE object'
EXEC MakeAqObjectType 'RemoteAE',		'AEObject', 'AqNET Remote AE object'
EXEC MakeAqObjectType 'Printer',		'PrinterObject', 'AqNET Printer object'
EXEC MakeAqObjectType 'Department',		'DeptObject', 'AqNET department object'
EXEC MakeAqObjectType 'Organization',	'OrgObject', 'AqNET organization object'
EXEC MakeAqObjectType 'UserGroup',		'GroupObject', 'AqNET user group object'
EXEC MakeAqObjectType 'UserAccount',	'UserObject', 'AqNET user object'

DROP PROCEDURE MakeAqObjectType

print 'create AqObject table'
CREATE TABLE AqObject
(
  ID		INT IDENTITY(0,1) PRIMARY KEY,
  Type 		INT not null REFERENCES AqObjectType(ID) ON DELETE NO ACTION,
  EntityName VARCHAR(64) not null, -- active directory user name size is 20
  FullName 	VARCHAR(128) not null,
  Hostname	VARCHAR(64)	 not null default '',
  Address	VARCHAR(64)	 not null default '',
  Port 		INT  not null default 0,
  SID 		VARCHAR(128)  not null default '',	-- security ID from single sign on
  Description 	VARCHAR(256)  not null default '',
  	CONSTRAINT AqObject_Unique UNIQUE(Type, EntityName, FullName, Hostname, Address, Port, SID)
)
CREATE INDEX Type_index on AqObject(Type)
CREATE INDEX EntityName_index on AqObject(EntityName)
CREATE INDEX FullName_index on AqObject(FullName)
CREATE INDEX Hostname_index on AqObject(Hostname)
CREATE INDEX Address_index on AqObject(Address)


-- make a null record at index 0
INSERT AqObject Select ID, '_NULL_', '', '', '', 0, '', 'A special record for null object'
	From dbo.AqObjectType Where TypeName='_Null_'

end

GO

IF NOT EXISTS (select * from information_schema.tables where table_name = 'PatientStudy' and table_type = 'BASE TABLE')
begin   
print 'create PatientStudy table'
CREATE TABLE PatientStudy
(
	StudyIndex INT IDENTITY(0,1) PRIMARY KEY,
	StudyInstanceUID VARCHAR(64) not null, 
	PatientsName VARCHAR(332) not null, 
	PatientID VARCHAR(64) not null DEFAULT '', 
	PatientsBirthDate VARCHAR(10) not null DEFAULT '', 
	PatientsSex	VARCHAR(16) not null DEFAULT '', 
	StudyDate VARCHAR(10) not null DEFAULT '', 
	StudyTime VARCHAR(16) not null DEFAULT '', 
	AccessionNumber VARCHAR(16) not null DEFAULT 0, 
	StudyID VARCHAR(16) not null DEFAULT '', 
	ReferringPhysiciansName VARCHAR(332) not null DEFAULT '', 
	Status INT not null DEFAULT 0,
		CONSTRAINT PatientStudy_Unique UNIQUE(StudyInstanceUID, PatientsName, PatientID, 
		PatientsBirthDate, PatientsSex, StudyDate, StudyTime, AccessionNumber, StudyID)  
)
CREATE INDEX StudyUID_index on PatientStudy(StudyInstanceUID)
CREATE INDEX PatientsName_index on PatientStudy (PatientsName)
CREATE INDEX PatientID_index on PatientStudy (PatientID)
CREATE INDEX StudyDate_index on PatientStudy (StudyDate)
CREATE INDEX StudyTime_index on PatientStudy (StudyTime)
CREATE INDEX AccessionNumber_index on PatientStudy (AccessionNumber)
CREATE INDEX StudyID_index on PatientStudy (StudyID)
CREATE INDEX ReferringPhysiciansName_index on PatientStudy (ReferringPhysiciansName)

-- defaule null record
Insert PatientStudy (StudyInstanceUID, PatientsName) values('', '')
end


IF NOT EXISTS (select * from information_schema.tables where table_name='PatientSeries' and table_type='BASE TABLE')
begin

print 'create SeriesAttribute table'
CREATE TABLE SeriesAttribute
(
  ID	INT IDENTITY(0,1) PRIMARY KEY,
  Name      VARCHAR(64) NOT NULL,
  Type      INT NOT NULL,
  Subtype	VARCHAR(64) NOT NULL,
  Description   VARCHAR(256)

)
CREATE UNIQUE INDEX Attrbute_index on SeriesAttribute (Name, Type, Subtype)
CREATE INDEX Name_index on SeriesAttribute(Name)
CREATE INDEX Type_index on SeriesAttribute(Type)
CREATE INDEX Subtype_index on SeriesAttribute(Subtype)

-- make a special record to link in case no information
INSERT INTO SeriesAttribute(Name, type, subtype, Description) 
	VALUES ('', -1, 'no type attribute', 'dummy entry for NULL attribute')
 

print 'create PatientSeries table'
CREATE TABLE PatientSeries
(
	SeriesIndex	INT IDENTITY(0,1) PRIMARY KEY,
	StudyIndex INT not null REFERENCES PatientStudy(StudyIndex) ON DELETE NO ACTION,
	SeriesInstanceUID VARCHAR(64) not null,
	SeriesNumber INT not null DEFAULT 0, 
	Modality VARCHAR(16) not null DEFAULT '', 
	SeriesInstances INT not null DEFAULT 0, 
	TransferSyntax INT not null DEFAULT 100, -- IMPLICIT_LITTLE_ENDIAN = 100
	Attribute INT not null REFERENCES SeriesAttribute(ID) ON DELETE NO ACTION,
	SourceAE INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
	CreateTime datetime not null DEFAULT GETDATE(),
	QRFlag tinyint not null DEFAULT 1 CHECK (QRFlag IN (0, 1)),
	Status int not null DEFAULT 0,
	Description VARCHAR(64) not null DEFAULT '',
		CONSTRAINT PatientSeries_Unique UNIQUE(StudyIndex, SeriesInstanceUID, SeriesNumber, Modality)


)
CREATE INDEX PatientSeries_index on PatientSeries (SeriesInstanceUID)
CREATE INDEX SeriesStudyIndex_index on PatientSeries (StudyIndex)
CREATE INDEX SeriesNumber_index on PatientSeries (SeriesNumber)
CREATE INDEX Modality_index on PatientSeries (Modality)
CREATE INDEX CreateTime_index on PatientSeries (CreateTime)

-- defaule null record
Insert PatientSeries (StudyIndex, SeriesInstanceUID, Attribute, SourceAE) values(0, '', 0, 0)

end

ELSE
begin
IF NOT EXISTS (SELECT * FROM AqNETHISTDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'Description' AND Table_Name ='PatientSeries')
	ALTER TABLE PatientSeries ADD Description VARCHAR(64) not null DEFAULT '' 
end


IF NOT EXISTS (select * from information_schema.tables where table_name ='Actions' and table_type = 'BASE TABLE')
begin   

print 'create Actions table'

CREATE TABLE Actions
(
  ID 		INT IDENTITY(1,1) PRIMARY KEY,
  ActionName 	VARCHAR(64)  not null, -- a display string for this action
  Description 	VARCHAR(256)  not null  default '' 
)
CREATE UNIQUE INDEX ActionName_index on Actions(ActionName)

INSERT INTO Actions Values('Login', '')
INSERT INTO Actions Values('Logout', '')
INSERT INTO Actions Values('Create', '')
INSERT INTO Actions Values('Delete', '')

INSERT INTO Actions Values('Reject', '')

INSERT INTO Actions Values('Receive', '')
INSERT INTO Actions Values('Send', '')
INSERT INTO Actions Values('Read', '')
INSERT INTO Actions Values('Write', '')

INSERT INTO Actions Values('Join', '')
INSERT INTO Actions Values('Leave', '')

INSERT INTO Actions Values('Start', '')
INSERT INTO Actions Values('Pause', '')
INSERT INTO Actions Values('Stop', '')
INSERT INTO Actions Values('Cancel', '')
INSERT INTO Actions Values('Resume', '')

INSERT INTO Actions Values('Change', '')
INSERT INTO Actions Values('Anonymize', '')
INSERT INTO Actions Values('Export', '')
INSERT INTO Actions Values('Import', '')
INSERT INTO Actions Values('Assign', '')
INSERT INTO Actions Values('Unassign', '')
INSERT INTO Actions Values('Film', '')
INSERT INTO Actions Values('Print', '')

INSERT INTO Actions Values('Retrieve', '')

end 
GO



IF NOT EXISTS (select * from information_schema.tables where table_name ='sysEventLog' and table_type = 'BASE TABLE')
begin   


print 'create sysEventLog table'
CREATE TABLE sysEventLog
(
  ID	 		INT IDENTITY(1,1) PRIMARY KEY,
  Actor			INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  Activity		INT not null REFERENCES Actions(ID) ON DELETE NO ACTION,
  ActOn			INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  Requestor		INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  ActionFrom	INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  ActionAt		INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  TransferTo	INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  TimeOfAction	datetime not null DEFAULT GETDATE(),
  Description 	VARCHAR(256) not null DEFAULT '',
  Status		INT not null DEFAULT 0 -- 0 means normal, otherwise anomal and should put detail in description
)

CREATE INDEX Actor_index on sysEventLog(Actor)
CREATE INDEX Activity_index on sysEventLog(Activity)
CREATE INDEX ActOn_index on sysEventLog(ActOn)
CREATE INDEX Requestor_index on sysEventLog(Requestor)
CREATE INDEX TimeOfAction_index on sysEventLog(TimeOfAction)
CREATE INDEX ActionFrom_index on sysEventLog(ActionFrom)
CREATE INDEX ActionAt_index on sysEventLog(ActionAt)
CREATE INDEX Status_index on sysEventLog(Status)

print 'create SeriesEventLog table'
CREATE TABLE SeriesEventLog
(
  ID	 		INT IDENTITY(1,1) PRIMARY KEY,
  Actor			INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  Activity		INT not null REFERENCES Actions(ID) ON DELETE NO ACTION,
  ActOn			INT not null REFERENCES PatientSeries(SeriesIndex) ON DELETE NO ACTION,
  Requestor		INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  ActionFrom	INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  ActionAt		INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  TransferTo	INT not null REFERENCES PatientSeries(SeriesIndex) ON DELETE NO ACTION,
  TimeOfAction	datetime not null DEFAULT GETDATE(),
  Description 	VARCHAR(256) not null DEFAULT '',
  Status		INT not null DEFAULT 0 -- 0 means normal, otherwise anomal and should put detail in description
)

CREATE INDEX Actor_index on SeriesEventLog(Actor)
CREATE INDEX Activity_index on SeriesEventLog(Activity)
CREATE INDEX ActOn_index on SeriesEventLog(ActOn)
CREATE INDEX Requestor_index on SeriesEventLog(Requestor)
CREATE INDEX TimeOfAction_index on SeriesEventLog(TimeOfAction)
CREATE INDEX ActionFrom_index on SeriesEventLog(ActionFrom)
CREATE INDEX ActionAt_index on SeriesEventLog(ActionAt)
CREATE INDEX Status_index on SeriesEventLog(Status)

print 'create StudyEventLog table'
CREATE TABLE StudyEventLog
(
  ID	 		INT IDENTITY(1,1) PRIMARY KEY,
  Actor			INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  Activity		INT not null REFERENCES Actions(ID) ON DELETE NO ACTION,
  ActOn			INT not null REFERENCES PatientStudy(StudyIndex) ON DELETE NO ACTION,
  Requestor		INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  ActionFrom	INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  ActionAt		INT not null REFERENCES AqObject(ID) ON DELETE NO ACTION,
  TransferTo	INT not null REFERENCES PatientStudy(StudyIndex) ON DELETE NO ACTION,
  TimeOfAction	datetime not null DEFAULT GETDATE(),
  Description 	VARCHAR(256) not null DEFAULT '',
  Status		INT not null DEFAULT 0 -- 0 means normal, otherwise anomal and should put detail in description
)

CREATE INDEX Actor_index on StudyEventLog(Actor)
CREATE INDEX Activity_index on StudyEventLog(Activity)
CREATE INDEX ActOn_index on StudyEventLog(ActOn)
CREATE INDEX Requestor_index on StudyEventLog(Requestor)
CREATE INDEX TimeOfAction_index on StudyEventLog(TimeOfAction)
CREATE INDEX ActionFrom_index on StudyEventLog(ActionFrom)
CREATE INDEX ActionAt_index on StudyEventLog(ActionAt)
CREATE INDEX Status_index on StudyEventLog(Status)
end 

GO



USE PxDcmDB

ALTER DATABASE PxDcmDB SET ARITHABORT ON 


-- the read status table links to main patient table and user table
-- Therefore, cannot record the history series, user and QR series read status 
-- If we really need that record, we can fish it out from Actions records.

IF NOT EXISTS (select * from information_schema.tables where table_name ='StudyReadStatus' and table_type = 'BASE TABLE')
begin   
print 'create StudyReadStatus table'
CREATE TABLE StudyReadStatus
(
	StudyLevelID INT not null REFERENCES StudyLevel(StudyLevelID) ON DELETE CASCADE,
	UserID 	INT not null REFERENCES UserAccount(AccountID) ON DELETE CASCADE,
	Status 	INT  NOT NULL,
  		CONSTRAINT StudyReadStatus_Unique PRIMARY KEY CLUSTERED ( UserID, StudyLevelID)
)
CREATE INDEX StudyReadStatus_StudyLevelID_index on StudyReadStatus (StudyLevelID)
CREATE INDEX StudyReadStatus_UserID_index on StudyReadStatus (UserID)

end


IF NOT EXISTS (select * from information_schema.tables where table_name ='SeriesReadStatus' and table_type = 'BASE TABLE')
begin   
print 'create SeriesReadStatus table'
CREATE TABLE SeriesReadStatus
(
	SeriesLevelID INT not null REFERENCES SeriesLevel(SeriesLevelID) ON DELETE CASCADE,
	UserID 	INT not null REFERENCES UserAccount(AccountID) ON DELETE CASCADE,
	Status 	INT  NOT NULL,
   		CONSTRAINT SeriesReadStatus_Unique PRIMARY KEY CLUSTERED (UserID, SeriesLevelID)
)
CREATE INDEX SeriesReadStatus_index on SeriesReadStatus (SeriesLevelID)
CREATE INDEX SeriesReadStatus_UserID_index on SeriesReadStatus (UserID)

end


-- begin of procedure UpdateStudyReadStatus
print 'create procedure UpdateStudyReadStatus'
IF EXISTS (SELECT name FROM sysobjects
      WHERE name = 'UpdateStudyReadStatus' AND type = 'P')
   DROP PROCEDURE UpdateStudyReadStatus
GO

CREATE PROCEDURE  UpdateStudyReadStatus (
	@StudyLevelID INT,
	@UserID INT
)

AS 

BEGIN
	SET NOCOUNT ON

	DECLARE @readStatus INT
	DECLARE @totalVInStatus INT
	DECLARE @totalNInSeries INT

	SET @readStatus = 0
	SET @totalVInStatus = 0
	SET @totalNInSeries = 0

	SELECT @totalNInSeries = COUNT(SeriesLevel.SeriesLevelID)
		FROM SeriesLevel 
		Where SeriesLevel.StudyLevelID=@StudyLevelID

	SELECT @totalVInStatus = SUM(SeriesReadStatus.Status)
		FROM SeriesLevel LEFT JOIN SeriesReadStatus 
		ON SeriesLevel.SeriesLevelID=SeriesReadStatus.SeriesLevelID AND
		UserID=@UserID 
		Where SeriesLevel.StudyLevelID=@StudyLevelID AND
		SeriesReadStatus.Status = 4

	IF @totalNInSeries = 0 OR @totalNInSeries is NULL
		SET @readStatus = 0 -- kDBIsUnknown
	else
	begin
		IF @totalVInStatus = 4*@totalNInSeries
			SET @readStatus = 4 -- kDBIsRead
		else
		begin
			IF @totalVInStatus = 0 OR @totalVInStatus is NULL
				SET @readStatus = 1 -- kDBIsUnread
			else
				SET  @readStatus = 2 -- kDBIsPartiallyRead
		end
	end

	
	IF EXISTS (SELECT * FROM StudyReadStatus 
		WHERE StudyLevelID = @StudyLevelID and UserID = @UserID)
		UPDATE StudyReadStatus SET Status=@readStatus 
		WHERE StudyLevelID = @StudyLevelID and UserID = @UserID
	ELSE 
	begin
	
	IF @readStatus > 0
	INSERT INTO StudyReadStatus (StudyLevelID, UserID, Status)
		VALUES (@StudyLevelID, @UserID, @readStatus )
	end

END

GO
-- end of UpdateStudyReadStatus


-- copy record from UserStudyStatus and UserSeriesStatus to StudyReadStatus and SeriesReadStatus
IF EXISTS (select * from PxDcmHistDB.information_schema.tables where table_name ='UserStudyStatus' and table_type = 'BASE TABLE')
BEGIN

	INSERT INTO SeriesReadStatus (SeriesLevelID, UserID, Status)
		SELECT SeriesLevel.SeriesLevelID, UserAccount.AccountID, s1.Status
			FROM PxDcmHistDB.dbo.UserSeriesStatus s1 join PxDcmHistDB.dbo.SeriesUIDIndex st 
			on st.SeriesIndex=s1.SeriesIndex join UserAccount 
			on s1.UserID=UserAccount.AccountID join SeriesLevel 
			on SeriesLevel.SeriesInstanceUID = st.SeriesInstanceUID


	DECLARE @StudyLevelID INT
	DECLARE @UserID INT

	DECLARE @c_series CURSOR

	SET @c_series =CURSOR LOCAL SCROLL FOR
	SELECT distinct StudyLevelID, UserID FROM dbo.SeriesReadStatus join SeriesLevel
			on dbo.SeriesReadStatus.SeriesLevelID = SeriesLevel.SeriesLevelID
			FOR READ ONLY
  	
	OPEN @c_series
	FETCH NEXT FROM @c_series INTO @StudyLevelID, @UserID
	WHILE @@FETCH_STATUS = 0
	BEGIN
		EXEC UpdateStudyReadStatus @StudyLevelID, @UserID
		FETCH NEXT FROM @c_series INTO @StudyLevelID, @UserID
	END
	CLOSE @c_series


END


-- remove old read status tables
IF EXISTS (select * from PxDcmHistDB.information_schema.tables where table_name ='ActionHistory' and table_type = 'BASE TABLE')
BEGIN

Drop Table PxDcmHistDB.dbo.ActionHistory
Drop Table PxDcmHistDB.dbo.UserSeriesStatus
Drop Table PxDcmHistDB.dbo.UserStudyStatus
Drop Table PxDcmHistDB.dbo.SeriesUIDIndex
Drop Table PxDcmHistDB.dbo.StudyUIDIndex
Drop Table PxDcmHistDB.dbo.DeletedUsers


END


IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'QueryStudyReadStatus' AND type = 'FN')
   DROP FUNCTION QueryStudyReadStatus
GO


-- convert to real IP for local AE
USE PxDcmDB

-- set status to 0 to disable instance updating
update SeriesLevel SET Status=0
GO

IF not EXISTS (select * from sysindexes where name = 'ModalitiesInStudy_index')
CREATE INDEX ModalitiesInStudy_index on StudyLevel (ModalitiesInStudy)

-- get host name  and it's IP
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

--Update LocalAE SET IPAddress=@host_ip WHERE IPAddress='127.0.0.1' AND AETitle <> 'AUTOVOX'

DECLARE @AE_ID INT
DECLARE @AETitle		VARCHAR(64)
DECLARE @HostName		VARCHAR(128)
DECLARE @IPAddress		VARCHAR(32)
DECLARE @Port			INT
DECLARE @c1 CURSOR

SET @c1 =CURSOR LOCAL SCROLL FOR 
SELECT ID, AETitle, HostName, IPAddress, Port FROM LocalAE
WHERE IPAddress='127.0.0.1' AND AETitle <> 'AUTOVOX' FOR READ ONLY

OPEN @c1
FETCH NEXT FROM @c1 INTO @AE_ID, @AETitle, @HostName, @IPAddress, @Port

WHILE @@FETCH_STATUS = 0
BEGIN
	IF NOT EXISTS (SELECT ID FROM LocalAE WHERE AETitle=@AETitle AND HostName=@HostName AND
		IPAddress=@host_ip AND Port=@Port)
	Update LocalAE SET IPAddress=@host_ip WHERE ID=@AE_ID
	FETCH NEXT FROM @c1 INTO @AE_ID, @AETitle, @HostName, @IPAddress, @Port
END
CLOSE @c1
	

SET @c1 =CURSOR LOCAL SCROLL FOR 
SELECT ID, AETitle, IPAddress, Port FROM LocalAE
WHERE HostName='LocalHost' FOR READ ONLY

OPEN @c1
FETCH NEXT FROM @c1 INTO @AE_ID, @AETitle, @IPAddress, @Port

SET @HostName = rtrim(host_name())
WHILE @@FETCH_STATUS = 0
BEGIN
	IF NOT EXISTS (SELECT ID FROM LocalAE WHERE AETitle=@AETitle AND HostName=@HostName AND
		IPAddress=@IPAddress AND Port=@Port)
	Update LocalAE SET HostName=@HostName WHERE ID=@AE_ID
	FETCH NEXT FROM @c1 INTO @AE_ID, @AETitle, @IPAddress, @Port
END

IF NOT EXISTS (SELECT DepartmentID FROM UserGroup WHERE Name='AqNET_Public') 
INSERT INTO UserGroup  (DepartmentId, Name, Privilege, Description) 
SELECT top 1 DepartmentID, 'AqNET_Public',  1024, 'global public group' FROM Department
	Where Name = 'AQNet'

CLOSE @c1
Go

-- jwu 04/12/05
-- this is a fix as a patch. Formal place is in CreateAQNetDB.sql
DECLARE @aqDefault7x24Schedule VARCHAR(64)
DECLARE @Schedule_ID int
DECLARE @dayIndex int
SET @aqDefault7x24Schedule = 'AqNET_Default_7x24'

IF NOT EXISTS (SELECT * FROM SCHEDULE WHERE Name = @aqDefault7x24Schedule)
BEGIN
	

	INSERT INTO Schedule (Name) values (@aqDefault7x24Schedule)
	SET @Schedule_ID = -1
	SELECT @Schedule_ID=ID FROM Schedule WHERE Name = @aqDefault7x24Schedule
	-- insert to RoutingSchedule table as 24 hours each day and 7 days each week
	if(@Schedule_ID > 0)
	BEGIN
		-- add default timerange
		SET @dayIndex = 0
		WHILE (@dayIndex < 7)
		BEGIN
			INSERT INTO TimeRange (ScheduleID, DayOfWeek, StartTime, EndTime) VALUES (@Schedule_ID, @dayIndex,'0000', '2359')
			SET @dayIndex = @dayIndex + 1
		END
	END 
END

--starting v1.6.1, we will have  LogItemsPerPage and ClientViewerInMainPage
-- add logItemsPerPage to webConfiguration table with 100 as default, ClientViewerInMainPage =1
print 'update WebConfiguration'
IF EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'WebConfiguration'))
BEGIN
	-- from release 1.5.2  
    IF NOT EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'LogItemsPerPage' AND Table_Name ='WebConfiguration') 
	BEGIN 
		--- not initialized  
		IF NOT EXISTS(SELECT * FROM PxDcmDB.dbo.WebConfiguration) 
			INSERT INTO PxDcmDB.dbo.WebConfiguration(PwdExpiryInterval, LockTimeOut, MaxLoginRetry, InactiveDaysAllowed, 
			SessionTimeOut, ItemsPerPage ) values (180, 10,5,90,20,25)

		--- add two more columns
		ALTER TABLE WebConfiguration ADD LogItemsPerPage INT DEFAULT (100) NOT NULL 
		ALTER TABLE WebConfiguration ADD ClientViewerInMainPage INT DEFAULT (1) NOT NULL 
	END
END

GO

-- for new installation, add default entry
IF NOT EXISTS(SELECT * FROM PxDcmDB.dbo.WebConfiguration) 
	INSERT INTO PxDcmDB.dbo.WebConfiguration(PwdExpiryInterval, LockTimeOut, MaxLoginRetry, InactiveDaysAllowed, 
	SessionTimeOut, ItemsPerPage, LogItemsPerPage, ClientViewerInMainPage ) values (180, 10,5,90,20,25, 100, 1) 


-- also existing in CreateAQNetDB.sql 
IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'AqNetOption'))
BEGIN 
print 'create AqNetOption table'
CREATE TABLE AqNetOption
(
	KeyStr VARCHAR(64) UNIQUE NOT NULL,
	ValueStr VARCHAR(256) NOT NULL,
)
END


IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.AqNetOption WHERE KeyStr = 'EnableAuditTrail' )  
	INSERT INTO PxDcmDB.dbo.AqNetOption (KeyStr, ValueStr) VALUES( 'EnableAuditTrail', '0')
IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.AqNetOption WHERE KeyStr = 'RequiredFreeSpaceOnDriveC' ) 
	INSERT INTO PxDcmDB.dbo.AqNetOption (KeyStr, ValueStr) VALUES( 'RequiredFreeSpaceOnDriveC', '1000')

IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'Comparator'))
BEGIN
	print 'create Comparator table'
	CREATE TABLE Comparator
	(
	  ID			INT PRIMARY KEY,
	  Op			VARCHAR(64) NOT NULL,
	  OpString		VARCHAR(64) NOT NULL 
	)

	INSERT INTO Comparator VALUES(1, '=', 'is')
	INSERT INTO Comparator VALUES(2, '<>', 'is not')
	INSERT INTO Comparator VALUES(3, 'LIKE', 'contains')
	INSERT INTO Comparator VALUES(4, 'NOT LIKE', 'doesn''t contains')
END


IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'DicomTag'))
BEGIN
	print 'create DicomTag table'
	CREATE TABLE DicomTag
	(
	  ID		INT IDENTITY(1,1) PRIMARY KEY,
	  Tag		BIGINT UNIQUE NOT NULL,
	  TagString	VARCHAR(128) NOT NULL
	)

	INSERT INTO DicomTag VALUES(0x00020016, 'Source Application Entity Title')
	INSERT INTO DicomTag VALUES(0x00080060, 'Modality')
	INSERT INTO DicomTag VALUES(0x00080090, 'Referring Physician''s Name')
	INSERT INTO DicomTag VALUES(0x00081060, 'Name of Physician(s) Reading Study')
	INSERT INTO DicomTag VALUES(0x0008103E, 'Series Description')
	INSERT INTO DicomTag VALUES(0x00081030, 'Study Description')
	INSERT INTO DicomTag VALUES(0x00180015, 'Body Part Examined')
END


IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'TagRule'))
BEGIN
	print 'create TagRule table'
	CREATE TABLE TagRule
	(
	  ID			INT IDENTITY(1,1) PRIMARY KEY,
	  DicomTagID		INT NOT NULL REFERENCES DicomTag(ID) ON DELETE CASCADE,
	  ComparatorID		INT NOT NULL REFERENCES Comparator(ID),
	  Value			VARCHAR(64) NOT NULL
		CONSTRAINT TagRule_Unique UNIQUE(DicomTagID, ComparatorID, Value)
	)
END



IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'TagFilter'))
BEGIN
	print 'create TagFilter table'
	CREATE TABLE TagFilter
	(
	  ID		INT IDENTITY(1,1) PRIMARY KEY,
	  Name		VARCHAR(32) UNIQUE NOT NULL,
	  Description 	VARCHAR (64)
	)
END



IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'TagFilterRules'))
BEGIN
	print 'create TagFilterRules table'
	CREATE TABLE TagFilterRules
	(
	  ID			INT IDENTITY(1,1) PRIMARY KEY,
	  TagFilterID		INT NOT NULL REFERENCES TagFilter(ID) ON DELETE CASCADE,
	  TagRuleID		INT NOT NULL REFERENCES TagRule(ID) ON DELETE CASCADE,
		CONSTRAINT TagFilterRules_Unique UNIQUE(TagFilterID, TagRuleID)
	)
END



IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'TagFilterGroupAssignment'))
BEGIN
	print 'create TagFilterGroupAssignment table'
	CREATE TABLE TagFilterGroupAssignment
	(
	  TagFilterID		INT NOT NULL REFERENCES TagFilter(ID) ON DELETE CASCADE,
	  GroupID		INT NOT NULL REFERENCES UserGroup (UserGroupID) ON DELETE CASCADE
		CONSTRAINT TagFilterGroupAssignment_Unique UNIQUE(TagFilterID, GroupID)
	)
END



IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'TagBasedRoutingPatternEntry'))
BEGIN
	print 'create TagBasedRoutingPatternEntry table'
	CREATE TABLE TagBasedRoutingPatternEntry
	(
		RoutingPatternID INT NOT NULL FOREIGN KEY REFERENCES RoutingPattern (ID) ON DELETE CASCADE,
		TagFilterID		INT REFERENCES TagFilter(ID) ON DELETE CASCADE,
		StoreTargetID INT NOT NULL FOREIGN KEY REFERENCES StoreTargetAE (AETitleID) ON DELETE CASCADE,
		CompressionMethod INT,
		CompressionFactor INT,
		CONSTRAINT TagBasedRoutingPatternEntry_Unique UNIQUE (RoutingPatternID,TagFilterID, StoreTargetID, CompressionMethod, CompressionFactor)
	)
END


IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'PrefetchPattern'))
BEGIN 
CREATE TABLE PrefetchPattern
(
	ID INT NOT NULL unique,
	TagFilterID INT NOT NULL REFERENCES TagFilter(ID) ON DELETE CASCADE,
	Modality VARCHAR(64) NOT NULL,
	StudyNotOlderThan INT NOT NULL,
	UnitType INT NOT NULL,		-- 0=days, 1=weeks, 2=months, 3=years
	MaxNumberResults INT NOT NULL  -- 0 = no constraint 
)
END


IF NOT EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'PrefetchPatternAE'))
BEGIN 
CREATE TABLE PrefetchPatternAE
(
	PrefetchPatternID INT NOT NULL REFERENCES PrefetchPattern (ID) ON DELETE CASCADE,
	QRSourceAEID INT NOT NULL REFERENCES QRSourceAE (AETitleID) ON DELETE CASCADE,
	CONSTRAINT PrefetchPatternAE_Unique UNIQUE (PrefetchPatternID,QRSourceAEID)
)
END


-- check if need population from RoutingPatternEntry in 1.5.0
IF EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'RoutingPatternEntry'))
BEGIN

	-- 1.Transfer RoutingPatternEntry table if it is not empty. It covers auto routing and partial filter group assignment
	-- 2.transfer localAEgroupassignment anyway to filter group assignment --- it will check if already populated in case 1

	DECLARE @localAETag BIGINT
	SET @localAETag = 0x00020016
	DECLARE @localAEID INT
	DECLARE @groupID INT
	DECLARE @comparatorID INT
	DECLARE @tagFilterID INT
	DECLARE @tagRuleID INT
	DECLARE @tagFilterName VARCHAR(64)
	DECLARE @localAETagID INT
	DECLARE @AETitle VARCHAR(64)
		
	DECLARE @c1 CURSOR
	DECLARE @AutoCounter INT
	DECLARE @tmpTagFilterID INT
	SET @AutoCounter = 0;
	SELECT @localAETagID = ID FROM DicomTag WHERE Tag = @localAETag
	SELECT @comparatorID = ID FROM Comparator WHERE OpString = 'is'

	
	-- case 1
	IF EXISTS (SELECT * FROM RoutingPatternEntry)
	BEGIN
		print 'transfer TagBasedRoutingPatternEntry'
		DECLARE @RoutingPatternID INT
		
		DECLARE @storeTargetID INT
		DECLARE @compressionMethod INT
		DECLARE @compressionFactor INT

		SET @c1 =CURSOR LOCAL SCROLL FOR
		SELECT RoutingPatternID, localAEID, storeTargetID, compressionMethod, compressionFactor FROM RoutingPatternEntry FOR READ ONLY

		OPEN @c1
		FETCH NEXT FROM @c1 INTO @RoutingPatternID, @localAEID,@storeTargetID, @compressionMethod,@compressionFactor 

		WHILE @@FETCH_STATUS = 0
		BEGIN
			SELECT @AETitle = AETitle FROM LocalAE WHERE ID = @localAEID

			-- check if there is entry in tagrule table
			SET @tagRuleID = -1
			SET @tagFilterID = -1

			SELECT @tagRuleID = ID FROM TagRule WHERE value = @AETitle AND comparatorID = @comparatorID AND DICOMTagID =@localAETagID 
			IF (@tagRuleID > 0) 
				BEGIN 
				-- tagfilter for this localAE exists
				SELECT @tagFilterID = ID FROM TagFilterRules WHERE tagRuleID = @tagRuleID
				END
			ELSE 
				BEGIN 
					INSERT INTO TagRule VALUES(@localAETagID, @comparatorID, @AETitle)
					SELECT @tagRuleID=ID FROM tagRule WHERE Value = @AETitle

					IF(@tagRuleID > 0)
					BEGIN
						SET @tmpTagFilterID = 0			
						-- make sure tagfilter name is unique
						WHILE (@tmpTagFilterID <> @tagFilterID)
						BEGIN 
							SET @tmpTagFilterID = @tagFilterID
							SET @AutoCounter = @AutoCounter +1 
							SET @tagFilterName =  'filter_'+ ltrim(STR(@AutoCounter))

							SELECT @tagFilterID = ID FROM PxDcmDB.dbo.TagFilter WHERE PxDcmDB.dbo.TagFilter.Name = @tagFilterName
							IF (@tagFilterID <> @tmpTagFilterID) continue
							ELSE break
						END

						INSERT INTO PxDcmDB.dbo.TagFilter VALUES(@tagFilterName, 'Populated')
						
						SELECT @tagFilterID=ID FROM PxDcmDB.dbo.TagFilter WHERE PxDcmDB.dbo.TagFilter.Name = @tagFilterName
					END	
					ELSE 
					    BEGIN
						FETCH NEXT FROM @c1 INTO @RoutingPatternID, @localAEID,@storeTargetID, @compressionMethod,@compressionFactor 
						Continue
					END
				END
			if(@tagFilterID >0)
			BEGIN
				-- TagFilterRules table
				IF NOT EXISTS (SELECT * FROM TagFilterRules WHERE tagFilterID=@tagFilterID AND tagRuleID=@tagRuleID) 
				INSERT INTO TagFilterRules Values(@tagFilterID, @tagRuleID)
				
				-- TagBasedRoutingPatternEntry
				INSERT INTO PxDcmDB.dbo.TagBasedRoutingPatternEntry VALUES( @RoutingPatternID, @tagFilterID, @storeTargetID, @compressionMethod,@compressionFactor )

				-- TagFilterGroupAssignment
				IF NOT EXISTS (SELECT * FROM TagFilterGroupAssignment WHERE TagFilterID = @tagFilterID) INSERT INTO TagFilterGroupAssignment SELECT @tagFilterID, GroupID FROM LocalAEGroupAssignment l WHERE l.localAEID = @localAEID
			END
 

			FETCH NEXT FROM @c1 INTO @RoutingPatternID, @localAEID,@storeTargetID, @compressionMethod,@compressionFactor 
		END
		CLOSE @c1
	END -- end of transfer from RoutingPatternEntry to TagBasedRoutingPatternEntry table

	-- case 2
	BEGIN
		SET @c1 =CURSOR LOCAL SCROLL FOR
		SELECT LocalAEID, groupID, AETitle FROM PxDcmDB.dbo.LocalAEGroupAssignment a INNER JOIN PxDcmDB.dbo.LocalAE l ON a.LocalAEID = l.ID FOR READ ONLY

		OPEN @c1
		FETCH NEXT FROM @c1 INTO @localAEID, @groupID, @AETitle 

		WHILE @@FETCH_STATUS = 0
		BEGIN
			-- check if the localAE exists in tagRule table
			SET @tagRuleID = -1
			SELECT @tagRuleID = ID FROM TagRule WHERE value = @AETitle AND comparatorID = @comparatorID AND DICOMTagID =@localAETagID 
			IF (@tagRuleID < 0) 
			BEGIN 
				INSERT INTO TagRule VALUES(@localAETagID, @comparatorID, @AETitle)
				SELECT @tagRuleID = ID FROM TagRule WHERE value = @AETitle AND comparatorID = @comparatorID AND DICOMTagID =@localAETagID 
			END
			if(@tagRuleID < 0) 
			BEGIN 
				FETCH NEXT FROM @c1 INTO @localAEID, @groupID,@AETitle
				continue
			END
			-- check tagFilterID in tagFilterRules based on tagRuleID. only one entry for population case
			SET @tagFilterID = -1
			SELECT @tagFilterID = tagFilterID FROM TagFilterRules WHERE tagRuleID=@tagRuleID
			if(	@tagFilterID < 0)
			BEGIN 
				-- create tagfilter for this localAE
				SET @tmpTagFilterID = 0
				SET @tagFilterID = -1
				-- make sure tagfilter name is unique
				WHILE (@tmpTagFilterID <> @tagFilterID)
				BEGIN 
					SET @tmpTagFilterID = @tagFilterID
					SET @AutoCounter = @AutoCounter +1 
					SET @tagFilterName =  'filter_'+ ltrim(STR(@AutoCounter))

					SELECT @tagFilterID = ID FROM PxDcmDB.dbo.TagFilter WHERE PxDcmDB.dbo.TagFilter.Name = @tagFilterName
					IF (@tagFilterID <> @tmpTagFilterID) continue
					ELSE break
				END

				INSERT INTO PxDcmDB.dbo.TagFilter VALUES(@tagFilterName, 'Populated')				
				SELECT @tagFilterID=ID FROM PxDcmDB.dbo.TagFilter WHERE PxDcmDB.dbo.TagFilter.Name = @tagFilterName

				IF NOT EXISTS (SELECT * FROM TagFilterRules WHERE tagFilterID=@tagFilterID AND tagRuleID=@tagRuleID) 
				INSERT INTO TagFilterRules Values(@tagFilterID, @tagRuleID)
			END
		 	if(@tagFilterID < 0) 	
			BEGIN 
				FETCH NEXT FROM @c1 INTO @localAEID, @groupID,@AETitle
				continue
			END
			-- TagFilterGroupAssignment
			-- SELECT @groupID = UserGroupID FROM UserGroup WHERE Name = @groupName
			 
			IF NOT EXISTS (SELECT * FROM TagFilterGroupAssignment WHERE TagFilterID = @tagFilterID AND GroupID = @groupID) 
			INSERT INTO TagFilterGroupAssignment VALUES (@tagFilterID, @groupID)
			 
			FETCH NEXT FROM @c1 INTO @localAEID, @groupID,@AETitle
		END
		CLOSE @c1
	END	-- end of case 2  

	DROP TABLE RoutingPatternEntry

END -- end of creating TagBasedRoutingPatternEntry table


GO

-- verify and update DB to patched version
USE PxDcmDB
if	( EXISTS (select * from PxDcmHistDB..AqObject) 
	and EXISTS (select * from AqNETHISTDB.information_schema.tables where table_name = 'PatientStudy' and table_type = 'BASE TABLE')
	and EXISTS (SELECT * FROM AqNETHISTDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'Description' AND Table_Name ='PatientSeries')
	and EXISTS (select * from PxDcmHistDB..Actions) 
	and EXISTS (select * from PxDcmHistDB.information_schema.tables where table_name ='StudyEventLog' and table_type = 'BASE TABLE')

	and EXISTS (select * from information_schema.tables where table_name ='StudyReadStatus' and table_type = 'BASE TABLE')
	and EXISTS (select * from information_schema.tables where table_name ='SeriesReadStatus' and table_type = 'BASE TABLE')
	and EXISTS (SELECT DepartmentID FROM UserGroup WHERE Name='AqNET_Public') 
	and EXISTS (SELECT * FROM PxDcmDB.INFORMATION_SCHEMA.COLUMNS WHERE COLUMN_NAME = 'LogItemsPerPage' AND Table_Name ='WebConfiguration') 
	and EXISTS (SELECT * FROM PxDcmDB.dbo.SYSOBJECTS WHERE ID = OBJECT_ID(N'PrefetchPatternAE'))

	)

	begin
	Update PxDcmDB.dbo.dbinfo Set MajorVersion=1, MinorVersion=6, BuildVersion=1,BuildMinorVersion=0 
	print 'success on patching database version from 1.5.0.0 to 1.6.1.0'
	end
else
	begin
	RAISERROR ('failed on verify, abort to set version to 1.6.1.0', 20, 1) with log	
	end
GO


