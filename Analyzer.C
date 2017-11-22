#define XMIN -0.5
#define XMAX 0.5
#define NBINS 100

Int_t GetIndex( Int_t fEvent, Int_t fInterval){

  Int_t fIndex = 0;
  Int_t fAux = (fEvent%fInterval - 1);

  if( fAux >= 0 ) {
    fIndex = fAux;
  } else {
    fIndex = fInterval-1;
  }
  return fIndex;
    
}

int Analyzer( TString fSymbol = "SOXL",
	      TString fFreq = "1wk",
	      TDatime fStartDate = TDatime("2009-01-01 00:00:00"),
	      TDatime fEndDate = TDatime("2017-11-17 00:00:00") ) {

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
			    +fFreq+" low prices;Counts from "+fStartDate.AsSQLString()
			    +" to "+fEndDate.AsString(),
			    NBINS,XMIN,XMAX);
  TH1F *fHDiffHH = new TH1F("fHDiffHH",
			    fSymbol+";Difference between "+fFreq+" high and previous "
			    +fFreq+" high prices;Counts from "+fStartDate.AsSQLString()
			    +" to "+fEndDate.AsSQLString(),
			    NBINS,XMIN,XMAX);
  TH1F *fHDiffLH = new TH1F("fHDiffLH",
			    fSymbol+";Difference between "+fFreq+" high and previous "
			    +fFreq+" low prices;Counts from "+fStartDate.AsSQLString()
			    +" to "+fEndDate.AsSQLString(),
			    NBINS,XMIN,XMAX);

  Float_t fPrevL;
  Float_t fPrevH;

  TDatime fDate;
  
  Int_t fEvent = 0;

  // Fast and Slow Moving Averages
  TString fSMAOption = "close";
  Int_t fFastSMAInterval = 7;
  Int_t fSlowSMAInterval = 26;
  Float_t fPriceFastSMA[fFastSMAInterval];
  Float_t fPriceSlowSMA[fSlowSMAInterval];
  TGraph *fGFastSMA = new TGraph();
  TGraph *fGSlowSMA = new TGraph();

  Float_t fNPriceHH;
  Float_t fNPriceLL;
  
  while(fReader.Next()){

    TString fSDt = static_cast<char*>(fDt.GetAddress());
    fSDt.Append(" 00:00:00");
    fDate = TDatime(fSDt);

    fPriceFastSMA[GetIndex(fEvent,fFastSMAInterval)] = *fC;
    fPriceSlowSMA[GetIndex(fEvent,fSlowSMAInterval)] = *fC;

    if (fEvent > fSlowSMAInterval){
 
      Float_t FSMA = TMath::Mean(fFastSMAInterval,&fPriceFastSMA[0]);
      Float_t SSMA = TMath::Mean(fSlowSMAInterval,&fPriceSlowSMA[0]);
      
      fGFastSMA->SetPoint(fGFastSMA->GetN(),fDate.Convert(),FSMA);
      fGSlowSMA->SetPoint(fGSlowSMA->GetN(),fDate.Convert(),SSMA);
    }
    
    fEvent++;
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
  //printf("Next Low price: %f\n",fHPriceLL);
  fHDiffLL->Draw();

  c1->cd(3);
  TF1 *fGausHH = new TF1("GausHH","gaus");
  fHDiffHH->Fit("GausHH","QM+");
  Float_t fHPriceHH = fPrevH*(1 + fGausHH->GetParameter(1));
  //printf("Next High price: %f\n",fHPriceHH);
  //fHDiffHH->Draw();
  
  TGraph *fGdSSMA = new TGraph();

  for (Int_t i = 1; i < fGSlowSMA->GetN(); i++){
    Double_t x1,y1,x2,y2;
    fGSlowSMA->GetPoint(i-1,x1,y1);
    fGSlowSMA->GetPoint(i,x2,y2);
    Double_t dy = y2 - y1;
    fGdSSMA->SetPoint(i-1,x2,dy/y1);
  }
  fGdSSMA->SetTitle(fSymbol+" Slow SMA Derivative; Date; Price");
  fGdSSMA->GetXaxis()->SetTimeDisplay(1);
  fGdSSMA->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  fGdSSMA->GetXaxis()->SetTimeOffset(0,"gmt");
  fGdSSMA->Draw("AL*");
  
  c1->cd(4);
  fGFastSMA->SetTitle(fSymbol+" Fast SMA; Date; Price");
  fGFastSMA->GetXaxis()->SetTimeDisplay(1);
  fGFastSMA->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  fGFastSMA->Draw("AP");
  fGSlowSMA->Draw("P SAME");
  fGdSSMA->Draw("l");
     
  c1->Print(fSymbol+".png");

  return 0;

}
