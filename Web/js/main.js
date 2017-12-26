var filename = "SMACrossover.root";
var volpainter = null;

JSROOT.OpenFile(filename, function(file) {

    file.ReadObject("AEO_VOL;1", function(obj) {
	JSROOT.draw("DVolArea", obj, "");
    });
    
    file.ReadObject("AEO_PRICE;1", function(obj) {
	JSROOT.draw("DPriceArea", obj);
    });
    
});
