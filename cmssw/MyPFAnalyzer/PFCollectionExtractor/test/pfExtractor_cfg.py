# test/pfExtractor_cfg.py
import os
import FWCore.ParameterSet.Config as cms
from FWCore.ParameterSet.VarParsing import VarParsing

# Select MC truth consumption explicitly so one cfg can safely serve data and MC jobs.
# Use literal filenames, without the analysis preset's automatic maxEvents suffix.
options = VarParsing()
options.register("maxEvents", -1, VarParsing.multiplicity.singleton,
                 VarParsing.varType.int, "Maximum events, -1 for all input events")
options.register("inputFiles", [], VarParsing.multiplicity.list,
                 VarParsing.varType.string, "MiniAOD input files")
options.register("outputFile", "ofile.root", VarParsing.multiplicity.singleton,
                 VarParsing.varType.string, "Exact output ROOT filename")
options.register("payloadSHA256", "", VarParsing.multiplicity.singleton,
                 VarParsing.varType.string, "SHA256 of the submitted CMSSW source archive")
options.register("sourceLFN", "", VarParsing.multiplicity.singleton,
                 VarParsing.varType.string, "Original MiniAOD LFN before worker staging")
options.register(
    "isMC",
    False,
    VarParsing.multiplicity.singleton,
    VarParsing.varType.bool,
    "Read packed/pruned generator truth and require generator event weights",
)
options.parseArguments()
if not options.inputFiles:
    raise RuntimeError("Provide inputFiles=file:/path/to/MiniAOD.root or an XRootD URL")

process = cms.Process("PFExtraction")

process.load("FWCore.MessageService.MessageLogger_cfi")

# Source: Your MiniAOD file
process.source = cms.Source("PoolSource",
    fileNames = cms.untracked.vstring(options.inputFiles)
)

# The analyzer
process.pfExtractor = cms.EDAnalyzer('PFCollectionExtractor',
                                     isMC = cms.bool(options.isMC),
                                     sourceFiles = cms.vstring([options.sourceLFN] if options.sourceLFN else options.inputFiles),
                                     payloadSHA256 = cms.string(options.payloadSHA256),
                                     cmsswVersion = cms.string(os.environ.get("CMSSW_VERSION", "unknown")),
                                     pfCandidates = cms.InputTag("packedPFCandidates"),
                                     muons = cms.InputTag("slimmedMuons"),
                                     vertices = cms.InputTag("offlineSlimmedPrimaryVertices"),
                                     # Packed truth supplies all stored status-1 final states.
                                     packedGenParticles = cms.InputTag("packedGenParticles"),
                                     # Pruned truth supplies the retained decay history.
                                     prunedGenParticles = cms.InputTag("prunedGenParticles"),
                                     generatorInfo = cms.InputTag("generator"),
                                     jets = cms.InputTag("slimmedJets")
)

# Define the path to run
process.p = cms.Path(process.pfExtractor)

# Number of events to process (-1 means all)
process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents)
)

# The TFileService configuration to specify the output file
process.TFileService = cms.Service("TFileService",
    fileName = cms.string(options.outputFile)
)
