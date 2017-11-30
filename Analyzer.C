#define XMIN -0.5
#define XMAX 0.5
#define NBINS 100

#include <TError.h>

TString gSymbol;
TString gFreq;
TDatime gStartDate;
TDatime gEndDate;

////////////////////////////////////////////////////////////////////////////////
/// Function to compute an index to reuse array filling it cyclically

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
/// AroonDown

TGraph *GetAroonDown(TFile *f,
		 Int_t fInterval = 25){

  TTreeReader fReader(gSymbol, f);
  TTreeReaderArray<char> fDt(fReader,"fDate");
  TTreeReaderValue<Float_t> fH(fReader,"fHigh");

  TDatime fDate;

  Int_t fEvent;
  Float_t Price[fInterval];
  TGraph *fGAroonDown = new TGraph();
  
  while(fReader.Next()){

    TString fSDt = static_cast<char*>(fDt.GetAddress());
    fSDt.Append(" 00:00:00");
    fDate = TDatime(fSDt);
    
    Float_t aroondown;
    Int_t inow = GetIndex(fEvent,fInterval);


    Price[inow] = *fH;

    
    if (fEvent > fInterval) {
      
      Int_t MinIndex;
      MinIndex  = TMath::LocMin(fInterval,Price);
      if ( MinIndex < inow || MinIndex == inow ) {
	aroondown = (Float_t)(fInterval - (inow - MinIndex))*100;
	
      } else {
	aroondown = (Float_t)(fInterval - (fInterval-MinIndex+inow))*100;	
      }
      aroondown = aroondown/(Float_t)fInterval;
      fGAroonDown->SetPoint(fGAroonDown->GetN(),fDate.Convert(),aroondown);
    }
    
    fEvent++;
  }
  
  fGAroonDown->SetTitle(Form("%s AroonUp(%d);Date;AroonUp",gSymbol.Data(),fInterval));
  fGAroonDown->GetXaxis()->SetTimeDisplay(1);
  fGAroonDown->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  fGAroonDown->GetXaxis()->SetTimeOffset(0,"gmt");
  fGAroonDown->GetYaxis()->SetRangeUser(0.,110.);
  fGAroonDown->SetLineColor(kRed);
  
  return fGAroonDown;
  
}

////////////////////////////////////////////////////////////////////////////////
/// AroonUp

TGraph *GetAroonUp(TFile *f,
		 Int_t fInterval = 25){

  TTreeReader fReader(gSymbol, f);
  TTreeReaderArray<char> fDt(fReader,"fDate");
  TTreeReaderValue<Float_t> fH(fReader,"fHigh");

  TDatime fDate;

  Int_t fEvent;
  Float_t Price[fInterval];
  TGraph *fGAroonUp = new TGraph();
  
  while(fReader.Next()){

    TString fSDt = static_cast<char*>(fDt.GetAddress());
    fSDt.Append(" 00:00:00");
    fDate = TDatime(fSDt);
    
    Float_t aroonup;
    Int_t inow = GetIndex(fEvent,fInterval);


    Price[inow] = *fH;

    
    if (fEvent > fInterval) {
      
      Int_t MaxIndex;
      MaxIndex  = TMath::LocMax(fInterval,Price);
      if ( MaxIndex < inow || MaxIndex == inow ) {
	aroonup = (Float_t)(fInterval - (inow - MaxIndex))*100;
	
      } else {
	aroonup = (Float_t)(fInterval - (fInterval-MaxIndex+inow))*100;	
      }
      aroonup = aroonup/(Float_t)fInterval;
      fGAroonUp->SetPoint(fGAroonUp->GetN(),fDate.Convert(),aroonup);	  
    }

    fEvent++;
  }
  fGAroonUp->SetTitle(Form("%s AroonUp(%d);Date;AroonUp",gSymbol.Data(),fInterval));
  fGAroonUp->GetXaxis()->SetTimeDisplay(1);
  fGAroonUp->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  fGAroonUp->GetXaxis()->SetTimeOffset(0,"gmt");
  fGAroonUp->GetYaxis()->SetRangeUser(0.,110.);
  fGAroonUp->SetLineColor(kGreen);

  return fGAroonUp;
  
}

////////////////////////////////////////////////////////////////////////////////
/// Aroon Oscillator
TGraph *GetAroon(TFile *f,
		 Int_t fInterval = 25){
  
  TGraph *fGAroon = new TGraph();
  TGraph *fGAroonUp = GetAroonUp(f,fInterval);
  TGraph *fGAroonDown = GetAroonDown(f,fInterval);
  for (Int_t i = 0; i < fGAroonUp->GetN(); i++){
    Double_t x1, y1, x2, y2;
    fGAroonUp->GetPoint(i,x1,y1);
    fGAroonDown->GetPoint(i,x2,y2);
    fGAroon->SetPoint(i,x1,y1-y2);
  }
  fGAroon->SetTitle(Form("%s Aroon Oscillator(%d);Date;Aroon",gSymbol.Data(),fInterval));
  fGAroon->GetXaxis()->SetTimeDisplay(1);
  fGAroon->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  fGAroon->GetXaxis()->SetTimeOffset(0,"gmt");
  fGAroon->GetYaxis()->SetRangeUser(-110.,110.);
  fGAroon->SetLineColor(kBlue);

  return fGAroon;
  
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

    if (fEvent >= fInterval){
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
/// Hi-Lo Analysis

Int_t HiLoAnalysis(TFile *f){

  TTreeReader fReader(gSymbol, f);
  TTreeReaderArray<char> fDt(fReader,"fDate");
  TTreeReaderValue<Float_t> fH(fReader,"fHigh");
  TTreeReaderValue<Float_t> fL(fReader,"fLow");

  TH1F *fHDiffLL = new TH1F("fHDiffLL",
			    gSymbol+";Difference between "+gFreq+" high and previous "
			    +gFreq+" low prices;Counts from "+gStartDate.AsSQLString()
			    +" to "+gEndDate.AsString(),
			    NBINS,XMIN,XMAX);
  TH1F *fHDiffHH = new TH1F("fHDiffHH",
			    gSymbol+";Difference between "+gFreq+" high and previous "
			    +gFreq+" high prices;Counts from "+gStartDate.AsSQLString()
			    +" to "+gEndDate.AsSQLString(),
			    NBINS,XMIN,XMAX);
  TH1F *fHDiffLH = new TH1F("fHDiffLH",
			    gSymbol+";Difference between "+gFreq+" high and previous "
			    +gFreq+" low prices;Counts from "+gStartDate.AsSQLString()
			    +" to "+gEndDate.AsSQLString(),
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

  TCanvas *cHiLo = new TCanvas("cHiLo","cHiLo",2048,1152);
  cHiLo->Divide(2,2);
  
  cHiLo->cd(1);
  //Need to Check: Is it a Poisson Distribution?
  TF1 *fPoisson = new TF1("fPoisson","[0]*TMath::Power(([1]/[2]),(x/[2]))*(TMath::Exp(-([1]/[2])))/TMath::Gamma((x/[2])+1)", -0.1, 0.5);
  //gStyle->SetOptFit();
  fPoisson->SetParameters(1, 1, 1);
  fHDiffLH->Fit("fPoisson","QR");
  fHDiffLH->Draw();

  cHiLo->cd(2);
  TF1 *fGausLL = new TF1("GausLL","gaus");
  fHDiffLL->Fit("GausLL","QM+");
  Float_t fHPriceLL = fPrevL*(1+fGausLL->GetParameter(1));
  printf("Next Low price: %f\n",fHPriceLL);
  fHDiffLL->Draw();

  cHiLo->cd(3);
  TF1 *fGausHH = new TF1("GausHH","gaus");
  fHDiffHH->Fit("GausHH","QM+");
  Float_t fHPriceHH = fPrevH*(1 + fGausHH->GetParameter(1));
  
  return 0;
  
}

////////////////////////////////////////////////////////////////////////////////
/// BollingerBands
TGraphErrors *GetBollingerBands(TFile *f,
			       Int_t fInterval = 20,
			       Float_t fW = 2.0
			       ){
  
  TTreeReader fReader(gSymbol, f);
  TTreeReaderArray<char> fDt(fReader,"fDate");
  TTreeReaderValue<Float_t> fC(fReader,"fClose");
  
  TDatime fDate;

  TGraph *fGSMA = GetSMA(f, fInterval, "close");

  TGraphErrors *fGBB = new TGraphErrors();

  Float_t Price[fInterval];

  Int_t fEvent = 0;
  Int_t inow = 0;

  while(fReader.Next()){
    
    TString fSDt = static_cast<char*>(fDt.GetAddress());
    fSDt.Append(" 00:00:00");
    fDate = TDatime(fSDt);

    inow = GetIndex(fEvent,fInterval);

    Price[inow] = *fC;

    if ( fEvent >= fInterval ) {
      Double_t x1, y1;
      Double_t b;
      fGSMA->GetPoint(fEvent-fInterval,x1,y1);
      b = fW*TMath::StdDev(fInterval,&Price[0]);
      Int_t n = fGBB->GetN();
      fGBB->SetPoint(n,x1,y1);
      fGBB->SetPointError(n,1.,b);
    }
    
    fEvent++;
  }
  fGBB->GetXaxis()->SetTimeDisplay(1);
  fGBB->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  fGBB->GetXaxis()->SetTimeOffset(0,"gmt");
  fGBB->SetFillColor(6);
  fGBB->SetFillStyle(3003);

  fGBB->SetTitle(Form("%s BollingerBands(%d);Date;BB",gSymbol.Data(),fInterval));  
  return fGBB;

}

////////////////////////////////////////////////////////////////////////////////
/// Volume Graph
TGraph *GetVolume(TFile *f){
  
  TTreeReader fReader(gSymbol, f);
  TTreeReaderArray<char> fDt(fReader,"fDate");
  TTreeReaderValue<Int_t> fVol(fReader,"fVolume");

  TDatime fDate;
  TGraph *fGVol = new TGraph();
  
  while(fReader.Next()){
    
    TString fSDt = static_cast<char*>(fDt.GetAddress());
    fSDt.Append(" 00:00:00");
    fDate = TDatime(fSDt);

    fGVol->SetPoint(fGVol->GetN(),fDate.Convert(),*fVol);

  }
  fGVol->SetTitle(Form("%s Volume;Date;Volume",gSymbol.Data()));
  fGVol->GetXaxis()->SetTimeDisplay(1);
  fGVol->GetXaxis()->SetTimeFormat("%Y/%m/%d");
  fGVol->GetXaxis()->SetTimeOffset(0,"gmt");
  return fGVol;
  
}

////////////////////////////////////////////////////////////////////////////////
/// Main Function
Int_t Analyzer( TString fSymbol = "SPY",
	      TString fFreq = "1d",
	      TDatime fStartDate = TDatime("2009-10-01 00:00:00"),
	      TDatime fEndDate = TDatime("2010-06-05 00:00:00")) {

  gSymbol = fSymbol;
  gFreq = fFreq;
  gStartDate = fStartDate;
  gEndDate = fEndDate;
  
  TFile *f = GetData(fSymbol, fFreq, fStartDate, fEndDate);
  f->ReOpen("READ");
  gSymbol = fSymbol;

  HiLoAnalysis(f);
  
  TCanvas *c1 = new TCanvas("c1","c1",2048,1152);
  c1->Divide(3,2);
  
  c1->cd(1);
  TGraph *fGSlowSMA = GetSMA(f, 10,"close");
  fGSlowSMA->SetLineColor(kBlue);
  fGSlowSMA->Draw("AL");
  TGraph *fGFastSMA = GetSMA(f, 6, "close");
  fGFastSMA->SetLineColor(kGreen);
  fGFastSMA->Draw("same");

  c1->cd(2);
  TGraph *fGAroonUp = GetAroonUp(f,25);
  fGAroonUp->Draw("al");
  TGraph *fGAroonDown = GetAroonDown(f,25);
  fGAroonDown->Draw("same");

  c1->cd(3);
  TGraph *fGAroon = GetAroon(f,25);
  fGAroon->Draw("al");

  c1->cd(4);
  TGraphErrors *fGBB = GetBollingerBands(f,20,2.0);
  fGBB->Draw("a3C");  

  c1->cd(5);
  TGraph *fGVol = GetVolume(f);
  fGVol->Draw("AB");
  
     
  c1->Print(fSymbol+".png");

  return 0;

}
////////////////////////////////////////////////////////////////////////////////


  
