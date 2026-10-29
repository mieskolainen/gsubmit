#!/usr/bin/env python3
"""Build the Condor worker payload and checksum manifest from active sources."""

import gzip
import hashlib
import io
import json
from pathlib import Path
import tarfile


def archive_bytes(source, paths):
    buffer = io.BytesIO()
    with gzip.GzipFile(fileobj=buffer, mode="wb", mtime=0, filename="") as compressed:
        with tarfile.open(fileobj=compressed, mode="w") as archive:
            for path in sorted(paths):
                entry = archive.gettarinfo(str(path), arcname=str(path.relative_to(source)))
                entry.uid = entry.gid = entry.mtime = 0
                entry.uname = entry.gname = ""
                entry.mode = 0o755 if path.suffix == ".sh" else 0o644
                with path.open("rb") as stream:
                    archive.addfile(entry, stream)
    return buffer.getvalue()


def package(root):
    source = root / "cmssw"
    sources = [path for path in source.rglob("*") if path.is_file() and path.suffix in {".cc", ".C", ".py", ".xml", ".sh"}]
    payload = archive_bytes(source, sources)
    destination = root / "jobs" / "cmssw.tgz"
    temporary = destination.with_suffix(".tgz.tmp")
    temporary.write_bytes(payload)
    temporary.replace(destination)
    print(f"Packaged {len(payload)} bytes into jobs/cmssw.tgz")
    files = [*sources, root / "README.md", root / "package_cmssw.py", root / "prepare_submission.py"]
    files += [
        root / "jobs" / name for name in (
            "cmssw.tgz", "master.sh", "master_mc.sh", "master_mc_bkg.sh", "run_ntuplizer.sh", "submit", "submit_mc",
            "submit_mc_bkg", "files_2016H.txt", "mc.txt", "mc_bkg.txt")
    ]
    record = {
        "schema_version": 3,
        "sha256": {
            str(path.relative_to(root)): hashlib.sha256(path.read_bytes()).hexdigest()
            for path in sorted(files)
        }
    }
    manifest = root / "MANIFEST.json"
    manifest.write_text(json.dumps(record, indent=2) + "\n")


if __name__ == "__main__":
    package(Path(__file__).resolve().parent)
