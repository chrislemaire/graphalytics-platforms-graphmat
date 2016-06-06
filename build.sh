GRAPHMAT_HOME=/var/scratch/wlngai/graphalytics/app/graphmatd/src

module load intel/compiler
module load intel-mpi
module load intel/mkl

mkdir -p bin/standard
(cd bin/standard && cmake -DCMAKE_BUILD_TYPE=Release ../../src -DGRAPHMAT_HOME=$GRAPHMAT_HOME && make all VERBOSE=1)

mkdir -p bin/granula
(cd bin/granula && cmake -DCMAKE_BUILD_TYPE=Release -DGRANULA=1 ../../src -DGRAPHMAT_HOME=$GRAPHMAT_HOME && make all VERBOSE=1)

rm -f bin/*/CMakeCache.txt
sed -i '40,63d' prepare-benchmark.sh
echo "echo This is the special distribution of GRAPHMAT.D" >> run-benchmark.sh

