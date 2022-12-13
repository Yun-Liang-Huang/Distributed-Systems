./client sdfs put ./src/ml_cluster/ml_scripts/vit_model.py mymodel
./client sdfs put ./src/ml_cluster/ml_scripts/vit_testset_small.testcase mytest
./client sdfs put ./src/ml_cluster/ml_scripts/vit_testset_small.label mylabel
./client ml add job1 mymodel mytest mylabel 5
./client ml start job1

./client membership join fa22-cs425-0407.cs.illinois.edu 8088 fa22-cs425-0410.cs.illinois.edu 8088
./client membership join fa22-cs425-0404.cs.illinois.edu 8088 fa22-cs425-0410.cs.illinois.edu 8088


# add all
script/add_all.sh

# init jobs
./client sdfs put ./src/ml_cluster/ml_scripts/vit_model.py my_vit_model
./client sdfs put ./src/ml_cluster/ml_scripts/vit_testset.testcase my_vit_test
./client sdfs put ./src/ml_cluster/ml_scripts/vit_testset.label my_vit_label
./client sdfs put ./src/ml_cluster/ml_scripts/vit_testset_large.testcase my_vit_test_lg
./client sdfs put ./src/ml_cluster/ml_scripts/vit_testset_large.label my_vit_label_lg

./client sdfs put ./src/ml_cluster/ml_scripts/bert_model.py my_bert_model
./client sdfs put ./src/ml_cluster/ml_scripts/bert_test_set.txt my_bert_test
./client sdfs put ./src/ml_cluster/ml_scripts/bert_test_set_label.txt my_bert_label

./client ml add job_bert_5 my_bert_model my_bert_test my_bert_label 5
./client ml add job_vit_5 my_vit_model my_vit_test my_vit_label 5
./client ml add job_vit_10 my_vit_model my_vit_test_lg my_vit_label_lg 10

./client ml start job_bert_5
./client ml start job_vit_5
./client ml start job_vit_10

# kill process
ps -aux | grep './server 8088' | awk '{print $2}' | head -n 1 | xargs -I {} kill {}

# delete all sdfs file
find . -name "*2oxY7SM9ky*" -exec /bin/rm {} \;
rm /tmp/file*

# one job
./client sdfs put ./src/ml_cluster/ml_scripts/vit_model.py my_vit_model
./client sdfs put ./src/ml_cluster/ml_scripts/vit_testset_small.testcase my_vit_test
./client sdfs put ./src/ml_cluster/ml_scripts/vit_testset_small.label my_vit_label
./client ml add job_vit_5 my_vit_model my_vit_test my_vit_label 5
./client ml start job_vit_5

# status
./client ml status job_vit_5
