// 'SCorrelatorJetTree.h'
// Derek Anderson
// 12.04.2022
//
// Class to construct a tree of
// jets from a specified set of
// events.
//
// Derived from code by Antonio
// Silva (thanks!!)

#ifndef SCORRELATORJETTREE_H
#define SCORRELATORJETTREE_H

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

// standard c include
#include <string>
#include <vector>
#include <cassert>
#include <sstream>
// f4a include
#include <fun4all/SubsysReco.h>
#include <fun4all/Fun4AllReturnCodes.h>
#include <fun4all/Fun4AllHistoManager.h>
// phool includes
#include <phool/phool.h>
#include <phool/getClass.h>
#include <phool/PHIODataNode.h>
#include <phool/PHNodeIterator.h>
#include <phool/PHCompositeNode.h>
// g4 includes
#include <g4main/PHG4Hit.h>
#include <g4main/PHG4Particle.h>
#include <g4main/PHG4TruthInfoContainer.h>
#include <g4jets/Jet.h>
#include <g4jets/Jetv1.h>
#include <g4jets/JetMap.h>
#include <g4jets/JetMapv1.h>
#include <g4jets/FastJetAlgo.h>
#include <g4vertex/GlobalVertex.h>
#include <g4vertex/GlobalVertexMap.h>
// tracking includes
#include <trackbase_historic/SvtxTrack.h>
#include <trackbase_historic/SvtxVertex.h>
#include <trackbase_historic/SvtxTrackMap.h>
#include <trackbase_historic/SvtxVertexMap.h>
// calo includes
#include <calobase/RawCluster.h>
#include <calobase/RawClusterUtility.h>
#include <calobase/RawClusterContainer.h>
#include <calobase/RawTower.h>
#include <calobase/RawTowerGeom.h>
#include <calobase/RawTowerContainer.h>
#include <calobase/RawTowerGeomContainer.h>
#include <calotrigger/CaloTriggerInfo.h>
// particle flow includes
#include <particleflowreco/ParticleFlowElement.h>
#include <particleflowreco/ParticleFlowElementContainer.h>
// fastjet includes
#include <fastjet/PseudoJet.hh>
#include <fastjet/JetDefinition.hh>
#include <fastjet/ClusterSequence.hh>
#include <fastjet/FunctionOfPseudoJet.hh>
// hepmc includes
#include <HepMC/GenEvent.h>
#include <HepMC/GenVertex.h>
#include <HepMC/GenParticle.h>
#include <phhepmc/PHHepMCGenEvent.h>
#include <phhepmc/PHHepMCGenEventMap.h>
// root includes
#include <TH1.h>
#include <TFile.h>
#include <TTree.h>
#include <TMath.h>
#include <TNtuple.h>
#include <TDirectory.h>

#pragma GCC diagnostic pop

using namespace std;
using namespace fastjet;
using namespace findNode;

// forward declarations
class TH1;
class TFile;
class TTree;
class PHG4Particle;
class PHCompositeNode;
class PHHepMCGenEvent;
class PHHepMCGenEventMap;
class PHG4TruthInfoContainer;
class Fun4AllHistoManager;
class RawClusterContainer;
class RawCluster;
class GlobalVertex;
class SvtxTrackMap;
class JetRecoEval;
class SvtxTrackEval;
class SvtxTrack;
class ParticleFlowElement;

// global constants
static const size_t NPart(2);
static const size_t NComp(3);
static const size_t NRange(2);
static const size_t NMoment(2);
static const size_t NInfoQA(4);
static const size_t NJetType(2);
static const size_t NCstType(4);
static const size_t NObjType(9);
static const size_t NDirectory(NObjType - 3);
static const double MassPion(0.140);



class SCorrelatorJetTree : public SubsysReco {

  public:

    // enums
    enum ALGO {
      ANTIKT    = 0,
      KT        = 1,
      CAMBRIDGE = 2
    };
    enum RECOMB {
      E_SCHEME   = 0,
      PT_SCHEME  = 1,
      PT2_SCHEME = 2,
      ET_SCHEME  = 3,
      ET2_SCHEME = 4
    };
    enum OBJECT {
      TRACK  = 0,
      ECLUST = 1,
      HCLUST = 2,
      FLOW   = 3,
      PART   = 4,
      TJET   = 5,
      RJET   = 6,
      TCST   = 7,
      RCST   = 8
    };
    enum CST_TYPE {
      TRACK_CST = 0,
      CALO_CST  = 1,
      FLOW_CST  = 2,
      PART_CST  = 3
    };
    enum INFO {
      PT  = 0,
      ETA = 1,
      PHI = 2,
      ENE = 3
    };

    // ctor/dtor
    SCorrelatorJetTree(const std::string &name = "SCorrelatorJetTree", const std::string &outfile = "correlator_jet_tree.root", const bool isMC = false, const bool debug = false);
    ~SCorrelatorJetTree() override;

    // F4A methods
    int Init(PHCompositeNode *)          override;
    int process_event(PHCompositeNode *) override;
    int End(PHCompositeNode *)           override;

    // particle flow setters
    void setParticleFlowMinEta(double etamin) {m_particleflow_mineta = etamin;}
    void setParticleFlowMaxEta(double etamax) {m_particleflow_maxeta = etamax;}
    void setParticleFlowEtaAcc(double etamin, double etamax);
    // particle flow getters
    double getParticleFlowMinEta() {return m_particleflow_mineta;}
    double getParticleFlowMaxEta() {return m_particleflow_maxeta;}

    // track setters
    void setTrackMinPt(double ptmin)   {m_track_minpt = ptmin;}
    void setTrackMaxPt(double ptmax)   {m_track_maxpt = ptmax;}
    void setTrackMinEta(double etamin) {m_track_mineta = etamin;}
    void setTrackMaxEta(double etamax) {m_track_maxeta = etamax;}
    void setTrackPtAcc(double ptmin, double ptmax);
    void setTrackEtaAcc(double etamin, double etamax);
    // track getters
    double getTrackMinPt()  {return m_track_minpt;}
    double getTrackMaxPt()  {return m_track_maxpt;}
    double getTrackMinEta() {return m_track_mineta;}
    double getTrackMaxEta() {return m_track_maxeta;}

    // emcal setters
    void setEMCalClusterMinPt(double ptmin)   {m_EMCal_cluster_minpt = ptmin;}
    void setEMCalClusterMaxPt(double ptmax)   {m_EMCal_cluster_maxpt = ptmax;}
    void setEMCalClusterMinEta(double etamin) {m_EMCal_cluster_mineta = etamin;}
    void setEMCalClusterMaxEta(double etamax) {m_EMCal_cluster_maxeta = etamax;}
    void setEMCalClusterPtAcc(double ptmin, double ptmax);
    void setEMCalClusterEtaAcc(double etamin, double etamax);
    // emcal getters
    double getEMCalClusterMinPt()  {return m_EMCal_cluster_minpt;}
    double getEMCalClusterMaxPt()  {return m_EMCal_cluster_maxpt;}
    double getEMCalClusterMinEta() {return m_EMCal_cluster_mineta;}
    double getEMCalClusterMaxEta() {return m_EMCal_cluster_maxeta;}

    // hcal setters
    void setHCalClusterMinPt(double ptmin)   {m_HCal_cluster_minpt = ptmin;}
    void setHCalClusterMaxPt(double ptmax)   {m_HCal_cluster_maxpt = ptmax;}
    void setHCalClusterMinEta(double etamin) {m_HCal_cluster_mineta = etamin;}
    void setHCalClusterMaxEta(double etamax) {m_HCal_cluster_maxeta = etamax;}
    void setHCalClusterPtAcc(double ptmin, double ptmax);
    void setHCalClusterEtaAcc(double etamin, double etamax);
    // hcal getters
    double getHCalClusterMinPt()  {return m_HCal_cluster_minpt;}
    double getHCalClusterMaxPt()  {return m_HCal_cluster_maxpt;}
    double getHCalClusterMinEta() {return m_HCal_cluster_mineta;}
    double getHCalClusterMaxEta() {return m_HCal_cluster_maxeta;}

    // particle setters
    void setParticleMinPt(double ptmin)   {m_MC_particle_minpt = ptmin;}
    void setParticleMaxPt(double ptmax)   {m_MC_particle_maxpt = ptmax;}
    void setParticleMinEta(double etamin) {m_MC_particle_mineta = etamin;}
    void setParticleMaxEta(double etamax) {m_MC_particle_maxeta = etamax;}
    void setParticlePtAcc(double ptmin, double ptmax);
    void setParticleEtaAcc(double etamin, double etamx);

    // constituent setters
    void setAddParticleFlow(bool b)  {m_add_particleflow = b;}
    void setAddTracks(bool b)        {m_add_tracks = b;}
    void setAddEMCalClusters(bool b) {m_add_EMCal_clusters = b;}
    void setAddHCalClusters(bool b)  {m_add_HCal_clusters = b;}
    // constituent getters
    bool getAddParticleFlow()  {return m_add_particleflow;}
    bool getAddTracks()        {return m_add_tracks;}
    bool getAddEMCalClusters() {return m_add_EMCal_clusters;}
    bool getAddHCalClusters()  {return m_add_HCal_clusters;}

    // jet setters
    void setR(double r) {m_jetr = r;}
    void setJetAlgo(ALGO jetalgo);
    void setRecombScheme(RECOMB recomb_scheme);
    void setJetParameters(double r, ALGO jetalgo, RECOMB recomb_scheme);
    // jet getters
    double                       getR()            {return m_jetr;}
    fastjet::JetAlgorithm        getJetAlgo()      {return m_jetalgo;}
    fastjet::RecombinationScheme getRecombScheme() {return m_recomb_scheme;}

    // i/o setters
    void setDoQualityPlots(bool q)          {m_doQualityPlots = q;}
    void setJetContainerName(std::string n) {m_jetcontainer_name = n;}
    void setSaveDST(bool s)                 {m_save_dst = s;}
    void setIsMC(bool b)                    {m_ismc = b;}
    void setSaveDSTMC(bool s)               {m_save_truth_dst = s;}
    // i/o getters
    bool        getDoQualityPlots()   {return m_doQualityPlots;}
    std::string getJetContainerName() {return m_jetcontainer_name;}
    bool        getSaveDST()          {return m_save_dst;}
    bool        getIsMC()             {return m_ismc;}
    bool        getSaveDSTMC()        {return m_save_truth_dst;}

  private:

    // jet methods
    void findJets(PHCompositeNode *topNode);
    void findMcJets(PHCompositeNode *topNode);
    void addParticleFlow(PHCompositeNode *topNode, std::vector<fastjet::PseudoJet> &particles, std::map<int, std::pair<Jet::SRC, int>> &fjMap);
    void addTracks(PHCompositeNode *topNode, std::vector<fastjet::PseudoJet> &particles, std::map<int, std::pair<Jet::SRC, int>> &fjMap);
    void addClusters(PHCompositeNode *topNode, std::vector<fastjet::PseudoJet> &particles, std::map<int, std::pair<Jet::SRC, int>> &fjMap);
    void addParticles(PHCompositeNode *topNode, std::vector<fastjet::PseudoJet> &particles, std::map<int, std::pair<Jet::SRC, int>> &fjMap);
    // constituent methods
    bool isAcceptableParticleFlow(ParticleFlowElement *pfPart);
    bool isAcceptableTrack(SvtxTrack *track);
    bool isAcceptableEMCalCluster(CLHEP::Hep3Vector &E_vec_cluster);
    bool isAcceptableHCalCluster(CLHEP::Hep3Vector &E_vec_cluster);
    bool isAcceptableParticle(HepMC::GenParticle *part);
    // i/o methods
    void initializeVariables();
    void initializeHists();
    void initializeTrees();
    void saveOutput();
    int  createJetNode(PHCompositeNode* topNode);
    void resetTreeVariables();

    // F4A histogram manager
    Fun4AllHistoManager *m_hm;

    // particle flow variables
    double m_particleflow_mineta;
    double m_particleflow_maxeta;
    // track variables
    double m_track_minpt;
    double m_track_maxpt;
    double m_track_mineta;
    double m_track_maxeta;
    // emcal variables
    double m_EMCal_cluster_minpt;
    double m_EMCal_cluster_maxpt;
    double m_EMCal_cluster_mineta;
    double m_EMCal_cluster_maxeta;
    // hcal variables
    double m_HCal_cluster_minpt;
    double m_HCal_cluster_maxpt;
    double m_HCal_cluster_mineta;
    double m_HCal_cluster_maxeta;
    // particle variables
    double m_MC_particle_minpt;
    double m_MC_particle_maxpt;
    double m_MC_particle_mineta;
    double m_MC_particle_maxeta;

    // constituent parameters
    bool m_add_particleflow;
    bool m_add_tracks;
    bool m_add_EMCal_clusters;
    bool m_add_HCal_clusters;
    // jet parameters
    double                        m_jetr;
    fastjet::JetAlgorithm         m_jetalgo;
    fastjet::RecombinationScheme  m_recomb_scheme;
    JetMapv1                     *m_jetMap;
    JetMapv1                     *m_truth_jetMap;
    // i/o parameters
    std::string  m_outfilename;
    std::string  m_jetcontainer_name;
    bool         m_doQualityPlots;
    bool         m_save_dst;
    bool         m_save_truth_dst;
    bool         m_ismc;
    bool         m_doDebug;

    // output file & trees
    TFile *m_outFile;
    TTree *m_recTree;
    TTree *m_truTree;

    // QA histograms
    TH1D *m_hJetArea[NJetType];
    TH1D *m_hJetNumCst[NJetType];
    TH1D *m_hNumObject[NObjType];
    TH1D *m_hSumCstEne[NCstType];
    TH1D *m_hObjectQA[NObjType][NInfoQA];
    TH1D *m_hNumCstAccept[NCstType][NMoment];

    // output reco event variables
    unsigned long m_recNumJets           = 0;
    long long     m_recPartonID[NPart]   = {-9999,  -9999};
    double        m_recPartonMomX[NPart] = {-9999., -9999.};
    double        m_recPartonMomY[NPart] = {-9999., -9999.};
    double        m_recPartonMomZ[NPart] = {-9999., -9999.};
    // output reco jet variables
    std::vector<unsigned long> m_recJetNCst;
    std::vector<unsigned int>  m_recJetId;
    std::vector<unsigned int>  m_recJetTruId;
    std::vector<double>        m_recJetE;
    std::vector<double>        m_recJetPt;
    std::vector<double>        m_recJetEta;
    std::vector<double>        m_recJetPhi;
    std::vector<double>        m_recJetArea;
    // output reco constituent variables
    std::vector<std::vector<double>> m_recCstZ;
    std::vector<std::vector<double>> m_recCstDr;
    std::vector<std::vector<double>> m_recCstE;
    std::vector<std::vector<double>> m_recCstJt;
    std::vector<std::vector<double>> m_recCstEta;
    std::vector<std::vector<double>> m_recCstPhi;

    // output truth event variables
    unsigned long m_truNumJets           = 0;
    long long     m_truPartonID[NPart]   = {-9999,  -9999};
    double        m_truPartonMomX[NPart] = {-9999., -9999.};
    double        m_truPartonMomY[NPart] = {-9999., -9999.};
    double        m_truPartonMomZ[NPart] = {-9999., -9999.};
    // output truth jet variables
    std::vector<unsigned long> m_truJetNCst;
    std::vector<unsigned int>  m_truJetId;
    std::vector<unsigned int>  m_truJetTruId;
    std::vector<double>        m_truJetE;
    std::vector<double>        m_truJetPt;
    std::vector<double>        m_truJetEta;
    std::vector<double>        m_truJetPhi;
    std::vector<double>        m_truJetArea;
    // output truth constituent variables
    std::vector<std::vector<double>> m_truCstZ;
    std::vector<std::vector<double>> m_truCstDr;
    std::vector<std::vector<double>> m_truCstE;
    std::vector<std::vector<double>> m_truCstJt;
    std::vector<std::vector<double>> m_truCstEta;
    std::vector<std::vector<double>> m_truCstPhi;

};

#endif

// end ------------------------------------------------------------------------
