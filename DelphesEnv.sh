DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" &> /dev/null && pwd )"

source /cvmfs/sft.cern.ch/lcg/views/LCG_100/x86_64-centos7-gcc9-opt/setup.sh
export DELPHES_HOME="$DIR"
export PYTHONPATH="$DIR/python:${PYTHONPATH}"
export LD_LIBRARY_PATH="$DIR:${LD_LIBRARY_PATH}"
export LIBRARY_PATH="$DIR:${LIBRARY_PATH}"
export HAS_PYTHIA8=true
