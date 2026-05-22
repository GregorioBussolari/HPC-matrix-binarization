#!/bin/bash

#SBATCH --account=tra25_Inginfbo
#SBATCH --partition=g100_usr_prod
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=48
#SBATCH -o job.out
#SBATCH -e job.err
#SBATCH --mail-user=anna.ciampolini@unibo.it
module load autoload intelmpi

if [ $# -lt 1 ]; then
    echo "Uso: sbatch launcher.sh <N>"
    exit 1
fi

N=$1

echo "Strong Scalability Test"
echo "Matrix size: ${N}x${N}"

for p in 1 2 4 8 16 24 32 40 48; do
    echo "Launching $p process on 1 node"
    # srun con numero di processi e numero di nodi calcolato
    srun -n $p ./bin $N
done
