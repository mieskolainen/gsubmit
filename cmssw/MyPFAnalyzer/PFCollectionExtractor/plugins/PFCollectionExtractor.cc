// plugins/PFCollectionExtractor.cc
#include <memory>
#include <vector>
#include <algorithm>
#include <cmath>
#include <unordered_map>
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/Exception.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "DataFormats/PatCandidates/interface/PackedCandidate.h"
#include "DataFormats/PatCandidates/interface/PackedGenParticle.h"
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/PatCandidates/interface/CompositeCandidate.h"
#include "DataFormats/VertexReco/interface/Vertex.h"
#include "DataFormats/HepMCCandidate/interface/GenParticle.h"
#include "DataFormats/Common/interface/TriggerResults.h"
#include "FWCore/Common/interface/TriggerNames.h"
#include "DataFormats/PatCandidates/interface/TriggerObjectStandAlone.h"
#include "DataFormats/PatCandidates/interface/Jet.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"
#include "TTree.h"

// Serialize event buffers and TTree::Fill because TFileService output is shared.
class PFCollectionExtractor : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit PFCollectionExtractor(const edm::ParameterSet&);
  ~PFCollectionExtractor() override;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // Helper function to determine particle type from PDG ID
  int getParticleId(int pdgId) const;

  // ----------member data ---------------------------
  edm::EDGetTokenT<edm::View<pat::PackedCandidate>> pfCandidatesToken_;
  edm::EDGetTokenT<edm::View<pat::Muon>> muonsToken_;
  edm::EDGetTokenT<reco::VertexCollection> vertexToken_;
  // Read final states and decay history with their actual, distinct EDM types.
  edm::EDGetTokenT<pat::PackedGenParticleCollection> packedGenParticlesToken_;
  edm::EDGetTokenT<reco::GenParticleCollection> prunedGenParticlesToken_;
  edm::EDGetTokenT<edm::TriggerResults> triggerResultsToken_;
  edm::EDGetTokenT<edm::View<pat::Jet>> jetsToken_;
  edm::EDGetTokenT<GenEventInfoProduct> genToken_;
  bool isMC_;
  TTree *outTree_;
  TTree *summaryTree_;
  unsigned long long nProcessed_ = 0;
  unsigned long long nTriggerPassed_ = 0;
  double sumGenWeights_ = 0.;
  double sumGenWeightsSquared_ = 0.;
  double sumAbsGenWeights_ = 0.;
  std::vector<std::string> sourceFiles_;
  std::string payloadSHA256_;
  std::string cmsswVersion_;
  bool isRealData_ = false;

  // Per-event buffers are serialized by the SharedResources analyzer.
  int schemaVersion_ = 3;
  mutable int runNumber_;
  mutable int lumiNumber_;
  // Event IDs are 64-bit in EDM and must not be truncated in the ROOT output.
  mutable unsigned long long eventNumber_;
  mutable int nCands_;
  mutable int nMuons_;
  mutable int nGenParticles_;
  mutable int nPrunedGenParticles_;
  mutable int nVertices_; 

  // Gen Event information
  mutable double genWeight_;
  
  // PF Candidate properties - 4-vectors
  mutable std::vector<float> candPt_;
  mutable std::vector<float> candEta_;
  mutable std::vector<float> candPhi_;
  mutable std::vector<float> candMass_;
  mutable std::vector<float> candEnergy_;
  mutable std::vector<float> candPx_;
  mutable std::vector<float> candPy_;
  mutable std::vector<float> candPz_;
  
  // PF Candidate properties - Identity
  mutable std::vector<int> candPdgId_;
  mutable std::vector<int> candCharge_;
  mutable std::vector<float> candPuppiW_;
  mutable std::vector<int> candParticleId_;
  
  // PF Candidate properties - Quality
  mutable std::vector<int> candQualityFlags_;
  
  // PF Candidate properties - Vertex information
  mutable std::vector<float> candVx_;
  mutable std::vector<float> candVy_;
  mutable std::vector<float> candVz_;
  mutable std::vector<float> candDxy_;
  mutable std::vector<float> candDz_;
  mutable std::vector<float> candDxyError_;
  mutable std::vector<float> candDzError_;
  
  // PF Candidate properties - Track information
  mutable std::vector<float> candTrackPt_;
  mutable std::vector<float> candTrackEta_;
  mutable std::vector<float> candTrackPhi_;
  mutable std::vector<float> candTrackP_;
  mutable std::vector<int> candTrackCharge_;
  mutable std::vector<int> candNumberOfHits_;
  mutable std::vector<int> candNumberOfPixelHits_;
  mutable std::vector<int> candTrackHighPurity_;
  
  // PF Candidate properties - Time
  mutable std::vector<float> candTime_;
  mutable std::vector<float> candTimeError_;
  
  // PF Candidate properties - Status
  mutable std::vector<int> candFromPV_;
  mutable std::vector<int> candIsMuon_;
  mutable std::vector<int> candIsElectron_;
  mutable std::vector<int> candIsPhoton_;
  mutable std::vector<int> candIsIsolatedChargedHadron_;
  
  // Muon properties
  mutable std::vector<float> muonPt_;
  mutable std::vector<float> muonEta_;
  mutable std::vector<float> muonPhi_;
  mutable std::vector<float> muonMass_;
  mutable std::vector<float> muonEnergy_;
  mutable std::vector<float> muonPx_;
  mutable std::vector<float> muonPy_;
  mutable std::vector<float> muonPz_;
  mutable std::vector<int> muonCharge_;
  mutable std::vector<float> muonVx_;
  mutable std::vector<float> muonVy_;
  mutable std::vector<float> muonVz_;
  mutable std::vector<float> muonDxy_;
  mutable std::vector<float> muonDz_;
  mutable std::vector<float> muonDxyError_;
  mutable std::vector<float> muonDzError_;
  mutable std::vector<float> muonIsoCharged_;
  mutable std::vector<float> muonIsoNeutral_;
  mutable std::vector<float> muonIsoPhoton_;
  mutable std::vector<float> muonIsoPU_;
  mutable std::vector<float> muonIsoSum_;
  mutable std::vector<float> muonTrackIso_;  // trackIso() returns a float
  mutable std::vector<int> muonIsLoose_;
  mutable std::vector<int> muonIsMedium_;
  mutable std::vector<int> muonIsTight_;
  mutable std::vector<int> muonIsSoft_;
  mutable std::vector<int> muonIsHighPt_;
  mutable std::vector<int> muonIsTracker_;
  mutable std::vector<int> muonIsGlobal_;
  mutable std::vector<int> muonIsStandalone_;
  mutable std::vector<int> muonIsPF_;
  mutable std::vector<int> muonIsTrigMatched_;
  mutable std::vector<int> muonNumberOfHits_;
  mutable std::vector<int> muonNumberOfPixelHits_;
  mutable std::vector<int> muonNumberOfMatchedStations_;
  mutable std::vector<int> muonNumberOfValidHits_;
  mutable std::vector<float> muonChi2_;
  mutable std::vector<float> muonNdof_;
  mutable std::vector<float> muonInnerTrackPt_;
  mutable std::vector<float> muonInnerTrackEta_;
  mutable std::vector<float> muonInnerTrackPhi_;
  mutable std::vector<float> muonInnerTrackP_;
  mutable std::vector<float> muonOuterTrackPt_;
  mutable std::vector<float> muonOuterTrackEta_;
  mutable std::vector<float> muonOuterTrackPhi_;
  mutable std::vector<float> muonOuterTrackP_;

  // Trigger
  mutable int triggerFiredIsoMu24_;
  
  // GenParticle properties
  mutable std::vector<float> genPt_;
  mutable std::vector<float> genEta_;
  mutable std::vector<float> genPhi_;
  mutable std::vector<float> genMass_;
  mutable std::vector<float> genEnergy_;
  mutable std::vector<float> genPx_;
  mutable std::vector<float> genPy_;
  mutable std::vector<float> genPz_;
  mutable std::vector<int> genPdgId_;
  mutable std::vector<int> genCharge_;
  mutable std::vector<int> genStatus_;
  mutable std::vector<int> genMotherPrunedIndex_;

  // Pruned GenParticle properties (decay history)
  mutable std::vector<float> prunedGenPt_;
  mutable std::vector<float> prunedGenEta_;
  mutable std::vector<float> prunedGenPhi_;
  mutable std::vector<float> prunedGenMass_;
  mutable std::vector<float> prunedGenEnergy_;
  mutable std::vector<float> prunedGenPx_;
  mutable std::vector<float> prunedGenPy_;
  mutable std::vector<float> prunedGenPz_;
  mutable std::vector<int> prunedGenPdgId_;
  mutable std::vector<int> prunedGenCharge_;
  mutable std::vector<int> prunedGenStatus_;
  mutable std::vector<unsigned int> prunedGenMotherOffsets_;
  mutable std::vector<int> prunedGenMotherIndices_;
  mutable std::vector<unsigned int> prunedGenDaughterOffsets_;
  mutable std::vector<int> prunedGenDaughterIndices_;

// Jet properties
  mutable int nJets_;
  mutable std::vector<float> jetPt_;
  mutable std::vector<float> jetEta_;
  mutable std::vector<float> jetPhi_;
  mutable std::vector<float> jetMass_;
  mutable std::vector<float> jetEnergy_;
  mutable std::vector<float> jetPx_;
  mutable std::vector<float> jetPy_;
  mutable std::vector<float> jetPz_;
  mutable std::vector<float> jetArea_;
  mutable std::vector<float> jetRawPt_;
  mutable std::vector<float> jetRawMass_;
  mutable std::vector<float> jetNeutralHadronEnergy_;
  mutable std::vector<float> jetNeutralHadronFraction_;
  mutable std::vector<float> jetChargedHadronEnergy_;
  mutable std::vector<float> jetChargedHadronFraction_;
  mutable std::vector<float> jetPhotonEnergy_;
  mutable std::vector<float> jetPhotonFraction_;
  mutable std::vector<float> jetElectronEnergy_;
  mutable std::vector<float> jetElectronFraction_;
  mutable std::vector<float> jetMuonEnergy_;
  mutable std::vector<float> jetMuonFraction_;
  mutable std::vector<float> jetHFHadronEnergy_;
  mutable std::vector<float> jetHFHadronFraction_;
  mutable std::vector<float> jetHFEMEnergy_;
  mutable std::vector<float> jetHFEMFraction_;
  mutable std::vector<int> jetCharge_;
  mutable std::vector<int> jetParticleId_;
  mutable std::vector<float> jetBTagCSVv2_;
  mutable std::vector<float> jetBTagDeepB_;
  mutable std::vector<float> jetBTagDeepC_;
  mutable std::vector<float> jetBTagDeepFlavB_;
  mutable std::vector<int> jetNConstituents_;
  mutable std::vector<int> jetNCharged_;
  mutable std::vector<int> jetNNeutrals_;
  mutable std::vector<int> jetNPV_;
  mutable std::vector<float> jetPUID_;
};

PFCollectionExtractor::PFCollectionExtractor(const edm::ParameterSet& iConfig)
{
  // Prevent concurrent mutation of the shared vectors and ROOT tree.
  usesResource(TFileService::kSharedResource);

  // Only consume required truth products for MC, so the same module also runs on data.
  isMC_ = iConfig.getParameter<bool>("isMC");
  pfCandidatesToken_ = consumes<edm::View<pat::PackedCandidate>>(iConfig.getParameter<edm::InputTag>("pfCandidates"));
  muonsToken_ = consumes<edm::View<pat::Muon>>(iConfig.getParameter<edm::InputTag>("muons"));
  vertexToken_ = consumes<reco::VertexCollection>(iConfig.getParameter<edm::InputTag>("vertices"));
  triggerResultsToken_ = consumes<edm::TriggerResults>(edm::InputTag("TriggerResults", "", "HLT"));
  jetsToken_ = consumes<edm::View<pat::Jet>>(iConfig.getParameter<edm::InputTag>("jets"));
  if (isMC_) {
    packedGenParticlesToken_ =
        consumes<pat::PackedGenParticleCollection>(iConfig.getParameter<edm::InputTag>("packedGenParticles"));
    prunedGenParticlesToken_ =
        consumes<reco::GenParticleCollection>(iConfig.getParameter<edm::InputTag>("prunedGenParticles"));
    genToken_ = consumes<GenEventInfoProduct>(iConfig.getParameter<edm::InputTag>("generatorInfo"));
  }
  
  // Access the TFileService to book the TTree
  edm::Service<TFileService> fs;
  outTree_ = fs->make<TTree>("pfTree", "Particle Flow Candidates and Muons");
  sourceFiles_ = iConfig.getParameter<std::vector<std::string>>("sourceFiles");
  payloadSHA256_ = iConfig.getParameter<std::string>("payloadSHA256");
  cmsswVersion_ = iConfig.getParameter<std::string>("cmsswVersion");
  summaryTree_ = fs->make<TTree>("eventSummary", "Inclusive processed-event normalization and provenance");
  summaryTree_->Branch("schemaVersion", &schemaVersion_);
  summaryTree_->Branch("isMC", &isMC_);
  summaryTree_->Branch("nProcessed", &nProcessed_);
  summaryTree_->Branch("nTriggerPassed", &nTriggerPassed_);
  summaryTree_->Branch("sumGenWeights", &sumGenWeights_);
  summaryTree_->Branch("sumGenWeightsSquared", &sumGenWeightsSquared_);
  summaryTree_->Branch("sumAbsGenWeights", &sumAbsGenWeights_);
  summaryTree_->Branch("sourceFiles", &sourceFiles_);
  summaryTree_->Branch("payloadSHA256", &payloadSHA256_);
  summaryTree_->Branch("cmsswVersion", &cmsswVersion_);
  
  // Event information branches
  // Version the changed truth schema so stale files are rejected unambiguously.
  outTree_->Branch("schemaVersion", &schemaVersion_, "schemaVersion/I");
  outTree_->Branch("run", &runNumber_, "run/I");
  outTree_->Branch("lumi", &lumiNumber_, "lumi/I");
  outTree_->Branch("event", &eventNumber_, "event/l");
  outTree_->Branch("genWeight", &genWeight_, "genWeight/D");
  outTree_->Branch("isRealData", &isRealData_);
  outTree_->Branch("nCands", &nCands_, "nCands/I");
  outTree_->Branch("nMuons", &nMuons_, "nMuons/I");
  // Persist both truth-table sizes so scalar counts always match stored vectors.
  outTree_->Branch("nGenParticles", &nGenParticles_, "nGenParticles/I");
  outTree_->Branch("nPrunedGenParticles", &nPrunedGenParticles_, "nPrunedGenParticles/I");
  outTree_->Branch("nVertices", &nVertices_, "nVertices/I");
  
  // PF Candidate branches
  outTree_->Branch("candPt", &candPt_);
  outTree_->Branch("candEta", &candEta_);
  outTree_->Branch("candPhi", &candPhi_);
  outTree_->Branch("candMass", &candMass_);
  outTree_->Branch("candEnergy", &candEnergy_);
  outTree_->Branch("candPx", &candPx_);
  outTree_->Branch("candPy", &candPy_);
  outTree_->Branch("candPz", &candPz_);
  outTree_->Branch("candPdgId", &candPdgId_);
  outTree_->Branch("candCharge", &candCharge_);
  outTree_->Branch("candParticleId", &candParticleId_);
  outTree_->Branch("candPuppiW", &candPuppiW_);
  outTree_->Branch("candQualityFlags", &candQualityFlags_);
  outTree_->Branch("candVx", &candVx_);
  outTree_->Branch("candVy", &candVy_);
  outTree_->Branch("candVz", &candVz_);
  outTree_->Branch("candDxy", &candDxy_);
  outTree_->Branch("candDz", &candDz_);
  outTree_->Branch("candDxyError", &candDxyError_);
  outTree_->Branch("candDzError", &candDzError_);
  outTree_->Branch("candTrackPt", &candTrackPt_);
  outTree_->Branch("candTrackEta", &candTrackEta_);
  outTree_->Branch("candTrackPhi", &candTrackPhi_);
  outTree_->Branch("candTrackP", &candTrackP_);
  outTree_->Branch("candTrackCharge", &candTrackCharge_);
  outTree_->Branch("candNumberOfHits", &candNumberOfHits_);
  outTree_->Branch("candNumberOfPixelHits", &candNumberOfPixelHits_);
  outTree_->Branch("candTrackHighPurity", &candTrackHighPurity_);
  outTree_->Branch("candTime", &candTime_);
  outTree_->Branch("candTimeError", &candTimeError_);
  outTree_->Branch("candFromPV", &candFromPV_);
  outTree_->Branch("candIsMuon", &candIsMuon_);
  outTree_->Branch("candIsElectron", &candIsElectron_);
  outTree_->Branch("candIsPhoton", &candIsPhoton_);
  outTree_->Branch("candIsIsolatedChargedHadron", &candIsIsolatedChargedHadron_);
  
  // Muon branches
  outTree_->Branch("muonPt", &muonPt_);
  outTree_->Branch("muonEta", &muonEta_);
  outTree_->Branch("muonPhi", &muonPhi_);
  outTree_->Branch("muonMass", &muonMass_);
  outTree_->Branch("muonEnergy", &muonEnergy_);
  outTree_->Branch("muonPx", &muonPx_);
  outTree_->Branch("muonPy", &muonPy_);
  outTree_->Branch("muonPz", &muonPz_);
  outTree_->Branch("muonCharge", &muonCharge_);
  outTree_->Branch("muonVx", &muonVx_);
  outTree_->Branch("muonVy", &muonVy_);
  outTree_->Branch("muonVz", &muonVz_);
  outTree_->Branch("muonDxy", &muonDxy_);
  outTree_->Branch("muonDz", &muonDz_);
  outTree_->Branch("muonDxyError", &muonDxyError_);
  outTree_->Branch("muonDzError", &muonDzError_);
  outTree_->Branch("muonIsoCharged", &muonIsoCharged_);
  outTree_->Branch("muonIsoNeutral", &muonIsoNeutral_);
  outTree_->Branch("muonIsoPhoton", &muonIsoPhoton_);
  outTree_->Branch("muonIsoPU", &muonIsoPU_);
  outTree_->Branch("muonIsoSum", &muonIsoSum_);
  outTree_->Branch("muonTrackIso", &muonTrackIso_);
  outTree_->Branch("muonIsLoose", &muonIsLoose_);
  outTree_->Branch("muonIsMedium", &muonIsMedium_);
  outTree_->Branch("muonIsTight", &muonIsTight_);
  outTree_->Branch("muonIsSoft", &muonIsSoft_);
  outTree_->Branch("muonIsHighPt", &muonIsHighPt_);
  outTree_->Branch("muonIsTracker", &muonIsTracker_);
  outTree_->Branch("muonIsGlobal", &muonIsGlobal_);
  outTree_->Branch("muonIsStandalone", &muonIsStandalone_);
  outTree_->Branch("muonIsPF", &muonIsPF_);
  outTree_->Branch("muonIsTrigMatched", &muonIsTrigMatched_);
  outTree_->Branch("muonNumberOfHits", &muonNumberOfHits_);
  outTree_->Branch("muonNumberOfPixelHits", &muonNumberOfPixelHits_);
  outTree_->Branch("muonNumberOfMatchedStations", &muonNumberOfMatchedStations_);
  outTree_->Branch("muonNumberOfValidHits", &muonNumberOfValidHits_);
  outTree_->Branch("muonChi2", &muonChi2_);
  outTree_->Branch("muonNdof", &muonNdof_);
  outTree_->Branch("muonInnerTrackPt", &muonInnerTrackPt_);
  outTree_->Branch("muonInnerTrackEta", &muonInnerTrackEta_);
  outTree_->Branch("muonInnerTrackPhi", &muonInnerTrackPhi_);
  outTree_->Branch("muonInnerTrackP", &muonInnerTrackP_);
  outTree_->Branch("muonOuterTrackPt", &muonOuterTrackPt_);
  outTree_->Branch("muonOuterTrackEta", &muonOuterTrackEta_);
  outTree_->Branch("muonOuterTrackPhi", &muonOuterTrackPhi_);
  outTree_->Branch("muonOuterTrackP", &muonOuterTrackP_);

  outTree_->Branch("genPt",&genPt_);
  outTree_->Branch("genEta",&genEta_);
  outTree_->Branch("genPhi",&genPhi_);
  outTree_->Branch("genMass",&genMass_);
  outTree_->Branch("genEnergy",&genEnergy_);
  outTree_->Branch("genPx",&genPx_);
  outTree_->Branch("genPy",&genPy_);
  outTree_->Branch("genPz",&genPz_);
  outTree_->Branch("genPdgId",&genPdgId_);
  outTree_->Branch("genCharge",&genCharge_);
  outTree_->Branch("genStatus",&genStatus_);
  // Link each packed final state to its first retained ancestor in the pruned table.
  outTree_->Branch("genMotherPrunedIndex",&genMotherPrunedIndex_);

  // Store selected decay history separately to avoid packed/pruned duplicates.
  outTree_->Branch("prunedGenPt", &prunedGenPt_);
  outTree_->Branch("prunedGenEta", &prunedGenEta_);
  outTree_->Branch("prunedGenPhi", &prunedGenPhi_);
  outTree_->Branch("prunedGenMass", &prunedGenMass_);
  outTree_->Branch("prunedGenEnergy", &prunedGenEnergy_);
  outTree_->Branch("prunedGenPx", &prunedGenPx_);
  outTree_->Branch("prunedGenPy", &prunedGenPy_);
  outTree_->Branch("prunedGenPz", &prunedGenPz_);
  outTree_->Branch("prunedGenPdgId", &prunedGenPdgId_);
  outTree_->Branch("prunedGenCharge", &prunedGenCharge_);
  outTree_->Branch("prunedGenStatus", &prunedGenStatus_);
  outTree_->Branch("prunedGenMotherOffsets", &prunedGenMotherOffsets_);
  outTree_->Branch("prunedGenMotherIndices", &prunedGenMotherIndices_);
  outTree_->Branch("prunedGenDaughterOffsets", &prunedGenDaughterOffsets_);
  outTree_->Branch("prunedGenDaughterIndices", &prunedGenDaughterIndices_);
  
  // Trigger branches
  outTree_->Branch("IsoMu24", &triggerFiredIsoMu24_, "IsoMu24/I");

  // Add jet branches
  outTree_->Branch("nJets", &nJets_, "nJets/I");
  outTree_->Branch("jetPt", &jetPt_);
  outTree_->Branch("jetEta", &jetEta_);
  outTree_->Branch("jetPhi", &jetPhi_);
  outTree_->Branch("jetMass", &jetMass_);
  outTree_->Branch("jetEnergy", &jetEnergy_);
  outTree_->Branch("jetPx", &jetPx_);
  outTree_->Branch("jetPy", &jetPy_);
  outTree_->Branch("jetPz", &jetPz_);
  outTree_->Branch("jetArea", &jetArea_);
  outTree_->Branch("jetRawPt", &jetRawPt_);
  outTree_->Branch("jetRawMass", &jetRawMass_);
  outTree_->Branch("jetNeutralHadronEnergy", &jetNeutralHadronEnergy_);
  outTree_->Branch("jetNeutralHadronFraction", &jetNeutralHadronFraction_);
  outTree_->Branch("jetChargedHadronEnergy", &jetChargedHadronEnergy_);
  outTree_->Branch("jetChargedHadronFraction", &jetChargedHadronFraction_);
  outTree_->Branch("jetPhotonEnergy", &jetPhotonEnergy_);
  outTree_->Branch("jetPhotonFraction", &jetPhotonFraction_);
  outTree_->Branch("jetElectronEnergy", &jetElectronEnergy_);
  outTree_->Branch("jetElectronFraction", &jetElectronFraction_);
  outTree_->Branch("jetMuonEnergy", &jetMuonEnergy_);
  outTree_->Branch("jetMuonFraction", &jetMuonFraction_);
  outTree_->Branch("jetHFHadronEnergy", &jetHFHadronEnergy_);
  outTree_->Branch("jetHFHadronFraction", &jetHFHadronFraction_);
  outTree_->Branch("jetHFEMEnergy", &jetHFEMEnergy_);
  outTree_->Branch("jetHFEMFraction", &jetHFEMFraction_);
  outTree_->Branch("jetCharge", &jetCharge_);
  outTree_->Branch("jetParticleId", &jetParticleId_);
  outTree_->Branch("jetBTagCSVv2", &jetBTagCSVv2_);
  outTree_->Branch("jetBTagDeepB", &jetBTagDeepB_);
  outTree_->Branch("jetBTagDeepC", &jetBTagDeepC_);
  outTree_->Branch("jetBTagDeepFlavB", &jetBTagDeepFlavB_);
  outTree_->Branch("jetNConstituents", &jetNConstituents_);
  outTree_->Branch("jetNCharged", &jetNCharged_);
  outTree_->Branch("jetNNeutrals", &jetNNeutrals_);
  outTree_->Branch("jetNPV", &jetNPV_);
  outTree_->Branch("jetPUID", &jetPUID_);
}

PFCollectionExtractor::~PFCollectionExtractor() { }

int PFCollectionExtractor::getParticleId(int pdgId) const {
  int absPdg = std::abs(pdgId);
  
  // Forward calorimeter candidates use detector-specific pseudo-PDG IDs.
  if (absPdg == 1) return 6;
  if (absPdg == 2) return 7;
  // Photon
  if (absPdg == 22) return 3;
  // Electron
  if (absPdg == 11) return 4;
  // Muon
  if (absPdg == 13) return 5;
  // Charged hadrons represented by packed PF candidates.
  if (absPdg == 211 || absPdg == 321 || absPdg == 2212) return 1;
  // Neutral hadrons represented by packed PF candidates.
  if (absPdg == 111 || absPdg == 130 || absPdg == 310 || absPdg == 2112) return 2;
  // Unknown
  return 0;
}

void PFCollectionExtractor::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  using namespace edm;
  
  // Get event information
  runNumber_ = iEvent.id().run();
  lumiNumber_ = iEvent.id().luminosityBlock();
  eventNumber_ = iEvent.id().event();
  isRealData_ = iEvent.isRealData();

  // Check the EDM event flag before entering either the data or MC path.
  // A forgotten isMC=True must never silently produce empty MC truth.
  if (isMC_ == iEvent.isRealData()) {
    throw cms::Exception("Configuration")
        << "PFCollectionExtractor: isMC=" << isMC_
        << " disagrees with Event::isRealData()=" << iEvent.isRealData()
        << " in event " << iEvent.id()
        << ". Pass isMC=True for simulation or isMC=False for collision data.";
  }

  // Generator event weight. Data events use the neutral multiplicative weight.
  genWeight_ = 1.0;
  if (isMC_) {
    const auto genEventInfo = iEvent.getHandle(genToken_);
    if (!genEventInfo.isValid()) {
      throw cms::Exception("ProductNotFound")
          << "PFCollectionExtractor is configured with isMC=True, but generatorInfo is missing in event "
          << runNumber_ << ":" << lumiNumber_ << ":" << eventNumber_ << ". Check the generatorInfo InputTag.";
    }
    genWeight_ = genEventInfo->weight();
    if (!std::isfinite(genWeight_)) {
      throw cms::Exception("InvalidMCWeight")
          << "Nonfinite generator weight in event " << iEvent.id();
    }
  }

  // Get trigger info
  edm::Handle<edm::TriggerResults> triggerResults;
  iEvent.getByToken(triggerResultsToken_, triggerResults);
  if (!triggerResults.isValid()) {
    throw cms::Exception("ProductNotFound")
        << "Missing HLT TriggerResults in event " << iEvent.id()
        << ". Trigger efficiency cannot be measured from an absent product.";
  }
  
  triggerFiredIsoMu24_ = 0;
  if (triggerResults.isValid()) {
    const edm::TriggerNames& triggerNames = iEvent.triggerNames(*triggerResults);
    for (size_t i = 0; i < triggerNames.size(); ++i) {
      const std::string& triggerName = triggerNames.triggerName(i);
      if (triggerName.find("HLT_IsoMu24") == 0 && triggerResults->accept(i))
	triggerFiredIsoMu24_ = 1;
    }
  }
  
  
  // Get primary vertex for muon ID
  Handle<reco::VertexCollection> vertices;
  iEvent.getByToken(vertexToken_, vertices);

  // Count good vertices (with ndof > 4 and |z| < 24 cm, rho < 2 cm)
  nVertices_ = 0;
  
  const reco::Vertex& primaryVertex = vertices->empty() ? reco::Vertex() : vertices->front();

  
  for (const auto& vtx : *vertices) {
    // Standard criteria for good vertices in CMS
    // Avoid using the primary vertex in counting if you want to count all vertices
    if (!vtx.isFake() && vtx.ndof() > 4 && 
        std::abs(vtx.z()) < 24.0 && 
        vtx.position().rho() < 2.0) {
      nVertices_++;
    }
  }
  
  
  
  // ============================================
  // Process PF Candidates
  // ============================================
  Handle<View<pat::PackedCandidate>> pfCandidates;
  iEvent.getByToken(pfCandidatesToken_, pfCandidates);
  nCands_ = pfCandidates->size();

  // Clear PF candidate vectors
  candPt_.clear();
  candEta_.clear();
  candPhi_.clear();
  candMass_.clear();
  candEnergy_.clear();
  candPx_.clear();
  candPy_.clear();
  candPz_.clear();
  candPdgId_.clear();
  candCharge_.clear();
  candPuppiW_.clear();
  candParticleId_.clear();
  candQualityFlags_.clear();
  candVx_.clear();
  candVy_.clear();
  candVz_.clear();
  candDxy_.clear();
  candDz_.clear();
  candDxyError_.clear();
  candDzError_.clear();
  candTrackPt_.clear();
  candTrackEta_.clear();
  candTrackPhi_.clear();
  candTrackP_.clear();
  candTrackCharge_.clear();
  candNumberOfHits_.clear();
  candNumberOfPixelHits_.clear();
  candTrackHighPurity_.clear();
  candTime_.clear();
  candTimeError_.clear();
  candFromPV_.clear();
  candIsMuon_.clear();
  candIsElectron_.clear();
  candIsPhoton_.clear();
  candIsIsolatedChargedHadron_.clear();

  // Reserve space
  candPt_.reserve(nCands_);
  candEta_.reserve(nCands_);
  candPhi_.reserve(nCands_);
  candMass_.reserve(nCands_);
  candEnergy_.reserve(nCands_);
  candPx_.reserve(nCands_);
  candPy_.reserve(nCands_);
  candPz_.reserve(nCands_);
  candPdgId_.reserve(nCands_);
  candCharge_.reserve(nCands_);
  candPuppiW_.reserve(nCands_);
  candParticleId_.reserve(nCands_);
  candQualityFlags_.reserve(nCands_);
  candVx_.reserve(nCands_);
  candVy_.reserve(nCands_);
  candVz_.reserve(nCands_);
  candDxy_.reserve(nCands_);
  candDz_.reserve(nCands_);
  candDxyError_.reserve(nCands_);
  candDzError_.reserve(nCands_);
  candTrackPt_.reserve(nCands_);
  candTrackEta_.reserve(nCands_);
  candTrackPhi_.reserve(nCands_);
  candTrackP_.reserve(nCands_);
  candTrackCharge_.reserve(nCands_);
  candNumberOfHits_.reserve(nCands_);
  candNumberOfPixelHits_.reserve(nCands_);
  candTrackHighPurity_.reserve(nCands_);
  candTime_.reserve(nCands_);
  candTimeError_.reserve(nCands_);
  candFromPV_.reserve(nCands_);
  candIsMuon_.reserve(nCands_);
  candIsElectron_.reserve(nCands_);
  candIsPhoton_.reserve(nCands_);
  candIsIsolatedChargedHadron_.reserve(nCands_);

  // Sort PF candidates by pT
  std::vector<const pat::PackedCandidate*> sortedCands;
  sortedCands.reserve(nCands_);
  for (const auto& cand : *pfCandidates) {
    sortedCands.push_back(&cand);
  }
  std::sort(sortedCands.begin(), sortedCands.end(),
            [](const pat::PackedCandidate* a, const pat::PackedCandidate* b) {
              return a->pt() > b->pt();
            });

  // Fill PF candidate vectors
  for (const auto* cand : sortedCands) {
    if (cand->charge()==0)
      continue;
    //if (cand->puppiWeight()==0.)
    //  continue;
    if (cand->fromPV()<2)
      continue;
    candPt_.push_back(cand->pt());
    candEta_.push_back(cand->eta());
    candPhi_.push_back(cand->phi());
    candMass_.push_back(cand->mass());
    candEnergy_.push_back(cand->energy());
    candPx_.push_back(cand->px());
    candPy_.push_back(cand->py());
    candPz_.push_back(cand->pz());
    candPdgId_.push_back(cand->pdgId());
    candCharge_.push_back(cand->charge());
    candPuppiW_.push_back(cand->puppiWeight());
    candParticleId_.push_back(getParticleId(cand->pdgId()));
    
    uint16_t qualityFlags = 0;
    if (cand->trackHighPurity()) qualityFlags |= 1 << 0;
    if (cand->hasTrackDetails()) qualityFlags |= 1 << 1;
    if (cand->isMuon()) qualityFlags |= 1 << 2;
    if (cand->isElectron()) qualityFlags |= 1 << 3;
    if (cand->isPhoton()) qualityFlags |= 1 << 4;
    if (cand->isIsolatedChargedHadron()) qualityFlags |= 1 << 5;
    candQualityFlags_.push_back(qualityFlags);
    
    candVx_.push_back(cand->vx());
    candVy_.push_back(cand->vy());
    candVz_.push_back(cand->vz());
    candDxy_.push_back(cand->dxy());
    candDz_.push_back(cand->dz());
    
    if (cand->hasTrackDetails()) {
      candDxyError_.push_back(cand->dxyError());
      candDzError_.push_back(cand->dzError());
    } else {
      candDxyError_.push_back(-999);
      candDzError_.push_back(-999);
    }
    
    if (cand->hasTrackDetails()) {
      candTrackPt_.push_back(cand->pt());
      candTrackEta_.push_back(cand->eta());
      candTrackPhi_.push_back(cand->phi());
      candTrackP_.push_back(cand->p());
      candTrackCharge_.push_back(cand->charge());
      candNumberOfHits_.push_back(cand->numberOfHits());
      candNumberOfPixelHits_.push_back(cand->numberOfPixelHits());
      candTrackHighPurity_.push_back(cand->trackHighPurity() ? 1 : 0);
    } else {
      candTrackPt_.push_back(-999);
      candTrackEta_.push_back(-999);
      candTrackPhi_.push_back(-999);
      candTrackP_.push_back(-999);
      candTrackCharge_.push_back(0);
      candNumberOfHits_.push_back(0);
      candNumberOfPixelHits_.push_back(0);
      candTrackHighPurity_.push_back(0);
    }
    
    candTime_.push_back(cand->time());
    candTimeError_.push_back(cand->timeError());
    candFromPV_.push_back(cand->fromPV());
    candIsMuon_.push_back(cand->isMuon() ? 1 : 0);
    candIsElectron_.push_back(cand->isElectron() ? 1 : 0);
    candIsPhoton_.push_back(cand->isPhoton() ? 1 : 0);
    candIsIsolatedChargedHadron_.push_back(cand->isIsolatedChargedHadron() ? 1 : 0);
  }
  // Report the number actually written after charge/from-PV filtering.
  nCands_ = static_cast<int>(candPt_.size());

  // ============================================
  // Process Muons
  // ============================================
  Handle<View<pat::Muon>> muons;
  iEvent.getByToken(muonsToken_, muons);
  nMuons_ = muons->size();

  // Clear muon vectors
  muonPt_.clear();
  muonEta_.clear();
  muonPhi_.clear();
  muonMass_.clear();
  muonEnergy_.clear();
  muonPx_.clear();
  muonPy_.clear();
  muonPz_.clear();
  muonCharge_.clear();
  muonVx_.clear();
  muonVy_.clear();
  muonVz_.clear();
  muonDxy_.clear();
  muonDz_.clear();
  muonDxyError_.clear();
  muonDzError_.clear();
  muonIsoCharged_.clear();
  muonIsoNeutral_.clear();
  muonIsoPhoton_.clear();
  muonIsoPU_.clear();
  muonIsoSum_.clear();
  muonTrackIso_.clear();
  muonIsLoose_.clear();
  muonIsMedium_.clear();
  muonIsTight_.clear();
  muonIsSoft_.clear();
  muonIsHighPt_.clear();
  muonIsTracker_.clear();
  muonIsGlobal_.clear();
  muonIsStandalone_.clear();
  muonIsPF_.clear();
  muonIsTrigMatched_.clear();
  muonNumberOfHits_.clear();
  muonNumberOfPixelHits_.clear();
  muonNumberOfMatchedStations_.clear();
  muonNumberOfValidHits_.clear();
  muonChi2_.clear();
  muonNdof_.clear();
  muonInnerTrackPt_.clear();
  muonInnerTrackEta_.clear();
  muonInnerTrackPhi_.clear();
  muonInnerTrackP_.clear();
  muonOuterTrackPt_.clear();
  muonOuterTrackEta_.clear();
  muonOuterTrackPhi_.clear();
  muonOuterTrackP_.clear();

  // Reserve space
  muonPt_.reserve(nMuons_);
  muonEta_.reserve(nMuons_);
  muonPhi_.reserve(nMuons_);
  muonMass_.reserve(nMuons_);
  muonEnergy_.reserve(nMuons_);
  muonPx_.reserve(nMuons_);
  muonPy_.reserve(nMuons_);
  muonPz_.reserve(nMuons_);
  muonCharge_.reserve(nMuons_);
  muonVx_.reserve(nMuons_);
  muonVy_.reserve(nMuons_);
  muonVz_.reserve(nMuons_);
  muonDxy_.reserve(nMuons_);
  muonDz_.reserve(nMuons_);
  muonDxyError_.reserve(nMuons_);
  muonDzError_.reserve(nMuons_);
  muonIsoCharged_.reserve(nMuons_);
  muonIsoNeutral_.reserve(nMuons_);
  muonIsoPhoton_.reserve(nMuons_);
  muonIsoPU_.reserve(nMuons_);
  muonIsoSum_.reserve(nMuons_);
  muonTrackIso_.reserve(nMuons_);
  muonIsLoose_.reserve(nMuons_);
  muonIsMedium_.reserve(nMuons_);
  muonIsTight_.reserve(nMuons_);
  muonIsSoft_.reserve(nMuons_);
  muonIsHighPt_.reserve(nMuons_);
  muonIsTracker_.reserve(nMuons_);
  muonIsGlobal_.reserve(nMuons_);
  muonIsStandalone_.reserve(nMuons_);
  muonIsPF_.reserve(nMuons_);
  muonIsTrigMatched_.reserve(nMuons_);
  muonNumberOfHits_.reserve(nMuons_);
  muonNumberOfPixelHits_.reserve(nMuons_);
  muonNumberOfMatchedStations_.reserve(nMuons_);
  muonNumberOfValidHits_.reserve(nMuons_);
  muonChi2_.reserve(nMuons_);
  muonNdof_.reserve(nMuons_);
  muonInnerTrackPt_.reserve(nMuons_);
  muonInnerTrackEta_.reserve(nMuons_);
  muonInnerTrackPhi_.reserve(nMuons_);
  muonInnerTrackP_.reserve(nMuons_);
  muonOuterTrackPt_.reserve(nMuons_);
  muonOuterTrackEta_.reserve(nMuons_);
  muonOuterTrackPhi_.reserve(nMuons_);
  muonOuterTrackP_.reserve(nMuons_);

  // Sort muons by pT
  std::vector<const pat::Muon*> sortedMuons;
  sortedMuons.reserve(nMuons_);
  for (const auto& muon : *muons) {
    sortedMuons.push_back(&muon);
  }
  std::sort(sortedMuons.begin(), sortedMuons.end(),
            [](const pat::Muon* a, const pat::Muon* b) {
              return a->pt() > b->pt();
            });

  // Fill muon vectors
  for (const auto* muon : sortedMuons) {
    // 4-vectors
    muonPt_.push_back(muon->pt());
    muonEta_.push_back(muon->eta());
    muonPhi_.push_back(muon->phi());
    muonMass_.push_back(muon->mass());
    muonEnergy_.push_back(muon->energy());
    muonPx_.push_back(muon->px());
    muonPy_.push_back(muon->py());
    muonPz_.push_back(muon->pz());
    muonCharge_.push_back(muon->charge());
    
    // Vertex
    muonVx_.push_back(muon->vx());
    muonVy_.push_back(muon->vy());
    muonVz_.push_back(muon->vz());
    
    // Dxy and Dz from inner track
    if (muon->innerTrack().isNonnull()) {
      muonDxy_.push_back(muon->innerTrack()->dxy(primaryVertex.position()));
      muonDz_.push_back(muon->innerTrack()->dz(primaryVertex.position()));
      muonDxyError_.push_back(muon->innerTrack()->dxyError());
      muonDzError_.push_back(muon->innerTrack()->dzError());
    } else {
      muonDxy_.push_back(-999);
      muonDz_.push_back(-999);
      muonDxyError_.push_back(-999);
      muonDzError_.push_back(-999);
    }
    
    // PF Isolation R04
    muonIsoCharged_.push_back(muon->pfIsolationR04().sumChargedHadronPt);
    muonIsoNeutral_.push_back(muon->pfIsolationR04().sumNeutralHadronEt);
    muonIsoPhoton_.push_back(muon->pfIsolationR04().sumPhotonEt);
    muonIsoPU_.push_back(muon->pfIsolationR04().sumPUPt);
    muonIsoSum_.push_back(muon->pfIsolationR04().sumChargedHadronPt + 
                          muon->pfIsolationR04().sumNeutralHadronEt + 
                          muon->pfIsolationR04().sumPhotonEt);
    
    // Track Isolation - returns a float
    muonTrackIso_.push_back(muon->trackIso());
    
    // Muon ID
    muonIsLoose_.push_back(muon->isLooseMuon() ? 1 : 0);
    muonIsMedium_.push_back(muon->isMediumMuon() ? 1 : 0);
    muonIsTight_.push_back(muon->isTightMuon(primaryVertex) ? 1 : 0);
    muonIsSoft_.push_back(muon->isSoftMuon(primaryVertex) ? 1 : 0);
    muonIsHighPt_.push_back(muon->isHighPtMuon(primaryVertex) ? 1 : 0);
    
    // Muon type
    muonIsTracker_.push_back(muon->isTrackerMuon() ? 1 : 0);
    muonIsGlobal_.push_back(muon->isGlobalMuon() ? 1 : 0);
    muonIsStandalone_.push_back(muon->isStandAloneMuon() ? 1 : 0);
    muonIsPF_.push_back(muon->isPFMuon() ? 1 : 0);

    // Check if muon fired trigger
    const pat::TriggerObjectStandAlone* trigObj = muon->triggerObjectMatchByPath("HLT_IsoMu24_v*");
    muonIsTrigMatched_.push_back(trigObj ? 1 : 0);
    
    // Track information
    if (muon->innerTrack().isNonnull()) {
      muonNumberOfHits_.push_back(muon->innerTrack()->numberOfValidHits());
      muonNumberOfPixelHits_.push_back(muon->innerTrack()->hitPattern().numberOfValidPixelHits());
    } else {
      muonNumberOfHits_.push_back(0);
      muonNumberOfPixelHits_.push_back(0);
    }
    
    muonNumberOfMatchedStations_.push_back(muon->numberOfMatchedStations());
    muonNumberOfValidHits_.push_back(muon->numberOfValidHits());
    
    // Fit quality
    if (muon->globalTrack().isNonnull()) {
      muonChi2_.push_back(muon->globalTrack()->chi2());
      muonNdof_.push_back(muon->globalTrack()->ndof());
    } else {
      muonChi2_.push_back(-999);
      muonNdof_.push_back(-999);
    }
    
    // Inner track
    if (muon->innerTrack().isNonnull()) {
      muonInnerTrackPt_.push_back(muon->innerTrack()->pt());
      muonInnerTrackEta_.push_back(muon->innerTrack()->eta());
      muonInnerTrackPhi_.push_back(muon->innerTrack()->phi());
      muonInnerTrackP_.push_back(muon->innerTrack()->p());
    } else {
      muonInnerTrackPt_.push_back(-999);
      muonInnerTrackEta_.push_back(-999);
      muonInnerTrackPhi_.push_back(-999);
      muonInnerTrackP_.push_back(-999);
    }
    
    // Outer track
    if (muon->outerTrack().isNonnull()) {
      muonOuterTrackPt_.push_back(muon->outerTrack()->pt());
      muonOuterTrackEta_.push_back(muon->outerTrack()->eta());
      muonOuterTrackPhi_.push_back(muon->outerTrack()->phi());
      muonOuterTrackP_.push_back(muon->outerTrack()->p());
    } else {
      muonOuterTrackPt_.push_back(-999);
      muonOuterTrackEta_.push_back(-999);
      muonOuterTrackPhi_.push_back(-999);
      muonOuterTrackP_.push_back(-999);
    }
  }

  // ============================================
  // Process generator truth
  //
  // packedGenParticles contains the stable final state, while
  // prunedGenParticles contains the selected decay history.  They are kept in
  // separate ROOT tables to avoid duplicating status-1 particles.
  // ============================================
  nGenParticles_ = 0;
  nPrunedGenParticles_ = 0;

  genPt_.clear();
  genEta_.clear();
  genPhi_.clear();
  genMass_.clear();
  genEnergy_.clear();
  genPx_.clear();
  genPy_.clear();
  genPz_.clear();
  genPdgId_.clear();
  genCharge_.clear();
  genStatus_.clear();
  genMotherPrunedIndex_.clear();

  prunedGenPt_.clear();
  prunedGenEta_.clear();
  prunedGenPhi_.clear();
  prunedGenMass_.clear();
  prunedGenEnergy_.clear();
  prunedGenPx_.clear();
  prunedGenPy_.clear();
  prunedGenPz_.clear();
  prunedGenPdgId_.clear();
  prunedGenCharge_.clear();
  prunedGenStatus_.clear();
  prunedGenMotherOffsets_.clear();
  prunedGenMotherIndices_.clear();
  prunedGenDaughterOffsets_.clear();
  prunedGenDaughterIndices_.clear();
  prunedGenMotherOffsets_.push_back(0);
  prunedGenDaughterOffsets_.push_back(0);

  if (isMC_) {
    // Fail loudly on MC instead of silently writing empty truth collections.
    const auto packedGenParticles = iEvent.getHandle(packedGenParticlesToken_);
    if (!packedGenParticles.isValid()) {
      throw cms::Exception("ProductNotFound")
          << "PFCollectionExtractor is configured with isMC=True, but packedGenParticles is missing in event "
          << runNumber_ << ":" << lumiNumber_ << ":" << eventNumber_ << ". Check the packedGenParticles InputTag.";
    }

    const auto prunedGenParticles = iEvent.getHandle(prunedGenParticlesToken_);
    if (!prunedGenParticles.isValid()) {
      throw cms::Exception("ProductNotFound")
          << "PFCollectionExtractor is configured with isMC=True, but prunedGenParticles is missing in event "
          << runNumber_ << ":" << lumiNumber_ << ":" << eventNumber_ << ". Check the prunedGenParticles InputTag.";
    }

    // DY response construction requires a populated final state and decay history.
    if (packedGenParticles->empty() || prunedGenParticles->empty()) {
      throw cms::Exception("EmptyMCTruth")
          << "Empty packed or pruned generator collection in event " << iEvent.id();
    }

    std::vector<const reco::GenParticle*> sortedPrunedGenParticles;
    sortedPrunedGenParticles.reserve(prunedGenParticles->size());
    for (const auto& gen : *prunedGenParticles) {
      sortedPrunedGenParticles.push_back(&gen);
    }
    std::sort(sortedPrunedGenParticles.begin(),
              sortedPrunedGenParticles.end(),
              [](const reco::GenParticle* a, const reco::GenParticle* b) { return a->pt() > b->pt(); });

    const auto nPruned = sortedPrunedGenParticles.size();
    prunedGenPt_.reserve(nPruned);
    prunedGenEta_.reserve(nPruned);
    prunedGenPhi_.reserve(nPruned);
    prunedGenMass_.reserve(nPruned);
    prunedGenEnergy_.reserve(nPruned);
    prunedGenPx_.reserve(nPruned);
    prunedGenPy_.reserve(nPruned);
    prunedGenPz_.reserve(nPruned);
    prunedGenPdgId_.reserve(nPruned);
    prunedGenCharge_.reserve(nPruned);
    prunedGenStatus_.reserve(nPruned);
    prunedGenMotherOffsets_.reserve(nPruned + 1);
    prunedGenDaughterOffsets_.reserve(nPruned + 1);

    // Map original pruned pointers to their pT-sorted ROOT indices.
    std::unordered_map<const reco::GenParticle*, int> prunedIndex;
    prunedIndex.reserve(nPruned);
    for (size_t index = 0; index < nPruned; ++index) {
      prunedIndex.emplace(sortedPrunedGenParticles[index], static_cast<int>(index));
    }

    for (const auto* gen : sortedPrunedGenParticles) {
      prunedGenPt_.push_back(gen->pt());
      prunedGenEta_.push_back(gen->eta());
      prunedGenPhi_.push_back(gen->phi());
      prunedGenMass_.push_back(gen->mass());
      prunedGenEnergy_.push_back(gen->energy());
      prunedGenPx_.push_back(gen->px());
      prunedGenPy_.push_back(gen->py());
      prunedGenPz_.push_back(gen->pz());
      prunedGenPdgId_.push_back(gen->pdgId());
      prunedGenCharge_.push_back(gen->charge());
      prunedGenStatus_.push_back(gen->status());

      // Flatten all mother links; offsets delimit each particle's variable-length list.
      for (size_t motherIndex = 0; motherIndex < gen->numberOfMothers(); ++motherIndex) {
        const auto* mother = dynamic_cast<const reco::GenParticle*>(gen->mother(motherIndex));
        const auto found = prunedIndex.find(mother);
        prunedGenMotherIndices_.push_back(found == prunedIndex.end() ? -1 : found->second);
      }
      prunedGenMotherOffsets_.push_back(static_cast<unsigned int>(prunedGenMotherIndices_.size()));

      // Flatten all daughter links instead of dropping every daughter after the first.
      for (size_t daughterIndex = 0; daughterIndex < gen->numberOfDaughters(); ++daughterIndex) {
        const auto* daughter = dynamic_cast<const reco::GenParticle*>(gen->daughter(daughterIndex));
        const auto found = prunedIndex.find(daughter);
        prunedGenDaughterIndices_.push_back(found == prunedIndex.end() ? -1 : found->second);
      }
      prunedGenDaughterOffsets_.push_back(static_cast<unsigned int>(prunedGenDaughterIndices_.size()));
    }
    nPrunedGenParticles_ = static_cast<int>(prunedGenPt_.size());

    std::vector<const pat::PackedGenParticle*> sortedPackedGenParticles;
    sortedPackedGenParticles.reserve(packedGenParticles->size());
    for (const auto& gen : *packedGenParticles) {
      sortedPackedGenParticles.push_back(&gen);
    }
    std::sort(sortedPackedGenParticles.begin(),
              sortedPackedGenParticles.end(),
              [](const pat::PackedGenParticle* a, const pat::PackedGenParticle* b) { return a->pt() > b->pt(); });

    const auto nPacked = sortedPackedGenParticles.size();
    genPt_.reserve(nPacked);
    genEta_.reserve(nPacked);
    genPhi_.reserve(nPacked);
    genMass_.reserve(nPacked);
    genEnergy_.reserve(nPacked);
    genPx_.reserve(nPacked);
    genPy_.reserve(nPacked);
    genPz_.reserve(nPacked);
    genPdgId_.reserve(nPacked);
    genCharge_.reserve(nPacked);
    genStatus_.reserve(nPacked);
    genMotherPrunedIndex_.reserve(nPacked);

    for (const auto* gen : sortedPackedGenParticles) {
      genPt_.push_back(gen->pt());
      genEta_.push_back(gen->eta());
      genPhi_.push_back(gen->phi());
      genMass_.push_back(gen->mass());
      genEnergy_.push_back(gen->energy());
      genPx_.push_back(gen->px());
      genPy_.push_back(gen->py());
      genPz_.push_back(gen->pz());
      genPdgId_.push_back(gen->pdgId());
      genCharge_.push_back(gen->charge());
      genStatus_.push_back(gen->status());

      int motherIndex = -1;
      if (gen->numberOfMothers() > 0) {
        const auto* mother = dynamic_cast<const reco::GenParticle*>(gen->mother(0));
        const auto found = prunedIndex.find(mother);
        if (found != prunedIndex.end()) {
          motherIndex = found->second;
        }
      }
      genMotherPrunedIndex_.push_back(motherIndex);
    }
    nGenParticles_ = static_cast<int>(genPt_.size());
  }

  // ============================================
  // Process AK4 CHS Jets
  // ============================================
  Handle<View<pat::Jet>> jets;
  iEvent.getByToken(jetsToken_, jets);
  nJets_ = jets->size();
  // Clear jet vectors
  jetPt_.clear();
  jetEta_.clear();
  jetPhi_.clear();
  jetMass_.clear();
  jetEnergy_.clear();
  jetPx_.clear();
  jetPy_.clear();
  jetPz_.clear();
  jetArea_.clear();
  jetRawPt_.clear();
  jetRawMass_.clear();
  jetNeutralHadronEnergy_.clear();
  jetNeutralHadronFraction_.clear();
  jetChargedHadronEnergy_.clear();
  jetChargedHadronFraction_.clear();
  jetPhotonEnergy_.clear();
  jetPhotonFraction_.clear();
  jetElectronEnergy_.clear();
  jetElectronFraction_.clear();
  jetMuonEnergy_.clear();
  jetMuonFraction_.clear();
  jetHFHadronEnergy_.clear();
  jetHFHadronFraction_.clear();
  jetHFEMEnergy_.clear();
  jetHFEMFraction_.clear();
  jetCharge_.clear();
  jetParticleId_.clear();
  jetBTagCSVv2_.clear();
  jetBTagDeepB_.clear();
  jetBTagDeepC_.clear();
  jetBTagDeepFlavB_.clear();
  jetNConstituents_.clear();
  jetNCharged_.clear();
  jetNNeutrals_.clear();
  jetNPV_.clear();
  jetPUID_.clear();

  // Reserve space
  jetPt_.reserve(nJets_);
  jetEta_.reserve(nJets_);
  jetPhi_.reserve(nJets_);
  jetMass_.reserve(nJets_);
  jetEnergy_.reserve(nJets_);
  jetPx_.reserve(nJets_);
  jetPy_.reserve(nJets_);
  jetPz_.reserve(nJets_);
  jetArea_.reserve(nJets_);
  jetRawPt_.reserve(nJets_);
  jetRawMass_.reserve(nJets_);
  jetNeutralHadronEnergy_.reserve(nJets_);
  jetNeutralHadronFraction_.reserve(nJets_);
  jetChargedHadronEnergy_.reserve(nJets_);
  jetChargedHadronFraction_.reserve(nJets_);
  jetPhotonEnergy_.reserve(nJets_);
  jetPhotonFraction_.reserve(nJets_);
  jetElectronEnergy_.reserve(nJets_);
  jetElectronFraction_.reserve(nJets_);
  jetMuonEnergy_.reserve(nJets_);
  jetMuonFraction_.reserve(nJets_);
  jetHFHadronEnergy_.reserve(nJets_);
  jetHFHadronFraction_.reserve(nJets_);
  jetHFEMEnergy_.reserve(nJets_);
  jetHFEMFraction_.reserve(nJets_);
  jetCharge_.reserve(nJets_);
  jetParticleId_.reserve(nJets_);
  jetBTagCSVv2_.reserve(nJets_);
  jetBTagDeepB_.reserve(nJets_);
  jetBTagDeepC_.reserve(nJets_);
  jetBTagDeepFlavB_.reserve(nJets_);
  jetNConstituents_.reserve(nJets_);
  jetNCharged_.reserve(nJets_);
  jetNNeutrals_.reserve(nJets_);
  jetNPV_.reserve(nJets_);
  jetPUID_.reserve(nJets_);


  // Sort jets by pT
  std::vector<const pat::Jet*> sortedJets;
  sortedJets.reserve(nJets_);
  for (const auto& jet : *jets) {
    sortedJets.push_back(&jet);
  }
  std::sort(sortedJets.begin(), sortedJets.end(),
          [](const pat::Jet* a, const pat::Jet* b) {
            return a->pt() > b->pt();
          });

  // Fill jet vectors
  for (const auto* jet : sortedJets) {
    // 4-vectors
    jetPt_.push_back(jet->pt());
    jetEta_.push_back(jet->eta());
    jetPhi_.push_back(jet->phi());
    jetMass_.push_back(jet->mass());
    jetEnergy_.push_back(jet->energy());
    jetPx_.push_back(jet->px());
    jetPy_.push_back(jet->py());
    jetPz_.push_back(jet->pz());
    
    // Jet properties
    jetArea_.push_back(jet->jetArea());
    jetRawPt_.push_back(jet->correctedJet("Uncorrected").pt());
    jetRawMass_.push_back(jet->correctedJet("Uncorrected").mass());

    // Constituent fractions
    jetNeutralHadronEnergy_.push_back(jet->neutralHadronEnergy());
    jetNeutralHadronFraction_.push_back(jet->neutralHadronEnergyFraction());
    jetChargedHadronEnergy_.push_back(jet->chargedHadronEnergy());
    jetChargedHadronFraction_.push_back(jet->chargedHadronEnergyFraction());
    jetPhotonEnergy_.push_back(jet->photonEnergy());
    jetPhotonFraction_.push_back(jet->photonEnergyFraction());
    jetElectronEnergy_.push_back(jet->electronEnergy());
    jetElectronFraction_.push_back(jet->electronEnergyFraction());
    jetMuonEnergy_.push_back(jet->muonEnergy());
    jetMuonFraction_.push_back(jet->muonEnergyFraction());
    jetHFHadronEnergy_.push_back(jet->HFHadronEnergy());
    jetHFHadronFraction_.push_back(jet->HFHadronEnergyFraction());
    jetHFEMEnergy_.push_back(jet->HFEMEnergy());
    jetHFEMFraction_.push_back(jet->HFEMEnergyFraction());

    // Identity
    jetCharge_.push_back(jet->charge());
    jetParticleId_.push_back(getParticleId(jet->pdgId()));
    
    // B-tagging discriminators (check if they exist)
    float csvv2 = -999;
    float deepB = -999;
    float deepC = -999;
    float deepFlavB = -999;
    
    // Try to get CSVv2
    if (jet->hasUserFloat("pfCombinedInclusiveSecondaryVertexV2BJetTags")) {
      csvv2 = jet->userFloat("pfCombinedInclusiveSecondaryVertexV2BJetTags");
    }
    // Try to get DeepCSV
    if (jet->hasUserFloat("pfDeepCSVJetTags:probb")) {
      deepB = jet->userFloat("pfDeepCSVJetTags:probb");
    }
    if (jet->hasUserFloat("pfDeepCSVJetTags:probc")) {
      deepC = jet->userFloat("pfDeepCSVJetTags:probc");
    }
    // Try to get DeepFlav
    if (jet->hasUserFloat("pfDeepFlavourJetTags:probb")) {
      deepFlavB = jet->userFloat("pfDeepFlavourJetTags:probb");
    }
  
    jetBTagCSVv2_.push_back(csvv2);
    jetBTagDeepB_.push_back(deepB);
    jetBTagDeepC_.push_back(deepC);
    jetBTagDeepFlavB_.push_back(deepFlavB);
  
    // Constituent counts
    jetNConstituents_.push_back(jet->numberOfDaughters());
    jetNCharged_.push_back(jet->chargedMultiplicity());
    jetNNeutrals_.push_back(jet->neutralMultiplicity());
    
    // Pileup ID - stored as user float
    float puID = -999;
    if (jet->hasUserFloat("pileupJetId:fullDiscriminant")) {
      puID = jet->userFloat("pileupJetId:fullDiscriminant");
    }
    jetPUID_.push_back(puID);

    // Number of primary vertices associated with the jet (if available)
    int npv = -999;
    if (jet->hasUserInt("pileupJetId:npv")) {
      npv = jet->userInt("pileupJetId:npv");
    }
    jetNPV_.push_back(npv);
  }
    
  // Fill the TTree for this event
  outTree_->Fill();
  ++nProcessed_;
  nTriggerPassed_ += triggerFiredIsoMu24_ != 0;
  sumGenWeights_ += genWeight_;
  sumGenWeightsSquared_ += genWeight_ * genWeight_;
  sumAbsGenWeights_ += std::abs(genWeight_);
}

void PFCollectionExtractor::endJob() {
  summaryTree_->Fill();
}

void PFCollectionExtractor::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  // Switch truth access explicitly so missing MC products cannot masquerade as data.
  desc.add<bool>("isMC", false);
  desc.add<std::vector<std::string>>("sourceFiles", std::vector<std::string>{});
  desc.add<std::string>("payloadSHA256", "");
  desc.add<std::string>("cmsswVersion", "");
  desc.add<edm::InputTag>("pfCandidates", edm::InputTag("packedPFCandidates"));
  desc.add<edm::InputTag>("muons", edm::InputTag("slimmedMuons"));
  desc.add<edm::InputTag>("vertices", edm::InputTag("offlineSlimmedPrimaryVertices"));
  // Keep final-state and history labels separate because their C++ product types differ.
  desc.add<edm::InputTag>("packedGenParticles", edm::InputTag("packedGenParticles"));
  desc.add<edm::InputTag>("prunedGenParticles", edm::InputTag("prunedGenParticles"));
  desc.add<edm::InputTag>("generatorInfo", edm::InputTag("generator"));
  desc.add<edm::InputTag>("jets", edm::InputTag("slimmedJets"));
  descriptions.add("pfCollectionExtractor", desc);
}

DEFINE_FWK_MODULE(PFCollectionExtractor);
