#!/bin/bash

#SBATCH --account=tra25_inginfbo
#SBATCH --partition=g100_usr_prod

#SBATCH -t 00:05:00
#SBATCH --nodes=1
#SBATCH --ntasks-per-node=1  # Run a single task per node, more explicit than '-n 1'
#SBATCH -c 48                #  number of CPU cores i.e. OpenMP threads per task
#SBATCH -o OMPjob.out
#SBATCH -e OMPjob.err

if [ $# -lt 1 ]; then
    echo "Uso: sbatch launcher.sh <N>"
    exit 1
fi

N=$1

echo "Weak Scalability Test"
echo "Starting from size: ${N}x${N}"


for I in 1 2 4 8 12 16 24 28 32 40 48; do
    Ntot=$(echo "($N * sqrt($I) + 0.999)/1" | bc)
    echo "Threads: $I Matrix: ${Ntot}x${Ntot}"
    srun --cpu-bind=cores ./bin $I $Ntot
done

