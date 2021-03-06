#include "PuppiContainer.hh"
#include "fastjet/Selector.hh"
#include "Math/ProbFunc.h"
#include "TMath.h"
#include <iostream>
#include <math.h>

PuppiContainer::PuppiContainer(bool iApplyCHS, bool iUseExp,double iPuppiWeightCut,std::vector<AlgoObj> &iAlgos) { 
  fApplyCHS        = iApplyCHS;
  fUseExp          = iUseExp;
  fPuppiWeightCut  = iPuppiWeightCut;
  fNAlgos = iAlgos.size();
  for(unsigned int i0 = 0; i0 < iAlgos.size(); i0++) { 
    PuppiAlgo pPuppiConfig(iAlgos[i0]);
    fPuppiAlgo.push_back(pPuppiConfig);
  }
}

void PuppiContainer::initialize(const std::vector<RecoObj> &iRecoObjects) { 
  //Clear everything
  fRecoParticles.resize(0);
  fPFParticles  .resize(0);
  fChargedPV    .resize(0);
  fPupParticles .resize(0);
  fWeights      .resize(0);
  fVals.resize(0);
  //fChargedNoPV.resize(0);
  //Link to the RecoObjects
  fPVFrac = 0.; 
  fNPV    = 1.;
  fRecoParticles = iRecoObjects;
  for (unsigned int i = 0; i < fRecoParticles.size(); i++){
    fastjet::PseudoJet curPseudoJet;
    curPseudoJet.reset_PtYPhiM(fRecoParticles[i].pt,fRecoParticles[i].eta,fRecoParticles[i].phi,fRecoParticles[i].m);
    if(fRecoParticles[i].id == 0 or  fRecoParticles[i].charge == 0)  curPseudoJet.set_user_index(0); // zero is neutral hadron
    if(fRecoParticles[i].id == 1 and fRecoParticles[i].charge != 0) curPseudoJet.set_user_index(fRecoParticles[i].charge); // from PV use the                             
    if(fRecoParticles[i].id == 2 and fRecoParticles[i].charge != 0) curPseudoJet.set_user_index(fRecoParticles[i].charge+5); // from NPV use the charge as key +5 as key           // fill vector of pseudojets for internal references
    fPFParticles.push_back(curPseudoJet);
    //Take Charged particles associated to PV
    if(fabs(fRecoParticles[i].id) == 1) fChargedPV.push_back(curPseudoJet);
    if(fabs(fRecoParticles[i].id) >= 1 ) fPVFrac+=1.;
    if(fNPV < fRecoParticles[i].vtxId) fNPV = fRecoParticles[i].vtxId;
  }
  //if(fNPV < 10) fNPV = 80.;
  if(fPVFrac != 0) { fPVFrac = double(fChargedPV.size())/fPVFrac;}
  else { fPVFrac = 0;}
  //std::cout << "Charged Particle PV " << fChargedPV.size() << " Charged PU particle " << fPVFrac << std::endl;
}
PuppiContainer::~PuppiContainer(){}

double PuppiContainer::goodVar(fastjet::PseudoJet &iPart,std::vector<fastjet::PseudoJet> &iParts, int iOpt,double iRCone) {
  double lPup = 0;
  lPup = var_within_R(iOpt,iParts,iPart,iRCone);
  return lPup;
}
double PuppiContainer::var_within_R(int iId, const vector<fastjet::PseudoJet> & particles, const fastjet::PseudoJet& centre, double R){
  if(iId == -1) return 1;
  fastjet::Selector sel = fastjet::SelectorCircle(R);
  sel.set_reference(centre);
  vector<fastjet::PseudoJet> near_particles = sel(particles);
  double var = 0;
  //double lSumPt = 0;
  //if(iId == 1) for(unsigned int i=0; i<near_particles.size(); i++) lSumPt += near_particles[i].pt();
  for(unsigned int i=0; i<near_particles.size(); i++){
    double pDEta = near_particles[i].eta()-centre.eta();
    double pDPhi = fabs(near_particles[i].phi()-centre.phi());
    if(pDPhi > 2.*3.14159265-pDPhi) pDPhi =  2.*3.14159265-pDPhi;
    double pDR2 = pDEta*pDEta+pDPhi*pDPhi;
    if(std::abs(pDR2)  <  0.0001) continue;
    if(iId == 0) var += (near_particles[i].pt()/pDR2);
    if(iId == 1) var += near_particles[i].pt();
    if(iId == 2) var += (1./pDR2);
    if(iId == 3) var += (1./pDR2);
    if(iId == 4) var += near_particles[i].pt();  
    if(iId == 5) var += (near_particles[i].pt()*(near_particles[i].pt()/pDR2));
  }
  if(iId == 1) var += centre.pt(); //Sum in a cone
  if(iId == 0 && var != 0) var = log(var);
  if(iId == 3 && var != 0) var = log(var);
  if(iId == 5 && var != 0) var = log(var);
  return var;
}
//In fact takes the median not the average
void PuppiContainer::getRMSAvg(int iOpt,std::vector<fastjet::PseudoJet> &iConstits,std::vector<fastjet::PseudoJet> &iParticles,std::vector<fastjet::PseudoJet> &iChargedParticles) { 
  for(unsigned int i0 = 0; i0 < iConstits.size(); i0++ ) { 
    double pVal = -1;
    //Calculate the Puppi Algo to use
    int  pPupId   = getPuppiId(iConstits[i0].pt(),iConstits[i0].eta());
    if(fPuppiAlgo[pPupId].numAlgos() <= iOpt) pPupId = -1;
    if(pPupId == -1) {fVals.push_back(-1); continue;}
    //Get the Puppi Sub Algo (given iteration)
    int  pAlgo    = fPuppiAlgo[pPupId].algoId   (iOpt); 
    bool pCharged = fPuppiAlgo[pPupId].isCharged(iOpt);
    double pCone  = fPuppiAlgo[pPupId].coneSize (iOpt);
    //Compute the Puppi Metric 
    if(!pCharged) pVal = goodVar(iConstits[i0],iParticles       ,pAlgo,pCone);
    if( pCharged) pVal = goodVar(iConstits[i0],iChargedParticles,pAlgo,pCone);
    fVals.push_back(pVal);
    if(std::isnan(pVal) || std::isinf(pVal)) cerr << "====> Value is Nan " << pVal << " == " << iConstits[i0].pt() << " -- " << iConstits[i0].eta() << endl;
    if(std::isnan(pVal) || std::isinf(pVal)) continue;
    fPuppiAlgo[pPupId].add(iConstits[i0],pVal,iOpt);
    /*   eta extrapolation code
        for(int i1 = 0; i1 < fNAlgos; i1++){
            pAlgo    = fPuppiAlgo[i1].algoId   (iOpt);
            pCharged = fPuppiAlgo[i1].isCharged(iOpt);
            pCone    = fPuppiAlgo[i1].coneSize (iOpt);
            double curVal = -1;
            if(!pCharged) curVal = goodVar(iConstits[i0],iParticles       ,pAlgo,pCone);
            if( pCharged) curVal = goodVar(iConstits[i0],iChargedParticles,pAlgo,pCone);
            //std::cout << "i1 = " << i1 << ", curVal = " << curVal << ", eta = " << iConstits[i0].eta() << ", pupID = " << pPupId << std::endl;                                
            fPuppiAlgo[i1].add(iConstits[i0],curVal,iOpt);
        }
    */
  }
  for(int i0 = 0; i0 < fNAlgos; i0++) fPuppiAlgo[i0].computeMedRMS(iOpt,fPVFrac);
}
int    PuppiContainer::getPuppiId(const float &iPt,const float &iEta) { 
  int lId = -1; 
  for(int i0 = 0; i0 < fNAlgos; i0++) { 
    if(fabs(iEta) < fPuppiAlgo[i0].etaMin()) continue;
    if(fabs(iEta) > fPuppiAlgo[i0].etaMax()) continue;
    if(iPt        < fPuppiAlgo[i0].ptMin())  continue;
    lId = i0; 
    break;
  }
  return lId;
}
double PuppiContainer::getChi2FromdZ(double iDZ) { 
  //We need to obtain prob of PU + (1-Prob of LV)
  // Prob(LV) = Gaus(dZ,sigma) where sigma = 1.5mm  (its really more like 1mm)
  //double lProbLV = ROOT::Math::normal_cdf_c(fabs(iDZ),0.2)*2.; //*2 is to do it double sided
  //Take iDZ to be corrected by sigma already
  double lProbLV = ROOT::Math::normal_cdf_c(fabs(iDZ),1.)*2.; //*2 is to do it double sided
  double lProbPU = 1-lProbLV;
  if(lProbPU <= 0) lProbPU = 1e-16;   //Quick Trick to through out infs
  if(lProbPU >= 0) lProbPU = 1-1e-16; //Ditto
  double lChi2PU = TMath::ChisquareQuantile(lProbPU,1);
  lChi2PU*=lChi2PU;
  return lChi2PU;
}
const std::vector<double> PuppiContainer::puppiWeights() {
  fPupParticles .resize(0);
  fWeights      .resize(0);
  fVals         .resize(0);
  for(int i0 = 0; i0 < fNAlgos; i0++) fPuppiAlgo[i0].reset();

    int lNMaxAlgo = 1;
  for(int i0 = 0; i0 < fNAlgos; i0++) lNMaxAlgo = TMath::Max(fPuppiAlgo[i0].numAlgos(),lNMaxAlgo);
  //Run through all compute mean and RMS
  int lNParticles    = fRecoParticles.size();
  for(int i0 = 0; i0 < lNMaxAlgo; i0++) { 
    getRMSAvg(i0,fPFParticles,fPFParticles,fChargedPV);
  }
  std::vector<double> pVals;
  for(int i0 = 0; i0 < lNParticles; i0++) {
    //Refresh
    pVals.clear();
    double pWeight = 1;
    //Get the Puppi Id and if ill defined move on
    int  pPupId   = getPuppiId(fRecoParticles[i0].pt,fRecoParticles[i0].eta);
    if(pPupId == -1) {
     fWeights .push_back(pWeight);
     continue;
   }
    // fill the p-values
    double pChi2   = 0;
    if(fUseExp){ 
      //Compute an Experimental Puppi Weight with delta Z info (very simple example)
      pChi2 = getChi2FromdZ(fRecoParticles[i0].dZ);
      //Now make sure Neutrals are not set
      if(fRecoParticles[i0].pfType > 3) pChi2 = 0;
    }
    //Fill and compute the PuppiWeight
    int lNAlgos = fPuppiAlgo[pPupId].numAlgos();
    for(int i1 = 0; i1 < lNAlgos; i1++) pVals.push_back(fVals[lNParticles*i1+i0]);
    pWeight = fPuppiAlgo[pPupId].compute(pVals,pChi2);
    //Apply the CHS weights
    if(fRecoParticles[i0].id == 1 && fApplyCHS ) pWeight = 1;
    if(fRecoParticles[i0].id == 2 && fApplyCHS ) pWeight = 0;
    //Basic Weight Checks
    if(std::isnan(pWeight)) std::cerr << "====> Weight is nan  : pt " << fRecoParticles[i0].pt << " -- eta : " << fRecoParticles[i0].eta << " -- Value" << fVals[i0] << " -- id :  " << fRecoParticles[i0].id << " --  NAlgos: " << lNAlgos << std::endl;
    //Basic Cuts      
    if(pWeight                         < fPuppiWeightCut) pWeight = 0;  //==> Elminate the low Weight stuff
    if(pWeight*fPFParticles[i0].pt()   < fPuppiAlgo[pPupId].neutralPt(fNPV) && fRecoParticles[i0].id == 0 ) pWeight = 0;  //threshold cut on the neutral Pt
    fWeights .push_back(pWeight);
    //Now get rid of the thrown out weights for the particle collection
    if(pWeight == 0) continue;
      //Produce
    fastjet::PseudoJet curjet( pWeight*fPFParticles[i0].px(), pWeight*fPFParticles[i0].py(), pWeight*fPFParticles[i0].pz(), pWeight*fPFParticles[i0].e());
    curjet.set_user_index(i0);//fRecoParticles[i0].id);
    fPupParticles.push_back(curjet);
  }
  return fWeights;
}


