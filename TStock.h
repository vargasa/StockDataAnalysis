#ifndef ROOT_TStock
#define ROOT_TStock

#include "TGraph.h"
#include "TGraphErrors.h"
#include "TGraphAsymmErrors.h"
#include "TApplication.h"
#include "TSystem.h"
#include "TMultiGraph.h"
#include "TH1F.h"
#include "THStack.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"
#include "TTreeReaderValue.h"
#include "TTreeReaderArray.h"
#include "TMath.h"
#include "TTimeStamp.h"

// using pointers of:
class TGraph;
class TMultiGraph;
class THStack;
class TH1F;
class TTree;

class TStock {

 protected:
  TString fSymbol;
  TString fFreq;
  TTimeStamp fStartDate;
  TTimeStamp fEndDate;
  TTree *fTree;

  TTreeReader fReader;
  TTreeReaderValue<TTimeStamp> fTS;
  TTreeReaderValue<Float_t> fO;
  TTreeReaderValue<Float_t> fL;
  TTreeReaderValue<Float_t> fH;
  TTreeReaderValue<Float_t> fC;
  TTreeReaderValue<Int_t> fVol;
 
  Double_t GetTimeWidth(TString Freq = "1wk") const;
  Int_t GetIndex(Int_t Event, Int_t Interval) const;

 public:
  TStock(TString Symbol = "", TString Freq = "1wk",
	 TString StartDate = "2009-01-01 00:00:00", TString EndDate = "now");
  ~TStock();
  TTree *GetData();
  TGraph *GetAroonDown(Int_t Interval = 25);
  TGraph *GetAroonUp(Int_t Interval = 25);
  TGraph *GetAroon(Int_t Interval = 25);
  TGraph *GetSMA(Int_t Interval = 6, Option_t *Option="close");
  TGraph *GetVWMA(Int_t Interval = 25, Option_t *Option="close");
  TGraphErrors *GetBollingerBands(Int_t Interval = 20, Float_t fW = 2.0);
  TGraph *GetDerivative(TGraph *fg);
  THStack *GetVolume();
  TMultiGraph *GetCandleStick();
  TString GetSymbol() const {return fSymbol;}
  TString GetFreq() const {return fFreq;}
  TTimeStamp GetStartDate() const {return fStartDate;}
  TTimeStamp GetEndDate() const {return fEndDate;}
  
  ClassDef(TStock,0);
  
};


#if defined(__CLING__)
#include "TStock.cxx"
#endif

#endif
