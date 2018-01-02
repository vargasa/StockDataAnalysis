#ifndef ROOT_TStock
#define ROOT_TStock

#include "TGraph.h"
#include "TGraphErrors.h"
#include "TMutligraph.h"
#include "TH1F.h"
#include "THStack.h"
#include "TFile.h"

Class TStock {

  RQ_OBJECT("TStock");

 private:
  TString fSymbol;
  TString fFreq;
  TTimeStamp fStartDate;
  TTimeStamp fEndDate;
  
  TFile *GetData();
  TFile *f;
  
  Double_t GetTimeWidth(TString);
  Int_t GetIndex GetIndex(Int_t Event, Int_t Interval);

 public:
  TStock(TString Symbol,TString Freq, TTimeStamp StartDate, TTimeStamp EndDate);
  ~TStock();
  TGraph *GetAroonDown(Int_t Interval);
  TGraph *GetAroonUp(Int_t Interval);
  TGraph *GetAroon(Int_t Interval);
  TGraph *GetSMA(Int_t Interval, Option_t *Option);
  TGraph *GetVWMA(Int_t Interval, Option_t *Option);
  TGraphErrors *GetBollingerBands(Int_t Interval, Float_t fW);
  TH1F *GetDerivative(TGraph *fg);
  THStack *GetVolume();
  TMultiGraph *GetCandleStick();

  ClassDef(TStock,0);
  
}

#endif
