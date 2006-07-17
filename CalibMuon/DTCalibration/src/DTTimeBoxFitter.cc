
/*
 *  See header file for a description of this class.
 *
 *  $Date: 2006/03/13 12:17:37 $
 *  $Revision: 1.1 $
 *  \author G. Cerminara - INFN Torino
 */

#include "CalibMuon/DTCalibration/interface/DTTimeBoxFitter.h"

#include <iostream>
#include <vector>

#include "TFile.h"
#include "TH1F.h"
#include "TMath.h"
#include "TF1.h"

using namespace std;

DTTimeBoxFitter::DTTimeBoxFitter() : theVerbosityLevel(0) {
  hDebugFile = new TFile("DTTimeBoxFitter.root", "RECREATE");
}



DTTimeBoxFitter::~DTTimeBoxFitter() {
  hDebugFile->Close();
}




/// Compute the ttrig (in ns) from the Time Box
pair<double, double> DTTimeBoxFitter::fitTimeBox(TH1F *hTimeBox) {
  // Check if the histo contains any entry
  if(hTimeBox->GetEntries() == 0) {
    cout << "[DTTimeBoxFitter]***Error: the time box contains no entry!" << endl;
    return make_pair(-1, -1);
  }


  // Get seeds for the fit
  // The TimeBox range to be fitted (the rising edge)
  double xFitMin=0;     // Min value for the fit
  double xFitMax=0;     // Max value for the fit 
  double xValue=0;      // The mean value of the gaussian
  double xFitSigma=0;   // The sigma of the gaussian
  double tBoxMax=0;     // The max of the time box, it is used as seed for gaussian integral

  //hTimeBox->Rebin(2); //FIXME: Temporary for low statistics

  getFitSeeds(hTimeBox, xValue, xFitSigma, tBoxMax, xFitMin, xFitMax);


  // Define the fitting function and use fit seeds
  TF1 *fIntGaus = new TF1("IntGauss", intGauss, xFitMin, xFitMax, 3); 
  fIntGaus->SetParName(0, "Constant");
  fIntGaus->SetParameter(0, tBoxMax);
  fIntGaus->SetParName(1, "Mean");
  fIntGaus->SetParameter(1, xValue);
  fIntGaus->SetParName(2, "Sigma");
  fIntGaus->SetParameter(2, xFitSigma);
  fIntGaus->SetLineColor(kRed);


  // Fit the histo
  char *option = "Q";
  if(theVerbosityLevel >= 2)
    option = "";

  hTimeBox->Fit("IntGauss", option, "",xFitMin, xFitMax);

  // Get fitted parameters
  double mean =  fIntGaus->GetParameter("Mean");
  double sigma = fIntGaus->GetParameter("Sigma");
  //   double constant = fIntGaus->GetParameter("Constant");
  double chiSquare = fIntGaus->GetChisquare()/fIntGaus->GetNDF();
  
  if(theVerbosityLevel >= 1) {
    cout << " === Fit Results: " << endl;
    cout << "     Fitted mean = " << mean << endl;
    cout << "     Fitted sigma = " << sigma << endl;
    cout << "     Reduced Chi Square = " << chiSquare << endl;
  }
  return make_pair(mean, sigma);
}



// Automatically compute the seeds the range to be used for time box fit
void DTTimeBoxFitter::getFitSeeds(TH1F *hTBox, double& mean, double& sigma, double& tBoxMax,
				    double& xFitMin, double& xFitMax) {
  if(theVerbosityLevel >= 1)
    cout << " === Looking for fit seeds in Time Box:" << endl;


  // The approximate width of the time box
  static const int tBoxWidth = 400; //FIXE: tune it

  const int nBins = hTBox->GetNbinsX();
  const int xMin = (int)hTBox->GetXaxis()->GetXmin();
  const int xMax = (int)hTBox->GetXaxis()->GetXmax();
  const int nEntries =  (int)hTBox->GetEntries();

  const double binValue = (double)(xMax-xMin)/(double)nBins;

  // Compute a threshold for TimeBox discrimination
  const double threshold = binValue*nEntries/(double)(tBoxWidth*2.);

  if(threshold < 10) {
    //FIXME: rebin?
  }

  hDebugFile->cd();
  TString hLName = TString(hTBox->GetName())+"L";
  TH1F hLTB(hLName.Data(), "Logic Time Box", nBins, xMin, xMax);
  // Loop over all time box bins and discriminate them accordigly to the threshold
  for(int i = 1; i <= nBins; i++) {
    if(hTBox->GetBinContent(i) > threshold)
      hLTB.SetBinContent(i, 1);
  }
  hLTB.Write();
  
  // Look for the time box in the "logic histo" and save beginning and lenght of each plateau
  vector< pair<int, int> > startAndLenght;

  int start = -1;
  int lenght = -1;
  for(int j = 1; j < nBins;j++) {
    int diff = (int)hLTB.GetBinContent(j+1)-(int)hLTB.GetBinContent(j);
    if(diff == 1) { // This is a rising edge
      start = j;
      lenght = 1;
    } else if(diff == -1) { // This is a falling edge
      startAndLenght.push_back(make_pair(start, lenght));
      start = -1;
      lenght = -1;
    } else if(diff == 0 && (int)hLTB.GetBinContent(j) == 1) { // This is a bin within the plateau
      lenght ++;
    }
  }

  // Look for the plateau of the right lenght
  int delta = 999999;
  int beginning = -1;
  int tbWidth = -1;
  for(vector< pair<int, int> >::const_iterator stAndL = startAndLenght.begin();
      stAndL != startAndLenght.end();
      stAndL++) {
    if(abs((*stAndL).second - tBoxWidth) < delta) {
      delta = abs((*stAndL).second - tBoxWidth);
      beginning = (*stAndL).first;
      tbWidth = (*stAndL).second;
    }
  }

  mean = xMin + beginning*binValue;
  sigma = 10; //FIXME: estimate it!

  tBoxMax = hTBox->GetMaximum();

  // Define the fit range
  xFitMin = mean-5.*sigma;
  xFitMax = mean+5.*sigma;

  if(theVerbosityLevel >= 1) {
    cout << "      Time Box Rising edge: " << mean << endl;
    cout << "      Time Box Width: " << tbWidth*binValue << endl;
    cout << "    = Seeds and range for fit:" << endl;
    cout << "       Seed mean = " << mean << endl;
    cout << "       Seed sigma = " << sigma << endl;
    cout << "       Fitting from = " << xFitMin << " to " << xFitMax << endl << endl;
  }
}

double intGauss(double *x, double *par) {
  double media = par[1];
  double sigma = par[2];
  double var = (x[0]-media)/(sigma*sqrt(2.));

  return 0.5*par[0]*(1+TMath::Erf(var));

}
