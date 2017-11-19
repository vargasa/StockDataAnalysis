#define XMIN -0.5
#define XMAX 0.5
#define NBINS 100

int Analyzer( TString fSymbol = "SOXL",
	      TString fFreq = "1wk",
	      TDatime fStartDate = TDatime("2010-01-01 00:00:00"),
	      TDatime fEndDate = TDatime("2017-11-10 00:00:00") ) {

  // 1d, 1wk, 1mo
  gSystem->Exec("sh getData.sh "+fSymbol+" "+fFreq+" '"+fStartDate.AsString()+"' "+"'"+fEndDate.AsString()+"'");

  TFile *f = new TFile(fSymbol+".root","RECREATE");
  TTree *tree = new TTree(fSymbol,"From CSV File");
  tree->ReadFile(fSymbol+".csv","fDate/C:fOpen/F:fHigh/F:fLow/F:fClose/F:fCloseAdj/F:fVolume/I",',');
  f->Write();

  TTreeReader fReader(fSymbol, f);
  TTreeReaderArray<char> fDt(fReader,"fDate");
  TTreeReaderValue<Float_t> fH(fReader,"fHigh");
  TTreeReaderValue<Float_t> fL(fReader,"fLow");
  TTreeReaderValue<Float_t> fO(fReader,"fOpen");
  TTreeReaderValue<Float_t> fC(fReader,"fClose");

  TH1F *fHDiffLL = new TH1F("fHDiffLL",
			    fSymbol+";Difference between "+fFreq+" high and previous "
			    +fFreq+" low prices;Counts from "+fStartDate.AsString()
			    +" to "+fEndDate.AsString(),
			    NBINS,XMIN,XMAX);
  TH1F *fHDiffHH = new TH1F("fHDiffHH",
			    fSymbol+";Difference between "+fFreq+" high and previous "
			    +fFreq+" high prices;Counts from "+fStartDate.AsString()
			    +" to "+fEndDate.AsString(),
			    NBINS,XMIN,XMAX);
  TH1F *fHDiffLH = new TH1F("fHDiffLH",
			    fSymbol+";Difference between "+fFreq+" high and previous "
			    +fFreq+" low prices;Counts from "+fStartDate.AsString()
			    +" to "+fEndDate.AsString(),
			    NBINS,XMIN,XMAX);

  Float_t fPrevL;
  Float_t fPrevH;
  
  Int_t fEvent = 0;

  Float_t fNPriceHH;
  Float_t fNPriceLL;
  
  Int_t fFail = 0;
  Int_t fSuccess = 0;
     
  while(fReader.Next()){

    if ( fEvent > 105 ) {
      
      if ( (fNPriceLL > *fL) && (fNPriceHH < *fH) ) {
	fSuccess++;
	printf("||======SUCESS======||\n");
      } else {
	fFail++;
	printf("||=====FAILURE======||\n");
      }
      printf("Prediction LP: %f ; Prediction HP : %f\n", fNPriceLL, fNPriceHH);
      printf("Actual LP: %f ; Actual HP: %f\n", *fL,*fH);
      printf("Difference LP: %.2f%% Difference HP: %.2f%%\n", (*fL-fNPriceLL)*100/fNPriceLL, (*fH-fNPriceHH)*100/fNPriceHH);
      

    }
    
    fEvent++;
    fHDiffLL->Fill( (*fL - fPrevL)/(fPrevL) );
    fHDiffLH->Fill( (*fH - fPrevL)/(fPrevL) );
    fHDiffHH->Fill( (*fH - fPrevH)/(fPrevH) );
    
    fPrevL = *fL;
    fPrevH = *fH;
    
    if ( fEvent > 104 ) {
      
      TF1 *fGausLL = new TF1("GausLL","gaus");
      fHDiffLL->Fit("GausLL","QM+");
      fNPriceLL = fPrevL*(1+fGausLL->GetParameter(1));
      //fHDiffLL->Reset();
      
      TF1 *fGausHH = new TF1("GausHH","gaus");
      fHDiffHH->Fit("GausHH","QM+");
      fNPriceHH = fPrevH*(1 + fGausHH->GetParameter(1));
      //fHDiffHH->Reset();
	
    } 
 
  }

  printf("Success:%d Failures:%d\n",fSuccess,fFail);
  
  
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
  //printf("Next Low price: %f\n",fHPriceLL);
  fHDiffLL->Draw();

  c1->cd(3);
  TF1 *fGausHH = new TF1("GausHH","gaus");
  fHDiffHH->Fit("GausHH","QM+");
  Float_t fHPriceHH = fPrevH*(1 + fGausHH->GetParameter(1));
  //printf("Next High price: %f\n",fHPriceHH);
  fHDiffHH->Draw();
  c1->Print(fSymbol+".png");
  
  return 0;

}
