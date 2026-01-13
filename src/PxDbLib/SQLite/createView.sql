CREATE VIEW UserStudyStatusView
AS

SELECT  DISTINCT AccountID, StudyLevel.StudyLevelID, 
0 as Status
FROM (StudyLevel CROSS JOIN UserAccount) ;
--
CREATE VIEW GroupStudyView
AS
SELECT  DISTINCT StudyLevel.StudyLevelID, GroupSeries.GroupID
FROM  StudyLevel JOIN  SeriesLevel ON StudyLevel.StudyLevelID = SeriesLevel.StudyLevelID
JOIN  GroupSeries ON GroupSeries.SeriesLevelID = SeriesLevel.SeriesLevelID ;

--UserStudyView
CREATE VIEW UserStudyView
AS
--Select from local member table
SELECT  
	PrivateDataReference.AuxRefStudyUID AS AuxRefStudyUID, 
	GroupStudyView.GroupID AS GroupID, -- can be NULL
	UserStudyStatusView.AccountID AS UserID,
	UserStudyStatusView.Status as ReadStatus, 
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
FROM  StudyLevel JOIN  UserStudyStatusView ON StudyLevel.StudyLevelID = UserStudyStatusView.StudyLevelID
LEFT OUTER JOIN  GroupStudyView ON StudyLevel.StudyLevelID = GroupStudyView.StudyLevelID 
LEFT OUTER JOIN  PrivateDataReference ON StudyLevel.StudyInstanceUID = PrivateDataReference.AuxRefStudyUID ;

--UserSeriesStatusView
CREATE VIEW UserSeriesStatusView
AS

SELECT  DISTINCT AccountID, SeriesLevel.SeriesLevelID, 
0 as Status
FROM (SeriesLevel CROSS JOIN UserAccount) ;
