# Note: you need to setup ssh-key first
# please follow the steps:
# 1. ssh-keygen -t rsa -f ~/.ssh/cs425 -C cs425
# 2. ssh-copy-id -i ~/.ssh/cs425 username@host         # so you may have to repeat this step 10 times to distribute your key to 10 machines

# Example:
# ./cluster_cmd.sh hyhuang3 ls
# ./cluster_cmd.sh hyhuang3 'cd cs425 && git pull && make clean; make'

ILLINI_ACCOUNT=$1
COMMAND=$2
GROUP_NUMBER=04
for CLUSTER_NUMBER in $(seq -w 1 10)
do
    echo "Executing command for ${CLUSTER_NUMBER} server"
    ssh -i ~/.ssh/cs425 ${ILLINI_ACCOUNT}@fa22-cs425-${GROUP_NUMBER}${CLUSTER_NUMBER}.cs.illinois.edu "${COMMAND}" &
done
