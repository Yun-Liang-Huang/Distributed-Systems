INTRODUCER=fa22-cs425-0402.cs.illinois.edu
PORT=8088

for CLUSTER_NUMBER in $(seq -w 1 10)
do
    ./client membership join fa22-cs425-04${CLUSTER_NUMBER}.cs.illinois.edu $PORT $INTRODUCER $PORT
    # sleep 1
done
