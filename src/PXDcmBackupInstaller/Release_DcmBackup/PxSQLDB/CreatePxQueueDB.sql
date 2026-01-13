
--USE master
--DROP DATABASE PxDcmDB
--CREATE DATABASE PxDcmDB

USE PxQueueDB

SET NOCOUNT ON

print 'create dbinfo table'
CREATE TABLE dbinfo
(
	MajorVersion int not null,
	MinorVersion int not null,
	BuildVersion int not null,
	BuildMinorVersion int not null,
	Note	VARCHAR(500) DEFAULT ''
)
-- Set DB to release version
INSERT INTO dbinfo(MajorVersion, MinorVersion, BuildVersion, BuildMinorVersion) VALUES (1,0,0,0)


print 'create sendQueue table'
CREATE TABLE sendQueue
(
	QueueID INT IDENTITY(1,1) PRIMARY KEY,
	SendLevel INT DEFAULT 0,
	StudyInstanceUID VARCHAR(64) , 
	SeriesInstanceUID VARCHAR(64) ,
	SOPInstanceUID VARCHAR(64) ,
	DestinationAE VARCHAR(64)  NOT NULL,
	Priority INT DEFAULT 0,
	Status INT DEFAULT 0,
	RetryCount INT DEFAULT 0,
	CreateTime datetime DEFAULT GETDATE(),
	AccessTime datetime DEFAULT GETDATE()
)

-- the ProcessedQueue with the same items  sendQueue 
print 'create resultQueue table'
CREATE TABLE resultQueue
(
	QueueID INT IDENTITY(1,1) PRIMARY KEY,
	SendLevel INT DEFAULT 0,
	StudyInstanceUID VARCHAR(64) , 
	SeriesInstanceUID VARCHAR(64) ,
	SOPInstanceUID VARCHAR(64) ,
	DestinationAE VARCHAR(64)  NOT NULL,
	Priority INT DEFAULT 0,
	Status INT DEFAULT 0,
	RetryCount INT DEFAULT 0,
	CreateTime datetime DEFAULT GETDATE(),
	AccessTime datetime DEFAULT GETDATE()
)
