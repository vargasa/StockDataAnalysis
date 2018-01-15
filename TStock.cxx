#include "TStock.h"

ClassImp(TStock);

////////////////////////////////////////////////////////////////////////////////
TStock::~TStock(){

  if(fTree) delete fTree;

}

////////////////////////////////////////////////////////////////////////////////

TStock::TStock(TString Symbol, TString Freq, TString StartDate, TString EndDate)
  :   fTS(fReader,"fTimeStamp"),
      fO(fReader,"fOpen"),
      fL(fReader,"fLow"),
      fH(fReader,"fHigh"),
      fC(fReader,"fClose"),
      fVol(fReader,"fVolume")
{

  Int_t yy, mm, dd;
  Int_t hh, min, sec;
  TTimeStamp sdate, edate;
  
  if (sscanf(StartDate.Data(), "%d-%d-%d %d:%d:%d", &yy, &mm, &dd, &hh, &min, &sec) == 6){
    sdate = TTimeStamp(yy,mm,dd,hh,min,sec,0,false);
  } else {
    Error("TStock::TStock()","StartDate not set propperly. Must be: yy-mm-dd hour:min:sec");
  }

  if(EndDate.Contains("now")) {
    edate = TTimeStamp();
  } else if (sscanf(EndDate.Data(), "%d-%d-%d %d:%d:%d", &yy, &mm, &dd, &hh, &min, &sec) == 6){
    edate = TTimeStamp(yy,mm,dd,hh,min,sec,0,false); 
  } else {
    Error("TStock::TStock()","EndDate not set propperly. Must be: yy-mm-dd hour:min:sec");
  }

  fSymbol = Symbol;
  fFreq = Freq;
  fStartDate = sdate;
  fEndDate = edate;
  fTree = 0;
  
}

////////////////////////////////////////////////////////////////////////////////
/// Time width window for Charts

Double_t TStock::GetTimeWidth(TString Freq) const{

  // Definition of the candlestick width
  // Period of time in seconds
  Freq.ToLower();
  Double_t twidth = 86400.;
  if (Freq.Contains("1d")) {
    twidth = 86400.;
  } else if(Freq.Contains("1wk")) {
    twidth = 604800.;
  } else if(Freq.Contains("1mo")) {
    twidth = 2.628e6;
  }
  
  return twidth;

}

////////////////////////////////////////////////////////////////////////////////
/// Compute index to reuse array filling it cyclically

Int_t TStock::GetIndex(Int_t Event, Int_t Interval) const{
  
  return Event%Interval;
  
}

////////////////////////////////////////////////////////////////////////////////
/// Get data from Yahoo! Finance API using getData.sh Shell Script

TTree *TStock::GetData(){

  if(fDBFile) {
    TTree *t1 = (TTree*)fDBFile->Get(fSymbol+"_"+fFreq+";1");
    if (t1) {
      fTree = t1;
      fReader.SetTree(fTree);
    }
  }
  
  if(fTree) return fTree;

  TTree *tree = new TTree(fSymbol,"From CSV File");

  Int_t ans = gSystem->Exec(Form("sh getData.sh '%s' '%s' '%d' '%d' /tmp/",
				 fSymbol.Data(),fFreq.Data(),fStartDate.GetDate(),fEndDate.GetDate()));

  if (ans != 0) {
    Error("TStock::GetData()","Data Download was unsucessfull");
    return 0;
  }
    
  tree->ReadFile("/tmp/"+fSymbol+".csv",
		 "fDate/C:fOpen/F:fHigh/F:fLow/F:fClose/F:fCloseAdj/F:fVolume/I",',');
  
  fReader.SetTree(tree);
  TTreeReaderArray<char> fDt(fReader,"fDate");
  
  TTimeStamp date;
  TBranch *bts = tree->Branch("fTimeStamp", &date);
  
  while(fReader.Next()){
    TString fSDt = static_cast<char*>(fDt.GetAddress());
    
    Int_t yy, mm, dd;
    if (sscanf(fSDt.Data(), "%d-%d-%d", &yy, &mm, &dd) == 3){
      date = TTimeStamp(yy,mm,dd,00,00,00,0,false);
      bts->Fill();
    } 
  }
  
  tree->ResetBranchAddresses();

  fTree = tree;
  fReader.SetTree(fTree);

  return tree;
}

////////////////////////////////////////////////////////////////////////////////
/// AroonDown

TGraph *TStock::GetAroonDown(Int_t Interval) {

  if (!fTree) {
    Error("TStock::GetAroonDown()","No data available. Did you forget to call GetData()?");
    return 0;
  }
  
  Int_t Event = 0;
  Float_t Price[Interval];
  TGraph *GAD = new TGraph();

  fReader.Restart();
  
  while(fReader.Next()){

    Int_t inow = GetIndex(Event,Interval);

    Price[inow] = *fL;

    if(Event >= Interval){

      Float_t aroondown = 0;
      Int_t MinIndex = 0;
      
      MinIndex = TMath::LocMin(Interval,Price);
            
      if ( MinIndex < inow || MinIndex == inow ) {
    	aroondown = (Float_t)(Interval - (inow - MinIndex))*100; 
      } else {
    	aroondown = (Float_t)(Interval - (Interval-MinIndex+inow))*100;	
      }
      aroondown = aroondown/((Float_t)Interval);
      GAD->SetPoint(GAD->GetN(),fTS->AsDouble(),aroondown);
    }
    
    Event++;
  }

  GAD->SetLineWidth(2);
  GAD->SetLineStyle(1);
  GAD->SetTitle(Form("%s AroonDown(%d);Date;AroonDown",fSymbol.Data(),Interval));
  GAD->GetXaxis()->SetTimeDisplay(1);
  GAD->GetXaxis()->SetTimeFormat("%b/%d/%y");
  GAD->GetXaxis()->SetTimeOffset(0,"gmt");
  GAD->GetYaxis()->SetRangeUser(0.,110.);
  GAD->SetLineColor(kRed);
  
  return GAD;

}

////////////////////////////////////////////////////////////////////////////////
/// AroonUp

TGraph *TStock::GetAroonUp(Int_t Interval){

  if (!fTree) {
    Error("TStock::GetAroonUp()","No data available. Did you forget to call GetData()?");
    return 0;
  }
  
  Int_t Event = 0;
  Float_t Price[Interval];
  TGraph *GAU = new TGraph();

  fReader.Restart();
  
  while(fReader.Next()){
	
    Float_t aroonup;
    Int_t inow = GetIndex(Event,Interval);

    Price[inow] = *fH;
    
    if (Event >= Interval) {
      
      Int_t MaxIndex;
      MaxIndex  = TMath::LocMax(Interval,Price);
      if ( MaxIndex < inow || MaxIndex == inow ) {
    	aroonup = (Float_t)(Interval - (inow - MaxIndex))*100;
	
      } else {
    	aroonup = (Float_t)(Interval - (Interval-MaxIndex+inow))*100;	
      }
      aroonup = aroonup/(Float_t)Interval;
    
      GAU->SetPoint(GAU->GetN(),fTS->AsDouble(),aroonup); 
    }

    Event++;
  }
  GAU->SetLineWidth(2);
  GAU->SetLineStyle(1);
  GAU->SetTitle(Form("%s AroonUp(%d);Date;AroonUp",fSymbol.Data(),Interval));
  GAU->GetXaxis()->SetTimeDisplay(1);
  GAU->GetXaxis()->SetTimeFormat("%b/%d/%y");
  GAU->GetXaxis()->SetTimeOffset(0,"gmt");
  GAU->GetYaxis()->SetRangeUser(0.,110.);
  GAU->SetLineColor(kGreen);

  return GAU;

}

////////////////////////////////////////////////////////////////////////////////
/// Aroon Oscillator

TGraph *TStock::GetAroon(Int_t Interval){

  if(!fTree){
    Error("TStock::GetAroon","No data available. Did you forget to call GetData()?");
    return 0;
  }

  TGraph *GAroon = new TGraph();
  TGraph *GAU = this->GetAroonUp(Interval);
  TGraph *GAD = this->GetAroonDown(Interval);
  for (Int_t i = 0; i < GAU->GetN(); i++){
    Double_t x1, y1, x2, y2;
    GAU->GetPoint(i,x1,y1);
    GAD->GetPoint(i,x2,y2);
    GAroon->SetPoint(i,x1,y1-y2);
  }
  GAroon->SetLineWidth(2);
  GAroon->SetLineStyle(1);
  GAroon->SetLineColor(kBlue);
  GAroon->SetTitle(Form("%s Aroon Oscillator(%d);Date;Aroon",fSymbol.Data(),Interval));
  GAroon->GetXaxis()->SetTimeDisplay(1);
  GAroon->GetXaxis()->SetTimeFormat("%b/%d/%y");
  GAroon->GetXaxis()->SetTimeOffset(0,"gmt");
  GAroon->GetYaxis()->SetRangeUser(-110.,110.);

  return GAroon;
}

////////////////////////////////////////////////////////////////////////////////
/// Simple Moving Average SMA

TGraph *TStock::GetSMA(Int_t Interval, Option_t *Option){

  if (!fTree) {
    Error("TStock::GetSMA()","No data available. Did you forget to call GetData()?");
    return 0;
  }
  
  Int_t Event = 0;

  Float_t Price[Interval];
  TGraph *GSMA = new TGraph();

  fReader.Restart();
  
  while(fReader.Next()){
    
    Price[GetIndex(Event,Interval)] = *fC;

    if (Event >= Interval){
      Float_t SMA = TMath::Mean(Interval,&Price[0]);     
      GSMA->SetPoint(GSMA->GetN(),fTS->GetSec(),SMA);
    }
    Event++;
    
  }
  GSMA->SetLineWidth(2);
  GSMA->SetLineStyle(1);
  GSMA->SetTitle(Form("%s SMA(%d);Date;SMA",fSymbol.Data(),Interval));
  GSMA->GetXaxis()->SetTimeDisplay(1);
  GSMA->GetXaxis()->SetTimeFormat("%b/%d/%y");
  GSMA->GetXaxis()->SetTimeOffset(0,"gmt");

  return GSMA;


}

////////////////////////////////////////////////////////////////////////////////
/// Volume Weighted Moving Average

TGraph *TStock::GetVWMA(Int_t Interval, Option_t *Option){

  if (!fTree) {
    Error("TStock::GetVWMA()","No data available. Did you forget to call GetData()?");
    return 0;
  }
  
  Int_t Event = 0;

  Float_t Price[Interval];
  Double_t Volume[Interval];
  TGraph *GVWMA = new TGraph();

  fReader.Restart();
  
  while(fReader.Next()){

    Int_t n = GetIndex(Event,Interval);
    Price[n] = *fC;
    Volume[n] = (Double_t)(*fVol);

    if (Event >= Interval){
      Float_t VMWA = TMath::Mean(&Price[0],&Price[Interval],&Volume[0]);
      GVWMA->SetPoint(GVWMA->GetN(),fTS->GetSec(),VMWA);
    }
    Event++;
    
  }
  GVWMA->SetLineColor(kRed);
  GVWMA->SetLineWidth(2);
  GVWMA->SetLineStyle(7);
  GVWMA->SetTitle(Form("%s SMA(%d);Date;SMA",fSymbol.Data(),Interval));
  GVWMA->GetXaxis()->SetTimeDisplay(1);
  GVWMA->GetXaxis()->SetTimeFormat("%b/%d/%y");
  GVWMA->GetXaxis()->SetTimeOffset(0,"gmt");

  return GVWMA;

}

////////////////////////////////////////////////////////////////////////////////
/// BollingerBands

TGraphErrors *TStock::GetBollingerBands(Int_t Interval, Float_t fW){

  if (!fTree) {
    Error("TStock::GetBollingerBands()","No data available. Did you forget to call GetData()?");
    return 0;
  }

  TGraph *GSMA = GetSMA(Interval, "close");

  TGraphErrors *GBB = new TGraphErrors();

  Float_t Price[Interval];

  Int_t Event = 0;
  Int_t inow = 0;

  fReader.Restart();
  
  while(fReader.Next()){
    
    inow = GetIndex(Event, Interval);

    Price[inow] = *fC;

    if ( Event >= Interval ) {
      Double_t x1, y1;
      Double_t b;
      GSMA->GetPoint(Event-Interval,x1,y1);
      b = fW*TMath::StdDev(Interval,&Price[0]);
      Int_t n = GBB->GetN();
      GBB->SetPoint(n,x1,y1);
      GBB->SetPointError(n,1.,b);
    }
    
    Event++;
  }
  // Can "3C" Option be added?
  GBB->SetFillColor(6);
  GBB->SetFillStyle(3003);
  GBB->GetXaxis()->SetTimeDisplay(1);
  GBB->GetXaxis()->SetTimeFormat("%b/%d/%y");
  GBB->GetXaxis()->SetTimeOffset(0,"gmt");

  GBB->SetTitle(Form("%s BollingerBands(%d);Date;BB",fSymbol.Data(),Interval));  
  return GBB;

}


////////////////////////////////////////////////////////////////////////////////
/// Relative numerical derivative working only for equaly distant x-data
/// Expressed in %terms with the previous period value as reference

TGraph *TStock::GetDerivative(TGraph *gr,Option_t *Option){

  TString opt = Option;
  opt.ToLower();

  TGraph *GDerivative = new TGraph();

  Double_t x1,y1,y2;
  Double_t dy;
  Int_t npoints = gr->GetN();

  if (opt.Contains("relative")) { 
    for (Int_t i = 1; i < npoints; i++){
      gr->GetPoint(i-1,x1,y1);
      gr->GetPoint(i,x1,y2);
      dy = 100.*(y2 - y1)/y1;
      GDerivative->SetPoint(GDerivative->GetN(),x1,dy);
    }
  } else {
    for (Int_t i = 1; i < npoints; i++){
      gr->GetPoint(i-1,x1,y1);
      gr->GetPoint(i,x1,y2);
      dy = y2 - y1;
      GDerivative->SetPoint(GDerivative->GetN(),x1,dy);
    }
  }
  
  GDerivative->SetFillColor(38);
  GDerivative->GetXaxis()->SetTimeDisplay(1);
  GDerivative->GetXaxis()->SetTimeFormat("%b/%d/%y");
  GDerivative->GetXaxis()->SetTimeOffset(0,"gmt");

  return GDerivative;

}

////////////////////////////////////////////////////////////////////////////////
/// Volume Graph

THStack *TStock::GetVolume(){

  if (!fTree) {
    Error("TStock::GetVolume()","No data available. Did you forget to call GetData()?");
    return 0;
  }
  
  THStack *HSVol = new THStack(Form("HSVol%s",fSymbol.Data()),"Volume");

  Int_t nbins = TMath::Nint((Double_t)(fEndDate.GetSec() - fStartDate.GetSec()) / GetTimeWidth(fFreq));
  
  TH1I *HVolG = new TH1I(Form("HVolG%s",fSymbol.Data()),"HVolG",
				nbins,fStartDate.GetSec(),fEndDate.GetSec());
  TH1I *HVolR = new TH1I(Form("HVolR%s",fSymbol.Data()),"HVolR",
				nbins,fStartDate.GetSec(),fEndDate.GetSec());

  fReader.Restart();
    
  while(fReader.Next()){

    if(*fO < *fC){
      HVolG->Fill(fTS->GetSec(),*fVol);
    } else {
      HVolR->Fill(fTS->GetSec(),*fVol);
    }

  }
  HVolG->SetFillColor(kGreen);
  HVolR->SetFillColor(kRed);
  HSVol->Add(HVolG,"HIST");
  HSVol->Add(HVolR,"HIST");
  HSVol->SetTitle(Form("%s Volume;Date;Volume",fSymbol.Data()));
  return HSVol;
  
}


////////////////////////////////////////////////////////////////////////////////
/// CandleStick

TMultiGraph *TStock::GetCandleStick(){

  if (!fTree) {
    Error("TStock::GetCandleStick()","No data available. Did you forget to call GetData()?");
    return 0;
  }

  TMultiGraph *GCandle = new TMultiGraph(Form("GCandle%s",fSymbol.Data()),"CandleStick");
  TGraphErrors *GOCG = new TGraphErrors(); //OpenCloseGreen
  TGraphErrors *GOCR = new TGraphErrors(); //OpenCloseRed
  TGraphAsymmErrors *GHL = new TGraphAsymmErrors(); //HighLow
  
  fReader.Restart();

  while(fReader.Next()){
    
    Double_t mdl = (*fO + *fC)/2.;
    Double_t l1 = TMath::Abs(*fO - mdl);
    
    Double_t twidth = GetTimeWidth(fFreq) * 0.33;

    Double_t tt = fTS->AsDouble();

    if (*fO < *fC) { 
      Int_t n = GOCG->GetN();
      GOCG->SetPoint(n,tt, mdl);
      GOCG->SetPointError(n,twidth,l1);
    } else {
      Int_t n = GOCR->GetN();
      GOCR->SetPoint(n,tt, mdl);
      GOCR->SetPointError(n,twidth,l1);
    }
    Int_t n = GHL->GetN();
    GHL->SetPoint(n,tt, mdl);
    Double_t l = mdl - *fL;
    Double_t h = *fH - mdl;
    GHL->SetPointError(n,0.,0.,l,h);
  }
  
  GOCR->SetFillColor(kRed);
  GOCG->SetFillColor(kGreen);
  GCandle->Add(GHL,"E");
  GCandle->Add(GOCG,"E2");
  GCandle->Add(GOCR,"E2");
  GCandle->SetTitle(Form("%s CandleStick;Date;Price",fSymbol.Data()));
  // Working from ROOT 6.13
  TH1F *h = GCandle->GetHistogram();
  if (h) {
    h->GetXaxis()->SetTimeDisplay(1);
    h->GetXaxis()->SetTimeFormat("%b/%d/%y");
    h->GetXaxis()->SetTimeOffset(0,"gmt");
  }
  
  return GCandle;

}
