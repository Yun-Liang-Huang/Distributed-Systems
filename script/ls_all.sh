PORT=8088

for CLUSTER_NUMBER in $(seq -w 1 10)
do
    echo "================= ${CLUSTER_NUMBER} ====================="
    ./client sdfs ls fa22-cs425-04${CLUSTER_NUMBER}.cs.illinois.edu $PORT $1
    # sleep 1
done
