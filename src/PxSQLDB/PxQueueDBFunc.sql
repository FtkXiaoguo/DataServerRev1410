
USE PxQueueDB
GO
-----------------------------------------------
-- for readQueue
print 'create function ChgReadQueueStatus '
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'ChgReadQueueStatus' AND type = 'P')
   DROP PROCEDURE ChgReadQueueStatus
GO

CREATE PROCEDURE ChgReadQueueStatus  (@id INT, @CurStatus INT ,@NewStatus INT ,@returnVal INT output )
AS
BEGIN

	DECLARE @readStatus INT
	SET @returnVal = 0

	DECLARE @c_QueueEntry CURSOR 
	SET @c_QueueEntry =CURSOR FOR
	SELECT Status  FROM dbo.sendQueue WHERE QueueID=@id 
	FOR UPDATE  

	OPEN @c_QueueEntry
	FETCH NEXT FROM @c_QueueEntry INTO @readStatus

	IF (@@FETCH_STATUS <> -1)
	BEGIN
		IF(@readStatus = @CurStatus)
		BEGIN
			UPDATE dbo.sendQueue 
			SET Status=@NewStatus , AccessTime = GETDATE()
			WHERE CURRENT OF @c_QueueEntry
			SET @returnVal = 1
		END
	END
	
	CLOSE @c_QueueEntry 
	
END
GO

-----------------------------------------------
-- for resultQueue
print 'create function ChgResultQueueStatus '
IF EXISTS (SELECT name FROM sysobjects 
         WHERE name = 'ChgResultQueueStatus' AND type = 'P')
   DROP PROCEDURE ChgResultQueueStatus
GO

CREATE PROCEDURE ChgResultQueueStatus  (@id INT, @CurStatus INT ,@NewStatus INT ,@returnVal INT output )
AS
BEGIN

	DECLARE @readStatus INT
	SET @returnVal = 0

	DECLARE @c_QueueEntry CURSOR 
	SET @c_QueueEntry =CURSOR FOR
	SELECT Status  FROM dbo.resultQueue WHERE QueueID=@id 
	FOR UPDATE  

	OPEN @c_QueueEntry
	FETCH NEXT FROM @c_QueueEntry INTO @readStatus

	IF (@@FETCH_STATUS <> -1)
	BEGIN
		IF(@readStatus = @CurStatus)
		BEGIN
			UPDATE dbo.resultQueue 
			SET Status=@NewStatus , AccessTime = GETDATE()
			WHERE CURRENT OF @c_QueueEntry
			SET @returnVal = 1
		END
	END
	
	CLOSE @c_QueueEntry 
	
END
GO