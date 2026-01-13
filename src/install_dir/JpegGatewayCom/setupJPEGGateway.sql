-- patch database first
USE PxDcmDB
GO

-- JPEG Gateway AE -------------

IF NOT EXISTS (select * from dbo.RemoteAE  
	Where  Port=0 and IPAddress='0.0.0.0' )
begin	
	INSERT INTO dbo.RemoteAE(
	AEName,
	AETitle,
	HostName,
	IPAddress,
	Port,
	Level,
	Description
	) 
	VALUES (
	'OutputJPEG',
	'OutputJPEG',
	'localhost',
	'0.0.0.0',
	0,
	1,
    'for outputJPEG'
	)

	INSERT dbo.StoreTargetAE Select ID FROM dbo.RemoteAE WHERE AEName='OutputJPEG'
end

-- Tag filter for Auto Routing -------------

-- define TagFilter ---
IF NOT EXISTS ( SELECT ID FROM dbo.TagFilter WHERE Name = 'allModality')
INSERT INTO dbo.TagFilter (Name, Description) values('allModality', 'all mdality filter');

declare @DicomTag_id int;	
SET @DicomTag_id = 0;
SELECT @DicomTag_id=ID FROM dbo.DicomTag WHERE Tag=524384;

declare @Comparator_id int;	
SET @Comparator_id = 0;
SELECT @Comparator_id=ID FROM dbo.Comparator WHERE OPString = 'is not';

IF NOT EXISTS (SELECT ID FROM dbo.TagRule WHERE 
	DicomTagID = @DicomTag_id AND ComparatorID = @DicomTag_id AND value = 'MR')
INSERT INTO dbo.TagRule (DicomTagID,ComparatorID,value) values(@DicomTag_id,@Comparator_id,'MR' );

declare @TagFiler_id int;	
SET @TagFiler_id = 0;
SELECT @TagFiler_id=ID FROM dbo.TagFilter WHERE Name = 'allModality';

declare @TagRule_id int;	
SET @TagRule_id = 0;
SELECT @TagRule_id=ID FROM dbo.TagRule WHERE 
	DicomTagID = @DicomTag_id AND ComparatorID = @DicomTag_id AND value = 'MR';

IF NOT EXISTS (SELECT TagFilterID FROM dbo.tagFilterRules WHERE TagFilterID = @TagFiler_id AND TagRuleID = @TagRule_id)
INSERT INTO dbo.tagFilterRules (TagFilterID,TagRuleID) values(@TagFiler_id,@TagRule_id);


-- set up Autorouting Pattern ---

IF NOT EXISTS ( SELECT ID FROM dbo.RoutingPattern WHERE Name = 'AutoRoutingPat')
INSERT INTO dbo.RoutingPattern (Name) values('AutoRoutingPat');

declare @RoutingPattern_id int ;
SET @RoutingPattern_id = 0;
SELECT @RoutingPattern_id=ID FROM dbo.RoutingPattern WHERE Name = 'AutoRoutingPat';


SET @TagFiler_id = 0;
SELECT @TagFiler_id=ID FROM dbo.TagFilter WHERE Name = 'allModality';

declare @StoreTarget_id int;	
SET @StoreTarget_id = 0;
SELECT @StoreTarget_id=ID FROM dbo.RemoteAE WHERE AEName = 'OutputJPEG';
 
IF NOT EXISTS ( SELECT * FROM dbo.TagBasedRoutingPatternEntry WHERE  
	RoutingPatternID = @RoutingPattern_id 
	AND TagFilterID = @TagFiler_id
	AND StoreTargetID = @StoreTarget_id)
INSERT INTO dbo.TagBasedRoutingPatternEntry (RoutingPatternID,TagFilterID,StoreTargetID) 
	values(@RoutingPattern_id,@TagFiler_id,@StoreTarget_id);

-- Turn on  Autorouting Pattern ---
IF NOT EXISTS ( SELECT * FROM dbo.RoutingSchedule WHERE  RoutingPatternID = @RoutingPattern_id )
INSERT INTO dbo.RoutingSchedule (ScheduleID,RoutingPatternID) values(1,@RoutingPattern_id);


-- add LocalAE  ---
IF NOT EXISTS ( SELECT * FROM dbo.LocalAE WHERE  AEName = 'JPEGGateway_AE' )
INSERT INTO dbo.LocalAE (AEName,AETitle,HostName,IPAddress,Port) 
values('JPEGGateway_AE','JPEGGateway_AE','localhost','100.100.0.0',104);


--- end   ----
