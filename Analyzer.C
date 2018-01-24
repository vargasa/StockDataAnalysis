#include "TStock.h"
#include <fstream>

/////////////////////////////////////////////////////////////////////
/// Allows to normalize Graph to get better comparisons between
/// different priced Stocks
void NormalizeGraph(TGraph *gr){

  Float_t scale = 1/TMath::MaxElement(gr->GetN(),gr->GetY());
  for (int i=0 ; i< gr->GetN();i++) gr->GetY()[i] *= scale;
  gr->GetYaxis()->SetRangeUser(0,1.0);
  //Double_t disp = gr->GetY()[0];
  //for (int i=0; i< gr->GetN(); i++) gr->GetY()[i] -= disp;
  
}

/////////////////////////////////////////////////////////////////////
/// Look for Aroon crossing zero with certain threshold, positive Threshold
/// Aroon will be crossing zero going up and negative Threshold will be
/// Aroon crossing zero going down.
Bool_t AroonZero(TStock *Stock, Int_t AroonPeriod, Int_t Period, Float_t Threshold){

  if (Stock->GetData()->GetEntries() < AroonPeriod){
    Error("AroonZero","Not enough data to compute Aroon");
    return false;
  }

  TGraph *ga = Stock->GetAroon(AroonPeriod);
  if (!ga) return 0;
  
  Int_t n = ga->GetN();
  n--;

  Double_t *Aroon = ga->GetY();
  
  if (Threshold > 0. && Aroon[n] < Threshold) return false;
  if (Threshold < 0. && Aroon[n] > Threshold) return false;

  if (Threshold > 0){
    for(Int_t i = 0; i < Period; i++){
      if ( Aroon[n-i] > 0.0 && Aroon[n-i-1] < 0.0) return true;
    }
  } else {
    for(Int_t i = 0; i < Period; i++){
      if ( Aroon[n-i] < 0.0 && Aroon[n-i-1] > 0.0) return true;
    }

  }
  
  return false;
}
/////////////////////////////////////////////////////////////////////
/// Positive derivative required for Period, last derivative
/// data point must be greater than Threshold
Bool_t PositiveDerivative(TStock *Stock, Int_t SMAPeriod, Int_t Period, Float_t Threshold){

  TGraph *g = Stock->GetSMA(SMAPeriod);
  TGraph *p = Stock->GetDerivative(g,"Relative");
  Int_t n = p->GetN();
  if (n < Period) return false;
  n--;
  if (p->GetY()[n] < Threshold) return false;

  for (Int_t i = 0; i < Period; i++){
    if (p->GetY()[n-i] < 0.0) return false;
  }

  return true;

}

/////////////////////////////////////////////////////////////////////
// Croossover of IFast over ISlow in the last *Period*
// Minimum difference between (now) between IFast and ISlow is Delta(%)
Bool_t Crossover(TStock *Stock, Int_t IFast = 6, Int_t ISlow = 10,
		 Int_t Period = 5, Float_t Delta = 0.0){

  // Look for SMA Crossovers
  TGraph *Fast = Stock->GetSMA(IFast);
  TGraph *Slow = Stock->GetSMA(ISlow);
  if (Slow->GetN() < Period) return false;
  Double_t prs[Period];
  Double_t prf[Period];
  Period = Period - 1;
  for (Int_t i = 0; i <= Period; i++) {
    Double_t y1,y2,t;
    Slow->GetPoint(Slow->GetN() - 1 - i,t,y1);
    Fast->GetPoint(Fast->GetN() -1 - i,t,y2);
    Int_t n = Period - i;
    prs[n] = y1;
    prf[n] = y2;
  }
  
  for (Int_t i = 0; i < Period; i++) {
    Int_t n = Period - i;
    Double_t now = prf[n]-prs[n];
    Double_t prev = prf[n-1]-prs[n-1];
        
    Float_t diff = 100.*(prf[Period] - prs[Period])/prs[Period];
    
    if (Delta > 0 && now  > 0 &&  prev < 0 && diff > Delta) {
      printf("F_SMA Crossover for %s.\nDelta: %.2f%%\nFound around Index:%d\n", Stock->GetSymbol().Data(), diff,i+1);
      return  true;
    } else if (Delta < 0 && now < 0 && prev > 0 && diff < Delta) {
      printf("S_SMA Crossover for %s. Delta: %.2f%%\nFound around Index:%d\n", Stock->GetSymbol().Data(), diff,i+1);
      return true;
    }
  }
  
  return false;

}

/////////////////////////////////////////////////////////////////////
/// Find if there is an inflection in the SMA(*SMAPeriod*) in the last
/// number of *Period* of time
Bool_t Inflection(TStock *Stock, Int_t SMAPeriod = 25, Int_t Period = 4){

  TGraph *g = Stock->GetSMA(SMAPeriod);
  TGraph *p = Stock->GetDerivative(g,"Relative");
  Int_t n = p->GetN();
  if (n < Period) return false;
  n--;
  for(Int_t i = 0; i < Period; i++){
    if ( p->GetY()[n-i] > 0.0 && p->GetY()[n-i-1] < 0.0) return true;
  }
  return false;

}

/////////////////////////////////////////////////////////////////////
TCanvas *HiLoAnalysis(TStock *Stock){
  
  Float_t xmin = -0.5;
  Float_t xmax = 0.5;
  Int_t nbins = 100;
  TString Symbol = Stock->GetSymbol();

  TTree *t = Stock->GetData();
  TTreeReader fReader(t);
  TTreeReaderValue<Float_t> fH(fReader,"fHigh");
  TTreeReaderValue<Float_t> fL(fReader,"fLow");

  TH1F *fHDiffLL = new TH1F("fHDiffLL_"+Symbol,
			    Symbol+";Difference between "+Stock->GetFreq()+" high and previous "
			    +Stock->GetFreq()+" low prices;Counts",
			    nbins,xmin,xmax);
  TH1F *fHDiffHH = new TH1F("fHDiffHH_"+Symbol,
			    Symbol+";Difference between "+Stock->GetFreq()+" high and previous "
			    +Stock->GetFreq()+" high prices;Counts",
			    nbins,xmin,xmax);
  TH1F *fHDiffLH = new TH1F("fHDiffLH_"+Symbol,
			    Symbol+";Difference between "+Stock->GetFreq()+" high and previous "
			    +Stock->GetFreq()+" low prices;Counts",
			    nbins,xmin,xmax);

  Float_t fPrevL;
  Float_t fPrevH;

  Int_t fEvent = 0;

  Float_t fNPriceHH;
  Float_t fNPriceLL;
  
  while(fReader.Next()){

    fEvent++;
    fHDiffLL->Fill( (*fL - fPrevL)/(fPrevL) );
    fHDiffLH->Fill( (*fH - fPrevL)/(fPrevL) );
    fHDiffHH->Fill( (*fH - fPrevH)/(fPrevH) );
    
    fPrevL = *fL;
    fPrevH = *fH; 

  }

  TCanvas *cHiLo = new TCanvas(Form("cHiLo%s",Symbol.Data()),
			       Form("cHiLo%s",Symbol.Data()),
			       2048,1152);
  cHiLo->Divide(2,2);
  
  cHiLo->cd(1);
  //Need to Check: Is it a Poisson Distribution?
  TF1 *fPoisson = new TF1("fPoisson","[0]*TMath::Power(([1]/[2]),(x/[2]))*(TMath::Exp(-([1]/[2])))/TMath::Gamma((x/[2])+1)", -0.1, 0.5);
  gStyle->SetOptFit();
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
  printf("Next High price: %f\n",fHPriceHH);
  fHDiffHH->Draw();
  
  return cHiLo;
  
}

////////////////////////////////////////////////////////////////////////////////
/// Simple Moving Average Crossover finder, fPeriod terms behind the actual term
/// are scanned for a crossover with a minimum relative difference of fDelta
/// The difference is computed between the last data point available and the crossover
TCanvas *GetCanvas(TStock *Stock, TStock *Benchmark, Int_t fFast = 6, Int_t fSlow = 10, Int_t fBB = 25){

  TCanvas *c1;
  TPad *pad1,*pad2,*pad3, *pad4, *pad5;

  c1 = new TCanvas(Stock->GetSymbol().Data(),"c1",2048,1152);
  c1->SetBatch(kTRUE);
  pad1 = new TPad("pad1", "p1",0.0,0.6,1.0,1.0);
  pad2 = new TPad("pad2", "p2",0.0,0.3,1.0,0.6);
  pad3 = new TPad("pad3", "p3",0.0,0.0,0.3,0.3);
  pad4 = new TPad("pad4", "p4",0.3,0.0,0.6,0.3);
  pad5 = new TPad("pad5", "p5",0.6,0.0,1.0,0.3);
  
  pad2->SetFillStyle(4000); //will be transparent
  pad2->SetFrameFillStyle(0);
  pad1->Draw();
  pad2->Draw();
  pad2->SetGrid();
  pad3->SetFillStyle(4000);
  pad3->Draw();
  pad3->SetGrid();
  pad4->Draw();
  pad4->SetGrid();
  pad5->Draw();
  pad5->SetGrid();

  TMultiGraph *fGCandle;
  TGraphErrors *fGBB;
  TGraph *fGSlowSMA;
  TGraph *fGFastSMA;
  TGraph *fGVWMA;
  TLegend *fLegend;
  THStack *fHSVol;
  TGraph *fGDerivative;
  
  pad1->cd();
  pad1->SetGrid();

  fGCandle = Stock->GetCandleStick();   
  fGBB = Stock->GetBollingerBands(fBB,2.0);
  fGCandle->Add(fGBB,"A3C");
  
  fGSlowSMA = Stock->GetSMA(fSlow,"close");
  fGSlowSMA->SetLineWidth(3);
  fGSlowSMA->SetLineColor(kBlue);
  fGCandle->Add(fGSlowSMA);
  fGFastSMA = Stock->GetSMA(fFast, "close");
  fGFastSMA->SetLineWidth(3);
  fGFastSMA->SetLineColor(kGreen);
  fGCandle->Add(fGFastSMA);
  fGVWMA = Stock->GetVWMA(fBB, "close");
  fGVWMA->SetLineWidth(2);
  fGVWMA->SetLineColor(kRed);
  fGVWMA->SetLineStyle(7);
  fGCandle->Add(fGVWMA);
  fGCandle->Draw("A");
  TDatime tStart = TDatime(2016,01,01,00,00,00);
  TDatime tEnd = TDatime(Stock->GetEndDate().GetSec());
  fGCandle->GetXaxis()->SetRangeUser(tStart.Convert(),tEnd.Convert());
  fGCandle->GetXaxis()->SetTimeDisplay(1);
  fGCandle->GetXaxis()->SetTimeFormat("%b/%d/%y");
  fGCandle->GetXaxis()->SetTimeOffset(0,"gmt");
  
  fLegend = new TLegend(0.1,0.7,0.2,0.9);
  fLegend->AddEntry(fGSlowSMA,Form("SMA(%d)",fSlow),"l");
  fLegend->AddEntry(fGFastSMA,Form("SMA(%d)",fFast),"l");
  fLegend->AddEntry(fGVWMA,Form("VWMA(%d)",fBB),"l");
  fLegend->AddEntry(fGBB,Form("BB(%d)",fBB));
  fLegend->Draw();
   
  pad2->cd();
  
  fHSVol = Stock->GetVolume();
  fHSVol->Draw();
  fHSVol->GetXaxis()->SetRangeUser(tStart.Convert(),tEnd.Convert());
  fHSVol->GetXaxis()->SetTimeDisplay(1);
  fHSVol->GetXaxis()->SetTimeFormat("%b/%d/%y");
  fHSVol->GetXaxis()->SetTimeOffset(0,"gmt");
  TH1 *hh = ((TH1 *)(fHSVol->GetStack()->Last()));
  hh->GetXaxis()->SetRangeUser(tStart.Convert(),tEnd.Convert());
  fHSVol->SetMaximum(hh->GetMaximum());
  
  pad3->cd();
  TGraph *smaux = Stock->GetSMA(fBB,"close");
  fGDerivative = Stock->GetDerivative(smaux,"Relative");
  fGDerivative->SetTitle(Form("First Relative Derivative SMA(%d)",fBB));
  fGDerivative->Draw("AB");
  fGDerivative->GetXaxis()->SetRangeUser(tStart.Convert(),tEnd.Convert());

  pad4->cd();
  TGraph *Garoon = Stock->GetAroon(14);
  Garoon->SetFillColor(kBlue);
  Garoon->Draw("AB");
  Garoon->GetXaxis()->SetRangeUser(tStart.Convert(),tEnd.Convert());

  pad5->cd();
  TGraph *smabench = Benchmark->GetSMA(fBB);
  NormalizeGraph(smabench);
  smabench->SetLineColor(kRed);
  //smabench->GetXaxis()->SetRangeUser(tStart.Convert(),tEnd.Convert());
  smabench->Draw("AL");
  //smaux->GetXaxis()->SetRangeUser(tStart.Convert(),tEnd.Convert());
    
  NormalizeGraph(smaux);
  smaux->Draw("SAME");

  return c1;

}

////////////////////////////////////////////////////////////////////////////////
/// Main Function
Int_t Analyzer(TString filename = "Symbols/NASDAQ.txt") {
  
  std::ifstream infile(filename.Data());

  std::string line;

  //TFile *fOut = new TFile("Output/SMACrossover.root","UPDATE");
  TFile *fDB = new TFile("Output/SymbolsDB.root","READ");
  
  while (std::getline(infile, line)){
    TString Symbol = TString(line);
    TString Freq = "1wk";
    TString StartDate = "2009-10-01 00:00:00";
    TString EndDate = "now";

    printf("Analyzing %s\n",Symbol.Data());
    TStock *s1 = new TStock(Symbol,Freq,StartDate,EndDate);
    TStock *bmark = new TStock("SPY",Freq,StartDate,EndDate);
    bmark->SetDBFile(fDB);
    bmark->GetData();
    s1->SetDBFile(fDB);
    
    TTree *data = s1->GetData();
    if (!data) continue;
    if (data->GetEntries() < 52) continue; //At least one year old
    TCanvas *cana = GetCanvas(s1,bmark);
    //TCanvas *cana = HiLoAnalysis(s1);
    if (cana) {
      //if(PositiveDerivative(s1,25,8,1.0)){
      //if(Crossover(s1,6,10,6,-1.0)){
      //if(Inflection(s1,25,16) && PositiveDerivative(s1,25,4,1.0)){
      if(AroonZero(s1,14,16,90)){
	// if(fOut->GetListOfKeys()->Contains(Symbol.Data())){
	//   fOut->Delete(s1->GetSymbol()+";1");
	// }
	//cana->Write(Form("HiLo_%s",s1->GetSymbol().Data()));
	cana->Print(Form("Output/%s.png",s1->GetSymbol().Data()));
      }
    }
    delete cana;
  }


  fDB->Close();

  // fOut->Close();
  
  return 0;

}
////////////////////////////////////////////////////////////////////////////////
 

