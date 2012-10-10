[TEAM Sempiternal]
prabhude@usc.edu
zamwar@usc.edu
kalluru@usc.edu
sungal@usc.edu

+<<Imp Note>>+
1. Our sent and received file name must be data1G.bin
2. File size of data1G.bin must be 1048576000, created using command `dd if=/dev/urandom of=data1G.bin bs=1048576 count=1000`eed 
3. We need to provide the file size as command line argument to receiver i.e server.
	To provide the correct file size to receiver/server, please make change in ServerStart1.sh, where we have line 
		./receiver $1 1048576000
	Change 1048576000 to the file size which will be used to send the file.

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

[Imp Execution of project steps]:
1. Find attached files: config.json, app1 directory with script files and executables, sempiternal_code directory consists of code files.

   + config.json 
     + app1
	+server
	  + ServerPreStart1.sh
	  + ServerStart1.sh
	  + receiver
	  + sender
	+client
	  + ClientPreStart1.sh
	  + ClientStart1.sh
	  + receiver
	  + sender

     + sempiternal_code
	+ client.cpp
	+ server.cpp
	+ common.h
	+ commonFunctions.cpp
	+ client.h
	+ server.h
	+ Makefile
	+ sender
	+ receiver

2. app2 directory must have same files as app1, depending on the team1 or team2.
3. [How to compile files]:
	a. Copy `sempiternal_code` directory to ubuntu node, and type 'make', which creates 2 executables named "receiver" i.e. server and "sender" i.e. client.

4. [How to run]
	a. Run receiver (server)
		./receiver <Server_UDP_Port> <file_size>
	b. Run sender (client)
		./sender <server Name/IP Address> <Server_UDP_Port> <File_to_be_transferred>

5. [How to Run Script]
	1. Copy config.json to script directory. data1G.bin is created in `/users/sc558bt/cs558l/testdata/data1G.bin`.
	2. Copy files from app1 to app1 and app2 directories both.
	3. Execute `./run_tournament.py -f config.json -s`