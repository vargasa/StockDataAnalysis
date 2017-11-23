#define XMIN -0.5
#define XMAX 0.5
#define NBINS 100

#include <TError.h>

TString gSymbol;

////////////////////////////////////////////////////////////////////////////////
/// Function to compute an index when using present and historical data when
/// using composed variables

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

////////////////////////////////////////////////////////////////////////////////
/// Get data from Yahoo! Finance API using getData.sh Shell Script

TFile *GetData( TString fSymbol,
	       TString fFreq,
	       TDatime fStartDate,
	       TDatime fEndDate){
  // 1d, 1wk, 1mo
  Int_t ans = gSystem->Exec("sh getData.sh "+fSymbol+" "+fFreq+" '"+fStartDate.AsString()+"' "+"'"+fEndDate.AsString()+"'");
  if (ans == 0) {
    TFile *f = new TFile(fSymbol+".root","RECREATE");
    TTree *tree = new TTree(fSymbol,"From CSV File");
    tree->ReadFile(fSymbol+".csv","fDate/C:fOpen/F:fHigh/F:fLow/F:fClose/F:fCloseAdj/F:fVolume/I",',');
    f->Write();
    return f;
  } else {
    printf("\nData Download was unsucessfull\n");
  }
  
}

////////////////////////////////////////////////////////////////////////////////
/// Simple Moving Average SMA

TGraph *GetSMA(TFile *f,
	       Int_t fInterval = 6,
	       Option_t *Option="close"){
  
  TTreeReader fReader(gSymbol, f);
  TTreeReaderArray<char> fDt(fReader,"fDate");
  TTreeReaderValue<Float_t> fH(fReader,"fHigh");
  TTreeReaderValue<Float_t> fL(fReader,"fLow");
  TTreeReaderValue<Float_t> fO(fReader,"fOpen");
  TTreeReaderValue<Float_t> fC(fReader,"fClose");

  TDatime fDate;
  Int_t fEvent = 0;

  Float_t fPrice[fInterval];
  TGraph *fGSMA = new TGraph();

  while(fReader.Next()){

    TString fSDt = static_cast<char*>(fDt.GetAddress());
    fSDt.Append(" 00:00:00");
    fDate = TDatime(fSDt);
    
    fPrice[GetIndex(fEvent,fInterval)] = *fC;

    if (fEvent > fInterval){
      Float_t SMA = TMath::Mean(fInterval,&fPrice[0]);     
      fGSMA->SetPoint(fGSMA->GetN(),fDate.Convert(),SMA);
    }
    fEvent++;
    
  }
  fGSMA->SetTitle(Form("%s SMA(%d);Date;SMA",gSymbol.Data(),fInterval));
  fGSMA->GetXaxis()->SetTimeDisplay(1);
  fGSMA->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  fGSMA->GetXaxis()->SetTimeOffset(0,"gmt");

  return fGSMA;

}

////////////////////////////////////////////////////////////////////////////////
/// Numerical derivative working only for equaly distant x-data 

TGraph *Derivative(TGraph *fg){
  
  TGraph *fgd = new TGraph();

  for (Int_t i = 1; i < fg->GetN(); i++){
    Double_t x1,y1,x2,y2;
    fg->GetPoint(i-1,x1,y1);
    fg->GetPoint(i,x2,y2);
    Double_t dy = y2 - y1;
    fgd->SetPoint(i-1,x2,dy);
  }
  
  fgd->GetXaxis()->SetTimeDisplay(1);
  fgd->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  fgd->GetXaxis()->SetTimeOffset(0,"gmt");

  return fgd;
}

////////////////////////////////////////////////////////////////////////////////
/// Main Function
Int_t Analyzer( TString fSymbol = "SOXL",
	      TString fFreq = "1wk",
	      TDatime fStartDate = TDatime("2009-01-01 00:00:00"),
	      TDatime fEndDate = TDatime("2017-11-17 00:00:00") ) {

  TFile *f = GetData(fSymbol, fFreq, fStartDate, fEndDate);
  f->ReOpen("READ");
  gSymbol = fSymbol;

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

  Float_t fNPriceHH;
  Float_t fNPriceLL;
  
  while(fReader.Next()){

    TString fSDt = static_cast<char*>(fDt.GetAddress());
    fSDt.Append(" 00:00:00");
    fDate = TDatime(fSDt);

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
  TGraph *fGSlowSMA = GetSMA(f, 10,"close");
  fGSlowSMA->Draw("AL");

  c1->cd(4);
  TF1 *fGausHH = new TF1("GausHH","gaus");
  fHDiffHH->Fit("GausHH","QM+");
  Float_t fHPriceHH = fPrevH*(1 + fGausHH->GetParameter(1));
  TGraph *fGdSlowSMA = Derivative(fGSlowSMA);
  fGdSlowSMA->SetTitle(Form("%s;Date;SlowSMA Derivative",fSymbol.Data()));
  fGdSlowSMA->Draw("AL*");
  
  //printf("Next High price: %f\n",fHPriceHH);
  //fHDiffHH->Draw();
     
  c1->Print(fSymbol+".png");

  return 0;

}
