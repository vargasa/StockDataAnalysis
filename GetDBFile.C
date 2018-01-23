#include "TStock.h"

Int_t GetDBFile(TString listfile = "Symbols/NASDAQ.txt", TString Freq = "1wk", TString StartDate = "2000-01-01 00:00:00") {

  gErrorIgnoreLevel = 2001;

  TString filename = "SymbolsDB";
  std::ifstream infile(listfile.Data());

  std::string line;

  TFile *fOut = new TFile("Output/"+filename+".root","UPDATE");

  while (std::getline(infile, line)){

    TString Symbol = TString(line);
    TStock *Stock = new TStock(Symbol,Freq,StartDate);
    
    TTree *t1 = (TTree*)fOut->Get(Symbol+"_"+Freq+";1");
    if (!t1) {
      cout << Symbol+"\t";
      TTree *tree = Stock->GetData();
      if(tree){
	tree->SetName(Form("%s_%s",Symbol.Data(),Freq.Data()));
	tree->Write();
	cout << "[Ok]\n";
      } else {
	cout << "[Error]\n";
      }
    }
    
  }
  
  fOut->Close();

  return 0;

}
