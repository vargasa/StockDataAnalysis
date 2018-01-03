ClassImp(WGraphsIP);


TStock::TStock(TString Symbol,TString Freq, TTimeStamp StartDate, TTimeStamp EndDate){

  fSymbol = Symbol;
  fFreq = Freq;
  fStartDate = StartDate;
  fEndDate = fEndDate;
  
}


Double_t TStock::GetTimeWidth(TString Freq = "1wk"){

  // Definition of the candlestick width
  // Period of time in seconds
  Freq.ToLower();
  Double_t twidth;
  if (Freq.Contains("1d")) {
    twidth = 86400.;
  } else if(Freq.Contains("1wk")) {
    twidth = 604800.;
  } else if(Freq.Contains("1mo")) {
    twidth = 2.628e6;
  }
  
  return twidth;

}

Int_t TStock::GetIndex(Int_t Event, Int_t Interval){

  Int_t Index = 0;
  Int_t Aux = (Event%Interval - 1);

  if( Aux >= 0 ) {
    Index = Aux;
  } else {
    Index = Interval-1;
  }
  return Index;
  
}

TTree* TStock::GetData(){
    
}

TGraph* TStock::GetAroonDown(Int_t Interval){

}

TGraph* TStock::GetAroonUp(Int_t Interval){

}

TGraph* TStock::GetAroon(Int_t Interval){

}

TGraph* TStock::GetSMA(Int_t Interval, Option_t *Option){

}

TGraph* TStock::GetVWMA(Int_t Interval, Option_t *Option){

}

TGraphErrors* TStock::GetBollingerBands(Int_t Interval, Float_t fW){

}

TH1F* TStock::GetDerivative(TGraph *fg){

}

THStack* TStock::GetVolume(){

}

TMultiGraph* TStock::GetCandleStick(){

}
