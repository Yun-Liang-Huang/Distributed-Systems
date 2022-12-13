## Name
Distributed Learning Cluster

## Description
A Distributed Learning Cluster is implemented that allows us to train and inference machine learning jobs. Load balance in the inference phase is ensured that query rates of different jobs are within 20% of each other. Moreover, the cluster has high-reliability fault tolerance, which can handle up to 2 simultaneous worker failures and handle the coordinator failure.

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

#### Before start using the Distributed Learning Cluster, servers need to be added into the group
([hostname] and [port] is the machine address that we want to execute the input command, default is localhost:8088)

To add a machine into the group, enter a join command including information of the new machine and the introducer

```
./client membership join <new member host> <new member port> [introducer hostname] [introducer port]

# example
Add machines one by one:

./client membership join fa22-cs425-0401.cs.illinois.edu 8088
./client membership join fa22-cs425-0402.cs.illinois.edu 8088
./client membership join fa22-cs425-0403.cs.illinois.edu 8088

Add all machines (10 VMs) at once:

script/add_all.sh 
```

To see the membership list on a machine, enter a membership command on that machine

```
./client membership list [hostname] [port]
```

#### Files (model, testset, label) need to be added to the SDFS
([hostname] and [port] is the machine address that we want to execute the input command, default is localhost:8088)

To add a file into the SDFS

```
./client sdfs put [hostname] [port] <localfilename> <sdfsfilename>

# example (uploading model, testset, and label)
./client sdfs put src/ml_cluster/ml_scripts/vit_model.py vit_model
./client sdfs put src/ml_cluster/ml_scripts/vit_testset.testcase vit_testset
./client sdfs put src/ml_cluster/ml_scripts/vit_testset.label vit_label
```

#### After the membership group is created and the required files are uploaded, we can start the inference phase of the Distributed Learning Cluster
([hostname] and [port] is the machine address that we want to execute the input command, default is localhost:8088)

To add an inference job, include the model, testset, label filenames on SDFS, and the batch size that the job intends to use

```
./client ml add [hostname] [port] <job name> <model> <testset> <label> <batch_size>

# example
./client ml add job_vit1 vit_model vit_testset vit_label 5
```

To remove an inference job

```
./client ml remove [hostname] [port] <job name>
```

To start an inference job

```
./client ml start [hostname] [port] <job name>

# example
./client ml start job_vit1
```

To stop an inference job

```
./client ml stop [hostname] [port] <job name>
```

To show status (e.g. latest query rate, processed query count, query processing time statistics) of an inference job

```
./client ml status [hostname] [port] <job name>
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

