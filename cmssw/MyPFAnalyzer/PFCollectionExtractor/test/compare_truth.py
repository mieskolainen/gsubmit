"""Compare every stored truth row and signed weight to its source MiniAOD event."""

import argparse
import math
import struct


def float32(value):
    return struct.unpack("f", struct.pack("f", value))[0]


def compare(source, output):
    import ROOT
    from DataFormats.FWLite import Events, Handle

    root = ROOT.TFile.Open(output)
    if not root or root.IsZombie():
        raise RuntimeError("Cannot read output ROOT file")
    tree = root.Get("pfExtractor/pfTree")
    if not tree or tree.GetEntries() == 0:
        raise RuntimeError("Missing or empty output tree")
    packed = Handle("std::vector<pat::PackedGenParticle>")
    pruned = Handle("std::vector<reco::GenParticle>")
    weight = Handle("GenEventInfoProduct")
    fields = ("Pt", "Eta", "Phi", "Mass", "Energy", "Px", "Py", "Pz", "PdgId", "Charge", "Status")
    checked = 0
    for index, event in enumerate(Events([source])):
        if index >= tree.GetEntries():
            break
        if tree.GetEntry(index) <= 0:
            raise RuntimeError("Unreadable output event")
        aux = event.eventAuxiliary()
        if aux.isRealData() or (tree.run, tree.lumi, tree.event) != (aux.run(), aux.luminosityBlock(), aux.event()):
            raise RuntimeError(f"Event identity mismatch at entry {index}")
        event.getByLabel("generator", weight)
        if not weight.isValid() or not math.isclose(tree.genWeight, weight.product().weight(), rel_tol=1e-14, abs_tol=0.):
            raise RuntimeError(f"Generator weight differs at entry {index}")
        for label, handle, prefix in (("packedGenParticles", packed, "gen"), ("prunedGenParticles", pruned, "prunedGen")):
            event.getByLabel(label, handle)
            if not handle.isValid():
                raise RuntimeError(f"Missing source {label}")
            expected = sorted(
                tuple(
                    float32(getattr(particle, field[0].lower() +
                                    field[1:])()) if column < 8 else int(getattr(particle, field[0].lower() + field[1:])())
                    for column, field in enumerate(fields)) for particle in handle.product())
            actual = sorted(zip(*(list(getattr(tree, prefix + field)) for field in fields)))
            if expected != actual:
                raise RuntimeError(f"{label} differs from source at entry {index}")
        checked += 1
    if checked != tree.GetEntries():
        raise RuntimeError("Source has fewer events than output")
    print(f"Source comparison passed for {checked} events, packed/pruned four-vectors, IDs and signed weights")
    root.Close()


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("input")
    parser.add_argument("output")
    args = parser.parse_args()
    compare(args.input, args.output)
