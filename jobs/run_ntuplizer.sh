#!/bin/bash
set -eo pipefail

input_lfn=${1:?Missing MiniAOD input LFN}
output_directory=${2:?Missing XRootD output directory}
is_mc=${3:?Missing True or False MC mode}
[[ "$input_lfn" == /store/* && "$input_lfn" == *.root ]]
[[ "$output_directory" == root://* ]]
[[ "$is_mc" == True || "$is_mc" == False ]]
input_name=$(basename "$input_lfn")
job_dir=$PWD
payload_sha256=$(sha256sum "$job_dir/cmssw.tgz" | cut -d ' ' -f 1)
xrdcp "root://cmsxrootd.fnal.gov/${input_lfn}" "$job_dir/$input_name"

# CMSSW environment scripts need not support nounset.
source /cvmfs/cms.cern.ch/cmsset_default.sh
export SCRAM_ARCH=el9_amd64_gcc12
scram p CMSSW CMSSW_14_0_0
cd CMSSW_14_0_0/src/
eval "$(scram runtime -sh)"
set -u
tar xzf "$job_dir/cmssw.tgz"
scram b -j 1
cd MyPFAnalyzer/PFCollectionExtractor/test/
cmsRun -j FrameworkJobReport.xml pfExtractor_cfg.py \
  isMC="$is_mc" inputFiles="file:$job_dir/$input_name" outputFile=ofile.root maxEvents=-1 \
  payloadSHA256="$payload_sha256" sourceLFN="$input_lfn"
read_events=$(python3 report_events.py FrameworkJobReport.xml)
root -l -b -q "validate_pf_tree.C(\"ofile.root\",${is_mc,,},false,-1,$read_events)"

# Publish only validated complete files. Existing outputs are not overwritten.
xrdcp --posc ofile.root "${output_directory%/}/$input_name"
echo "Done."
