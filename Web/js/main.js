var filename = "SMACrossover.root";

function createGUI() {
    
    var h = new JSROOT.HierarchyPainter("", "ExplorerDiv");
    
    h.SetDisplay("tabs", "DrawingDiv");
    
    h.OpenRootFile(filename, function() {
	h.display("GPS;1");
    });
    
}
