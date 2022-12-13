# Note: you need to setup ssh-key first
# please follow the steps:
# 1. ssh-keygen -t rsa -f ~/.ssh/cs425 -C cs425
# 2. ssh-copy-id -i ~/.ssh/cs425 username@host         # so you may have to repeat this step 10 times to distribute your key to 10 machines

# Example:
# ./stop_cluster_server.sh hyhuang3

ILLINI_ACCOUNT=$1
GROUP_NUMBER=04
for CLUSTER_NUMBER in $(seq -w 1 10)
do
    echo "Executing command for ${CLUSTER_NUMBER} server"
    ssh -i ~/.ssh/cs425 ${ILLINI_ACCOUNT}@fa22-cs425-${GROUP_NUMBER}${CLUSTER_NUMBER}.cs.illinois.edu "ps -aux | grep './server 8088' | awk '{print \$2}' | head -n 1 | xargs -I {} kill {}" &
done
