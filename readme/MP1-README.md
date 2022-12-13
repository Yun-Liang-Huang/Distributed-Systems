# CS425

## Name
Distributed Log Querier

## Description
A Distributed Log Querier is implemented that allows us to query distributed log files on multiple machines, from any one of those machines.

## Compile
To compile the code, execute the Makefile
```
make
```

## Execution
After the code is compile, first, execute the server code followed by a specific port number (default is 8088) for all VMs to be queried logs from
```
./server <port number>
```

To query logs from all VMs, enter one of those VMs and execute client code with parameters

```
./client grep <query type> <search string / regex pattern> <file pattern>
```

Query type:

| Input        | Action        |
|--------------|---------------|
| (empty)      | Exact string search            |
| -i           | Case insensitive string search |
| -regex       | Regular expression             |


### Scenarios

- Search for the given string in a single file
```
./client grep "duke.com" "vm1.log"
```


- Search for the given string in multiple files
```
./client grep "duke.com" ".*.log"
```

- Case insensitive search using grep -i
```
./client grep -i "duke.com" ".*.log"
```

- Regular expression search using grep -regex
```
./client grep -regex "init.*" ".*.log"
```

## Unit Testing
We use `GoogleTest` for testing and use `Bazel` to build `GoogleTest`.
Need to install `Bazel`, please follow the installation guide [here](https://bazel.build/install).

```
bazel test --test_output=all //:query_test
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
1. socket gethostbyname(), https://stackoverflow.com/a/52728208
2. Implode a vector of strings into a comma-separated string in C++, https://www.techiedelight.com/implode-a-vector-of-strings-into-a-comma-separated-string-in-cpp/
3. C++ execute linux command, https://stackoverflow.com/a/478960
4. C++ Socket programming, https://www.geeksforgeeks.org/socket-programming-cc/
5. C++ SOCKETS - SERVER & CLIENT, https://www.bogotobogo.com/cplusplus/sockets_server_client.php
6. Bazel, https://bazel.build/
7. Google test, https://github.com/google/googletest
8. C++ Makefile, https://stackoverflow.com/a/2908351
9. Case insensitive https://stackoverflow.com/questions/3152241/case-insensitive-stdstring-find
10. Regular expression https://www.softwaretestinghelp.com/regex-in-cpp/
11. Get current work directory https://www.delftstack.com/howto/cpp/get-current-directory-cpp/
