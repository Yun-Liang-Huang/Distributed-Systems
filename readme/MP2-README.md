## Name
Distributed Group Membership

## Description
A Distributed Group Membership is implemented that allows us to maintain the membership list in the system.

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

To add a machine into the group, enter a join command including information of the new machine and the introducer

```
./client join <new member host> <new member port> <introducer host> <introducer port>
```

To leave the group voluntarily, enter a leave command on that machine

```
./client leave <member host> <member port>
```

To see the membership list on a machine, enter a membership command on that machine

```
./client membership <member host> <member port>
```

## Unit Testing
We use `GoogleTest` for testing and use `Bazel` to build `GoogleTest`.
Need to install `Bazel`, please follow the installation guide [here](https://bazel.build/install).

```
bazel test --test_output=all //:<test_file>
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
1. Hostname to IP address, https://stackoverflow.com/questions/9400756/ip-address-from-host-name-in-windows-socket-programming
2. Set timestamp, https://stackoverflow.com/questions/9089842/c-chrono-system-time-in-milliseconds-time-operations
3. C++ sleep function, https://www.softwaretestinghelp.com/cpp-sleep/
4. UDP socket set timeout, https://stackoverflow.com/questions/13547721/udp-socket-set-timeout

