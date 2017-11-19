#define XMIN -0.5
#define XMAX 0.5
#define NBINS 100

int Analyzer(){

  TString fStartDate = "Nov 10 2016";
  TString fEndDate = "Nov 10 2017";
  TString fSymbol = "SOXL";
  TString fFreq = "1wk";  // 1d, 1wk, 1mo
  gSystem->Exec("sh getData.sh "+fSymbol+" "+fFreq+" '"+fStartDate+"' "+"'"+fEndDate+"'");

  TFile *f = new TFile(fSymbol+".root","RECREATE");
  TTree *tree = new TTree(fSymbol,"From CSV File");
  tree->ReadFile(fSymbol+".csv","fDate/C:fOpen/F:fHigh/F:fLow/F:fClose/F:fCloseAdj/F:fVolume/I",',');
  f->Write();

  TTreeReader fReader(fSymbol, f);
  TTreeReaderValue<Float_t> fH(fReader,"fHigh");
  TTreeReaderValue<Float_t> fL(fReader,"fLow");
  TTreeReaderValue<Float_t> fO(fReader,"fOpen");
  TTreeReaderValue<Float_t> fC(fReader,"fClose");

  TH1F *fHDiffLL = new TH1F("fHDiffLL",fSymbol+";Difference between "+fFreq+" high and previous "+fFreq+" low prices;Counts from "+fStartDate+" to "+fEndDate,NBINS,XMIN,XMAX);
  TH1F *fHDiffHH = new TH1F("fHDiffHH",fSymbol+";Difference between "+fFreq+" high and previous "+fFreq+" high prices;Counts from "+fStartDate+" to "+fEndDate,NBINS,XMIN,XMAX);
  TH1F *fHDiffLH = new TH1F("fHDiffLH",fSymbol+";Difference between "+fFreq+" high and previous "+fFreq+" low prices;Counts from "+fStartDate+" to "+fEndDate,NBINS,XMIN,XMAX);

  Float_t fPrevL;
  Float_t fPrevH;
   
  while(fReader.Next()){
    fHDiffLL->Fill( (*fL - fPrevL)/(fPrevL) );
    fHDiffLH->Fill( (*fH - fPrevL)/(fPrevL) );
    fHDiffHH->Fill( (*fH - fPrevH)/(fPrevH) );
    fPrevL = *fL;
    fPrevH = *fH;		    
  }
  
  TCanvas *c1 = new TCanvas("c1","c1",2048,1152);
  c1->Divide(2,2);
  
  c1->cd(1);
  //Need to Check: Is it a Poisson Distribution?
  TF1 *fPoisson = new TF1("fPoisson","[0]*TMath::Power(([1]/[2]),(x/[2]))*(TMath::Exp(-([1]/[2])))/TMath::Gamma((x/[2])+1)", -0.1, 0.5);
  //gStyle->SetOptFit();
  fPoisson->SetParameters(1, 1, 1);
  fHDiffLH->Fit("fPoisson","QR");
  fHDiffLH->Draw();

  c1->cd(2);
  TF1 *fGausLL = new TF1("GausLL","gaus");
  fHDiffLL->Fit("GausLL","QM+");
  Float_t fHPriceLL = fPrevL*(1+fGausLL->GetParameter(1));
  printf("Next Low price: %f\n",fHPriceLL);
  fHDiffLL->Draw();

  c1->cd(3);
  TF1 *fGausHH = new TF1("GausHH","gaus");
  fHDiffHH->Fit("GausHH","QM+");
  Float_t fHPriceHH = fPrevH*(1 + fGausHH->GetParameter(1));
  printf("Next High price: %f\n",fHPriceHH);
  fHDiffHH->Draw();
  c1->Print(fSymbol+".png");
  
  return 0;

}
