#!/bin/bash
#SBATCH -L work:0 -p tmp2 -q nolimit -t 2 -N 1
#SBATCH -A pr28fa

./makeme.sh
./API
