## Name
Simple Distributed File System

## Description
A Simple Distributed File System is implemented that allows us to maintain files in the system.

## Compile
To compile the code, execute the Makefile
```
make
```

## Execution
After the code is compile, first, execute the server code followed by a specific port number (default is 8088) for all machines
```
./server <port number>
```

#### Before start using the SDFS, servers need to be added into the group

To add a machine into the group, enter a join command including information of the new machine and the introducer

```
./client membership join <new member host> <new member port> <introducer host> <introducer port>
```

To leave the group voluntarily, enter a leave command on that machine

```
./client membership leave <member host> <member port>
```

To see the membership list on a machine, enter a membership command on that machine

```
./client membership list <member host> <member port>
```

#### After the membership group is created, we can start the operations in the SDFS
(member host and member port is the machine address that we want to execute the input command)


To add a file into the SDFS

```
./client sdfs put <member host> <member port> <localfilename> <sdfsfilename>
```

To fetch a file from the SDFS

```
./client sdfs get <member host> <member port> <sdfsfilename> <localfilename>
```

To delete a file on the SDFS

```
./client sdfs delete <member host> <member port> <sdfsfilename>
```

To list all machine addresses where this file is currently being stored

```
./client sdfs ls <member host> <member port> <sdfsfilename>
```

To list all files currently being stored at this machine

```
./client sdfs store <member host> <member port>
```

To get all the last num-versions versions of the file

```
./client sdfs get-versions <member host> <member port> <sdfsfilename> <num-versions> <localfilename>
```

## Unit Testing
We use write our tests in mytest.cpp.

```
make mytest
./mytest
```

## Cluster Scripts
Execute command on all nodes in the cluster
```
$ ./cluster_cmd.sh <NetID> <COMMAND>

# example
$ ./cluster_cmd.sh hyhuang3 "cd cs425 && git pull && make clean; make"
```

Start running `server` on all nodes
```
./start_cluster_server.sh <NetID>
```

Stop running `server` on all nodes
```
./stop_cluster_server.sh <NetID>
```

Note: you may need to setup ssh-key on the machines to execute above scripts, see the script file (`cluster_cmd.sh`) for more information.

## Reference
1. Send and receive files over C++ TCP sockets https://stackoverflow.com/questions/63494014/sending-files-over-tcp-sockets-c-windows

