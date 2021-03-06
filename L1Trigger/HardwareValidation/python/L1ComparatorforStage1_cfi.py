import FWCore.ParameterSet.Config as cms

l1compareforstage1 = cms.EDProducer("L1Comparator",

    RCTsourceData = cms.InputTag("gctDigis"),
    RCTsourceEmul = cms.InputTag("valRctDigis"),
    
    GCTsourceData = cms.InputTag("caloStage1LegacyFormatDigis"),
    GCTsourceEmul = cms.InputTag("valCaloStage1LegacyFormatDigis"),
    
    DTPsourceData = cms.InputTag("dttfDigis"),
    DTPsourceEmul = cms.InputTag("valDtTriggerPrimitiveDigis"),

    DTFsourceData = cms.InputTag("dttfDigis"),
    DTFsourceEmul = cms.InputTag("valDttfDigis"),
    
    RPCsourceData = cms.InputTag("gtDigis"),
    RPCsourceEmul = cms.InputTag("valRpcTriggerDigis"),
    
    GMTsourceData = cms.InputTag("gtDigis"),
    GMTsourceEmul = cms.InputTag("valGmtDigis"),
    
    FEDsourceData = cms.untracked.InputTag("rawDataCollector"),
    FEDsourceEmul = cms.untracked.InputTag("rawDataCollector"),

    DumpMode = cms.untracked.int32(0),
    DumpFile = cms.untracked.string('dump.txt'),
    
    VerboseFlag = cms.untracked.int32(0),
    stage1_layer2_ = cms.bool(True),
    
    # ECAL TPG (ETP), HCAL TPG (HTP), CSC TF (CTF) LTC, GT not supported - expert modules in DQM
    COMPARE_COLLS = cms.untracked.vuint32(
       0,  0,  1,  1,  0,  1,  0,  0,  1,  0,  1, 0
    # ETP,HTP,RCT,GCT,DTP,DTF,CTP,CTF,RPC,LTC,GMT,GT
    )
)

