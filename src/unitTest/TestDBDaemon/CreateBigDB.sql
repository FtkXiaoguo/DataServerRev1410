 

--'create testBigDB table'
CREATE TABLE testBigDb
(
	BigDbID INTEGER PRIMARY KEY AUTOINCREMENT,
	intDat1 INTEGER not null,
	intDat2 INTEGER not null,
	intDat3 INTEGER not null,
	intDat4 INTEGER not null,
	bitDat1	VARCHAR(2048) DEFAULT '',
	bitDat2	VARCHAR(2048) DEFAULT '',
	bitDat3	VARCHAR(2048) DEFAULT ''
);
 
