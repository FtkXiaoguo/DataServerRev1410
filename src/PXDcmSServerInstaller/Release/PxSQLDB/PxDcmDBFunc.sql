-- patch database first
USE PxDcmDB
GO

-- new tables or new initializations -------------

IF NOT EXISTS (select * from PxDcmDB.dbo.dbinfo  
	Where  MajorVersion=1 and MinorVersion=8 and BuildVersion=0 and BuildMinorVersion=0 )
begin	
	-- quit script
	RAISERROR ('Wrong version database or version patching failed, abort!', 20, 1) with log

end
GO

USE PxDcmHistDB

ALTER DATABASE PxDcmHistDB SET ARITHABORT ON 

--begin of MakeAqObjectType

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

	SELECT ID FROM dbo.AqObjectType WHERE TypeName=@TypeName
	
END
--end of MakeAqObjectType
GO

--begin of MakeAqObject
print 'create procedure MakeAqObject'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeAqObject' AND type = 'P')
   DROP PROCEDURE MakeAqObject
GO

CREATE PROCEDURE MakeAqObject( 
  @Type 		INT,
  @EntityName	VARCHAR(64),
  @FullName 	VARCHAR(128),
  @Hostname		VARCHAR(64),
  @Address		VARCHAR(64),
  @Port 		INT = 0,
  @DomainName 	VARCHAR(128) = '',
  @Description 	VARCHAR(256) = ''
)

AS
BEGIN
	SET NOCOUNT ON

	declare @id int
	SET @id = 0

	SELECT @id=ID FROM dbo.AqObject WHERE Type=@Type and EntityName=@EntityName and FullName=@FullName 
			and Hostname=@Hostname and Address=@Address and Port=@Port and DomainName=@DomainName

	If @id = 0

	begin
		INSERT INTO dbo.AqObject(
		Type,
		EntityName,
		FullName,
		Hostname,
		Address,
		Port,
		DomainName,
		Description
		) 
		VALUES (
		@Type,
		@EntityName,
		@FullName,
		@Hostname,
		@Address,
		@Port,
		@DomainName,
		@Description
		)

		SELECT @id=ID FROM dbo.AqObject WHERE Type=@Type and EntityName=@EntityName and FullName=@FullName 
				and Hostname=@Hostname and Address=@Address and Port=@Port and DomainName=@DomainName
	end
	
	SELECT @id

END
--end of MakeAqObject
GO


-- view for AE Object
print 'create view for AE Object'
IF EXISTS (select * from information_schema.tables 
	where table_name ='AEObject' and  table_type = 'VIEW')
   DROP VIEW AEObject
GO


CREATE VIEW AEObject (ID, AEType, AE_Title, AEName, Hostname, IPAddress, Port, Description)
WITH   SCHEMABINDING 
AS

SELECT o.ID, CASE WHEN TypeName='LocalAE' THEN 1 ELSE 0 end as IsLocalAE, EntityName, 
	FullName, Hostname, Address, Port, o.Description
	FROM dbo.AqObject o Join dbo.AqObjectType on dbo.AqObjectType.ID=o.Type
	Where TypeName= 'LocalAE' or TypeName= 'RemoteAE' or TypeName= '_Null_'
GO

--CREATE UNIQUE CLUSTERED INDEX AEObjectID_index on AEObject(ID)
--GO



--begin of MakeAEObject
print 'create procedure MakeAEObject'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeAEObject' AND type = 'P')
   DROP PROCEDURE MakeAEObject
GO

CREATE PROCEDURE MakeAEObject( 
  @IsLocalAE 	INT,
  @AE_Title		VARCHAR(64),
  @AEName 		VARCHAR(128),
  @Hostname		VARCHAR(64),
  @IPAddress	VARCHAR(64),
  @Port 		INT,
  @Description 	VARCHAR(256) = ''
)

AS
BEGIN
	
	SET NOCOUNT ON
	declare @otype INT
		
	SET @otype = 0
	IF @IsLocalAE <> 0
		SET @otype = (Select ID From AqObjectType Where TypeName='LocalAE')
	ELSE
		SET @otype = (Select ID From AqObjectType Where TypeName='RemoteAE')

	EXEC MakeAqObject @otype, @AE_Title, @AEName, @Hostname, @IPAddress, @Port, '', @Description

END
--end of MakeAEObject
GO

-- view for User Object
print 'create view UserObject'
IF EXISTS (select * from information_schema.tables 
	where table_name ='UserObject' and  table_type = 'VIEW')
   DROP VIEW UserObject
GO


CREATE VIEW UserObject (ID, Username, LastName, FirstName, Email, Description, DomainName)
WITH   SCHEMABINDING 
AS

SELECT o.ID, EntityName, FullName, Hostname, Address, o.Description, DomainName
	FROM dbo.AqObject o Join dbo.AqObjectType on dbo.AqObjectType.ID=o.Type
	Where TypeName= 'UserAccount' or TypeName= '_Null_'
GO

--begin of MakeUserObject
print 'create procedure MakeUserObject'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeUserObject' AND type = 'P')
   DROP PROCEDURE MakeUserObject
GO

CREATE PROCEDURE MakeUserObject( 
	@Username 	VARCHAR(20),
	@LastName 	VARCHAR(32),
	@FirstName 	VARCHAR(32),
	@Email 		VARCHAR(64),
	@DomainName	VARCHAR(128) = '',
	@Description VARCHAR(128) = ''
	)

AS
BEGIN
	SET NOCOUNT ON
	Declare @ID 			INT
	Declare @oEmail 		VARCHAR(64)
	Declare @oDescription 	VARCHAR(64)
	Declare @typeID			INT
	
	SET @ID = 0
	SELECT @ID=ID, @oEmail=Email, @oDescription=Description FROM dbo.UserObject 
		WHERE Username=@Username and LastName=@LastName and FirstName=@FirstName and DomainName=@DomainName 
		
	If @ID = 0
	begin 
		Select @typeID=ID FROM dbo.AqObjectType Where TypeName= 'UserAccount'

		if(@LastName is null)
			SET @LastName = ''

		if(@FirstName is null)
			SET @FirstName = ''

		INSERT INTO dbo.AqObject(
		Type,
		EntityName,
		FullName,	
		Hostname,
		Address,
		DomainName,
		Description
		) 
		VALUES (
		@typeID,
		@Username,
		@LastName,
		@FirstName,
		@Email,
		@DomainName,
		@Description
		)
	end
	Else
	begin
		If @oEmail <> @Email or @oDescription <> @Description
			Update dbo.UserObject Set Email=@Email, Description = @Description WHERE ID=@ID
	end
	
	SELECT ID FROM dbo.UserObject WHERE Username=@Username and LastName=@LastName 
		and FirstName=@FirstName and DomainName=@DomainName 

END
--end of MakeUserObject
GO


print 'create procedure MakeSeriesAttribute'
IF EXISTS (SELECT name FROM sysobjects WHERE name = 'MakeSeriesAttribute' AND type = 'P')
	DROP PROCEDURE MakeSeriesAttribute
GO

CREATE PROCEDURE MakeSeriesAttribute(
	@Name      VARCHAR(64),
	@Type 		int,
	@SubType 	VARCHAR(64),
	@ProcessName VARCHAR(64),
	@ProcessType VARCHAR(64),
	@Description 	VARCHAR(256) = ''
)
AS
BEGIN
	SET NOCOUNT ON
		
	declare @id int
	SET @id = 0

	Select @id = ID FROM dbo.SeriesAttribute 
		Where Name=@Name and Type=@Type and subtype=@SubType

	if @id = 0
	begin	
	INSERT INTO dbo.SeriesAttribute(Name, Type, SubType, ProcessName, ProcessType, Description) 
		VALUES (@Name, @Type, @SubType, @ProcessName, @ProcessType, @Description)
	end
	
	SELECT @id
END
--end of MakeSeriesAttribute
GO


--begin of MakePatient
-- if the SeriesInstanceUID is empty, only study record is checked
-- The returned query is studyindex, otherwise series record will be checked
-- and return query is seriesindex

print 'create procedure MakePatient'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakePatient' AND type = 'P')
   DROP PROCEDURE MakePatient
GO


CREATE PROCEDURE MakePatient  (
	@StudyInstanceUID VARCHAR(64), 
	@PatientsName VARCHAR(332), 
	@PatientID VARCHAR(64), 
	@PatientsBirthDate VARCHAR(10), 
	@PatientsSex	VARCHAR(16), 
	@StudyDate VARCHAR(10), 
	@StudyTime VARCHAR(16), 
	@AccessionNumber VARCHAR(16), 
	@StudyID VARCHAR(16), 
	@ReferringPhysiciansName VARCHAR(332), 
	@SeriesInstanceUID VARCHAR(64), 
	@SeriesNumber INT, 
	@Modality VARCHAR(16),
	@SeriesInstances INT,
	@TransferSyntax INT,
	@QRFlag tinyint,
	@SeriesDescription VARCHAR(64), 
	@Attribute INT = 0,
	@SourceAE INT = 0 -- '_NULL_' object
	)
AS
BEGIN
	SET NOCOUNT ON
	declare @StudyIndex INT
	declare @SeriesIndex INT

	SET @StudyIndex = 0

	SELECT @StudyIndex=StudyIndex FROM dbo.PatientStudy 
		WHERE StudyInstanceUID=@StudyInstanceUID and PatientsName=@PatientsName and 
		PatientID=@PatientID and PatientsBirthDate=@PatientsBirthDate and  
		PatientsSex=@PatientsSex and StudyDate=@StudyDate and StudyTime=@StudyTime and 
		AccessionNumber=@AccessionNumber and StudyID=@StudyID
		

	IF @StudyIndex = 0
	begin
		INSERT INTO dbo.PatientStudy(
		StudyInstanceUID, 
		PatientsName, 
		PatientID, 
		PatientsBirthDate, 
		PatientsSex, 
		StudyDate, 
		StudyTime, 
		AccessionNumber, 
		StudyID, 
		ReferringPhysiciansName
		) 
		VALUES (
		@StudyInstanceUID,
		@PatientsName,
		@PatientID,
		@PatientsBirthDate, 
		@PatientsSex, 
		@StudyDate, 
		@StudyTime, 
		@AccessionNumber, 
		@StudyID, 
		@ReferringPhysiciansName
		)

		SELECT @StudyIndex=StudyIndex FROM dbo.PatientStudy 
			WHERE StudyInstanceUID=@StudyInstanceUID and PatientsName=@PatientsName and 
			PatientID=@PatientID and PatientsBirthDate=@PatientsBirthDate and  
			PatientsSex=@PatientsSex and StudyDate=@StudyDate and StudyTime=@StudyTime and 
			AccessionNumber=@AccessionNumber and StudyID=@StudyID
	end

	IF @SeriesInstanceUID = '' or @StudyIndex = 0
	begin
		
		SELECT @StudyIndex, 0 
		RETURN -- stop for study level only
	end

	SET @SeriesIndex = 0

	SELECT @SeriesIndex=SeriesIndex FROM dbo.PatientSeries 
		WHERE StudyIndex=@StudyIndex and SeriesInstanceUID=@SeriesInstanceUID and 
		SeriesNumber=@SeriesNumber and Modality=@Modality
	IF @SeriesIndex = 0
	begin
		IF @QRFlag <> 0 SET @QRFlag = 1
		INSERT INTO dbo.PatientSeries (
		StudyIndex,
		SeriesInstanceUID, 
		SeriesNumber, 
		Modality,
		SeriesInstances,
		TransferSyntax,
		QRFlag,
		Description,
		Attribute,
		SourceAE
		) 
		VALUES (
		@StudyIndex,
		@SeriesInstanceUID, 
		@SeriesNumber, 
		@Modality,
		@SeriesInstances,
		@TransferSyntax,
		@QRFlag,
		@SeriesDescription,
		@Attribute,
		@SourceAE
		)

		SELECT @SeriesIndex=SeriesIndex FROM dbo.PatientSeries 
			WHERE StudyIndex=@StudyIndex and SeriesInstanceUID=@SeriesInstanceUID and 
			SeriesNumber=@SeriesNumber and Modality=@Modality
	end

	SELECT @StudyIndex, @SeriesIndex

END
--end of MakePatient
GO


IF EXISTs (SELECT name FROM sysobjects WHERE name ='MakeActions' AND type='p')
DROP PROCEDURE MakeActions
GO 

Create PROCEDURE MakeActions(@ActionName VARCHAR (64), @Description VARCHAR ( 256))
AS
BEGIN 
	SET NOCOUNT ON

	IF NOT Exists (Select ID from Actions WHERE ActionName = @ActionName)
		INSERT INTO Actions (ActionName, Description) Values(@ActionName, @Description)
	Select ID from Actions WHERE ActionName = @ActionName
END
GO


-- SerieEventLogView
print 'create view SerieEventLogView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'SerieEventLogView' AND type = 'V')
   DROP VIEW SerieEventLogView
GO

CREATE VIEW SerieEventLogView
AS
SELECT DISTINCT sel.*, pss.studyIndex,pss.seriesIndex, pst.patientsName,pst.PatientID,pst.PatientsSex, pst.PatientsBirthDate,
pst.StudyID, pst.AccessionNumber, pst.ReferringPhysiciansName,pst.studyDate, pss.seriesInstanceUID, pss.seriesNumber, 
pss.seriesInstances, pss.Modality 
FROM PxDcmHistDB.dbo.patientStudy pst 
JOIN PxDcmHistDB.dbo.patientSeries pss ON pst.studyIndex =  pss.studyIndex
JOIN PxDcmHistDB.dbo.SeriesEventLog sel ON sel.Acton = pss.seriesIndex
GO

-- TransferToView
print 'create view TransferToView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'TransferToView' AND type = 'V')
   DROP VIEW TransferToView
GO

CREATE VIEW TransferToView
AS
SELECT DISTINCT  pss.seriesIndex, pst.patientsName 
FROM PxDcmHistDB.dbo.patientStudy pst 
JOIN PxDcmHistDB.dbo.patientSeries pss ON pst.studyIndex =  pss.studyIndex
 
GO


print 'create view RequestorView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'RequestorView' AND type = 'V')
   DROP VIEW RequestorView
GO

CREATE VIEW RequestorView
AS
SELECT o.ID, TypeName, EntityName, 
	FullName, Hostname, Address, Port, DomainName, o.Description
	FROM dbo.AqObject o Join dbo.AqObjectType on dbo.AqObjectType.ID=o.Type  
GO

print 'create view ActionFromView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'ActionFromView' AND type = 'V')
   DROP VIEW ActionFromView
GO

CREATE VIEW ActionFromView  (ID, TypeName, EntityName, FullName, HostName, IPAddress, Port, Description, DomainName)
AS
SELECT o.ID, TypeName, EntityName, 
	FullName, Hostname, Address, Port, DomainName, o.Description
	FROM dbo.AqObject o Join dbo.AqObjectType on dbo.AqObjectType.ID=o.Type  
GO

print 'create view ActionAtView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'ActionAtView' AND type = 'V')
   DROP VIEW ActionAtView
GO

CREATE VIEW ActionAtView  (ID, TypeName, EntityName, FullName, HostName, IPAddress, Port, Description, DomainName)
AS
SELECT o.ID, TypeName, EntityName, 
	FullName, Hostname, Address, Port, DomainName, o.Description
	FROM dbo.AqObject o Join dbo.AqObjectType on dbo.AqObjectType.ID=o.Type  
GO

-- view for get total events
print 'create view AllEvents'
IF EXISTS (select * from information_schema.tables where table_name ='AllEvents' and  table_type = 'VIEW')
   DROP VIEW AllEvents
GO


CREATE VIEW AllEvents
AS

SELECT 'Patient Study' as EventsType, s.*,
		d.PatientsName + ' ' + d.AccessionNumber + ' ' + d.StudyInstanceUID as 'On object'
from dbo.StudyEventLog s join dbo.PatientStudy d on d.StudyIndex = s.ActOn
 
UNION
SELECT 'Patient Series' as EventsType, s.*,
		d1.PatientsName + ' ' + d1.AccessionNumber + ' ' + d2.SeriesNumber + ' ' 
		+ d2.Modality + ' ' +	d2.SeriesInstanceUID as 'On object'
from dbo.SeriesEventLog s join dbo.PatientSeries d2 on d2.SeriesIndex = s.ActOn
join dbo.PatientStudy d1 on d2.StudyIndex = d1.StudyIndex

UNION

SELECT 'System' as EventsType,  s.*, 
	d.Type + ' ' + d.EntityName + ' ' + d.FullName + ' ' + d.Hostname as 'On object'
from dbo.sysEventLog s join dbo.AqObject d on d.id = s.ActOn


GO


USE PxDcmDB

ALTER DATABASE PxDcmDB SET ARITHABORT ON 

-- view for SeriesLevel with correct instance number
print 'create view SeriesView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'SeriesView' AND type = 'V')
   DROP VIEW SeriesView
GO


CREATE VIEW SeriesView
AS


SELECT  SeriesLevelID,
	s.StudyLevelID,
	s.SeriesInstanceUID, 
	s.SeriesNumber, 
	s.SeriesDescription, 
	s.Modality, 
	s.BodyPartExamined, 
	s.ViewPosition, 
	CASE WHEN s.NumberOfSeriesRelatedInstances<>0 
		THEN s.NumberOfSeriesRelatedInstances
	 	ELSE (SELECT COUNT(*) FROM InstanceLevel
		WHERE SeriesLevelID=s.SeriesLevelID)
	END
	as NumberOfSeriesRelatedInstances,

	s.StationName, 

	s.OfflineFlag,
	s.QRFlag,
	s.ModifyTime,
	s.HoldToDate,
	s.Status,
	s.SeriesDate, 
	s.SeriesTime
		
FROM SeriesLevel s
GO


--begin of MakeImportUserAccount
print 'create procedure MakeImportUserAccount'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeImportUserAccount' AND type = 'P')
   DROP PROCEDURE MakeImportUserAccount
GO

CREATE PROCEDURE MakeImportUserAccount( 
	@Username VARCHAR(20),
	@DomainName	VARCHAR(128),
	@LastName 	VARCHAR(32),
	@FirstName 	VARCHAR(32),
	@Email 		VARCHAR(64)
	)
AS
BEGIN
	SET NOCOUNT ON
	declare @DomainID INT
	declare @AccountID INT

	SET @DomainID = 0
	SET @AccountID = 0

	Select @DomainID=DomainID from DomainT Where Name=@DomainName and type <> 0
	IF @DomainID = 0
	BEGIN
		return
	END

	Select @AccountID=AccountID from UserAccount Where Username=@Username and DomainID=@DomainID
	IF @AccountID = 0
		INSERT UserAccount (DomainID, Username, Password, LastName, FirstName, Email) 
			VALUES (@DomainID, @Username, '', @LastName, @FirstName, @Email)
	
	Select AccountID, DomainID from UserAccount Where Username=@Username and DomainID=@DomainID
END

--end of MakeImportUserAccount
GO

--begin of MakeUserObjectFromUserAccount
print 'create procedure MakeUserObjectFromUserAccount'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeUserObjectFromUserAccount' AND type = 'P')
   DROP PROCEDURE MakeUserObjectFromUserAccount
GO

CREATE PROCEDURE MakeUserObjectFromUserAccount( 
	@UserID INT, 
	@Username VARCHAR(20) = '',
	@DomainID	INT = 0
	)

AS
BEGIN
	SET NOCOUNT ON
	declare @LastName 	VARCHAR(32)
	declare @MiddleName VARCHAR(32)
	declare @FirstName 	VARCHAR(32)
	declare @Email 		VARCHAR(64)
	declare @DomainName VARCHAR(128)
	declare @Description 	VARCHAR(128)
	declare @AccountID INT


	SET @AccountID = 0

	IF (@UserID > 0)
		SELECT @AccountID=AccountID, @Username=Username, @LastName=LastName, @MiddleName=MiddleName, 
			@FirstName=FirstName, @Email=Email, @Description=u.Description, @DomainName=d.Name
			FROM dbo.UserAccount u JOIN dbo.DomainT d ON d.DomainID=u.DomainID
			WHERE u.AccountID=@UserID
	ELSE
		SELECT @AccountID=AccountID, @LastName=LastName, @MiddleName=MiddleName, 
			@FirstName=FirstName, @Email=Email, @Description=u.Description, @DomainName=d.Name
			FROM dbo.UserAccount u JOIN dbo.DomainT d ON d.DomainID=u.DomainID
			WHERE Username=@Username and u.DomainID = @DomainID
	
	IF @AccountID <> 0
		EXEC PxDcmHistDB.dbo.MakeUserObject @Username, @LastName, 
			@FirstName, @Email, @DomainName, @Description


END
--end of MakeUserObjectFromUserAccount
GO


--begin of MakeAEObjectFromSystem
print 'create procedure MakeAEObjectFromSystem'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeAEObjectFromSystem' AND type = 'P')
   DROP PROCEDURE MakeAEObjectFromSystem
GO

CREATE PROCEDURE MakeAEObjectFromSystem( 
	@ID 		INT,
	@AEName		VARCHAR(128),
	@IsLocal	INT = 0
)

AS
BEGIN
	SET NOCOUNT ON

	declare @AETitle		VARCHAR(64)
	declare @HostName		VARCHAR(128)
	declare @IPAddress		VARCHAR(32)
	declare @Port			INT
	declare @Description	VARCHAR(128)
			

	SET @AETitle = ''

	IF @ID > 0
	begin
		IF @IsLocal <> 0
			SELECT @AEName=AEName, @AETitle=AETitle, @HostName=HostName, @IPAddress=IPAddress,
				  @Port=Port, @Description=Description 
				  FROM dbo.LocalAE WHERE ID=@ID
		ELSE
			SELECT @AEName=AEName, @AETitle=AETitle, @HostName=HostName, @IPAddress=IPAddress,
				  @Port=Port, @Description=Description 
				  FROM dbo.RemoteAE WHERE ID=@ID
	end
	ELSE
	begin
		IF @IsLocal <> 0
			SELECT @AETitle=AETitle, @HostName=HostName, @IPAddress=IPAddress,
				  @Port=Port, @Description=Description 
				  FROM dbo.LocalAE WHERE AEName=@AEName
		ELSE
			SELECT @AETitle=AETitle, @HostName=HostName, @IPAddress=IPAddress,
				  @Port=Port, @Description=Description 
				  FROM dbo.RemoteAE WHERE AEName=@AEName
	end
		
	IF @AETitle <> ''
		EXEC PxDcmHistDB.dbo.MakeAEObject @IsLocal, @AETitle, @AEName, @HostName,
				@IPAddress, @Port, @Description

END
--end of MakeAEObjectFromSystem
GO

--begin of MakePatientFromSeries
print 'create procedure MakePatientFromSeries'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakePatientFromSeries' AND type = 'P')
   DROP PROCEDURE MakePatientFromSeries
GO

CREATE PROCEDURE MakePatientFromSeries  (
	@SeriesLevelUID VARCHAR(64), 
	@SeriesLevelID INT,
	@SourceAE INT = 0 -- '_NULL_' object
	)
AS
BEGIN
	SET NOCOUNT ON

	declare @StudyInstanceUID VARCHAR(64)
	declare @PatientsName VARCHAR(332)
	declare @PatientID VARCHAR(64)
	declare @PatientsBirthDate VARCHAR(10)
	declare @PatientsSex	VARCHAR(16)
	declare @StudyDate VARCHAR(10)
	declare @StudyTime VARCHAR(16)
	declare @AccessionNumber VARCHAR(16)
	declare @StudyID VARCHAR(16)
	declare @ReferringPhysiciansName VARCHAR(332)
	declare @SeriesInstanceUID VARCHAR(64)
	declare @SeriesNumber INT
	declare @Modality VARCHAR(16)
	declare @SeriesInstances INT
	declare @TransferSyntax INT
	declare @QRFlag tinyint
	declare @Attribute INT
	declare @SeriesDescription VARCHAR(64)
	--declare @SourceAE INT default 0,

	declare @Name VARCHAR(64)
	declare @Type          INT
	declare @Subtype		VARCHAR(64)
	declare @ProcessName VARCHAR(64)
	declare @ProcessType VARCHAR(64)

	IF @SeriesLevelUID <> ''
		begin
		
		SET @SeriesLevelID = 0

		SELECT  @StudyInstanceUID=StudyInstanceUID, @PatientsName=PatientsName, 
			@PatientID=PatientID, @PatientsBirthDate=PatientsBirthDate,
			@PatientsSex=PatientsSex, @StudyDate=StudyDate, @StudyTime=StudyTime, 
			@AccessionNumber=AccessionNumber, @StudyID=StudyID,
			@ReferringPhysiciansName=ReferringPhysiciansName, 
			@SeriesLevelID=SeriesLevelID, @SeriesNumber=SeriesNumber, 
			@Modality=Modality, @QRFlag=QRFlag, @SeriesInstances=NumberOfSeriesRelatedInstances,
			@SeriesDescription=SeriesDescription
			FROM dbo.StudyLevel JOIN dbo.SeriesLevel 
			ON dbo.StudyLevel.StudyLevelID = dbo.SeriesLevel.StudyLevelID
			WHERE dbo.SeriesLevel.SeriesInstanceUID=@SeriesLevelUID

		IF @SeriesLevelID = 0
		return

		SET @SeriesInstanceUID = @SeriesLevelUID

		end
	ELSE

		begin
		
		SET @SeriesInstanceUID = ''

		SELECT  @StudyInstanceUID=StudyInstanceUID, @PatientsName=PatientsName, 
			@PatientID=PatientID, @PatientsBirthDate=PatientsBirthDate,
			@PatientsSex=PatientsSex, @StudyDate=StudyDate, @StudyTime=StudyTime, 
			@AccessionNumber=AccessionNumber, @StudyID=StudyID,
			@ReferringPhysiciansName=ReferringPhysiciansName, 
			@SeriesInstanceUID=SeriesInstanceUID, @SeriesNumber=SeriesNumber, 
			@Modality=Modality, @QRFlag=QRFlag, @SeriesInstances=NumberOfSeriesRelatedInstances,
			@SeriesDescription=SeriesDescription
			FROM dbo.StudyLevel JOIN dbo.SeriesLevel 
			ON dbo.StudyLevel.StudyLevelID = dbo.SeriesLevel.StudyLevelID
			WHERE dbo.SeriesLevel.SeriesLevelID=@SeriesLevelID

		IF @SeriesInstanceUID = ''
			return

		end


	-- get transfer method
	SET @TransferSyntax = 0
	Select top 1 @TransferSyntax=TransferSyntax 
		From dbo.InstanceView Where SeriesLevelID=@SeriesLevelID
	
	-- get attribute if the series is private data, otherwise set attribute to 0 for dummy record
	SET @Attribute = 0
	SELECT TOP 1 @Attribute=ID, @Name=Name, @Type=Type, @Subtype=Subtype, @ProcessName=ProcessName,
		@ProcessType=ProcessType FROM dbo.PrivateData WHERE AuxSeriesUID=@SeriesInstanceUID
	-- check if this series is private data
	IF @Attribute <> 0
	begin
		SET @Attribute = 0
		Select @Attribute=ID From PxDcmHistDB.dbo.SeriesAttribute
			Where Name=@Name and Type=@Type and Subtype=@Subtype
		IF @Attribute = 0
		begin
			Insert PxDcmHistDB.dbo.SeriesAttribute Values(@Name, @Type, @Subtype, 
			@ProcessName, @ProcessType, 'auto inserted attribute')
			Select @Attribute=ID From PxDcmHistDB.dbo.SeriesAttribute
				Where Name=@Name and Type=@Type and Subtype=@Subtype
		end
	end

	
	EXEC PxDcmHistDB.dbo.MakePatient  
		@StudyInstanceUID, 
		@PatientsName, 
		@PatientID, 
		@PatientsBirthDate, 
		@PatientsSex, 
		@StudyDate, 
		@StudyTime, 
		@AccessionNumber, 
		@StudyID, 
		@ReferringPhysiciansName, 
		@SeriesInstanceUID, 
		@SeriesNumber, 
		@Modality,
		@SeriesInstances,
		@TransferSyntax,
		@QRFlag,
		@SeriesDescription,
		@Attribute,
		@SourceAE
	

END
--end of MakePatientFromSeries
GO


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


print 'create procedure UpdateStudyModality'
IF EXISTS (SELECT name FROM sysobjects
      WHERE name = 'UpdateStudyModality' AND type = 'P')
   DROP PROCEDURE UpdateStudyModality
GO
CREATE PROC UpdateStudyModality
@filter	varchar(6)	= 'FILTER'
AS
BEGIN
DECLARE @studyLID			int
DECLARE @ModalitiesInStudy varchar(128)

SET 	NOCOUNT ON
SET 	STATISTICS IO OFF

	IF ((@filter = '') OR ((@filter != 'FILTER') AND (@filter != 'ALL'))) RETURN

	DECLARE @c_study 		CURSOR


	if @filter = 'FILTER'
	begin
		SET @c_study = CURSOR LOCAL SCROLL FOR
		SELECT StudyLevelID FROM dbo.StudyLevel WHERE ModalitiesInStudy = '' OR ModalitiesInStudy IS NULL
		FOR READ ONLY
	end
	else if(@filter = 'ALL')
	begin
		SET @c_study = CURSOR LOCAL SCROLL FOR
		SELECT StudyLevelID FROM dbo.StudyLevel
		FOR READ ONLY
	end
	else
	begin
		RETURN
	end
	
	OPEN @c_study
	FETCH NEXT FROM @c_study INTO @studyLID
	WHILE @@FETCH_STATUS = 0
	BEGIN
	
		SET @ModalitiesInStudy=''
		SELECT @ModalitiesInStudy = @ModalitiesInStudy + CASE WHEN @ModalitiesInStudy='' THEN '' ELSE '/' END + 
		ISNULL(LTRIM(RTRIM(Modality)), '') FROM dbo.SeriesLevel WHERE StudyLevelID=@studyLID AND Status = 0 GROUP BY Modality
	
		UPDATE dbo.StudyLevel SET 
		ModalitiesInStudy = LEFT(@ModalitiesInStudy, 16)
		WHERE StudyLevelID=@studyLID
	
	
		FETCH NEXT FROM @c_study INTO @studyLID
	END
	
	CLOSE @c_study
	DEALLOCATE @c_study
END
GO
-- end of UpdateStudyModality

-- no trigger for deleting, because when study is not read or read, removing one series
--  will not change the status.


-- begin of procedure SetUserReadStatus
print 'create procedure SetUserReadStatus'
IF EXISTS (SELECT name FROM sysobjects
      WHERE name = 'SetUserReadStatus' AND type = 'P')
   DROP PROCEDURE SetUserReadStatus
GO

CREATE PROCEDURE  SetUserReadStatus (
	@SeriesUID varchar(64),
	@UserID INT,
	@Status INT
)

AS 

BEGIN
	SET NOCOUNT ON
	DECLARE @StudyLevelID INT
	DECLARE @SeriesLevelID INT
	DECLARE @readStatus INT

	SET @SeriesLevelID=0
	Select @StudyLevelID=StudyLevelID, @SeriesLevelID=SeriesLevelID 
		From SeriesLevel Where SeriesInstanceUID=@SeriesUID

	IF @SeriesLevelID = 0
		Return

	SET @readStatus = -99
	
	SELECT @readStatus = Status FROM SeriesReadStatus 
		WHERE SeriesLevelID=@SeriesLevelID and UserID=@UserID
	
	-- has same read status, no updating need
	IF @readStatus = @Status
		return

	IF @readStatus = -99
		INSERT INTO SeriesReadStatus (SeriesLevelID, UserID, Status)
			VALUES (@SeriesLevelID, @UserID, @Status )
	ELSE
		UPDATE SeriesReadStatus SET Status=@Status 
			WHERE SeriesLevelID = @SeriesLevelID and UserID = @UserID

	EXEC UpdateStudyReadStatus @StudyLevelID, @UserID

END

GO
-- end of SetUserReadStatus


-- view for get GetUserStudies
print 'create view StudyReadStatusView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'StudyReadStatusView' AND type = 'V')
   DROP VIEW StudyReadStatusView
GO


CREATE VIEW StudyReadStatusView
AS

/*
SELECT  DISTINCT AccountID, StudyLevel.StudyLevelID, 
dbo.QueryStudyReadStatus(AccountID, StudyLevel.StudyLevelID) As Status
FROM dbo.StudyLevel CROSS JOIN dbo.UserAccount
*/

SELECT  DISTINCT AccountID, StudyLevel.StudyLevelID, 
CASE WHEN dbo.StudyReadStatus.Status IS NULL 
THEN 0 ELSE dbo.StudyReadStatus.Status END as Status
FROM (dbo.StudyLevel CROSS JOIN dbo.UserAccount) LEFT JOIN 
dbo.StudyReadStatus on StudyLevel.StudyLevelID=StudyReadStatus.StudyLevelID 
and StudyReadStatus.UserID=UserAccount.AccountID

GO

-- view for get Group Study
print 'create view GroupStudyView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'GroupStudyView' AND type = 'V')
   DROP VIEW GroupStudyView
GO

CREATE VIEW GroupStudyView
AS
SELECT  DISTINCT StudyLevel.StudyLevelID, GroupSeries.GroupID
FROM dbo.StudyLevel JOIN dbo.SeriesLevel ON StudyLevel.StudyLevelID = SeriesLevel.StudyLevelID
JOIN dbo.GroupSeries ON GroupSeries.SeriesLevelID = SeriesLevel.SeriesLevelID

GO

-- view for get GetUserStudies
print 'create view UserStudyView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'UserStudyView' AND type = 'V')
   DROP VIEW UserStudyView
GO

CREATE VIEW UserStudyView
AS
--Select from local member table
SELECT  
	CASE WHEN PrivateDataReference.AuxRefStudyUID IS NULL THEN 0 ELSE 1 END AS HasAux, 
	GroupStudyView.GroupID AS GroupID, -- can be NULL
	StudyReadStatusView.AccountID AS UserID,
	StudyReadStatusView.Status as ReadStatus, 
	StudyLevel.StudyInstanceUID,
	PatientsName, 
	PatientID, 
	PatientsBirthDate, 
	PatientsSex, 
	StudyDate, 
	StudyTime, 
	AccessionNumber, 
	StudyID, 
	ReadingPhysiciansName,
	ReferringPhysiciansName, 
	ModalitiesInStudy, 
	StudyDescription, 
	NumberOfStudyRelatedSeries,
	NumberOfStudyRelatedInstances,
	CharacterSet,

	StudyLevel.StudyLevelID
FROM dbo.StudyLevel JOIN dbo.StudyReadStatusView ON StudyLevel.StudyLevelID = StudyReadStatusView.StudyLevelID
LEFT OUTER JOIN dbo.GroupStudyView ON StudyLevel.StudyLevelID = GroupStudyView.StudyLevelID 
LEFT OUTER JOIN dbo.PrivateDataReference ON StudyLevel.StudyInstanceUID = PrivateDataReference.AuxRefStudyUID 

GO

print 'create function GetSeriesAuxMask '
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'GetSeriesAuxMask' AND type = 'FN')
   DROP FUNCTION GetSeriesAuxMask
GO

CREATE FUNCTION GetSeriesAuxMask  (@seriesUID varchar(64))
RETURNS int
AS
BEGIN
	DECLARE @auxMask INT
	SET @auxMask = 0
	DECLARE @auxDatatype INT
	SET @auxDatatype = 0
	
	DECLARE @c_AuxSeries CURSOR 
	SET @c_AuxSeries =CURSOR LOCAL SCROLL FOR
	SELECT DISTINCT Type FROM dbo.PrivateData WHERE AuxSeriesUID=@seriesUID 
	FOR READ ONLY
	
	DECLARE @c_AuxRefSeries CURSOR
	SET @c_AuxRefSeries =CURSOR LOCAL SCROLL FOR
	SELECT DISTINCT PrivateData.Type FROM  dbo.PrivateData JOIN
		dbo.PrivateDataReference ON PrivateData.ID = PrivateDataReference.PrivateDataID 
		JOIN dbo.SeriesLevel ON PrivateDataReference.AuxRefSeriesUID =
		SeriesLevel.SeriesInstanceUID WHERE SeriesLevel.SeriesInstanceUID=@seriesUID
		FOR READ ONLY

	OPEN @c_AuxSeries
	FETCH NEXT FROM @c_AuxSeries INTO @auxDatatype
	IF (@@FETCH_STATUS <> -1)
	BEGIN
		SET @auxMask = 8192 	-- 8192 -> kpRTVSIsAux	= (1<<13)
		WHILE @@FETCH_STATUS = 0
		BEGIN
			SET @auxMask = @auxMask|@auxDatatype
			FETCH NEXT FROM @c_AuxSeries INTO @auxDatatype
		END
	END
	ELSE
	BEGIN
		OPEN @c_AuxRefSeries
		FETCH NEXT FROM @c_AuxRefSeries INTO @auxDatatype
		WHILE @@FETCH_STATUS = 0
		BEGIN
			SET @auxMask = @auxMask|@auxDatatype
			FETCH NEXT FROM @c_AuxRefSeries INTO @auxDatatype
		END
		CLOSE @c_AuxRefSeries
	END
	
	CLOSE @c_AuxSeries
	DEALLOCATE @c_AuxSeries
	DEALLOCATE @c_AuxRefSeries
	
	RETURN(@auxMask)
END
GO


print 'create function GetSeriesReadStatus'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'GetSeriesReadStatus' AND type = 'FN')
   DROP FUNCTION GetSeriesReadStatus
GO

CREATE FUNCTION GetSeriesReadStatus (@userID INT, @seriesUID varchar(64))
RETURNS int
AS
BEGIN
	DECLARE @readStatus INT
	SET @readStatus = 0
	
	SELECT @readStatus = u.Status
	FROM dbo.SeriesReadStatus u join dbo.SeriesLevel s 
		on s.SeriesLevelID = u.SeriesLevelID
	WHERE s.SeriesInstanceUID =@seriesUID AND u.UserID = @userID

	RETURN (@readStatus)
END
GO


-- view for get GetUserSeries
print 'create view SeriesReadStatusView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'SeriesReadStatusView' AND type = 'V')
   DROP VIEW SeriesReadStatusView
GO


CREATE VIEW SeriesReadStatusView
AS

SELECT  DISTINCT AccountID, SeriesLevel.SeriesLevelID, 
CASE WHEN dbo.SeriesReadStatus.Status IS NULL 
THEN 0 ELSE dbo.SeriesReadStatus.Status END as Status
FROM (dbo.SeriesLevel CROSS JOIN dbo.UserAccount) LEFT JOIN 
dbo.SeriesReadStatus on SeriesLevel.SeriesLevelID=SeriesReadStatus.SeriesLevelID
and SeriesReadStatus.UserID=UserAccount.AccountID

GO

print 'create view UserAssignedGroupView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'UserAssignedGroupView' AND type = 'V')
   DROP VIEW UserAssignedGroupView
GO

CREATE VIEW UserAssignedGroupView
AS
SELECT dbo.UserDefaultGroup.*, 1 as DefaultGroup from dbo.UserDefaultGroup
UNION ALL
SELECT dbo.UserOtherGroup.* , 0 as DefaultGroup from dbo.UserOtherGroup

GO

-- view for get GetUserSeries
print 'create view UserSeriesView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'UserSeriesView' AND type = 'V')
   DROP VIEW UserSeriesView
GO

CREATE VIEW UserSeriesView
AS
--Select from local member table

SELECT DISTINCT
	GroupSeries.GroupID AS GroupID, -- can be NULL
	SeriesReadStatusView.AccountID AS UserID,
	SeriesReadStatusView.Status as ReadStatus, 
	--dbo.GetSeriesAuxMask(SeriesInstanceUID) AS AuxMask,
	StudyInstanceUID,
	PatientsName, 
	PatientID, 
	PatientsBirthDate, 
	PatientsSex, 
	StudyDate, 
	StudyTime, 
	AccessionNumber, 
	StudyID, 
	ReadingPhysiciansName,
	ReferringPhysiciansName, 
	ModalitiesInStudy, 
	StudyDescription, 
	SeriesInstanceUID, 
	SeriesDate,
	SeriesTime,
	SeriesNumber, 
	SeriesDescription, 
	Modality, 
	BodyPartExamined,
	NumberOfSeriesRelatedInstances,	 
	
	OfflineFlag,
	ModifyTime,
	HoldToDate,
	s.Status,
	
	s.SeriesLevelID

FROM dbo.SeriesView s JOIN dbo.SeriesReadStatusView ON s.SeriesLevelID = SeriesReadStatusView.SeriesLevelID
JOIN dbo.StudyLevel ON StudyLevel.StudyLevelID = s.StudyLevelID
LEFT JOIN dbo.GroupSeries ON GroupSeries.SeriesLevelID = s.SeriesLevelID

/*
FROM ((dbo.SeriesLevel JOIN dbo.StudyLevel ON StudyLevel.StudyLevelID = SeriesLevel.StudyLevelID) 
CROSS JOIN dbo.UserAccount
LEFT JOIN dbo.GroupSeries ON GroupSeries.SeriesLevelID = SeriesLevel.SeriesLevelID) 
LEFT JOIN dbo.UserAssignedGroupView ON GroupSeries.GroupID=UserAssignedGroupView.GroupID
LEFT JOIN dbo.SeriesReadStatus ON SeriesLevel.SeriesInstanceUID = SeriesReadStatus.SeriesUID AND
UserAccount.AccountID = SeriesReadStatus.UserID
*/
GO


-- view for get GetUserSeriesX
print 'create view UserSeriesViewX'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'UserSeriesViewX' AND type = 'V')
   DROP VIEW UserSeriesViewX
GO

CREATE VIEW UserSeriesViewX
AS

SELECT DISTINCT
	dbo.GetSeriesAuxMask(SeriesInstanceUID) AS AuxMask,
	UserSeriesView.*

FROM UserSeriesView

GO


-- view for get UserReadStatusView
print 'create view UserReadStatusView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'UserReadStatusView' AND type = 'V')
   DROP VIEW UserReadStatusView
GO

CREATE VIEW UserReadStatusView

AS
select distinct GroupSeries.SerieslevelID, SeriesReadStatusView.Status, 
SeriesReadStatusView.AccountID from GroupSeries join UserAssignedGroupView 
on GroupSeries.GroupID = UserAssignedGroupView.GroupID  join SeriesReadStatusView
on SeriesReadStatusView.AccountID = UserAssignedGroupView.AccountID 
and SeriesReadStatusView.SeriesLevelID = GroupSeries.SeriesLevelID

GO


----------------------------------------------------

-- Stored procedures

--begin of UpdateStudyInfo
print 'create procedure UpdateStudyInfo'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'UpdateStudyInfo' AND type = 'P')
   DROP PROCEDURE UpdateStudyInfo
GO

CREATE PROCEDURE UpdateStudyInfo
   @studyLID int
AS 

SET NOCOUNT ON
-- UpdateStudyInfo will update NumberOfStudyRelatedSeries, ModalitiesInStudy, and 
-- NumberOfStudyRelatedInstances for study, and NumberOfSeriesRelatedInstances for Series
	

DECLARE @NumberOfStudyRelatedSeries int, @NumberOfStudyRelatedInstances int

SET @NumberOfStudyRelatedSeries=0
SET @NumberOfStudyRelatedInstances=0

SELECT @NumberOfStudyRelatedSeries = count(*), @NumberOfStudyRelatedInstances = 
	SUM(NumberOfSeriesRelatedInstances) FROM dbo.SeriesLevel WHERE StudyLevelID=@studyLID AND Status = 0

DECLARE @ModalitiesInStudy varchar(128)
SET @ModalitiesInStudy=''
SELECT @ModalitiesInStudy = @ModalitiesInStudy + CASE WHEN @ModalitiesInStudy='' THEN '' ELSE '/' END + 
	Modality FROM dbo.SeriesLevel WHERE StudyLevelID=@studyLID AND Status = 0 GROUP BY Modality


UPDATE dbo.StudyLevel SET 
NumberOfStudyRelatedSeries = @NumberOfStudyRelatedSeries,
NumberOfStudyRelatedInstances = @NumberOfStudyRelatedInstances,
ModalitiesInStudy = LEFT(@ModalitiesInStudy, 16)
WHERE StudyLevelID=@studyLID

GO
--end of UpdateStudyInfo


--begin of MakeStudy
print 'create procedure MakeStudy'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeStudy' AND type = 'P')
   DROP PROCEDURE MakeStudy
GO

CREATE PROCEDURE MakeStudy  (
	@StudyInstanceUID VARCHAR(64), 
	@PatientsName VARCHAR(332), 
	@PatientID VARCHAR(64), 
	@PatientsBirthDate VARCHAR(10), 
	@PatientsSex	VARCHAR(16), 
	@PatientsAge	INT, 
	@StudyDate VARCHAR(10), 
	@StudyTime VARCHAR(16), 
	@AccessionNumber VARCHAR(16), 
	@StudyID VARCHAR(16), 
	@ReadingPhysiciansName VARCHAR(332),
	@ReferringPhysiciansName VARCHAR(332), 
	@ModalitiesInStudy VARCHAR(64), 
	@StudyDescription VARCHAR(64), 
	@NumberOfStudyRelatedSeries INT, 
	@NumberOfStudyRelatedInstances INT, 
	@CharacterSet VARCHAR(16)
	)
	--Status INT not null)
AS
BEGIN
	SET NOCOUNT ON
	IF @NumberOfStudyRelatedSeries < 1
		SET @NumberOfStudyRelatedSeries = 1

	IF NOT EXISTS(SELECT StudyLevelID FROM dbo.StudyLevel 
		WHERE StudyInstanceUID=@StudyInstanceUID )
		INSERT INTO dbo.StudyLevel (
		StudyInstanceUID, 
		PatientsName, 
		PatientID, 
		PatientsBirthDate, 
		PatientsSex, 
		PatientsAge, 
		StudyDate, 
		StudyTime, 
		AccessionNumber, 
		StudyID, 
		ReadingPhysiciansName,
		ReferringPhysiciansName , 
		ModalitiesInStudy, 
		StudyDescription, 
		NumberOfStudyRelatedSeries, 
		NumberOfStudyRelatedInstances, 
		CharacterSet
		) 
		VALUES (
		@StudyInstanceUID,
		@PatientsName,
		@PatientID,
		@PatientsBirthDate, 
		@PatientsSex, 
		@PatientsAge, 
		@StudyDate, 
		@StudyTime, 
		@AccessionNumber, 
		@StudyID, 
		@ReadingPhysiciansName,
		@ReferringPhysiciansName, 
		@ModalitiesInStudy, 
		@StudyDescription, 
		@NumberOfStudyRelatedSeries, 
		@NumberOfStudyRelatedInstances, 
		@CharacterSet
		)
	
END
--end of MakeStudy
GO

--begin of MakeSeries
print 'create procedure MakeSeries'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeSeries' AND type = 'P')
   DROP PROCEDURE MakeSeries
GO

CREATE PROCEDURE MakeSeries  (
	@StudyLID INT,
	@SeriesInstanceUID VARCHAR(64), 
	@SeriesNumber INT, 
	@SeriesDescription VARCHAR(64),
	@Modality VARCHAR(16), 
	@BodyPartExamined VARCHAR(16), 
	@ViewPosition VARCHAR(16), 
	--@NumberOfSeriesRelatedInstances INT,
	@StationName VARCHAR(16),
	@SeriesDate VARCHAR(10),
	@SeriesTime VARCHAR(16),
	@Manufacturer VARCHAR(32)
	)
	--Status int not null,
	--ModifyTime datetime DEFAULT GETDATE(),
AS
BEGIN

	declare @NumberOfSeriesRelatedInstances int
	Set @NumberOfSeriesRelatedInstances = -99

	SELECT @NumberOfSeriesRelatedInstances=NumberOfSeriesRelatedInstances FROM dbo.SeriesLevel 
		WHERE SeriesInstanceUID=@SeriesInstanceUID

	IF (@NumberOfSeriesRelatedInstances = -99)
	begin
		INSERT INTO dbo.SeriesLevel (
		StudyLevelID,
		SeriesInstanceUID, 
		SeriesNumber, 
		SeriesDescription,
		Modality, 
		BodyPartExamined, 
		ViewPosition, 
		NumberOfSeriesRelatedInstances, 
		StationName,
		Status,
		SeriesDate,
		SeriesTime,
		Manufacturer
		) 
		VALUES (
		@StudyLID,
		@SeriesInstanceUID, 
		@SeriesNumber, 
		@SeriesDescription,
		@Modality, 
		@BodyPartExamined, 
		@ViewPosition, 
		0, --@NumberOfSeriesRelatedInstances, 
		@StationName,
		1, -- new series in pushing
		@SeriesDate,
		@SeriesTime,
		@Manufacturer
		)
		
		EXEC UpdateStudyInfo @StudyLID
	end
	else
	begin
		if(@NumberOfSeriesRelatedInstances > 0)
			Update 	dbo.SeriesLevel	Set NumberOfSeriesRelatedInstances=0
			WHERE SeriesInstanceUID=@SeriesInstanceUID
	end
	-- could add code to upadte read status from history

END	
--end of MakeSeries
GO

-- view for get GetInstance
print 'create view InstanceView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'InstanceView' AND type = 'V')
   DROP VIEW InstanceView
GO

CREATE VIEW InstanceView
AS
SELECT dbo.InstanceLevel.*, 'PxDcmDB' as DBName FROM dbo.InstanceLevel
-- delete PxDcmDB2 and PxDcmDB3 2010/03/15 By K.Ko
GO

--begin of MakeInstance
print 'create procedure MakeInstance'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeInstance' AND type = 'P')
   DROP PROCEDURE MakeInstance
GO

CREATE PROCEDURE MakeInstance  (
	@SOPInstanceUID VARCHAR(64),
	@SeriesLID int,
	@SOPClassUID VARCHAR(64),

	@TransferSyntax int,

	@InstanceNumber int,
	@Rows smallint,
	@Columns smallint,
	@NumberOfFrames smallint,
	@ImageType VARCHAR(64),
--#1 2012/02/10 K.Ko reduce the instanceLevel's field
	@PixelOffset int,
	@DataSize	int 
	)
AS
BEGIN
	SET NOCOUNT ON
	DECLARE @SOPClassID int
	DECLARE @err int
	
	-- since application will do the check, we dsiable check here to gain 100% speed
	--IF EXISTS(SELECT seriesLevelID FROM InstanceView Where seriesLevelID=@SeriesLID 
	--		AND SOPInstanceUID=@SOPInstanceUID)
	--	RETURN

	SET @SOPClassID = 0

	SELECT @SOPClassID=SOPClassID FROM dbo.SOPClassUIDs 
		WHERE SOPClassUID=@SOPClassUID 
	
	IF(@SOPClassID < 1)
	begin
		INSERT INTO	dbo.SOPClassUIDs (SOPClassUID) VALUES(@SOPClassUID)
		select @err = @@error
		if (@err != 0)
		begin
			RAISERROR ('Warning. Failed to insert in SOPClassIDs table.', 16, 1)
			RETURN
		end
	end

	IF(@SOPClassID < 1)
		SELECT @SOPClassID=SOPClassID FROM dbo.SOPClassUIDs
			WHERE SOPClassUID=@SOPClassUID 

	IF(@SOPClassID < 1)
	BEGIN
		RAISERROR ('Bad SOPClassID', 16, 1)
		RETURN
	END

	INSERT INTO dbo.InstanceLevel (
		SOPInstanceUID,
		SeriesLevelID,
		SOPClassID,
		
		TransferSyntax,

		InstanceNumber,
		Rows,
		Columns,
		NumberOfFrames,
		ImageType,
--#1 2012/02/10 K.Ko reduce the instanceLevel's field
		PixelOffset,
		DataSize
		)
		VALUES (
		@SOPInstanceUID,
		@SeriesLID,
		@SOPClassID,
	
		@TransferSyntax,

		@InstanceNumber,
		@Rows,
		@Columns,
		@NumberOfFrames,
		@ImageType,
--#1 2012/02/10 K.Ko reduce the instanceLevel's field
		@PixelOffset,
		@DataSize
	 
		)

	-- increase NumberOfSeriesRelatedInstances
	--UPDATE dbo.SeriesLevel SET NumberOfSeriesRelatedInstances = 
	--	(select 1+NumberOfSeriesRelatedInstances from dbo.SeriesLevel where SeriesLevelID = @SeriesLID)
	--	WHERE SeriesLevelID = @SeriesLID

END	
--end of MakeInstance
GO


--begin of UpdateStudyInfo
print 'create procedure UpdateStudyInfo'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'UpdateStudyInfo' AND type = 'P')
   DROP PROCEDURE UpdateStudyInfo
GO

CREATE PROCEDURE UpdateStudyInfo
   @studyLID int
AS 

SET NOCOUNT ON
-- UpdateStudyInfo will update NumberOfStudyRelatedSeries, ModalitiesInStudy, and 
-- NumberOfStudyRelatedInstances for study, and NumberOfSeriesRelatedInstances for Series
	

DECLARE @NumberOfStudyRelatedSeries int, @NumberOfStudyRelatedInstances int

SET @NumberOfStudyRelatedSeries=0
SET @NumberOfStudyRelatedInstances=0

SELECT @NumberOfStudyRelatedSeries = count(*), @NumberOfStudyRelatedInstances = 
	SUM(NumberOfSeriesRelatedInstances) FROM dbo.SeriesLevel WHERE StudyLevelID=@studyLID

DECLARE @ModalitiesInStudy varchar(128)
SET @ModalitiesInStudy=''
SELECT @ModalitiesInStudy = @ModalitiesInStudy + CASE WHEN @ModalitiesInStudy='' THEN '' ELSE '/' END + 
	Modality FROM dbo.SeriesLevel WHERE StudyLevelID=@studyLID GROUP BY Modality


UPDATE dbo.StudyLevel SET 
NumberOfStudyRelatedSeries = @NumberOfStudyRelatedSeries,
NumberOfStudyRelatedInstances = @NumberOfStudyRelatedInstances,
ModalitiesInStudy = LEFT(@ModalitiesInStudy, 16)
WHERE StudyLevelID=@studyLID

GO
--end of UpdateStudyInfo

--exec sp_helptext 'UpdateStudyInfo'


-- begin of OnSeriesCompleted
print 'create procedure OnSeriesCompleted'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'OnSeriesCompleted' AND type = 'P')
   DROP PROCEDURE OnSeriesCompleted
GO

CREATE PROCEDURE OnSeriesCompleted 
	@seriesUID varchar(64)
AS 
SET NOCOUNT ON
-- OnSeriesCompleted will update cached information in related study and series

DECLARE @studyLID INT
DECLARE @seriesLID INT
DECLARE @NumberOfSeriesRelatedInstances  INT

SELECT @studyLID=StudyLevelID, @seriesLID=SeriesLevelID FROM dbo.SeriesLevel 
	WHERE SeriesInstanceUID=@seriesUID

SELECT @NumberOfSeriesRelatedInstances=COUNT(*) FROM dbo.InstanceView WHERE SeriesLevelID=@seriesLID

IF @NumberOfSeriesRelatedInstances > 0
BEGIN
	UPDATE dbo.SeriesLevel SET 	NumberOfSeriesRelatedInstances = @NumberOfSeriesRelatedInstances, 
		ModifyTime = GETDATE(), Status = 0	WHERE SeriesLevelID=@seriesLID

	EXEC UpdateStudyInfo @studyLID
END
ELSE
BEGIN
	DELETE  dbo.SeriesLevel WHERE SeriesLevelID=@seriesLID
END

GO
-- end of OnSeriesCompleted


-- begin of trigger on series deleting
print 'create trigger on series deleting'
IF EXISTS (SELECT name FROM sysobjects
      WHERE name = 'TrigSeriesDelete' AND type = 'TR')
   DROP TRIGGER TrigSeriesDelete
GO

CREATE TRIGGER TrigSeriesDelete
on dbo.SeriesLevel
FOR DELETE
AS
BEGIN 
	-- removed updating aux infor to avoid lost aux info for Q/R series
	SET NOCOUNT ON
	DECLARE @studyLID INT
	DECLARE @seriesLID INT
	DECLARE @seriesUID varchar(64)
	DECLARE @auxID INT
	DECLARE @auxStudyUID varchar(64)
	DECLARE @c_series CURSOR
	SET @c_series =CURSOR LOCAL SCROLL FOR
	   SELECT  StudyLevelID, SeriesLevelID, SeriesInstanceUID FROM deleted
	   FOR READ ONLY

	OPEN @c_series
	FETCH NEXT FROM @c_series INTO @studyLID, @seriesLID, @seriesUID
	WHILE @@FETCH_STATUS = 0
	BEGIN
		-- delete aux data if it is a tempalte series
		SET @auxStudyUID=''
		SELECT TOP 1 @auxID=ID,@auxStudyUID=AuxStudyUID FROM dbo.PrivateData WHERE AuxSeriesUID=@seriesUID
		-- check if this series is template by compare to template study UID
		--IF(@auxStudyUID = '2.16.840.1.114053.2100.9.2')
			DELETE dbo.PrivateData WHERE ID=@auxID

		-- delete PxDcmDB2 and PxDcmDB3 2010/03/15 By K.Ko
	
		EXEC UpdateStudyInfo @studyLID
		DELETE dbo.StudyLevel WHERE StudyLevelID=@studyLID AND NumberOfStudyRelatedSeries=0
		
		FETCH NEXT FROM @c_series INTO @studyLID, @seriesLID, @seriesUID
	END

	CLOSE @c_series
	DEALLOCATE @c_series
END
   
GO
-- end of trigger on series deleting


-- display localAE and remoteAE togather, ID and AEName may not unique
print 'create view AE_UNION'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'AE_UNION' AND type = 'V')
   DROP VIEW AE_UNION
GO

CREATE VIEW AE_UNION
AS
SELECT 
	ID,  
	AEName,
	AETitle,
	HostName,
	IPAddress,
	Port,
	Level,
	Priority,
	Description,
	1 AS IsLocalAE
FROM dbo.LocalAE
UNION ALL
SELECT 
	ID,  
	AEName,
	AETitle,
	HostName,
	IPAddress,
	Port,
	Level,
	Priority,
	Description,
	0 AS IsLocalAE
FROM dbo.RemoteAE
GO

-- display localAE and remoteAE togather with AE option, ID and AEName may not unique
print 'create view AEView'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'AEView' AND type = 'V')
   DROP VIEW AEView
GO

CREATE VIEW AEView
AS

SELECT
	ID,   
	AEName,
	AETitle,
	HostName,
	IPAddress,
	Port,
	Level,
	Priority,
	Description,
	IsLocalAE,
	CASE WHEN QRAllowedAE.AETitleID IS NULL THEN 0 ELSE 1 END AS QRAllowed,
	CASE WHEN StoreTargetAE.AETitleID IS NULL THEN 0 ELSE 1 END AS StoreTarget,
	CASE WHEN   QRSourceAE.AETitleID IS NULL THEN 0 ELSE 1 END AS   QRSource

FROM dbo.AE_UNION 
LEFT JOIN dbo.QRAllowedAE ON AE_UNION.ID=QRAllowedAE.AETitleID
LEFT JOIN dbo.StoreTargetAE ON AE_UNION.ID=StoreTargetAE.AETitleID
LEFT JOIN dbo.QRSourceAE ON AE_UNION.ID=QRSourceAE.AETitleID

GO


--begin of MakeRemoreAE
print 'create procedure MakeRemoreAE'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeRemoreAE' AND type = 'P')
   DROP PROCEDURE MakeRemoreAE
GO

CREATE PROCEDURE MakeRemoreAE  (
	@AEName			VARCHAR(128),
	@AETitle		VARCHAR(64),
	@HostName		VARCHAR(128),
	@IPAddress		VARCHAR(32),
	@Port			INT,
	@Level			INT,
	@Priority		INT,
	@Description    VARCHAR(128),
	@AEStore		INT,
	@AEQRAllowed	INT,
	@AEQRSource		INT
	)
AS
BEGIN

	SET NOCOUNT ON	
	DECLARE @AEID int
	SET @AEID = 0

	BEGIN TRAN

	-- make sure same constrain record exists
	SELECT @AEID=ID FROM dbo.RemoteAE
		--WITH (HOLDLOCK, TABLOCKX)
		WHERE AETitle=@AETitle AND HostName=@HostName 
			AND IPAddress=@IPAddress AND Port = Port
	
	-- make sure no same AETitle record exists if store option checked
	IF(@AEID < 1 AND @AEStore > 0) 
	BEGIN
		SELECT @AEID=ID FROM dbo.RemoteAE
			WHERE AETitle=@AETitle
	END

	IF(@AEID < 1)
	BEGIN
		INSERT INTO dbo.RemoteAE (
		AEName,		
		AETitle,	
		HostName,	
		IPAddress,	
		Port,		
		Level,
		Priority,
		Description 
		) 
		VALUES (
		@AEName,		
		@AETitle,	
		@HostName,	
		@IPAddress,	
		@Port,		
		@Level,
		@Priority,
		@Description 
		)

		SELECT @AEID=ID FROM dbo.RemoteAE WHERE AETitle=@AETitle 
			AND HostName=@HostName AND IPAddress=@IPAddress AND Port = Port
				
		IF(@AEID > 0 AND @AEStore > 0) --
		BEGIN
			INSERT INTO dbo.StoreTargetAE (AETitleID) VALUES (@AEID)
		END

		IF(@AEID > 0 AND @AEQRAllowed > 0) --
		BEGIN
			INSERT INTO dbo.QRAllowedAE (AETitleID) VALUES (@AEID)
		END

		IF(@AEID > 0 AND @AEQRSource > 0) --
		BEGIN
			INSERT INTO dbo.QRSourceAE (AETitleID) VALUES (@AEID)
		END

	END
	COMMIT TRAN 

	-- return the RemoteAE ID
	--SELECT RemoteAEID=@AEID not work yet

END	
--end of MakeRemoreAE
GO

-- begin of Deleting trigger on UserAccount--putback this code from 1.6.6.1 on 2006-10-17 srini
print 'Deleting trigger on UserAccount'
IF EXISTS (SELECT name FROM sysobjects
      WHERE name = 'TrigUserAccountDelete' AND type = 'TR')
   DROP TRIGGER TrigUserAccountDelete
GO


print 'create procedure GetDBSizeInfo'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'GetDBSizeInfo' AND type = 'P')
   DROP PROCEDURE GetDBSizeInfo
GO

CREATE PROCEDURE GetDBSizeInfo(
 @database_size_KB dec(15,0) OUTPUT,
 @unallocated_space_KB dec(15,0) OUTPUT,
 @reserved_KB  dec(15,0) OUTPUT,
 @data_size_KB dec(15,0) OUTPUT,
 @index_size_KB dec(15,0) OUTPUT,
 @unused_KB dec(15,0) OUTPUT,
 @Instance_Rows int OUTPUT,
 @Instance_data_KB  dec(15,0) OUTPUT,
 @Instance_index_size_KB  dec(15,0) OUTPUT)
AS
BEGIN
	-- sp_spaceused @updateusage = true
	SET NOCOUNT ON
	declare @pages int   -- Working variable for size calc.
	declare @dbsize dec(15,0)
	declare @logsize dec(15)
	declare @bytesperpage dec(15,0)
	declare @pagesperMB  dec(15,0)
	declare @id int

	dbcc updateusage(0) with no_infomsgs
	create table #spt_space
	(
	 rows  int null,
	 reserved dec(15) null,
	 data  dec(15) null,
	 indexp  dec(15) null,
	 unused  dec(15) null 
	)

	set @database_size_KB = 0.
	set @unallocated_space_KB = 0.
	set @reserved_KB = 0.
	set @data_size_KB = 0.
	set @index_size_KB = 0.
	set @unused_KB = 0.
	set @Instance_Rows = 0
	set @Instance_data_KB = 0.
	set @Instance_index_size_KB = 0.

	select @dbsize = sum(convert(dec(15),size))
	from dbo.sysfiles
	where (status & 64 = 0)

	select @logsize = sum(convert(dec(15),size))
	from dbo.sysfiles
	where (status & 64 <> 0)

	select @bytesperpage = low
	from master.dbo.spt_values
	where number = 1
	and type = 'E'
	select @pagesperMB = 1048576 / @bytesperpage

	select @database_size_KB = 
		((@dbsize + @logsize) / @pagesperMB)* 1000,
		@unallocated_space_KB = 
		((@dbsize -  
		(select sum(convert(dec(15),reserved)) 
		 from sysindexes
		  where indid in (0, 1, 255)
		)) / @pagesperMB) * 1000

	--  Now calculate the summary data.  
	-- reserved: sum(reserved) where indid in (0, 1, 255)  

	insert into #spt_space (reserved)
	select sum(convert(dec(15),reserved))
	from sysindexes 
	where indid in (0, 1, 255)

	--data: sum(dpages) where indid < 2 + sum(used) where indid = 255 (text)
	select @pages = sum(convert(dec(15),dpages)) 
	from sysindexes 
	where indid < 2  
	select @pages = @pages + isnull(sum(convert(dec(15),used)), 0) 
	from sysindexes 
	where indid = 255 
	update #spt_space  
	set data = @pages  

	--index: sum(used) where indid in (0, 1, 255) - data
	update #spt_space 
	set indexp = (select sum(convert(dec(15),used)) 
	from sysindexes 
	 where indid in (0, 1, 255)) 
	   - data

	--unused: sum(reserved) - sum(used) where indid in (0, 1, 255)
	update #spt_space 
	set unused = reserved 
	- (select sum(convert(dec(15),used)) 
	 from sysindexes 
	  where indid in (0, 1, 255)) 

	select @reserved_KB = reserved * d.low / 1024.,
		@data_size_KB = data * d.low / 1024.,
		@index_size_KB = indexp * d.low / 1024.,
		@unused_KB = unused * d.low / 1024. 
		from #spt_space, master.dbo.spt_values d 
		where d.number = 1 
		and d.type = 'E'

	--Now calculate the summary data for instance level.

	select @id = id	from sysobjects 
	where id = object_id('InstanceLevel')

	update #spt_space 
	set rows = i.rows  
	from sysindexes i  
	where i.indid < 2  
	 and i.id = @id

	select @pages = sum(dpages) 
	from sysindexes 
	where indid < 2  
	 and id = @id
   
	select @pages = @pages + isnull(sum(used), 0) 
	from sysindexes 
	where indid = 255 
	and id = @id 
	update #spt_space 
	set data = @pages 


	update #spt_space  
	set indexp = (select sum(used) 
	from sysindexes 
	 where indid in (0, 1, 255) 
	  and id = @id) 
	   - data

	select @Instance_Rows = rows,
	@Instance_data_KB = data * d.low / 1024.,
	@Instance_index_size_KB = indexp * d.low / 1024. 
	from #spt_space, master.dbo.spt_values d 
	where d.number = 1 
	and d.type = 'E' 
END

GO


-- copy the same procedure to all DB, because procedure can not switch database

USE PxDcmHistDB
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'GetDBSizeInfo' AND type = 'P')
   DROP PROCEDURE GetDBSizeInfo

GO

USE PxDcmDB
DECLARE @X VARCHAR(4000)
SET @X = (SELECT Text FROM dbo.syscomments WHERE ID = OBJECT_ID('GetDBSizeInfo'))


USE PxDcmHistDB
print 'create procedure GetDBSizeInfo in PxDcmHistDB'
EXEC (@X)
GO


--begin of PackHistDB
print 'create procedure PackHistDB'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'PackHistDB' AND type = 'P')
   DROP PROCEDURE PackHistDB
GO

CREATE PROCEDURE PackHistDB (  @BackupDir VARCHAR(256) )

AS
BEGIN
	SET NOCOUNT ON

	declare @database_size_KB dec(15,0)
	declare @unallocated_space_KB dec(15,0)
	declare @reserved_KB  dec(15,0)
	declare @data_size_KB dec(15,0)
	declare @index_size_KB dec(15,0)
	declare @unused_KB dec(15,0)
	declare @Instance_Rows int
	declare @Instance_data_KB  dec(15,0)
	declare @Instance_index_size_KB  dec(15,0)


	declare @IsMSDE int
	Set @IsMSDE = PATINDEX('%Desktop Engine%', @@VERSION)
	
	--'Get PxDcmHistDB data size information'


	EXEC PxDcmHistDB.dbo.GetDBSizeInfo
		@database_size_KB OUTPUT,
		@unallocated_space_KB OUTPUT,
		@reserved_KB OUTPUT,
		@data_size_KB OUTPUT,
		@index_size_KB OUTPUT,
		@unused_KB OUTPUT,
		@Instance_Rows OUTPUT,  
		@Instance_data_KB OUTPUT,  
		@Instance_index_size_KB OUTPUT

	
	IF @database_size_KB > 1800000
		print	'database size: '+CONVERT(varchar(30), @database_size_KB)+' KB'
	
END
--end of PackHistDB
GO



USE PxDcmDB
GO

print 'create procedure MoveOneSeries'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MoveOneSeries' AND type = 'P')
   DROP PROCEDURE MoveOneSeries
GO

CREATE PROCEDURE MoveOneSeries(@SeriesID int, @dbNumber int)
AS
BEGIN
	SET NOCOUNT ON

	IF(@dbNumber != 2 AND @dbNumber != 3)
		return

	delete dbo.InstanceLevel Where SeriesLevelID = @SeriesID
	COMMIT TRAN 
END
GO

-- view for Group rights
print 'create view for Group rights'
IF EXISTS (select * from information_schema.tables 
	where table_name ='ViewGroupRights' and  table_type = 'VIEW')
   DROP VIEW ViewGroupRights
GO


CREATE VIEW ViewGroupRights (GroupID, RightsName, RightsValue)
WITH   SCHEMABINDING 
AS

SELECT r.GroupID, n.Name, v.RValue FROM dbo.UserGroupRights r join dbo.RightsName n 
	on r.RightsKeyID=n.NameID join dbo.RightsValue v on r.RightsValueID=v.ValueID
GO



--begin of SetGroupRights

IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'SetGroupRights' AND type = 'P')
   DROP PROCEDURE SetGroupRights
GO


CREATE PROCEDURE SetGroupRights(@GroupID INT, @RName VARCHAR(64), @RValue VARCHAR(800) )

AS
BEGIN
	declare @NameID INT,
			@ValueID INT
	SET NOCOUNT ON

	SET @NameID=0
	Select @NameID=NameID From RightsName Where Name=@RName
	
	if @NameID = 0
	begin
		INSERT INTO RightsName VALUES (@RName)
		Select @NameID=NameID From RightsName Where Name=@RName
	end


	SET @ValueID=0
	Select @ValueID=ValueID From RightsValue Where RValue=@RValue
	
	if @ValueID = 0
	begin
		INSERT INTO RightsValue VALUES (@RValue)
		Select @ValueID=ValueID From RightsValue Where RValue=@RValue
	end

	if exists (select * from UserGroupRights where GroupID=@GroupID and RightsKeyID=@NameID
		and RightsValueID=@ValueID)
	return

	if exists (select * from UserGroupRights where GroupID=@GroupID and RightsKeyID=@NameID)
		update UserGroupRights set RightsValueID=@ValueID where GroupID=@GroupID and RightsKeyID=@NameID
	else
		insert UserGroupRights VALUES(@GroupID, @NameID, @ValueID)
	
END
--end of SetGroupRights
GO



--begin of RemoveGroupRights

IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'RemoveGroupRights' AND type = 'P')
   DROP PROCEDURE RemoveGroupRights
GO


CREATE PROCEDURE RemoveGroupRights(@GroupID INT, @RName VARCHAR(64))

AS
BEGIN
	declare @NameID INT
	SET NOCOUNT ON

	SET @NameID=0
	Select @NameID=NameID From RightsName Where Name=@RName
	
	if @NameID > 0
		delete UserGroupRights where GroupID=@GroupID and RightsKeyID=@NameID
	
END
--end of RemoveGroupRights
GO


print 'create function GroupHasTheRights '
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'GroupHasTheRights' AND type = 'FN')
   DROP FUNCTION GroupHasTheRights
GO

CREATE FUNCTION GroupHasTheRights  (@GroupID INT, @RName VARCHAR(64), @RValue VARCHAR(800))
RETURNS int
AS
BEGIN
	declare @rights int	
	SET @rights = 0
	IF EXISTS (Select * from ViewGroupRights where GroupID=@GroupID and RightsName=@RName and RightsValue=@RValue)
		SET @rights = 1
	RETURN(@rights)
END
GO


print 'create function CanGroupAccessAllData '
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'CanGroupAccessAllData' AND type = 'FN')
   DROP FUNCTION CanGroupAccessAllData
GO

CREATE FUNCTION CanGroupAccessAllData  (@GroupID INT)
RETURNS int
AS
BEGIN
	declare @rights int	
	SET @rights = 0
	IF EXISTS (Select * from ViewGroupRights where GroupID=@GroupID and RightsName='AccessAllData' and RightsValue='1')
		SET @rights = 1
	RETURN(@rights)
END
GO

--begin of MakeDomainGroups
print 'create procedure MakeDomainGroups'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeDomainGroups' AND type = 'P')
   DROP PROCEDURE MakeDomainGroups
GO

CREATE PROCEDURE MakeDomainGroups( 
	@GroupName VARCHAR(64),
	@DomainName	VARCHAR(128),
	@SID VARBINARY(128)
	)
AS
BEGIN
	SET NOCOUNT ON
	declare @DomainID INT
	declare @UserGroupID INT


	SET @DomainID = 0
	SET @UserGroupID = 0

	Select @DomainID=DomainID from DomainT Where Name=@DomainName
	IF @DomainID = 0
	BEGIN
		return
	END

	IF NOT EXISTS (Select UserGroupID from UserGroup Where name=@Groupname and DomainID=@DomainID)
		INSERT UserGroup (DomainID, Name, Privilege, SID) VALUES(@DomainID, @GroupName, 7, @SID)
		
	Select UserGroupID from UserGroup Where name=@Groupname and DomainID=@DomainID

END

--end of MakeDomainGroups
GO

 
print 'create procedure MakeStudyAssignmentWithLocalAEAndGroup'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeStudyAssignmentWithLocalAEAndGroup' AND type = 'P')
   DROP PROCEDURE MakeStudyAssignmentWithLocalAEAndGroup
GO

CREATE PROCEDURE MakeStudyAssignmentWithLocalAEAndGroup (@LocalAEID int, @GroupID int )
AS
BEGIN
	SET NOCOUNT ON
	DECLARE @localAETag BIGINT
	DECLARE @localAETagID INT
	DECLARE @comparatorID INT
	DECLARE @tagFilterID INT
	DECLARE @tagRuleID INT
	DECLARE @AETitle VARCHAR(64)
	DECLARE @tagFilterName VARCHAR(64)
	DECLARE @AutoCounter INT
	DECLARE @tmpTagFilterID INT

	SET @AutoCounter = 0;
	SET @localAETag = 0x00020016
	SELECT @AETitle = AETitle FROM LocalAE WHERE ID = @LocalAEID
	SELECT @localAETagID = ID FROM DicomTag WHERE Tag = @localAETag
	SELECT @comparatorID = ID FROM Comparator WHERE OpString = 'is'

	-- 1. check if the localAE exists in tagRule table
	SET @tagRuleID = -1 
	SELECT @tagRuleID = ID FROM TagRule WHERE value = @AETitle AND comparatorID = @comparatorID AND DICOMTagID =@localAETagID 
	IF (@tagRuleID < 0) 
	BEGIN 
		INSERT INTO TagRule VALUES(@localAETagID, @comparatorID, @AETitle)
		SELECT @tagRuleID = ID FROM TagRule WHERE value = @AETitle AND comparatorID = @comparatorID AND DICOMTagID =@localAETagID 
	END

	-- 2. check tagFilterID in tagFilterRules based on tagRuleID. 
	SET @tagFilterID = -1
	SELECT @tagFilterID = tagFilterID FROM TagFilterRules INNER JOIN TagFilter ON TagFilter.ID = TagFilterRules.tagfilterID 
		WHERE tagRuleID=@tagRuleID AND TagFilter.Name like @AETitle+ltrim(STR(@LocalAEID)) +'%'
	if(@tagFilterID < 0)
	BEGIN 
		-- create tagfilter for this localAE
		SET @tmpTagFilterID = 0
		SET @tagFilterID = -1
		--  tagfilter name is based on local AE name. make sure tagfilter name is unique
		WHILE (@tmpTagFilterID <> @tagFilterID)
		BEGIN 
			SET @tmpTagFilterID = @tagFilterID
			
			SET @tagFilterName = @AETitle+ltrim(STR(@LocalAEID + @AutoCounter))
			SET @AutoCounter = @AutoCounter +1  
	
			SELECT @tagFilterID = ID FROM PxDcmDB.dbo.TagFilter WHERE PxDcmDB.dbo.TagFilter.Name = @tagFilterName
			IF (@tagFilterID <> @tmpTagFilterID) continue
			ELSE break
		END
		INSERT INTO PxDcmDB.dbo.TagFilter VALUES(@tagFilterName, 'AqNET studies assignment')				
		SELECT @tagFilterID=ID FROM PxDcmDB.dbo.TagFilter WHERE PxDcmDB.dbo.TagFilter.Name = @tagFilterName
	
		IF NOT EXISTS (SELECT * FROM TagFilterRules WHERE tagFilterID=@tagFilterID AND tagRuleID=@tagRuleID) 
		INSERT INTO TagFilterRules Values(@tagFilterID, @tagRuleID)
	END

	-- 3. tagfilter & group assignment
	IF NOT EXISTS (SELECT * FROM TagFilterGroupAssignment WHERE TagFilterID = @tagFilterID AND GroupID = @GroupID) 
		INSERT INTO TagFilterGroupAssignment VALUES (@tagFilterID, @GroupID)
			
END
--end of MakeStudyAssignmentWithLocalAEAndGroup
GO
 
print 'create view for localAE group Basic assignment'
IF EXISTS (select * from information_schema.tables 
	where table_name ='BasicLocalAEGroupAssignmentView' and  table_type = 'VIEW')
   DROP VIEW BasicLocalAEGroupAssignmentView
GO
CREATE VIEW BasicLocalAEGroupAssignmentView (groupID, localAEID)   
AS
SELECT tfga.groupID, LocalAE.ID from LocalAE 
INNER JOIN TagRule tr ON tr.value = localAE.AETitle
INNER JOIN tagfilterRules tfr ON tfr.tagruleID = tr.ID
INNER JOIN TagFilterGroupAssignment tfga ON tfga.tagfilterID = tfr.tagfilterID
INNER JOIN tagfilter tf ON tf.ID = tfr.tagfilterID
WHERE tf.name like localAE.AETitle +ltrim(STR(localAE.ID)) +'%' AND tr.comparatorID in (select ID from comparator where opstring = 'is' OR opstring = 'contains')
AND tr.DICOMTAGID = (select ID from DicomTag where tag = 0x00020016)
GO


print 'create view for localAE group Advanced assignment '
IF EXISTS (select * from information_schema.tables 
	where table_name ='AdvancedLocalAEGroupAssignmentView' and  table_type = 'VIEW')
   DROP VIEW AdvancedLocalAEGroupAssignmentView
GO
CREATE VIEW AdvancedLocalAEGroupAssignmentView (groupID, localAEID)   
AS
SELECT tfga.groupID, LocalAE.ID from LocalAE 
INNER JOIN TagRule tr ON tr.value = localAE.AETitle
INNER JOIN tagfilterRules tfr ON tfr.tagruleID = tr.ID
INNER JOIN TagFilterGroupAssignment tfga ON tfga.tagfilterID = tfr.tagfilterID
INNER JOIN tagfilter tf ON tf.ID = tfr.tagfilterID
WHERE tf.name not like localAE.AETitle +ltrim(STR(localAE.ID)) +'%' AND tr.comparatorID in (select ID from comparator where opstring = 'is' OR opstring = 'contains')
AND tr.DICOMTAGID = (select ID from DicomTag where tag = 0x00020016)
GO

print 'create procedure SP_DBSizeInfo'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'SP_DBSizeInfo' AND type = 'P')
   DROP PROCEDURE SP_DBSizeInfo
GO

CREATE PROCEDURE SP_DBSizeInfo(@DBName varchar(32))
AS
BEGIN

	declare @database_size_KB dec(15,0)
	declare @unallocated_space_KB dec(15,0)
	declare @reserved_KB  dec(15,0)
	declare @data_size_KB dec(15,0)
	declare @index_size_KB dec(15,0)
	declare @unused_KB dec(15,0)
	declare @Instance_Rows int
	declare @Instance_data_KB  dec(15,0)
	declare @Instance_index_size_KB  dec(15,0)

	declare @SQL nvarchar(1024)
	DECLARE @PDef NVARCHAR(500)

	if object_id(@DBName + N'.dbo.GetDBSizeInfo') is NOT NULL

	set @SQL = N'EXEC '+ @DBName + N'.dbo.GetDBSizeInfo @database_size_KB OUTPUT,
		@unallocated_space_KB OUTPUT,
		@reserved_KB OUTPUT,
		@data_size_KB OUTPUT,
		@index_size_KB OUTPUT,
		@unused_KB OUTPUT,
		@Instance_Rows OUTPUT,  
		@Instance_data_KB OUTPUT,  
		@Instance_index_size_KB OUTPUT'

	Set @PDef = N'@database_size_KB dec(15,0)  OUTPUT,
			@unallocated_space_KB dec(15,0) OUTPUT,
			@reserved_KB  dec(15,0) OUTPUT,
			@data_size_KB dec(15,0) OUTPUT,
			@index_size_KB dec(15,0) OUTPUT,
			@unused_KB dec(15,0) OUTPUT,
			@Instance_Rows int OUTPUT,
			@Instance_data_KB  dec(15,0) OUTPUT,
			@Instance_index_size_KB  dec(15,0) OUTPUT'


	exec sp_executesql @SQL, @PDef, @database_size_KB OUTPUT,
			@unallocated_space_KB OUTPUT,
			@reserved_KB OUTPUT,
			@data_size_KB OUTPUT,
			@index_size_KB OUTPUT,
			@unused_KB OUTPUT,
			@Instance_Rows OUTPUT,
			@Instance_data_KB OUTPUT,
			@Instance_index_size_KB OUTPUT

	select 	@database_size_KB as 'database_size_KB',
		@unallocated_space_KB as 'unallocated_space_KB',
		@reserved_KB as 'reserved_KB',
		@data_size_KB as 'data_size_KB',
		@index_size_KB as 'index_size_KB',
		@unused_KB as 'unused_KB',
		@Instance_Rows as 'Instance_Rows',  
		@Instance_data_KB as 'Instance_data_KB',  
		@Instance_index_size_KB as 'Instance_index_size_KB'
END
GO



print 'create procedure GetInstanceSizeInfo'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'GetInstanceSizeInfo' AND type = 'P')
   DROP PROCEDURE GetInstanceSizeInfo
GO

CREATE PROCEDURE GetInstanceSizeInfo(@DBName varchar(32), 
	@InstanceSize int OUTPUT,  
	@table_size_KB dec(15,0) OUTPUT)
AS
BEGIN

	declare @database_size_KB dec(15,0)
	declare @unallocated_space_KB dec(15,0)
	declare @reserved_KB  dec(15,0)
	declare @data_size_KB dec(15,0)
	declare @index_size_KB dec(15,0)
	declare @unused_KB dec(15,0)
	declare @Instance_Rows int
	declare @Instance_data_KB  dec(15,0)
	declare @Instance_index_size_KB  dec(15,0)

	declare @SQL nvarchar(1024)
	DECLARE @PDef NVARCHAR(500)
	set @SQL = N'EXEC '+ @DBName + N'.dbo.GetDBSizeInfo @database_size_KB OUTPUT,
		@unallocated_space_KB OUTPUT,
		@reserved_KB OUTPUT,
		@data_size_KB OUTPUT,
		@index_size_KB OUTPUT,
		@unused_KB OUTPUT,
		@Instance_Rows OUTPUT,  
		@Instance_data_KB OUTPUT,  
		@Instance_index_size_KB OUTPUT'

	Set @PDef = N'@database_size_KB dec(15,0)  OUTPUT,
			@unallocated_space_KB dec(15,0) OUTPUT,
			@reserved_KB  dec(15,0) OUTPUT,
			@data_size_KB dec(15,0) OUTPUT,
			@index_size_KB dec(15,0) OUTPUT,
			@unused_KB dec(15,0) OUTPUT,
			@Instance_Rows int OUTPUT,
			@Instance_data_KB  dec(15,0) OUTPUT,
			@Instance_index_size_KB  dec(15,0) OUTPUT'


	exec sp_executesql @SQL, @PDef, @database_size_KB OUTPUT,
			@unallocated_space_KB OUTPUT,
			@reserved_KB OUTPUT,
			@data_size_KB OUTPUT,
			@index_size_KB OUTPUT,
			@unused_KB OUTPUT,
			@Instance_Rows OUTPUT,
			@Instance_data_KB OUTPUT,
			@Instance_index_size_KB OUTPUT


	select @table_size_KB=@Instance_data_KB+@Instance_index_size_KB
	if(@Instance_Rows <= 0)
		SET @Instance_Rows = 1
	select @InstanceSize = (1000*@table_size_KB)/(@Instance_Rows)

END
GO



print 'create procedure DefragPxDcmDB'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'DefragPxDcmDB' AND type = 'P')
   DROP PROCEDURE DefragPxDcmDB
GO

CREATE PROCEDURE DefragPxDcmDB(@dbname varchar(64), @oneInstanceTopDBSize int, @CDriveSpaceMB int)
AS
BEGIN

	declare @InstanceSize int 
	declare @table_size_KB dec(15,0)
	declare @cmd varchar(1024)
	declare @Ncmd nvarchar(1024)
	declare @RetCode int

	print 'Check '+@dbname 
	EXEC PxDcmDB..GetInstanceSizeInfo @dbname, @InstanceSize output, @table_size_KB output

	print 'instance size: (table) '+ CONVERT(varchar(30), @table_size_KB) +'KB, (record) ' +
			CONVERT(varchar(30), @InstanceSize) +'bytes'

	if(@InstanceSize < @oneInstanceTopDBSize or @table_size_KB < 100000)
		return

	if((@CDriveSpaceMB-1000) < (@table_size_KB/1000))
		RAISERROR ('Not enough disk space in C dirve abort!', 20, 1) with log
		

	if exists (select program_name from master.dbo.sysprocesses where program_name='AQNetDICOMServer')
		RAISERROR ('DICOM server not shutdown,  abort!', 20, 1) with log
	
	if exists (select program_name from master.dbo.sysprocesses where program_name='AQNetImageServer')
		RAISERROR ('Image server not shutdown,  abort!', 20, 1) with log
	

	print '>>>>>>>>>>>  defrag '+@dbname+ ' at '+CONVERT(varchar, getdate(), 108)

	-- use raiseerror to flash print buffer	
	select @cmd= CONVERT(varchar, getdate(), 108)+' bulk copy to C:\'+@dbname+'.tbl'
	RAISERROR (@cmd, 0, 1) WITH NOWAIT 
	
	set @cmd = 'bcp "'+@dbname+'..InstanceLevel" out "C:\'+@dbname+'.tbl" -n -T -h TABLOCK' 
	exec @RetCode=master..xp_cmdshell @cmd, no_output 
	if (@RetCode <> 0)
		RAISERROR ('bulk copy failed abort!', 20, 1) with log

	print CONVERT(varchar, getdate(), 108)+' successed bulk copy'

	select @cmd= CONVERT(varchar, getdate(), 108)+' Clean '+@dbname
	RAISERROR (@cmd, 0, 1) WITH NOWAIT 

	set @ncmd = 'TRUNCATE TABLE '+@dbname+'..InstanceLevel'
	exec sp_executesql @ncmd
	
	select @cmd= CONVERT(varchar, getdate(), 108)+' Shrink '+@dbname
	RAISERROR (@cmd, 0, 1) WITH NOWAIT 

	set @ncmd = 'DBCC SHRINKDATABASE ( ' +@dbname+' )'
	exec sp_executesql @ncmd

	if (@dbname = 'PxDcmDB')
	begin
		select @cmd=@dbname+'..GroupSeries'		dbcc dbreindex(@cmd)
		select @cmd=@dbname+'..Serieslevel'		dbcc dbreindex(@cmd)
		select @cmd=@dbname+'..studylevel'		dbcc dbreindex(@cmd)
		exec sp_executesql @ncmd
	end
	
	DBCC SHRINKDATABASE (tempdb )
	
	select @cmd= CONVERT(varchar, getdate(), 108)+' Load data from C:\'+@dbname+'.tbl to '+@dbname
	RAISERROR (@cmd, 0, 1) WITH NOWAIT 

	set @ncmd = 'BULK INSERT '+@dbname+'..InstanceLevel FROM ''C:\'+ @dbname +
	   '.tbl'' WITH 
 	(
		DATAFILETYPE  = ''native'',
		BATCHSIZE  = 5000,
		MAXERRORS  = 1,
		TABLOCK  
	)'

	exec @RetCode=sp_executesql @ncmd
	if (@RetCode <> 0 or @@error <> 0)
		RAISERROR ('bulk insert failed abort! Data did not get in database, call for help !!!', 20, 1) with log
	
	select @cmd= CONVERT(varchar, getdate(), 108)+' Loaded data, delete file C:\'+@dbname+'.tbl'
	RAISERROR (@cmd, 0, 1) WITH NOWAIT 

	set @cmd = 'del C:\'+@dbname+'.tbl' 
	exec master..xp_cmdshell @cmd, no_output
	
	set @ncmd = 'DBCC SHRINKDATABASE ( ' +@dbname+' )'
	exec sp_executesql @ncmd

	EXEC PxDcmDB..GetInstanceSizeInfo @dbname, @InstanceSize output, @table_size_KB output

	print 'defraged instance size: (table) '+ CONVERT(varchar(30), @table_size_KB) +'KB, (record) ' +
	 CONVERT(varchar(30), @InstanceSize) +'bytes'

	select @cmd= '<<<<<<<<<<  Finshed defrag '+@dbname+ ' at '+CONVERT(varchar, getdate(), 108)
	RAISERROR (@cmd, 0, 1) WITH NOWAIT 


print ''

END
GO


-- view for patient privateData
print 'create view for patient AuxData'
IF EXISTS (select * from information_schema.tables 
	where table_name ='PatientAuxDataView' and  table_type = 'VIEW')
   DROP VIEW PatientAuxDataView
GO


CREATE VIEW PatientAuxDataView (PatientID, PatientsName, StudyInstanceUID, AccessionNumber, StudyDescription,
	SeriesInstanceUID, SeriesDescription, Modality, ModifyTime, SeriesDate, SeriesTime, VolumeID, ReferenceSortKey,
	PrivateDataID, AuxStudyUID, AuxSeriesUID, AuxSOPUID, Type, Name, Date, Subtype, ProcessName, ProcessType, VolumesHash, ParameterHash )
WITH   SCHEMABINDING 
AS

SELECT DISTINCT PatientID, PatientsName, StudyInstanceUID, AccessionNumber, StudyDescription,
	SeriesInstanceUID, SeriesDescription, Modality, ModifyTime, SeriesDate, SeriesTime, VolumeID, PKey,
	PrivateData.ID, AuxStudyUID, AuxSeriesUID, AuxSOPUID, Type, Name, Date, Subtype, ProcessName, ProcessType, VolumesHash, ParameterHash 
	FROM  dbo.PrivateData JOIN
	dbo.PrivateDataReference ON PrivateData.ID = PrivateDataReference.PrivateDataID 
	JOIN dbo.SeriesLevel ON PrivateDataReference.AuxRefSeriesUID =
	SeriesLevel.SeriesInstanceUID join dbo.Studylevel on SeriesLevel.studylevelID = StudyLevel.studylevelID
GO


-- view for data process job
print 'create view for data process job'
IF EXISTS (select * from information_schema.tables 
	where table_name ='DataProcessJobView' and  table_type = 'VIEW')
   DROP VIEW DataProcessJobView
GO


CREATE VIEW DataProcessJobView (JobID, JobName, JobDescription, ProcessOrder, ProcessorID,
		ProcessName, Handler,ProcessorDescription  )
WITH   SCHEMABINDING 
AS
	SELECT j.ID, j.JobName, j.Description, pl.JobOrder, p.id, p.ProcessName, 
		p.Handler, 	p.Description
	FROM dbo.DataProcessJob j join dbo.DataJobProcessList pl on
	j.ID = pl.JobID
	join dbo.DataProcessor p
	on pl.Processor = p.ID 
	
	-- can not use order by in view
	--order by pl.JobID, pl.JobOrder
GO

--- added by jwu 05/2006 ---
print 'create procedure MakeTagFilterProcessorAssignment'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeTagFilterProcessorAssignment' AND type = 'P')
   DROP PROCEDURE MakeTagFilterProcessorAssignment
GO

CREATE PROCEDURE MakeTagFilterProcessorAssignment (@TagFilterID int, @ProcessorName VARCHAR(128) )
AS
BEGIN 
	IF (@TagFilterID < 0 ) return -1
	DECLARE @tmpJobID INT
	DECLARE @jobID INT
	DECLARE @processorID INT
	DECLARE @count INT

	SET @tmpJobID = -1; 
	SET @jobID = -1; 
	SET @processorID = -1;
	SELECT @processorID = ID FROM PxDcmDB.dbo.DataProcessor WHERE ProcessName = @ProcessorName
	if(@processorID < 0) return -1; -- failed

	DECLARE @c_jobs CURSOR 
	SET @c_jobs =CURSOR LOCAL SCROLL FOR
	SELECT DISTINCT JobID FROM PxDcmDB.dbo.DataJobProcessList WHERE Processor = @processorID 
	FOR READ ONLY
	
	-- 1. check if any job contain this processor only   
	OPEN @c_jobs
	FETCH NEXT FROM @c_jobs INTO @tmpJobID
	WHILE @@FETCH_STATUS = 0
	BEGIN
		SET @count = 0
		SELECT @count = count(Processor) from dataJobProcessList WHERE JobID = @tmpJobID
		if (@count = 1) 
		BEGIN
			SET @jobID = @tmpJobID
			break;
		END
		FETCH NEXT FROM @c_jobs INTO @tmpJobID
	END
	CLOSE @c_jobs
	DEALLOCATE @c_jobs

	if(@jobID < 0 ) -- job contain this processor only is not existing, try to create one 
	BEGIN 
		SET @count = 0
		select @count = count(*) from dataProcessJob 
		if (@count > 0 )
			SELECT TOP 1 @tmpJobID = ID from dataProcessJob order by ID DESC
		else SET @tmpJobID = 0

		DECLARE @jobName varchar(64)
		SET @jobName = 'PJob' + CONVERT(varchar(25),  @tmpJobID+1) 

		INSERT INTO DataProcessJob VALUES (@jobName, 'job auto creation');
		SELECT  @jobID = ID from DataProcessJob WHERE JobName = @jobName
		IF (@jobID < 0) return -1 -- failed

		IF NOT EXISTS (SELECT ID FROM  DataJobProcessList WHERE JOBID = @jobID AND Processor = @processorID AND JobOrder = 1 ) 
			INSERT into DataJobProcessList values (@jobID, @processorID, 1)
	END

	-- so far @jobID is valid 
	IF NOT EXISTS (SELECT ID FROM DataProcessPattern WHERE TagFilterID = @TagFilterID AND Job = @jobID ) 
		INSERT into DataProcessPattern values (@TagFilterID, @jobID, '')
	return 0 -- successed
			
END
--end of MakeTagFilterProcessorAssignment
GO

print 'create procedure MakeTagFilterProcessorsAssignment'
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'MakeTagFilterProcessorsAssignment' AND type = 'P')
   DROP PROCEDURE MakeTagFilterProcessorsAssignment
GO

CREATE PROCEDURE MakeTagFilterProcessorsAssignment(@TagFilterID int, @ProcessorNames VARCHAR(1000), @Delimiter CHAR(1) )
AS
BEGIN 
	IF len(@Delimiter)<>1 return -1 -- failed
	IF (@TagFilterID < 0) return -1

	DECLARE @Start INT
	DECLARE @Length INT, @JobOrder INT
	DECLARE @ProcessorNumber INT

	set @Start=1
	Set @Start=ISNULL(CHARINDEX(@Delimiter, @ProcessorNames, @Start),0)	
	IF @Start=0 -- only one processor
		return EXEC MakeTagFilterProcessorAssignment @TagFilterID, @ProcessorNames

	-- split Processors into temp table
	DECLARE @TmpStr varchar (1000)
	SET @TmpStr = @ProcessorNames
	SET @JobOrder = 1
	
	DECLARE @SplitProcessor TABLE (processorID INT NOT NULL, jobOrder INT not null)
	DECLARE @processorID INT
	SET @processorID = 0

	while  (@JobOrder >= 1) -- borrow it as ture case 
	BEGIN
		SET @Start= 0
		SET @Length = len(@TmpStr)
		SET @Start=ISNULL(CHARINDEX(@Delimiter, @TmpStr, @Start),0)
		IF @Start=0 -- last processor case
		BEGIN
			SELECT @processorID = ID from dataProcessOr where processName =@TmpStr
			INSERT into @SplitProcessor values(@processorID, @JobOrder)
			break 
		END
		SELECT @processorID = ID from dataProcessOr where processName = SUBSTRING(@TmpStr, 0, @Start)
		INSERT into @SplitProcessor values(@processorID, @JobOrder )
		SELECT @TmpStr = SUBSTRING(@TmpStr, @Start+1, @Length - @Start)
		SET @JobOrder = @JobOrder +1
	END	
	SELECT @ProcessorNumber = count(*) from @SplitProcessor

	-- check if this is a job with processors + order
	DECLARE @jobID INT, @tmpJobID INT 
	SET @jobID = -1;
	DECLARE  @TTableProcessorID INT 
	DECLARE @processorCount INT

	DECLARE @c_jobs CURSOR 
	SET @c_jobs =CURSOR LOCAL SCROLL FOR
	SELECT DISTINCT ID FROM DataProcessJob 
	FOR READ ONLY

	OPEN @c_jobs
	FETCH NEXT FROM @c_jobs INTO @tmpJobID
	WHILE @@FETCH_STATUS = 0
	BEGIN
		SELECT @processorCount = count(Processor) from dataJobProcessList WHERE JobID = @tmpJobID
		if(@processorCount = @ProcessorNumber)
		BEGIN 
			SET @jobID = @tmpJobID
			SET @TTableProcessorID = 0
			SET @ProcessorID = 0
			SET @JobOrder =1
			WHile @processorCount >0
			BEGIN
				SELECT @ProcessorID = Processor from datajobProcessList where jobOrder = @JobOrder AND JobID = @tmpJobID
				SELECT @TTableProcessorID =processorID  from @SplitProcessor where jobOrder = @JobOrder
				if(@ProcessorID <> @TTableProcessorID)
				BEGIN 
					SET @jobID = -1
					break
				END
				SET @processorCount = @processorCount -1;
				SET @JobOrder = @JobOrder +1
			END
		END
		-- if the jobID is valid, then job is existing
		if(@jobID > 0) break;
		FETCH NEXT FROM @c_jobs INTO @tmpJobID
	END
	CLOSE @c_jobs
	DEALLOCATE @c_jobs

	--if(@jobID>0 ) print 'existing job found'
	DECLARE @count INT
	if(@jobID < 0 ) -- job contain processors only is not existing, try to create one 
	BEGIN 
		SET @count = 0
		select @count = count(*) from dataProcessJob 
		if (@count > 0 )
			SELECT TOP 1 @tmpJobID = ID from dataProcessJob order by ID DESC
		else SET @tmpJobID = 0

		DECLARE @jobName varchar(64)
		SET @jobName = 'PJob' + CONVERT(varchar(25),  @tmpJobID+1) 

		INSERT INTO DataProcessJob VALUES (@jobName, 'job auto creation');
		SELECT  @jobID = ID from DataProcessJob WHERE JobName = @jobName
		IF (@jobID < 0) return -1 -- failed
		--if(@jobID>0 ) print 'new job created'
		INSERT into DataJobProcessList (jobID, processor, jobOrder) 
			Select @jobID, processorID, jobOrder from @SplitProcessor 

	END

	-- so far jobID is valid
	IF NOT EXISTS (SELECT ID FROM DataProcessPattern WHERE TagFilterID = @TagFilterID AND Job = @jobID ) 
		INSERT into DataProcessPattern values (@TagFilterID, @jobID, '')
	return 0 -- successed
END
--end of MakeTagFilterProcessorsAssignment
GO

--- end jwu 05/2006 ----
