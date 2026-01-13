INSERT INTO StudyLevel(StudyInstanceUID,PatientsName,PatientID) 
      VALUES('11111','testName','1234');
INSERT INTO SeriesLevel(StudyLevelID,SeriesInstanceUID,SeriesNumber,Modality) 
      VALUES(1,'222',100,'CT');
INSERT INTO InstanceLevel(SOPInstanceUID,SeriesLevelID,SOPClassID,InstanceNumber,Rows ,Columns ) 
      VALUES('1222',1,'27722',1,512,512);

INSERT INTO GroupSeries(GroupID,SeriesLevelID)
      VALUES(1,1);