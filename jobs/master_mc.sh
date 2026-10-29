#!/bin/bash
set -euo pipefail
worker_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
exec bash "$worker_dir/run_ntuplizer.sh" "${1:?Missing MiniAOD LFN}" \
  "${2:?Missing output campaign URL}/DYToMuMu_pomflux_Pt-30_TuneCP5_13TeV-pythia8" True
