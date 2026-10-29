#!/bin/bash
# Run from an initialized, built CMSSW release with a genuine MiniAODSIM input.
set -euo pipefail
input=${1:?Usage: test_mc_truth.sh file:/path/to/MiniAODSIM.root [maxEvents]}
max_events=${2:-100}
test_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
work_dir=$(mktemp -d "${TMPDIR:-/tmp}/diffz-mc-truth.XXXXXX")
cd "$work_dir"

cmsRun -j FrameworkJobReport.xml "$test_dir/pfExtractor_cfg.py" \
  isMC=True inputFiles="$input" outputFile=ofile.root maxEvents="$max_events"
read_events=$(python3 "$test_dir/report_events.py" FrameworkJobReport.xml)
root -l -b -q "$test_dir/validate_pf_tree.C(\"ofile.root\",true,false,-1,$read_events)"
python3 "$test_dir/compare_truth.py" "$input" ofile.root

# The original failure mode must terminate before writing an event.
if cmsRun "$test_dir/pfExtractor_cfg.py" \
  isMC=False inputFiles="$input" outputFile=wrong_mode.root maxEvents=1 > wrong_mode.log 2>&1
then
  echo "ERROR: simulation was accepted in data mode" >&2
  exit 1
fi
if ! grep -q 'disagrees with Event::isRealData' wrong_mode.log
then
  cat wrong_mode.log >&2
  echo "ERROR: wrong-mode job failed for an unexpected reason" >&2
  exit 1
fi
echo "MC truth and wrong-mode checks passed. Outputs: $work_dir"
