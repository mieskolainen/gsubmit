# Grid submission
### mikael.mieskolainen@cern.ch (2026)

Run on lxplus with the standalone package on EOS, for example `/eos/user/m/mamiesko/grid_submission`.

## 1. Setup

Set `ntuple_package` to your `grid_submission` directory on EOS. Run the following sections in the same shell.

```bash
ntuple_package=/eos/user/m/mamiesko/grid_submission
cd "$ntuple_package"
module load lxbatch/eossubmit
source /cvmfs/cms.cern.ch/cmsset_default.sh
voms-proxy-init --voms cms --valid 48:00
export X509_USER_PROXY=$(voms-proxy-info -path)
```

The worker payload is already packaged. After editing sources, rebuild it with `python3 "$ntuple_package/package_cmssw.py"`.

## 2. Test the analyzer

Required before production. Builds CMSSW and checks 100 events from each MC sample, including truth, signed weights, event counts and rejection of incorrect MC mode.

```bash
(
  set -eo pipefail
  export SCRAM_ARCH=el9_amd64_gcc12
  ntuple_test=$(mktemp -d /tmp/diffz-ntuplizer.XXXXXX)
  cd "$ntuple_test"
  scram p CMSSW CMSSW_14_0_0
  cd CMSSW_14_0_0/src
  eval "$(scram runtime -sh)"
  tar xzf "$ntuple_package/jobs/cmssw.tgz"
  scram b -j 4
  for ntuple_list in mc.txt mc_bkg.txt
  do
    read -r ntuple_input < "$ntuple_package/jobs/$ntuple_list"
    bash MyPFAnalyzer/PFCollectionExtractor/test/test_mc_truth.sh \
      "root://cmsxrootd.fnal.gov/$ntuple_input" 100
  done
)
```

## 3. Prepare and submit

Queues all data, SD and inclusive DY files to the existing project EOS location. One Condor job processes one MiniAOD file and validates it before upload.

```bash
(
  set -euo pipefail
  ntuple_campaign="fiducial_$(date -u +%Y%m%dT%H%M%SZ)"
  ntuple_jobs="$ntuple_package/campaigns/$ntuple_campaign"
  python3 "$ntuple_package/prepare_submission.py" \
    --output "$ntuple_jobs" \
    --eos-output "root://eosproject-a.cern.ch//eos/project/a/agentic/public/bonn/nopuppi"
  bash "$ntuple_jobs/submit_campaign.sh"
)
```

Add `--samples sd dy` to prepare MC only, or `--max-files 1` for a pilot. Preparation alone does not submit jobs. Do not rerun `submit_campaign.sh` unless intentionally resubmitting its file lists.

Check jobs with `condor_q`. Logs and `campaign.json` are under `campaigns/`. Validated ntuples go to the existing dataset subdirectories under `/eos/project/a/agentic/public/bonn/nopuppi`, matching `configs/diffz_workflow.yaml`. Workers do not overwrite existing files. Archive previous outputs before regenerating the same inputs.
