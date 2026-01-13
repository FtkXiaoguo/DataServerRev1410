--create dbinfo table
CREATE TABLE dbinfo
(
	MajorVersion INTEGER not null,
	MinorVersion INTEGER not null,
	BuildVersion INTEGER not null,
	BuildMinorVersion INTEGER not null,
	Note	VARCHAR(500) DEFAULT ''
);
-- Set DB to release version
INSERT INTO dbinfo(MajorVersion, MinorVersion, BuildVersion, BuildMinorVersion) VALUES (1,0,0,0);

--create StudyLevel table'
CREATE TABLE StudyLevel
(
	StudyLevelID INTEGER PRIMARY KEY AUTOINCREMENT,
	StudyInstanceUID VARCHAR(64) not null ,
	PatientsName VARCHAR(332), 
	PatientID VARCHAR(64) DEFAULT '', 
	PatientsBirthDate VARCHAR(10), 
	PatientsSex	VARCHAR(16), 
	PatientsAge	INTEGER, 
	StudyDate VARCHAR(10) DEFAULT '', 
	StudyTime VARCHAR(16) DEFAULT '', 
	AccessionNumber VARCHAR(16), 
	StudyID VARCHAR(16) DEFAULT '', 
	ReadingPhysiciansName VARCHAR(332),
	ReferringPhysiciansName VARCHAR(332), 
	ModalitiesInStudy VARCHAR(64), 
	StudyDescription VARCHAR(64), 
	NumberOfStudyRelatedSeries INTEGER, 
	NumberOfStudyRelatedInstances INTEGER, 
	CharacterSet VARCHAR(256), 
	Status INTEGER DEFAULT 0,
	AccessTime VARCHAR(16) DEFAULT ''
);
CREATE UNIQUE INDEX StudyInstanceUID_index on StudyLevel(StudyInstanceUID);
CREATE INDEX PatientsName_index on StudyLevel (PatientsName);
CREATE INDEX PatientID_index on StudyLevel (PatientID);
CREATE INDEX StudyDate_index on StudyLevel (StudyDate);
CREATE INDEX ModalitiesInStudy_index on StudyLevel (ModalitiesInStudy);
CREATE INDEX StudyAccessTime_index on StudyLevel (AccessTime);

--create SeriesLevel table
CREATE TABLE SeriesLevel
(
	SeriesLevelID	INTEGER PRIMARY KEY AUTOINCREMENT,
	-- use NO ACTION link to avoid CASCADE deleting too slow
	StudyLevelID INTEGER not null REFERENCES StudyLevel(StudyLevelID) ,
	SeriesInstanceUID VARCHAR(64) not null,
	SeriesNumber INTEGER, 
	SeriesDescription VARCHAR(64), 
	Modality VARCHAR(16), 
	BodyPartExamined VARCHAR(16), 
	ViewPosition VARCHAR(16), 
	NumberOfSeriesRelatedInstances INTEGER, 
	StationName VARCHAR(16), 
	OfflineFlag INTEGER DEFAULT 0 ,
	QRFlag tinyint DEFAULT 0 ,
	ModifyTime VARCHAR(16) DEFAULT  '',
	HoldToDate VARCHAR(16) DEFAULT  '',
	Status INTEGER DEFAULT 0,
	SeriesDate VARCHAR(10) DEFAULT '', 
	SeriesTime VARCHAR(16) DEFAULT '',
	Manufacturer VARCHAR(32)
);
CREATE UNIQUE INDEX SeriesInstanceUID_index on SeriesLevel (SeriesInstanceUID);
CREATE INDEX SeriesStudyLevelID_index on SeriesLevel (StudyLevelID);
CREATE INDEX SeriesModifyTime_index on SeriesLevel (ModifyTime);

-- create InstanceSOPID SOPClassUID table'
CREATE TABLE SOPClassUIDs
(
	SOPClassID	INTEGER PRIMARY KEY AUTOINCREMENT,
	SOPClassUID VARCHAR(64) not null
);
CREATE UNIQUE INDEX SOPClassUID_index on SOPClassUIDs (SOPClassUID);

--create InstanceLevel table
CREATE TABLE InstanceLevel
(
	InstanceLevelID INTEGER PRIMARY KEY AUTOINCREMENT,
	SOPInstanceUID VARCHAR(64),
	SeriesLevelID INTEGER not null REFERENCES SeriesLevel(SeriesLevelID) ,
	SOPClassID INTEGER not null REFERENCES SOPClassUIDs(SOPClassID),

	TransferSyntax INTEGER, -- IMPLICIT_LITTLE_ENDIAN = 100

	InstanceNumber INTEGER not null,
	Rows INTEGER not null,
	Columns INTEGER not null,
	NumberOfFrames INTEGER,

	ImageType VARCHAR(64),
	BitsAllocated INTEGER,
	BitsStored INTEGER,
	HighBit INTEGER,
	PixelRepresentation INTEGER,
	PhotometricInterpretation INTEGER,
	PlanarConfiguration INTEGER,

	WindowWidth float,
	WindowCenter float,
	SmallestPixelValue INTEGER,
	LargestPixelValue INTEGER,
	SamplesPerPixel INTEGER,

	PixelSpacingX float,
	PixelSpacingY float,
	AspectRatio float,
	RescaleSlope real,
	RescaleIntercept real,

	PatientOrientation VARCHAR(32),
	SlicePosition float,  --MC_ATT_RT_IMAGE_POSITION
	SliceThickness float,
	ImagePositionX float,
	ImagePositionY float,
	ImagePositionZ float,
	ImageOrientationPatientXX real,
	ImageOrientationPatientXY real,
	ImageOrientationPatientXZ real,
	ImageOrientationPatientYX real,
	ImageOrientationPatientYY real,
	ImageOrientationPatientYZ real,

	PixelOffset INTEGER,
	DataSize	INTEGER,
	referencedSOPInstanceUID VARCHAR(64),

	Status INTEGER DEFAULT 0,
	ImageDate VARCHAR(10) DEFAULT '', 
	ImageTime VARCHAR(16) DEFAULT '',
	WasLossyCompressed tinyint,
	ScanOptions VARCHAR(16)
);
CREATE INDEX InstanceSeriesLevelID_index on InstanceLevel (SeriesLevelID);
 
--create PrivateData table
CREATE TABLE PrivateData
(
  ID			INTEGER PRIMARY KEY AUTOINCREMENT,
  AuxStudyUID	VARCHAR(64) NOT NULL,
  AuxSeriesUID	VARCHAR(64) NOT NULL,
  AuxSOPUID		VARCHAR(64) NOT NULL,
  Type          INTEGER NOT NULL,
  Name			VARCHAR(64) NOT NULL, 
  Date			VARCHAR(16) NOT NULL,
  Subtype		VARCHAR(64) NOT NULL,
  ProcessName   VARCHAR(64) DEFAULT (''),
  ProcessType   VARCHAR(64) DEFAULT (''),
  VolumesHash  VARCHAR(64) DEFAULT (''),
  ParameterHash	VARCHAR(64) DEFAULT ('')
);
CREATE UNIQUE INDEX AuxSOPUID_index on PrivateData (AuxSOPUID, Type, Name, Subtype, ProcessName, ProcessType, VolumesHash, ParameterHash);
CREATE INDEX AuxStudyUID_index on PrivateData (AuxStudyUID);
CREATE INDEX AuxSeriesUID_index on PrivateData (AuxSeriesUID);

--create PrivateDataReference table
CREATE TABLE PrivateDataReference
(
  PrivateDataID     INTEGER NOT NULL REFERENCES PrivateData(ID) ,
  AuxRefStudyUID	VARCHAR(64) NOT NULL,
  AuxRefSeriesUID	VARCHAR(64) NOT NULL,
  VolumeID			VARCHAR(64) DEFAULT (''),
  PKey				INTEGER PRIMARY KEY AUTOINCREMENT ,
	CONSTRAINT PrivateDataReference_Unique UNIQUE( PrivateDataID, AuxRefSeriesUID, VolumeID)
);
CREATE INDEX PrivateDataID_index on PrivateDataReference (PrivateDataID);
CREATE INDEX AuxRefStudyUID_index on PrivateDataReference (AuxRefStudyUID);
CREATE INDEX AuxRefSeriesUID_index on PrivateDataReference (AuxRefSeriesUID);

--create LocalAE table'
CREATE TABLE LocalAE
(
  ID			INTEGER PRIMARY KEY AUTOINCREMENT,
  AEName		VARCHAR(128)  UNIQUE,
  AETitle		VARCHAR(64)  NOT NULL,
  HostName		VARCHAR(128) NOT NULL,
  IPAddress		VARCHAR(32) NOT NULL,
  Port			INTEGER NOT NULL,
  Level			INTEGER,   /* 1 Enabled or 0 Disabled */
  Priority int DEFAULT 2,
  Description           VARCHAR(128) 
--			CONSTRAINTEGER LocalAE_Unique UNIQUE(AETitle, HostName, IPAddress, Port)  
);

--create RemoteAE table'
CREATE TABLE RemoteAE 
(
  ID			INTEGER PRIMARY KEY AUTOINCREMENT,
  AEName		VARCHAR(128)  UNIQUE,
  AETitle		VARCHAR(64)  NOT NULL,
  HostName		VARCHAR(128) NOT NULL,
  IPAddress		VARCHAR(32) NOT NULL,
  Port			INTEGER NOT NULL,
  Level			INTEGER, /* May be used later on for retrieve and query option */
  Priority int DEFAULT 2,
  Description           VARCHAR(128)
--			CONSTRAINTEGER RemoteAE_Unique UNIQUE(AETitle, HostName, IPAddress, Port)  
);

--create QRAllowedAE table'
CREATE TABLE QRAllowedAE 
(
  AETitleID	INTEGER UNIQUE not null REFERENCES RemoteAE(ID)  
);

--create StoreTargetAE table'
CREATE TABLE StoreTargetAE 
(
  AETitleID	INTEGER UNIQUE not null REFERENCES RemoteAE(ID)  
);

--create QRSourceAE table'
CREATE TABLE QRSourceAE
(
  AETitleID	INTEGER UNIQUE not null REFERENCES RemoteAE(ID)  
);

--create AutoRoutingAE table'
CREATE TABLE AutoRoutingAE
(
  LocalAEID		INTEGER not null REFERENCES LocalAE(ID)  ,
  StoreTargetAEID	INTEGER not null REFERENCES StoreTargetAE(AETitleID)  ,
  CompressionMethod INT,
  CompressionFactor INT
--			CONSTRAINTEGER  AutoRoutingAE_Unique UNIQUE(LocalAEID, StoreTargetAEID)  
);

--create printer table'
CREATE TABLE Printer
(
  ID			INTEGER PRIMARY KEY AUTOINCREMENT,
  Name			VARCHAR(32) UNIQUE NOT NULL,
  AETitle		VARCHAR(64) NOT NULL,
  IPAddress		VARCHAR(32) NOT NULL,
  Port			INTEGER NOT NULL,
  Color			INTEGER NOT NULL,
  MediaSize		VARCHAR(16) NOT NULL,
  DestType 		VARCHAR(16) NOT NULL,
  Orientation 	VARCHAR(16) NOT NULL,
  MagType  		VARCHAR(16) NOT NULL,
  MediaType 	VARCHAR(16) NOT NULL,
  Manufacturer	VARCHAR(32),
  Model			VARCHAR(32),
  Location		VARCHAR(32)
--			CONSTRAINTEGER Printer_Unique UNIQUE(AETitle, IPAddress, Port)  
	);

-- managment tables

--create Organization talbe'
CREATE TABLE Organization
(
  OrganizationID 	INTEGER PRIMARY KEY AUTOINCREMENT,
  Name 			VARCHAR(128) UNIQUE NOT NULL,
  Address 		VARCHAR(256),
  Phone 		VARCHAR(32),
  Fax 			VARCHAR(32),
  Description 		VARCHAR(128)
);

INSERT INTO Organization(Name,Address,Phone,Fax,Description) 
		VALUES ('AQNet','','','','');

--create Domain talbe'
CREATE TABLE DomainT
(
	DomainID 	INTEGER PRIMARY KEY AUTOINCREMENT,
	Name 		VARCHAR(128) UNIQUE NOT NULL,	/* rtviz.com */
	OrganizationId 	INTEGER REFERENCES Organization(OrganizationID)   NOT NULL,
	Description 	VARCHAR(128) DEFAULT '',
	Type			INTEGER DEFAULT 0 /* 0 -> build in */
);

INSERT INTO DomainT (Name, OrganizationId) 
SELECT 'AQNet', OrganizationID FROM Organization Where Name = 'AQNet' ;

--create UserGroup talbe'
CREATE TABLE  UserGroup
(
	UserGroupID	INTEGER PRIMARY KEY AUTOINCREMENT,
	DomainID 		INTEGER not null REFERENCES DomainT(DomainID)  ,
	Name 			VARCHAR(64) NOT NULL,
	Privilege		INTEGER NOT NULL,
	Description 	VARCHAR(128) default '',
	SID 			VARBINARY(128) default 0
--		CONSTRAINTEGER UserGroup_Unique UNIQUE(Name, DomainID)
);

INSERT INTO UserGroup  (DomainID, Name, Privilege, Description) 
SELECT DomainID, 'Administrators', 63, 'Administrator of the AQNet Web' FROM DomainT
	Where Name = 'AQNet';

INSERT INTO UserGroup  (DomainID, Name, Privilege, Description) 
SELECT DomainID, 'regular', 7, 'Regular user of the AQNet Web'  FROM DomainT
	Where Name = 'AQNet';

INSERT INTO UserGroup  (DomainID, Name, Privilege, Description) 
SELECT DomainID, 'shared',  7, 'Special shared user of the AQNet Web' FROM DomainT
	Where Name = 'AQNet';

INSERT INTO UserGroup  (DomainID, Name, Privilege, Description) 
SELECT DomainID, 'AqNET_Public',  1024, 'global public group' FROM DomainT
	Where Name = 'AQNet';

---

--create RightsName talbe'
CREATE TABLE  RightsName
(
	NameID 	INTEGER PRIMARY KEY AUTOINCREMENT,
	Name 		VARCHAR(64) NOT NULL
);
CREATE UNIQUE INDEX RightsName__index on RightsName (Name);

--create RightsValue talbe'
CREATE TABLE  RightsValue
(
	ValueID 	INTEGER PRIMARY KEY AUTOINCREMENT,
	RValue 		VARCHAR(800) NOT NULL
);
CREATE UNIQUE INDEX RightsValue__index on RightsValue (RValue);

INSERT INTO RightsName(Name) VALUES ('AccessAllData');
INSERT INTO RightsValue(RValue) VALUES ('1');

--create UserGroupRights talbe'
CREATE TABLE  UserGroupRights
(
	GroupID 		INTEGER not null REFERENCES UserGroup(UserGroupID)  ,
	RightsKeyID 	INTEGER not null REFERENCES RightsName(NameID) ,
	RightsValueID	INTEGER not null REFERENCES RightsValue(ValueID) 
 -- 	CONSTRAINTEGER UserGroupRights_Unique PRIMARY KEY CLUSTERED (GroupID, RightsKeyID)
);

INSERT INTO UserGroupRights (GroupID, RightsKeyID, RightsValueID) 
SELECT UserGroupID, NameID, ValueID FROM UserGroup, RightsName, RightsValue
	Where UserGroup.Name='Administrators' or UserGroup.Name='shared';


--create UserAccount talbe'
CREATE TABLE UserAccount
(
  AccountID 	INTEGER PRIMARY KEY AUTOINCREMENT,
  Username 		VARCHAR(20) NOT NULL,
  Password 		VARCHAR(32) NOT NULL,
  HomeDir 		VARCHAR(256),
  LastName 		VARCHAR(32),
  MiddleName 	VARCHAR(32),
  FirstName 	VARCHAR(32),
  Title 		VARCHAR(64),
  Address 		VARCHAR(256),
  City 			VARCHAR(64),
  State 		VARCHAR(64),
  Zip 			VARCHAR(10),
  Country 		VARCHAR(64),
  Phone 		VARCHAR(32),
  Cell 			VARCHAR(32),
  Fax 			VARCHAR(32),
  Pager 		VARCHAR(32),
  Email 		VARCHAR(64),
  Status 		VARCHAR(8) CHECK ( Status IN ('ENABLED', 'DISABLED')) DEFAULT 'ENABLED',
  Description 	VARCHAR(128) DEFAULT '',
  PwdExpireTime VARCHAR(16),
  LastLoginTime VARCHAR(16),
  RoamingProfile INTEGER Default 1,
  LoginRetry	INTEGER DEFAULT 0,
  DomainID 		INTEGER REFERENCES DomainT(DomainID) 

);
CREATE UNIQUE INDEX UserAccount__index on UserAccount (Username, DomainID);

-- explicitly insert to useraccount for scan and shared. password expire time is set to 1900 
-- (dateTime =0) == nerver expire, roaming profile = 1 for admin, but 0 for shared as default 
INSERT INTO UserAccount (DomainID, Username, Password, LastName, FirstName, Email,Status, PwdExpireTime, 
		RoamingProfile)	SELECT DomainID, 'scan', 'R.~@!p2jl9;+(PZa', '', '', 'support@terarecon.com', 'ENABLED', 0, 1
	FROM DomainT Where Name = 'AQNet';

INSERT INTO UserAccount (DomainID, Username, Password, LastName, FirstName, Email,Status, PwdExpireTime, 
	RoamingProfile)	SELECT DomainID, 'shared','?!f8bF3bbz>nNl|/', '', '', 'support@terarecon.com', 'ENABLED', 0, 0
	FROM DomainT Where Name = 'AQNet';


--create UserDefaultGroup talbe'
CREATE TABLE UserDefaultGroup
(
  AccountID		INTEGER PRIMARY KEY REFERENCES UserAccount(AccountID)  ,
  GroupID 		INTEGER not null REFERENCES UserGroup(UserGroupID)  
);

INSERT INTO UserDefaultGroup (AccountID, GroupID) 
SELECT a.AccountID, g.UserGroupID FROM UserAccount a,  UserGroup g 
WHERE a.Username = 'scan' and g.Name = 'Administrators';

INSERT INTO UserDefaultGroup (AccountID, GroupID) 
SELECT a.AccountID, g.UserGroupID FROM UserAccount a,  UserGroup g 
WHERE a.Username = 'shared' and g.Name = 'shared';


--create UserOtherGroup talbe'
CREATE TABLE UserOtherGroup
(
  AccountID		INTEGER not null REFERENCES UserAccount(AccountID)  ,
  GroupID 		INTEGER not null REFERENCES UserGroup(UserGroupID)  
 -- 			CONSTRAINTEGER UserOtherGroup_Unique PRIMARY KEY CLUSTERED (GroupID, AccountID)
);
CREATE INDEX UserOtherGroup_index on UserOtherGroup (AccountID);


--create QRSourceAEGroupAssignment talbe'
CREATE TABLE QRSourceAEGroupAssignment
(
  QRSourceAEID		INTEGER NOT NULL REFERENCES QRSourceAE(AETitleID)  ,
  GroupID		INTEGER NOT NULL REFERENCES UserGroup (UserGroupID)  
  --			CONSTRAINTEGER QRSourceAEGroupAssignment_Unique PRIMARY KEY CLUSTERED (QRSourceAEID, GroupID)
);


--create GroupSeries talbe'
CREATE TABLE GroupSeries
(
  GroupID 	INTEGER NOT NULL  REFERENCES UserGroup(UserGroupID)  ,
  SeriesLevelID INTEGER not null REFERENCES SeriesLevel(SeriesLevelID)  
--			CONSTRAINTEGER GroupSeries_Unique PRIMARY KEY CLUSTERED (GroupID, SeriesLevelID)
);
CREATE INDEX GroupSeries_SeriesLevelID_index on GroupSeries (SeriesLevelID);
CREATE INDEX GroupSeries_GroupID_index on GroupSeries (GroupID);


-- the read status table links to main patient table and user table
-- Therefore, cannot record the history series, user and QR series read status 
-- If we really need that record, we can fish it out from Actions records.
--create StudyReadStatus table'
CREATE TABLE StudyReadStatus
(
	StudyLevelID INTEGER not null REFERENCES StudyLevel(StudyLevelID)  ,
	UserID 	INTEGER not null REFERENCES UserAccount(AccountID)  ,
	Status 	INTEGER  NOT NULL
 -- 		CONSTRAINTEGER StudyReadStatus_Unique PRIMARY KEY CLUSTERED ( UserID, StudyLevelID)
);
CREATE INDEX StudyReadStatus_StudyLevelID_index on StudyReadStatus (StudyLevelID);
CREATE INDEX StudyReadStatus_UserID_index on StudyReadStatus (UserID);


--create SeriesReadStatus table'
CREATE TABLE SeriesReadStatus
(
	SeriesLevelID INTEGER not null REFERENCES SeriesLevel(SeriesLevelID)  ,
	UserID 	INTEGER not null REFERENCES UserAccount(AccountID)  ,
	Status 	INTEGER  NOT NULL
 --  		CONSTRAINTEGER SeriesReadStatus_Unique PRIMARY KEY CLUSTERED (UserID, SeriesLevelID)
);
CREATE INDEX SeriesReadStatus_index on SeriesReadStatus (SeriesLevelID);
CREATE INDEX SeriesReadStatus_UserID_index on SeriesReadStatus (UserID);

--create PrinterGroupAssignment talbe'
CREATE TABLE PrinterGroupAssignment
(
  PrinterID		INTEGER NOT NULL REFERENCES Printer(ID)  ,
  GroupID		INTEGER NOT NULL REFERENCES UserGroup (UserGroupID)  
  --			CONSTRAINTEGER PrinterGroupAssignment_Unique UNIQUE(PrinterID, GroupID)
);

--create WebConfiguration talbe'
CREATE TABLE WebConfiguration
(
	PwdExpiryInterval INTEGER NOT NULL,
	LockTimeOut INTEGER NOT NULL,
	MaxloginRetry INTEGER NOT NULL,
	InactiveDaysAllowed INTEGER NOT NULL,
	SessionTimeOut INTEGER NOT NULL,
	ItemsPerPage INTEGER NOT NULL,
	LogItemsPerPage INTEGER NOT NULL,
  	ClientViewerInMainPage INTEGER NOT NULL
);

INSERT INTO  WebConfiguration(PwdExpiryInterval, LockTimeOut, MaxLoginRetry, 
	InactiveDaysAllowed, SessionTimeOut, ItemsPerPage, LogItemsPerPage, ClientViewerInMainPage ) 
	values (180, 10,5,90,20,25, 100, 1) ;

--create RoutingPattern table'
CREATE TABLE RoutingPattern
(
	ID INTEGER PRIMARY KEY AUTOINCREMENT,
	Name VARCHAR(64) NOT NULL UNIQUE
);

--create Schedule table'
CREATE TABLE Schedule
(
	ID INTEGER PRIMARY KEY AUTOINCREMENT,
	Name VARCHAR(64) NOT NULL UNIQUE
);

--create TimeRange table'
CREATE TABLE TimeRange
(
	ScheduleID INTEGER NOT NULL REFERENCES Schedule (ID)  ,
	DayOfWeek INTEGER NOT NULL CHECK (DayOfWeek >= 0 and DayOfWeek <= 6),
	StartTime VARCHAR(5),
	EndTime VARCHAR(5)
--	CONSTRAINTEGER TimeRange_Unique UNIQUE (ScheduleID, DayOfWeek, StartTime, EndTime)
);

--create RoutingSchedule table'
CREATE TABLE RoutingSchedule
(
	ScheduleID INTEGER NOT NULL  REFERENCES Schedule (ID)  ,
	RoutingPatternID INTEGER NOT NULL  REFERENCES RoutingPattern (ID)  ,	
	StartDate DATETIME,
	EndDate DATETIME,
	Repeat INTEGER DEFAULT 0
--	CONSTRAINTEGER RoutingSchedule_Unique UNIQUE (ScheduleID, RoutingPatternID, StartDate, EndDate)
);

--create TempRoutingSchedule table'
CREATE TABLE TempRoutingSchedule
(
	RoutingPatternID INTEGER NOT NULL  REFERENCES RoutingPattern (ID)  ,	
	EndDate DATETIME,
	SuspendOthers INTEGER DEFAULT 0		
);


--create Comparator table'
CREATE TABLE Comparator
(
  ID			INTEGER PRIMARY KEY,
  Op			VARCHAR(64) NOT NULL,
  OpString		VARCHAR(64) NOT NULL 
);
CREATE UNIQUE INDEX ComparatorOp__index on Comparator (Op);
CREATE UNIQUE INDEX ComparatorOpString__index on Comparator (OpString);

INSERT INTO Comparator VALUES(1, '=', 'is');
INSERT INTO Comparator VALUES(2, '<>', 'is not');
INSERT INTO Comparator VALUES(3, 'LIKE', 'contains');
INSERT INTO Comparator VALUES(4, 'NOT LIKE', 'doesn''t contains');


--create DicomTag table'
CREATE TABLE DicomTag
(
  ID		INTEGER PRIMARY KEY AUTOINCREMENT,
  Tag		BIGINTEGER UNIQUE NOT NULL,
  TagString	VARCHAR(128) NOT NULL
);
CREATE UNIQUE INDEX DicomTagTagString__index on DicomTag (TagString);

--INSERT INTO DicomTag VALUES(0x00020016, 'Source Application Entity Title');
--INSERT INTO DicomTag VALUES(0x00080060, 'Modality');
--INSERT INTO DicomTag VALUES(0x00080090, 'Referring Physician''s Name');
--INSERT INTO DicomTag VALUES(0x00081060, 'Name of Physician(s) Reading Study');
--INSERT INTO DicomTag VALUES(0x0008103E, 'Series Description');
--INSERT INTO DicomTag VALUES(0x00081030, 'Study Description');
--INSERT INTO DicomTag VALUES(0x00180015, 'Body Part Examined');

--create TagRule table'
CREATE TABLE TagRule
(
  ID			INTEGER PRIMARY KEY AUTOINCREMENT,
  DicomTagID		INTEGER NOT NULL REFERENCES DicomTag(ID)  ,
  ComparatorID		INTEGER NOT NULL REFERENCES Comparator(ID),
  Value			VARCHAR(64) NOT NULL
--	CONSTRAINTEGER TagRule_Unique UNIQUE(DicomTagID, ComparatorID, Value)
);


--create TagFilter table'
CREATE TABLE TagFilter
(
  ID		INTEGER PRIMARY KEY AUTOINCREMENT,
  Name		VARCHAR(32) UNIQUE NOT NULL,
  Description 	VARCHAR (64)
);


--create TagFilterRules table'
CREATE TABLE TagFilterRules
(
  ID			INTEGER PRIMARY KEY AUTOINCREMENT,
  TagFilterID		INTEGER NOT NULL REFERENCES TagFilter(ID)  ,
  TagRuleID		INTEGER NOT NULL REFERENCES TagRule(ID) 
--	CONSTRAINTEGER TagFilterRules_Unique UNIQUE(TagFilterID, TagRuleID)
);


--create TagFilterGroupAssignment table'
CREATE TABLE TagFilterGroupAssignment
(
  TagFilterID		INTEGER NOT NULL REFERENCES TagFilter(ID)  ,
  GroupID		INTEGER NOT NULL REFERENCES UserGroup (UserGroupID)  
--	CONSTRAINTEGER TagFilterGroupAssignment_Unique UNIQUE(TagFilterID, GroupID)
);


--create TagBasedRoutingPatternEntry table'
CREATE TABLE TagBasedRoutingPatternEntry
(
	RoutingPatternID INTEGER NOT NULL  REFERENCES RoutingPattern (ID)  ,
	TagFilterID		INTEGER REFERENCES TagFilter(ID)  ,
	StoreTargetID INTEGER NOT NULL  REFERENCES StoreTargetAE (AETitleID)  ,
	CompressionMethod INT,
	CompressionFactor INT
--	CONSTRAINTEGER TagBasedRoutingPatternEntry_Unique UNIQUE (RoutingPatternID,TagFilterID, StoreTargetID, CompressionMethod, CompressionFactor)
);


CREATE TABLE PrefetchPattern
(
	ID INTEGER NOT NULL unique,
	TagFilterID INTEGER NOT NULL REFERENCES TagFilter(ID)  ,
	Modality VARCHAR(64) NOT NULL,
	StudyNotOlderThan INTEGER NOT NULL,
	UnitType INTEGER NOT NULL,		-- 0=days, 1=weeks, 2=months, 3=years
	MaxNumberResults INTEGER NOT NULL  -- 0 = no constraint 
);


CREATE TABLE PrefetchPatternAE
(
	PrefetchPatternID INTEGER NOT NULL REFERENCES PrefetchPattern (ID)  ,
	QRSourceAEID INTEGER NOT NULL REFERENCES QRSourceAE (AETitleID)  
--	CONSTRAINTEGER PrefetchPatternAE_Unique UNIQUE (PrefetchPatternID,QRSourceAEID)
);


--create FilmingPattern table'
CREATE TABLE FilmingPattern
(
	TagFilterID int not null  REFERENCES TagFilter(ID)  ,
	PrinterID int not null  REFERENCES Printer(ID)  ,
	Options int not null, -- delete original series after print? 0=no, 1=yes
	SkipN int, -- >=0
	DisplayMode varchar(16) -- eg. 2x2, 2x4
--	CONSTRAINTEGER FilmingPattern_Unique UNIQUE (TagFilterID, PrinterID)
);


CREATE TABLE DataClassifyPattern
(
	ID INTEGER PRIMARY KEY AUTOINCREMENT,
	TagFilterID INTEGER NOT NULL UNIQUE REFERENCES TagFilter(ID)  ,
	TypeTag BIGINTEGER NOT NULL REFERENCES DicomTag(Tag)  ,
	Description  VARCHAR(128)
);


CREATE TABLE DataProcessJob
(
	ID INTEGER PRIMARY KEY AUTOINCREMENT,
	JobName	VARCHAR(64)  UNIQUE,
	Description  VARCHAR(128)
);

CREATE TABLE DataProcessor
(
	ID INTEGER PRIMARY KEY AUTOINCREMENT,
	ProcessName	VARCHAR(128)  UNIQUE,
	Handler	VARCHAR(128)  UNIQUE,
	Description  VARCHAR(128)
);


CREATE TABLE DataJobProcessList
(
	ID INTEGER PRIMARY KEY AUTOINCREMENT,
	JobID INTEGER NOT NULL REFERENCES DataProcessJob(ID)  ,
	Processor INTEGER NOT NULL REFERENCES DataProcessor(ID)  ,
	JobOrder	INTEGER NOT NULL
--	CONSTRAINTEGER DataJobProcessList_Unique UNIQUE (JobID, Processor,JobOrder)
);


CREATE TABLE DataProcessPattern
(
	ID INTEGER PRIMARY KEY AUTOINCREMENT,
	TagFilterID INTEGER NOT NULL REFERENCES TagFilter(ID)  ,
	Job INTEGER NOT NULL REFERENCES DataProcessJob(ID)  ,
	Description  VARCHAR(128)
--	CONSTRAINTEGER DataProcessPattern_Unique UNIQUE (TagFilterID, Job)
);

--create AqNetOption table'
CREATE TABLE AqNetOption
(
	KeyStr VARCHAR(64) NOT NULL,
	ValueStr VARCHAR(256) NOT NULL,
	Display VARCHAR(80)  default ''
);

CREATE UNIQUE INDEX AqNetOption__Key_index on AqNetOption (KeyStr);
CREATE UNIQUE INDEX AqNetOption__Display_index on AqNetOption (Display);

INSERT INTO  AqNetOption (KeyStr, ValueStr, Display) 
	VALUES( 'EnableAuditTrail', '0', 'Enalbe audit trail');

INSERT INTO  AqNetOption (KeyStr, ValueStr, Display) 
	VALUES( 'RequiredFreeSpaceOnDriveC', '1000', 'Required free space on drive C');

INSERT INTO  AqNetOption (KeyStr, ValueStr, Display) 
	VALUES( 'EnableSSO', '0', 'Enable single sign on');
 

INSERT INTO  AqNetOption (KeyStr, ValueStr, Display) 
	VALUES( 'OneInstanceTopDBSize', '350', 'Instance record average top size in bytes for fragment warning');

