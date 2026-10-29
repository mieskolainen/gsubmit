"""Read the primary-input event count from a successful CMSSW job report."""

import sys
import xml.etree.ElementTree as ET


def events_read(path):
    root = ET.parse(path).getroot()
    if root.findall(".//FrameworkError"):
        raise ValueError("FrameworkJobReport contains a FrameworkError")
    inputs = [node for node in root.findall(".//InputFile") if node.findtext("InputType", "primaryFiles").strip() == "primaryFiles"]
    if not inputs:
        raise ValueError("FrameworkJobReport has no primary input files")
    counts = [int(node.findtext("EventsRead", "-1")) for node in inputs]
    if any(count < 0 for count in counts) or sum(counts) == 0:
        raise ValueError("FrameworkJobReport has missing or empty EventsRead")
    return sum(counts)


if __name__ == "__main__":
    print(events_read(sys.argv[1]))
