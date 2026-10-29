#!/usr/bin/env python3
"""Freeze a complete ntuple production campaign without submitting jobs."""

import argparse
import hashlib
import json
from pathlib import Path
import re
import shlex
import shutil
from urllib.parse import urlsplit

SAMPLES = {
    "data": ("files_2016H.txt", "submit", "2016H"),
    "sd": ("mc.txt", "submit_mc", "DYToMuMu_pomflux_Pt-30_TuneCP5_13TeV-pythia8"),
    "dy": ("mc_bkg.txt", "submit_mc_bkg", "ZToMuMu_M-50To120_TuneCP5_13TeV-powheg-pythia8"),
}


def prepare(root, destination, output_url, samples, max_files=None):
    parsed = urlsplit(output_url)
    if parsed.scheme != "root" or not parsed.netloc or not parsed.path.startswith("//eos/"):
        raise ValueError("Use root://HOST//eos/PATH for the output campaign")
    if not re.fullmatch(r"root://[A-Za-z0-9.:-]+//eos/[A-Za-z0-9_./-]+", output_url):
        raise ValueError("Output URL contains unsupported characters")
    if max_files is not None and max_files < 1:
        raise ValueError("max-files must be positive")
    jobs = root / "jobs"
    # Reject stale or edited payloads before freezing a campaign.
    manifest = json.loads((root / "MANIFEST.json").read_text())
    for name, digest in manifest["sha256"].items():
        if hashlib.sha256((root / name).read_bytes()).hexdigest() != digest:
            raise ValueError(f"Package changed: {name}. Run package_cmssw.py first")
    queues = {}
    for sample in samples:
        listing, _, _ = SAMPLES[sample]
        entries = [line.strip() for line in (jobs / listing).read_text().splitlines() if line.strip()]
        if max_files is not None:
            entries = entries[:max_files]
        if not entries or any(not re.fullmatch(r"/store/[A-Za-z0-9_./-]+\.root", entry) for entry in entries):
            raise ValueError(f"Empty or malformed input list for {sample}")
        if len({Path(entry).name for entry in entries}) != len(entries):
            raise ValueError(f"Duplicate output basenames in {sample}")
        queues[sample] = entries
    destination.mkdir(parents=True, exist_ok=False)
    (destination / "logs").mkdir()
    for name in ("cmssw.tgz", "run_ntuplizer.sh", "master.sh", "master_mc.sh", "master_mc_bkg.sh"):
        shutil.copy2(jobs / name, destination / name)
    lines = [
        "#!/bin/bash", "set -euo pipefail", 'cd "$(dirname "${BASH_SOURCE[0]}")"',
        'export X509_USER_PROXY=${X509_USER_PROXY:-$(voms-proxy-info -path)}',
        'voms-proxy-info -file "$X509_USER_PROXY" -exists -valid 10:00', f"campaign_output={shlex.quote(output_url.rstrip('/'))}"
    ]
    for sample, entries in queues.items():
        listing, description, dataset = SAMPLES[sample]
        shutil.copy2(jobs / description, destination / description)
        (destination / listing).write_text("\n".join(entries) + "\n")
        lines += [
            f"xrdfs {shlex.quote(parsed.netloc)} mkdir -p {shlex.quote(parsed.path.rstrip('/') + '/' + dataset)}",
            f'condor_submit {description} "output_base=$campaign_output" "proxy_path=$X509_USER_PROXY"'
        ]
    runner = destination / "submit_campaign.sh"
    runner.write_text("\n".join(lines) + "\n")
    runner.chmod(0o755)
    record = {
        "output": output_url,
        "schema_version": 3,
        "payload_sha256": hashlib.sha256((jobs / "cmssw.tgz").read_bytes()).hexdigest(),
        "files_per_sample": {
            sample: len(entries)
            for sample, entries in queues.items()
        },
        "sha256": {
            path.name: hashlib.sha256(path.read_bytes()).hexdigest()
            for path in sorted(destination.iterdir()) if path.is_file()
        },
    }
    (destination / "campaign.json").write_text(json.dumps(record, indent=2) + "\n")
    print(json.dumps(record["files_per_sample"], sort_keys=True))
    print(f"Prepared {sum(map(len, queues.values()))} jobs. Submit with: bash {runner}")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--eos-output", required=True)
    parser.add_argument("--samples", nargs="+", choices=tuple(SAMPLES), default=list(SAMPLES))
    parser.add_argument("--max-files", type=int)
    args = parser.parse_args()
    prepare(
        Path(__file__).resolve().parent, args.output.resolve(), args.eos_output, list(dict.fromkeys(args.samples)), args.max_files)
