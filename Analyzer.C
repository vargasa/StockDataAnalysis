#include "TStock.h"
#include <fstream>

////////////////////////////////////////////////////////////////////////////////
/// Simple Moving Average Crossover finder, fPeriod terms behind the actual term
/// are scanned for a crossover with a minimum relative difference of fDelta
/// The difference is computed between the last data point available and the crossover
TCanvas *SMACrossoverScreener(TStock *Stock, Int_t fFast = 6, Int_t fSlow = 10,
			      Int_t fPeriod = 5, Float_t fDelta = 0.00, Int_t fBB = 25){

  TCanvas *c1;
  TPad *pad1,*pad2,*pad3;

  c1 = new TCanvas(Stock->GetSymbol().Data(),"c1",2048,1152);
  c1->SetBatch(kTRUE);
  pad1 = new TPad("pad1", "p1",0.0,0.50,1.0,1.0);
  pad2 = new TPad("pad2", "p2",0.0,0.25,1.0,0.50);
  pad3 = new TPad("pad3", "p3",0.0,0.0,1.0,0.25);
  pad2->SetFillStyle(4000); //will be transparent
  pad2->SetFrameFillStyle(0);
  pad1->Draw();
  pad2->Draw();
  pad2->SetGrid();
  pad3->SetFillStyle(4000);
  pad3->Draw();
  pad3->SetGrid();

  TMultiGraph *fGCandle;
  TGraphErrors *fGBB;
  TGraph *fGSlowSMA;
  TGraph *fGFastSMA;
  TGraph *fGVWMA;
  TLegend *fLegend;
  THStack *fHSVol;
  TH1F *fGDerivative;
  
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
  fGDerivative = Stock->GetDerivative(Stock->GetSMA(fBB,"close"));
  fGDerivative->SetTitle("First Relative Derivative SMA(fBB)");
  fGDerivative->Draw();
  fGDerivative->GetXaxis()->SetRangeUser(tStart.Convert(),tEnd.Convert());
   
  Float_t smaprime; // Require Positive derivative of SMA(fBB)
  smaprime = fGDerivative->GetBinContent(fGDerivative->GetNbinsX());
  
   // Look for SMA Crossovers
   Double_t prs[fPeriod];
   Double_t prf[fPeriod];
   for (Int_t i = 0; i < fPeriod; i++) {
     Double_t y1,y2,t;
     fGSlowSMA->GetPoint(fGSlowSMA->GetN()-1 - i,t,y1);
     fGFastSMA->GetPoint(fGFastSMA->GetN()-1 - i,t,y2);
     prs[fPeriod-i-1] = y1;
     prf[fPeriod-i-1] = y2;
   }
   
   for (Int_t i = 1; i < fPeriod-1; i++) {
     
     Double_t prev = prf[i-1]-prs[i-1];
     Double_t aft = prf[i+1]-prs[i+1];
     
     if ( aft  > 0 &&  prev < 0 && smaprime > 0.0) {
       Float_t diff = (prf[fPeriod-1] - prs[i])/prs[i];
       if ( diff > fDelta ){
   	 printf("F_SMA Crossover for %s. Delta: %.2f%%\n", Stock->GetSymbol().Data(), diff*100.);
   	 return c1;
       }
       break;
     } else if (aft < 0 && prev > 0)  {
       printf("S_SMA Crossover for %s\n", Stock->GetSymbol().Data());
       break;
     }
   }

   return 0;

}

////////////////////////////////////////////////////////////////////////////////
/// Main Function
Int_t Analyzer(TString filename = "Symbols/NASDAQ.txt") {
  
  std::ifstream infile(filename.Data());

  std::string line;

  TFile *fOut = new TFile("Output/SMACrossover.root","UPDATE");
  
  while (std::getline(infile, line)){
    TString Symbol = TString(line);
    TString Freq = "1wk";
    TString StartDate = "2009-10-01 00:00:00";
    TString EndDate = "now";

    printf("Analyzing %s\n",Symbol.Data());
    TStock *s1 = new TStock(Symbol,Freq,StartDate,EndDate);
    
    TTree *data = s1->GetData();
    if (data) {
      
      TCanvas *cana = SMACrossoverScreener(s1,6,10,6,0.00);
      if (cana) {	
	if(fOut->GetListOfKeys()->Contains(Symbol.Data())){
	  fOut->Delete(s1->GetSymbol()+";1");
	}
      	cana->Write(s1->GetSymbol().Data());
      }
      delete cana;
    }
  }

  fOut->Close();
  
  return 0;

}
////////////////////////////////////////////////////////////////////////////////
 

