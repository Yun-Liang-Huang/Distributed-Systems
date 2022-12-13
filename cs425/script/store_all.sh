PORT=8088

for CLUSTER_NUMBER in $(seq -w 1 10)
do
    echo "================= ${CLUSTER_NUMBER} ====================="
    ./client sdfs store fa22-cs425-04${CLUSTER_NUMBER}.cs.illinois.edu $PORT
    # sleep 1
done
