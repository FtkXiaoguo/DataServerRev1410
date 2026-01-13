USE PxDcmDB
GO

------------------------------
-- delete all Stored Procedures
declare @procName varchar(500)
declare cur cursor 

for select [name] from sys.objects where type = 'p'
open cur
fetch next from cur into @procName
while @@fetch_status = 0
begin
    exec('drop procedure [' + @procName + ']')
    fetch next from cur into @procName
end
close cur
deallocate cur

------------------------------
-- delete all views

declare @SQL nvarchar(max)

set @SQL = 
  (
  select 'drop view '+name+'; '
  from sys.views
  for xml path('')
  )

exec (@SQL)

------------------------------
-- delete  Constraint 
-- default of PatientID
DECLARE @ConstraintName nvarchar(200)

SELECT @ConstraintName = obj.name FROM sys.objects AS obj
JOIN sys.columns AS clm ON obj.object_id = clm.default_object_id
WHERE obj.type = 'D' AND obj.parent_object_id = OBJECT_ID('StudyLevel') AND clm.name = 'PatientID'

IF @ConstraintName IS NOT NULL
    EXEC('ALTER TABLE StudyLevel DROP CONSTRAINT ' + @ConstraintName)
GO

------------------------------
-- delete  index  
DROP INDEX PatientsName_index ON StudyLevel
DROP INDEX PatientID_index ON StudyLevel

------------------------------
-- change fields to unicode
ALTER TABLE StudyLevel ALTER COLUMN PatientsName NVARCHAR(332)
ALTER TABLE StudyLevel ALTER COLUMN PatientID NVARCHAR(64)
ALTER TABLE StudyLevel ALTER COLUMN ReadingPhysiciansName NVARCHAR(332)
ALTER TABLE StudyLevel ALTER COLUMN ReferringPhysiciansName NVARCHAR(332)
ALTER TABLE StudyLevel ALTER COLUMN StudyDescription NVARCHAR(64)
ALTER TABLE StudyLevel ALTER COLUMN CharacterSet VARCHAR(256) 
--
ALTER TABLE SeriesLevel ALTER COLUMN SeriesDescription NVARCHAR(64)

--ALTER TABLE PatientStudy  ALTER COLUMN PatientsName NVARCHAR(332)
--ALTER TABLE PatientStudy ALTER COLUMN PatientID NVARCHAR(64)
--ALTER TABLE PatientStudy ALTER COLUMN ReferringPhysiciansName NVARCHAR(332)

------------------------------
-- add  Constraint 
-- default of PatientID
ALTER TABLE dbo.StudyLevel
  ADD CONSTRAINT DF_default_Column_PatientID
  DEFAULT '' FOR PatientID 
 

------------------------------
-- add  index 
CREATE INDEX PatientsName_index on StudyLevel (PatientsName)
CREATE INDEX PatientID_index on StudyLevel (PatientID)
 

------------------------------
