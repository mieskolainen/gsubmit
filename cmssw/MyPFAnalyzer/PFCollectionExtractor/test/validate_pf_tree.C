#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

#include "TFile.h"
#include "TSystem.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"

namespace {
  void require(bool condition, const std::string& message) {
    if (!condition) {
      throw std::runtime_error(message);
    }
  }

  template <typename T>
  void requireSize(Long64_t event, const char* branch, const std::vector<T>& values, int expected) {
    require(static_cast<int>(values.size()) == expected,
            "event " + std::to_string(event) + ": " + branch + " length does not match its count branch");
  }

  void validateOffsets(Long64_t event,
                       const char* label,
                       const std::vector<unsigned int>& offsets,
                       const std::vector<int>& indices,
                       int particleCount) {
    // Offsets use the N+1 ragged-array convention, including {0} for an empty table.
    require(static_cast<int>(offsets.size()) == particleCount + 1,
            "event " + std::to_string(event) + ": invalid " + label + " offset count");
    require(!offsets.empty() && offsets.front() == 0 && offsets.back() == indices.size(),
            "event " + std::to_string(event) + ": " + label + " offsets do not span the flat index array");
    require(std::is_sorted(offsets.begin(), offsets.end()),
            "event " + std::to_string(event) + ": " + label + " offsets are not monotonic");
    require(std::all_of(indices.begin(), indices.end(), [particleCount](int index) {
              return index >= -1 && index < particleCount;
            }),
            "event " + std::to_string(event) + ": " + label + " contains an out-of-range index");
  }

  void validateImpl(const char* input, bool expectMC, bool requirePions, Long64_t maxEvents, Long64_t expectedEvents) {
    TFile file(input, "READ");
    require(!file.IsZombie(), std::string("cannot open ") + input);

    auto* tree = dynamic_cast<TTree*>(file.Get("pfExtractor/pfTree"));
    require(tree != nullptr, "missing pfExtractor/pfTree");
    require(tree->GetEntries() > 0, "tree has no events");
    require(expectedEvents < 0 || tree->GetEntries() == expectedEvents,
            "output entries disagree with FrameworkJobReport input EventsRead");

    const std::vector<std::string> requiredBranches = {
        "schemaVersion",
        "event",
        "genWeight",
        "isRealData",
        "IsoMu24",
        "nCands",
        "candPt",
        "nGenParticles",
        "nPrunedGenParticles",
        "genPt",
        "genEta",
        "genPhi",
        "genMass",
        "genEnergy",
        "genPx",
        "genPy",
        "genPz",
        "genPdgId",
        "genCharge",
        "genStatus",
        "genMotherPrunedIndex",
        "prunedGenPt",
        "prunedGenEta",
        "prunedGenPhi",
        "prunedGenMass",
        "prunedGenEnergy",
        "prunedGenPx",
        "prunedGenPy",
        "prunedGenPz",
        "prunedGenPdgId",
        "prunedGenCharge",
        "prunedGenStatus",
        "prunedGenMotherOffsets",
        "prunedGenMotherIndices",
        "prunedGenDaughterOffsets",
        "prunedGenDaughterIndices"};
    for (const auto& branch : requiredBranches) {
      require(tree->GetBranch(branch.c_str()) != nullptr, "missing branch " + branch);
    }

    TTreeReader reader(tree);
    TTreeReaderValue<int> schemaVersion(reader, "schemaVersion");
    TTreeReaderValue<double> genWeight(reader, "genWeight");
    TTreeReaderValue<bool> isRealData(reader, "isRealData");
    TTreeReaderValue<int> triggerFired(reader, "IsoMu24");
    TTreeReaderValue<int> nCands(reader, "nCands");
    TTreeReaderValue<std::vector<float>> candPt(reader, "candPt");
    TTreeReaderValue<int> nGenParticles(reader, "nGenParticles");
    TTreeReaderValue<int> nPrunedGenParticles(reader, "nPrunedGenParticles");

    TTreeReaderValue<std::vector<float>> genPt(reader, "genPt");
    TTreeReaderValue<std::vector<float>> genEta(reader, "genEta");
    TTreeReaderValue<std::vector<float>> genPhi(reader, "genPhi");
    TTreeReaderValue<std::vector<float>> genMass(reader, "genMass");
    TTreeReaderValue<std::vector<float>> genEnergy(reader, "genEnergy");
    TTreeReaderValue<std::vector<float>> genPx(reader, "genPx");
    TTreeReaderValue<std::vector<float>> genPy(reader, "genPy");
    TTreeReaderValue<std::vector<float>> genPz(reader, "genPz");
    TTreeReaderValue<std::vector<int>> genPdgId(reader, "genPdgId");
    TTreeReaderValue<std::vector<int>> genCharge(reader, "genCharge");
    TTreeReaderValue<std::vector<int>> genStatus(reader, "genStatus");
    TTreeReaderValue<std::vector<int>> genMotherPrunedIndex(reader, "genMotherPrunedIndex");

    TTreeReaderValue<std::vector<float>> prunedGenPt(reader, "prunedGenPt");
    TTreeReaderValue<std::vector<float>> prunedGenEta(reader, "prunedGenEta");
    TTreeReaderValue<std::vector<float>> prunedGenPhi(reader, "prunedGenPhi");
    TTreeReaderValue<std::vector<float>> prunedGenMass(reader, "prunedGenMass");
    TTreeReaderValue<std::vector<float>> prunedGenEnergy(reader, "prunedGenEnergy");
    TTreeReaderValue<std::vector<float>> prunedGenPx(reader, "prunedGenPx");
    TTreeReaderValue<std::vector<float>> prunedGenPy(reader, "prunedGenPy");
    TTreeReaderValue<std::vector<float>> prunedGenPz(reader, "prunedGenPz");
    TTreeReaderValue<std::vector<int>> prunedGenPdgId(reader, "prunedGenPdgId");
    TTreeReaderValue<std::vector<int>> prunedGenCharge(reader, "prunedGenCharge");
    TTreeReaderValue<std::vector<int>> prunedGenStatus(reader, "prunedGenStatus");
    TTreeReaderValue<std::vector<unsigned int>> prunedGenMotherOffsets(reader, "prunedGenMotherOffsets");
    TTreeReaderValue<std::vector<int>> prunedGenMotherIndices(reader, "prunedGenMotherIndices");
    TTreeReaderValue<std::vector<unsigned int>> prunedGenDaughterOffsets(reader, "prunedGenDaughterOffsets");
    TTreeReaderValue<std::vector<int>> prunedGenDaughterIndices(reader, "prunedGenDaughterIndices");

    Long64_t checked = 0;
    Long64_t chargedPions = 0;
    unsigned long long passed = 0;
    double sumw = 0., sumw2 = 0., sumabsw = 0.;
    while (reader.Next() && (maxEvents <= 0 || checked < maxEvents)) {
      const auto event = reader.GetCurrentEntry();
      const int packedCount = *nGenParticles;
      const int prunedCount = *nPrunedGenParticles;
      require(std::isfinite(*genWeight), "event " + std::to_string(event) + ": nonfinite generator weight");

      // Schema 3 preserves the dual truth layout and adds normalization provenance.
      require(*schemaVersion == 3, "event " + std::to_string(event) + ": unsupported schema version");
      require(*isRealData != expectMC, "event data/MC flag disagrees with requested validation mode");
      // Counts describe the filtered vectors actually written, not the source collections.
      requireSize(event, "candPt", *candPt, *nCands);
      requireSize(event, "genPt", *genPt, packedCount);
      requireSize(event, "genEta", *genEta, packedCount);
      requireSize(event, "genPhi", *genPhi, packedCount);
      requireSize(event, "genMass", *genMass, packedCount);
      requireSize(event, "genEnergy", *genEnergy, packedCount);
      requireSize(event, "genPx", *genPx, packedCount);
      requireSize(event, "genPy", *genPy, packedCount);
      requireSize(event, "genPz", *genPz, packedCount);
      requireSize(event, "genPdgId", *genPdgId, packedCount);
      requireSize(event, "genCharge", *genCharge, packedCount);
      requireSize(event, "genStatus", *genStatus, packedCount);
      requireSize(event, "genMotherPrunedIndex", *genMotherPrunedIndex, packedCount);

      requireSize(event, "prunedGenPt", *prunedGenPt, prunedCount);
      requireSize(event, "prunedGenEta", *prunedGenEta, prunedCount);
      requireSize(event, "prunedGenPhi", *prunedGenPhi, prunedCount);
      requireSize(event, "prunedGenMass", *prunedGenMass, prunedCount);
      requireSize(event, "prunedGenEnergy", *prunedGenEnergy, prunedCount);
      requireSize(event, "prunedGenPx", *prunedGenPx, prunedCount);
      requireSize(event, "prunedGenPy", *prunedGenPy, prunedCount);
      requireSize(event, "prunedGenPz", *prunedGenPz, prunedCount);
      requireSize(event, "prunedGenPdgId", *prunedGenPdgId, prunedCount);
      requireSize(event, "prunedGenCharge", *prunedGenCharge, prunedCount);
      requireSize(event, "prunedGenStatus", *prunedGenStatus, prunedCount);

      validateOffsets(event, "mother", *prunedGenMotherOffsets, *prunedGenMotherIndices, prunedCount);
      validateOffsets(event, "daughter", *prunedGenDaughterOffsets, *prunedGenDaughterIndices, prunedCount);

      // Packed mother indices point into the separate pruned table, or use -1 when unavailable.
      require(std::all_of(genMotherPrunedIndex->begin(),
                          genMotherPrunedIndex->end(),
                          [prunedCount](int index) { return index >= -1 && index < prunedCount; }),
              "event " + std::to_string(event) + ": invalid packed-to-pruned mother index");

      if (expectMC) {
        require(packedCount > 0 && prunedCount > 0,
                "event " + std::to_string(event) + ": MC truth table is unexpectedly empty");
        require(std::all_of(genStatus->begin(), genStatus->end(), [](int status) { return status == 1; }),
                "event " + std::to_string(event) + ": packed truth contains a non-status-1 particle");
        require(std::is_sorted(genPt->begin(), genPt->end(), std::greater<float>()),
                "event " + std::to_string(event) + ": packed truth is not sorted by decreasing pT");
        chargedPions += std::count_if(
            genPdgId->begin(), genPdgId->end(), [](int pdgId) { return std::abs(pdgId) == 211; });
      } else {
        require(packedCount == 0 && prunedCount == 0,
                "event " + std::to_string(event) + ": data event contains generator particles");
        require(*genWeight == 1.0f, "event " + std::to_string(event) + ": data generator weight is not 1");
      }
      sumw += *genWeight;
      sumw2 += *genWeight * *genWeight;
      sumabsw += std::abs(*genWeight);
      passed += *triggerFired != 0;
      ++checked;
    }

    const Long64_t expected = maxEvents <= 0 ? tree->GetEntries() : std::min(maxEvents, tree->GetEntries());
    require(checked == expected, "failed to read all requested tree entries");
    auto* summary = dynamic_cast<TTree*>(file.Get("pfExtractor/eventSummary"));
    require(summary != nullptr && summary->GetEntries() == 1, "missing single-row eventSummary");
    TTreeReader summaryReader(summary);
    TTreeReaderValue<int> summarySchema(summaryReader, "schemaVersion");
    TTreeReaderValue<bool> summaryMC(summaryReader, "isMC");
    TTreeReaderValue<unsigned long long> processed(summaryReader, "nProcessed");
    TTreeReaderValue<unsigned long long> triggered(summaryReader, "nTriggerPassed");
    TTreeReaderValue<double> totalw(summaryReader, "sumGenWeights");
    TTreeReaderValue<double> totalw2(summaryReader, "sumGenWeightsSquared");
    TTreeReaderValue<double> totalabsw(summaryReader, "sumAbsGenWeights");
    require(summaryReader.Next(), "unreadable eventSummary");
    require(*summarySchema == 3 && *summaryMC == expectMC, "invalid eventSummary schema or MC mode");
    require(*processed == static_cast<unsigned long long>(tree->GetEntries()), "eventSummary count differs from tree");
    require(std::isfinite(*totalw) && std::isfinite(*totalw2) && std::isfinite(*totalabsw),
            "nonfinite eventSummary weights");
    if (checked == tree->GetEntries()) {
      require(*triggered == passed, "eventSummary trigger count differs from tree");
      require(std::abs(*totalw - sumw) <= 1e-10 * std::max(1., sumabsw), "eventSummary sumw differs from tree");
      require(std::abs(*totalw2 - sumw2) <= 1e-10 * std::max(1., sumw2), "eventSummary sumw2 differs from tree");
      require(std::abs(*totalabsw - sumabsw) <= 1e-10 * std::max(1., sumabsw), "eventSummary sumabsw differs from tree");
    }
    require(!requirePions || chargedPions > 0,
            "no charged pions found in the checked packed-generator final states");
    std::cout << "validated " << checked << "/" << tree->GetEntries() << " events in " << input
              << "; charged packed pions found: " << chargedPions << std::endl;
  }
}  // namespace

void validate_pf_tree(const char* input, bool expectMC, bool requirePions = false, Long64_t maxEvents = 100,
                      Long64_t expectedEvents = -1) {
  try {
    validateImpl(input, expectMC, requirePions, maxEvents, expectedEvents);
  } catch (const std::exception& error) {
    std::cerr << "validation failed: " << error.what() << std::endl;
    // Return a nonzero process status so batch wrappers cannot upload a bad file.
    gSystem->Exit(1);
  }
}
