#include "HyperBinningHistogram.h"
#include "HyperBinningPainter1D.h"
#include "HyperBinningPainter2D.h"


/**
The most basic constructor - just pass my the HyperVolumeBinning
*/
HyperBinningHistogram::HyperBinningHistogram(const HyperVolumeBinning& binning) :
  HistogramBase(binning.getNumBins()),
  _binning(binning)
{
  WELCOME_LOG << "Good day from the HyperBinningHistogram() Constructor"; 
}

/**
Constuctor that adaptively bins the HyperPointSet provided,
within the limits provided, and using the specified binning
algorithm. Additionally, binning options can be selected which 
are passed to the binning algorithm.

Binning Algorithms:

  - HyperBinningAlgorithms::SMART             (see HyperBinningMakerSmart for details on the algorithm          )
  - HyperBinningAlgorithms::MINT              (see HyperBinningMakerMint for details on the algorithm           )
  - HyperBinningAlgorithms::MINT_SMART        (see HyperBinningMakerMintSmart for details on the algorithm      )
  - HyperBinningAlgorithms::MINT_RANDOM       (see HyperBinningMakerMintRandomise for details on the algorithm  )
  - HyperBinningAlgorithms::SMART_RANDOM      (see HyperBinningMakerSmartRandomise for details on the algorithm )
  - HyperBinningAlgorithms::LIKELIHOOD        (see HyperBinningMakerLikelihood for details on the algorithm     )
  - HyperBinningAlgorithms::SMART_LIKELIHOOD  (see HyperBinningMakerSmartLikelihood for details on the algorithm)

Binning Algorithm Options:

  - AlgOption::StartDimension     (int dim                  )
  - AlgOption::BinningDimensions  (std::vector<int> dims    )
  - AlgOption::RandomSeed         (int seed                 )
  - AlgOption::MinBinWidth        (double width             )
  - AlgOption::MinBinWidth        (HyperPoint widths        ) 
  - AlgOption::MinBinContent      (double val               )
  - AlgOption::MinShadowBinContent(double val               )
  - AlgOption::UseWeights         (bool   val = true        )
  - AlgOption::UseShadowData      (const HyperPointSet& data)
  - AlgOption::Empty              (                         )


*/
HyperBinningHistogram::HyperBinningHistogram(
  const HyperCuboid&   binningRange, 
  const HyperPointSet& points, 
  HyperBinningAlgorithms::Alg alg, 
  AlgOption opt0 ,
  AlgOption opt1 ,  
  AlgOption opt2 ,
  AlgOption opt3 ,
  AlgOption opt4 ,
  AlgOption opt5 ,
  AlgOption opt6 ,
  AlgOption opt7 ,
  AlgOption opt8 ,
  AlgOption opt9 
) :
  HistogramBase(0),
  HyperFunction(binningRange),
  _binning( HyperVolumeBinning(binningRange.getDimension()) )
{

  HyperBinningAlgorithms algSetup(alg);
  algSetup.addAlgOption(opt0);
  algSetup.addAlgOption(opt1);
  algSetup.addAlgOption(opt2);
  algSetup.addAlgOption(opt3);
  algSetup.addAlgOption(opt4);
  algSetup.addAlgOption(opt5);
  algSetup.addAlgOption(opt6);
  algSetup.addAlgOption(opt7);
  algSetup.addAlgOption(opt8);
  algSetup.addAlgOption(opt9);

  HyperBinningMaker* binnningMaker = algSetup.getHyperBinningMaker(binningRange, points);
  binnningMaker->makeBinning();
 
  *this = *binnningMaker->getHyperBinningHistogram();

  delete binnningMaker;

}

/**
Load a HyperBinningHistogram from file
*/
HyperBinningHistogram::HyperBinningHistogram(TString filename, int dim) :
  HistogramBase(0),
  _binning(dim)
{
  WELCOME_LOG << "Good day from the HyperBinningHistogram() Constructor";
  load(filename);

  setFuncLimits( getLimits() );
}

/**
Private constructor
*/
HyperBinningHistogram::HyperBinningHistogram(int dim) :
  HistogramBase(0),
  _binning(dim)
{
  WELCOME_LOG << "Good day from the HyperBinningHistogram() Constructor";
}

/**
Fill the HyperBinningHistogram with a HyperPoint and aspecified weight
*/
int HyperBinningHistogram::fill(const HyperPoint& coords, double weight){

  int binNumber = _binning.getBinNum(coords);
  this->fillBase(binNumber, weight);
  return binNumber;
}

/**
Fill the HyperBinningHistogram with a HyperPoint. If the 
HyperPoint has a weight, use it.
*/
int HyperBinningHistogram::fill(const HyperPoint& coords){

  int binNumber = _binning.getBinNum(coords);
  this->fillBase(binNumber, coords.getWeight(0));
  return binNumber;
}

/**
Get the bin content where the given HyperPoint lies
*/
double HyperBinningHistogram::getVal(const HyperPoint& point) const{ 
  
  int binNumber = _binning.getBinNum(point);
  return this->getBinContent(binNumber);

}

/**
Add a HyperPointSet to the HyperBinningHistogram - if any of
the HyperPoints are weighted, they will be used.
*/
void HyperBinningHistogram::fill(const HyperPointSet& points){

  for(unsigned i = 0; i < points.size(); i++){
    fill(points.at(i), points.at(i).getWeight());
  }

}


/**
Get the limits of the histogram
*/
HyperCuboid HyperBinningHistogram::getLimits() const{
  return _binning.getLimits();
}


/**
Merge two HyperBinningHistograms
*/
void HyperBinningHistogram::merge( const HistogramBase& other ){
  
  const HyperBinningHistogram* histOther = dynamic_cast<const HyperBinningHistogram*>(&other);

  if (histOther == 0){
    ERROR_LOG << "The object passed to HyperBinningHistogram::merge is not of type ";
    ERROR_LOG << "HyperBinningHistogram, so cannot merge";
    return;    
  }

  _binning.mergeBinnings( histOther->_binning );
  HistogramBase::merge( other );

}



/**
Set the bin contents of the histogram using parsed function.
Will set bin errors to zero and use bin centers for evaluating
function
*/
void HyperBinningHistogram::setContentsFromFunc(const HyperFunction& func){
  
  int nbins = getNBins();
  
  for (int i = 0; i < nbins; i++){
    HyperPoint binCenter = _binning.getBinHyperVolume(i).getAverageCenter();
    double funcVal = func.getVal(binCenter);
    setBinContent(i, funcVal);
    setBinError  (i, 0  );
  }
  

}




/**
Draw the HyperBinningHistogram - the drawing class
used depends on the dimensionality of the data.
This just plots the raw bin contents, not the 
frequency density.
*/
void HyperBinningHistogram::draw(TString path){
  
  if (_binning.getDimension() == 1){
    HyperBinningPainter1D painter(this);
    painter.draw(path);  
  }
  else if (_binning.getDimension() == 2){

    HyperBinningPainter2D painter(this);
    painter.draw(path);
  } 
  else{
    HyperBinningPainter   painter(this);
    painter.draw(path);
  }

}

/**
Draw the frequency density of the HyperBinningHistogram 
- the drawing class used depends on the dimensionality of the data.
*/
void HyperBinningHistogram::drawDensity(TString path){
  
  if (_binning.getDimension() == 1){
    HyperBinningPainter1D painter(this);
    painter.useDensity(true);
    painter.draw(path);  
  }
  else if (_binning.getDimension() == 2){
    HyperBinningPainter2D painter(this);
    painter.useDensity(true);
    painter.draw(path);
  } 
  else{
    HyperBinningPainter   painter(this);
    painter.useDensity(true);
    painter.draw(path);
  }

}



/**
Print all info about the HyperBinningHistogram
*/
void HyperBinningHistogram::printFull() const{

  for(int i = 0; i < _binning.getNumBins(); i++){
    INFO_LOG << "Bin Content " << i << ": " << _binContents[i] << "      SumW2: " << _sumW2[i];
    _binning.getBinHyperVolume(i).getHyperCuboid(0).print();
  }

  INFO_LOG << "Overflow: " << _binContents[_nBins] << std::endl;

}


/**
 \todo remember how this works
*/
void HyperBinningHistogram::project(TH1D* histogram, const HyperCuboid& cuboid, double content, int dimension) const{

  double hyperLowEdge  = cuboid.getLowCorner() .at(dimension);
  double hyperHighEdge = cuboid.getHighCorner().at(dimension);
  double totWidth = hyperHighEdge - hyperLowEdge;
  int lowBin   = histogram->GetXaxis()->FindFixBin(hyperLowEdge); 
  int highBin  = histogram->GetXaxis()->FindFixBin(hyperHighEdge); 
  
  if (lowBin==highBin) histogram->Fill(hyperLowEdge, content);
  else{

    //first deal with the highest and lowest bin as there will be a fractional overlap with the HyperCuboid

    double widthInLowBin  = histogram->GetXaxis()->GetBinUpEdge(lowBin) - hyperLowEdge;
    double widthInHighBin = hyperHighEdge - histogram->GetXaxis()->GetBinLowEdge(highBin);
    double eventsInLowBin  = (widthInLowBin /totWidth)*content;
    double eventsInHighBin = (widthInHighBin/totWidth)*content;
    histogram->Fill(hyperLowEdge , eventsInLowBin);
    histogram->Fill(hyperHighEdge, eventsInHighBin);

    //now do the bins in the middle

    for(int bin = (lowBin + 1); bin <= (highBin - 1); bin++){
      double lowEdge  = histogram->GetXaxis()->GetBinLowEdge(bin);
      double highEdge = histogram->GetXaxis()->GetBinUpEdge (bin);
      double events   = ((highEdge - lowEdge)/totWidth)*content;
      histogram->Fill( histogram->GetXaxis()->GetBinCenter(bin) , events);
    }

  }

}

/**
 \todo remember how this works
*/
void HyperBinningHistogram::project(TH1D* histogram, const HyperVolume& hyperVolume, double content, int dimension) const{

  double volume = hyperVolume.volume();
  for(int i = 0; i < hyperVolume.size(); i++){
    const HyperCuboid& cuboid = hyperVolume.getHyperCuboid(i);
    double cuboidVolume = cuboid.volume();
    double cuboidContent = (content*cuboidVolume)/volume;
    project(histogram, cuboid, cuboidContent, dimension);
  }

}

/**
 \todo remember how this works
*/
HyperBinningHistogram HyperBinningHistogram::slice(std::vector<int> sliceDims, std::vector<double> sliceVals) const{
  
  //for (unsigned i = 0; i < sliceDims.size(); i++){
  //  std::cout << sliceDims.at(i) << "  " << sliceVals.at(i) << std::endl;
  //}

  int nStartingDims = _binning.getDimension();
  int nSliceDims    = sliceDims.size();
  int nEndDims      = nStartingDims - nSliceDims;

  HyperVolumeBinning temp(nEndDims);
  
  HyperPoint point(nStartingDims);
  for (int i = 0; i < nSliceDims; i++) point.at(sliceDims.at(i)) = sliceVals.at(i);

  std::vector<double> binContents;
  std::vector<double> binErrors  ;
  
  //std::cout << "Looping over bins" << std::endl;

  for (int i = 0; i < getNBins(); i++){
    
    //std::cout << "Trying to slice bin " << i << " of " << getNBins() << std::endl;

    HyperVolume vol       = getBinning().getBinHyperVolume(i);
    //std::cout << "  ----- Got original HyperVolume" << std::endl;
    //vol.print();

    HyperVolume slicedVol = vol.slice(point, sliceDims);
    //std::cout << "  ----- Got sliced HyperVolume" << std::endl;
    //slicedVol.print();

    if (slicedVol.size() == 0) continue;

    temp.addHyperVolume(slicedVol);
    
    binContents.push_back( getBinContent(i) );
    binErrors  .push_back( getBinError  (i) );
  }  
  
  //std::cout << "Now setting bin contents" << std::endl;

  HyperBinningHistogram slicedHist(temp);

  for (unsigned i = 0; i < binContents.size(); i++){
    slicedHist.setBinContent(i, binContents.at(i) );
    slicedHist.setBinError  (i, binErrors  .at(i) );
  }
  
  //std::cout << "Now dealing with the HyperNames" << std::endl;

  HyperName names(nEndDims);
  int count = 0;

  for (int i = 0; i < nStartingDims; i++){
    bool doesExist = false;
    for (int j = 0; j < nSliceDims; j++){
      int dim = sliceDims.at(j);
      if (i == dim) doesExist = true;
    }

    if (doesExist == false) { 
      //std::cout << "names.at(" << count << ") = _binning.getNames().at(" << i << ")" << std::endl;
      names.at(count) = _binning.getNames().at(i); count++; 
    }
  }  

  slicedHist.setNames(names);

  //std::cout << "Done" << std::endl;


  return slicedHist;

}

/**
 \todo remember how this works
*/
HyperBinningHistogram HyperBinningHistogram::slice(int dim, double val) const{

  std::vector<int> sliceDims;
  std::vector<double> sliceVals;
  sliceDims.push_back(dim);
  sliceVals.push_back(val);

  return slice(sliceDims, sliceVals);

}


void HyperBinningHistogram::draw2DSlice(TString path, int sliceDimX, int sliceDimY, const HyperPoint& slicePoint) const{
  
  //std::cout << "draw2DSlice(" << path << ", " << sliceDimX << ", " << sliceDimY << ", " << slicePoint << ")" << std::endl;

  std::vector<int   > _sliceDims;
  std::vector<double> _sliceVals;

  for (int i = 0; i < slicePoint.getDimension(); i++){
    if (i == sliceDimX) continue;
    if (i == sliceDimY) continue;
    _sliceDims.push_back( i                );
    _sliceVals.push_back( slicePoint.at(i) );
  }

  HyperBinningHistogram sliceHist = slice( _sliceDims, _sliceVals );
  sliceHist.draw(path);

}

void HyperBinningHistogram::draw2DSliceSet(TString path, int sliceDimX, int sliceDimY, int sliceSetDim, int nSlices, const HyperPoint& slicePoint) const{

  HyperPoint slicePointCp(slicePoint);

  double min = getBinning().getMin(sliceSetDim);
  double max = getBinning().getMax(sliceSetDim);
  double width = (max - min)/double(nSlices);
  
  for (int i = 0; i < nSlices; i++){
    double val = min + width*(i + 0.5);
    slicePointCp.at(sliceSetDim) = val;
    
    TString uniquePath = path;
    uniquePath += "_sliceNum";
    uniquePath +=  i;
    draw2DSlice(uniquePath, sliceDimX, sliceDimY, slicePointCp);

  }
  

}

void HyperBinningHistogram::draw2DSliceSet(TString path, int sliceDimX, int sliceDimY, int nSlices, const HyperPoint& slicePoint) const{
  

  
  for (int i = 0; i < slicePoint.getDimension(); i++){

    if (i == sliceDimX) continue;
    if (i == sliceDimY) continue;
    
    TString thsPath = path;
    thsPath += "_scanDim";
    thsPath += i;

    draw2DSliceSet(thsPath, sliceDimX, sliceDimY, i, nSlices, slicePoint);

  }
  

}

void HyperBinningHistogram::draw2DSliceSet(TString path, int nSlices, const HyperPoint& slicePoint) const{
  

  
  for (int i = 0; i < slicePoint.getDimension(); i++){
    for (int j = 0; j < slicePoint.getDimension(); j++){
      
      if (i >= j) continue;

      TString thsPath = path;
      thsPath += "_";
      thsPath += i;
      thsPath += "vs";
      thsPath += j;

      draw2DSliceSet(thsPath, i, j, nSlices, slicePoint);
    }
  }
  

}


/**
 \todo remember how this works
*/
TH1D HyperBinningHistogram::project(int dim, int bins, TString name) const{
  
  double lowEdge  = _binning.getMin(dim);
  double highEdge = _binning.getMax(dim);

  TH1D projection(name, name, bins, lowEdge, highEdge);
  projection.GetXaxis()->SetTitle(_binning.getNames().at(dim));

  for(int i = 0; i < _binning.getNumBins(); i++){
    project(&projection, _binning.getBinHyperVolume(i), this->getBinContent(i), dim);
  }
  
  for (int i = 1; i <= projection.GetNbinsX(); i++){
    projection.SetBinError(i, 0.0);
  }

  return projection;

}

/**
 \todo remember how this works
*/
void HyperBinningHistogram::drawProjection(TString path, int dim, int bins) const{
  
  TH1D projection = project(dim, bins);
  RootPlotter1D plotter(&projection, 300, 300);
  plotter.setMin(0.0);
  plotter.plot(path);

}

/**
 \todo remember how this works
*/
void HyperBinningHistogram::drawAllProjections(TString path, int bins) const{

  for(int i = 0; i < _binning.getDimension(); i++){
    TString thisPath = path + "_"; thisPath += i;
    drawProjection(thisPath, i, bins);
  }

}

/**
 \todo remember how this works
*/
void HyperBinningHistogram::compareProjection    (TString path, int dim, const HyperBinningHistogram& other, int bins) const{
  TH1D projection      = project(dim, bins);
  TH1D projectionOther = other.project(dim, bins, "projection2");
  RootPlotter1D plotter(&projection, 300, 300);
  plotter.add(&projectionOther);
  plotter.setMin(0.0);
  plotter.plotWithRatio(path);
}

/**
 \todo remember how this works
*/
void HyperBinningHistogram::compareAllProjections(TString path, const HyperBinningHistogram& other, int bins) const{
  for(int i = 0; i < _binning.getDimension(); i++){
    TString thisPath = path + "_"; thisPath += i;
    compareProjection(thisPath, i, other, bins);
  }  
}

/**
Save the HyperBinningHistogram to a TFile
*/
void HyperBinningHistogram::save(TString filename){

  TFile* file = new TFile(filename, "RECREATE");

  if (file == 0){
    ERROR_LOG << "Could not open TFile in HyperBinningHistogram::save(" << filename << ")";
    return;
  }

  //save the bin contents
  this->saveBase();
  //save the binning
  _binning.save();

  file->Write();
  file->Close();

}

/**
Load the HyperBinningHistogram from a TFile
*/
void HyperBinningHistogram::load(TString filename){

  //save the binning
  _binning.load(filename);
  //save the bin contents
  this->loadBase(filename);

}

/**
Get the volume of a HyperVolume bin
*/
double HyperBinningHistogram::getBinVolume(int bin) const{
  return _binning.getBinHyperVolume(bin).volume();
}

/**
Destructor
*/
HyperBinningHistogram::~HyperBinningHistogram(){
  GOODBYE_LOG << "Goodbye from the HyperBinningHistogram() Constructor"; 
}


/*
void HyperBinningHistogram::printOptimisationStatistics(){

  INFO_LOG << "With no optimisation we would perform " << _nIntegrationsWOtrick << " integrations";
  INFO_LOG << "In fact we only performed " << _nIntegrationsWtrick << " integrations";
  INFO_LOG << "This used approximately " << (_nIntegrationsWtrick/_nIntegrationsWOtrick)*100.0 << "% of time";

}

HyperPoint HyperBinningHistogram::findAdaptiveSigma(const HyperPoint& point, const HyperPoint& sigmas) const{
  
  int nBins = getBinning().getNumBins();
  int dim   = getBinning().getDimension();

  HyperPoint binWidthSum(dim, 0.0);
  double     weightSum = 0.0;
  
  HyperPointSet points = makePointsAtGaussianExtremes(point, sigmas, 3.0);
  std::vector<int> binNumbers = getBinning().getBinNumsContainingPoints(points);
  int nSelectedBins = binNumbers.size();
  
  _nIntegrationsWtrick  += nSelectedBins;
  _nIntegrationsWOtrick += nBins;


  for (int i = 0; i < nSelectedBins; i++){
    double integral = intgrateGaussianOverBin(point, sigmas, binNumbers.at(i));
    HyperPoint width(dim, 0.0);
    for (int j = 0; j < dim; j++) width.at(j) = getBinning().getBinHyperVolume(binNumbers.at(i)).getMax(j) - getBinning().getBinHyperVolume(binNumbers.at(i)).getMin(j);
    width = width*integral;
    binWidthSum = binWidthSum + width;
    weightSum   += integral;
  }  
  
  double dimensionalityScale = pow(1.4, dim) - 1.0; 

  //makes the ratio of weights between 1.0 and 2.0 sigma approximatly equal.
  //I hope this means the 'smoothing' is approximatly the same for any dimensonality.

  return binWidthSum/(weightSum*dimensionalityScale);

}

double HyperBinningHistogram::adaptiveGaussianKernal(const HyperPoint& point, double smoothing ) const{
  
  int dim = point.getDimension();
  
  double dimensionalityScale = pow(1.4, dim) - 1.0;
  HyperPoint initalSigma = getBinning().getAverageBinWidth()*(1.0/dimensionalityScale);
  
  //initalSigma.print();

  HyperPoint sigma = findAdaptiveSigma(point, initalSigma );
  sigma = sigma * smoothing;
  
  //sigma.print();

  return gaussianKernal(point, sigma );


}


double HyperBinningHistogram::intgrateGaussianOverHyperCuboid(const HyperPoint& mean, const HyperPoint& sigmas, const HyperCuboid& cuboid) const{
  
  double multiInt = 1.0;


  for (int i = 0; i < cuboid.getDimension(); i++){

    double i_lowEdge  = cuboid.getLowCorner ().at(i);
    double i_highEdge = cuboid.getHighCorner().at(i);
    double i_mean     = mean  .at(i);
    double i_sigma    = sigmas.at(i);

    double low  = (i_lowEdge  - i_mean)/i_sigma;
    double high = (i_highEdge - i_mean)/i_sigma;

    if (high < -3.0 || low > 3.0) return 0.0;

    double integralHigh  = TMath::Freq(high);
    double integralLow   = TMath::Freq(low );
    
    multiInt *= (integralHigh - integralLow);
    
  }

  return multiInt;

}

double HyperBinningHistogram::intgrateGaussianOverHyperVolume(const HyperPoint& point, const HyperPoint& sigmas, const HyperVolume& volume) const{
  
  double nCuboids = volume.getHyperCuboids().size();
  
  double multiInt = 0.0;

  for (int i = 0; i < nCuboids; i++){
    multiInt += intgrateGaussianOverHyperCuboid(point, sigmas, volume.getHyperCuboid(i));
  }

  return multiInt;

}

double HyperBinningHistogram::intgrateGaussianOverBin(const HyperPoint& point, const HyperPoint& sigmas, int bin) const{
  
  return intgrateGaussianOverHyperVolume( point, sigmas, getBinning().getBinHyperVolume(bin) );
}

HyperPointSet HyperBinningHistogram::makePointsAtGaussianExtremes(const HyperPoint& mean, const HyperPoint& widths, double numSigma) const{
  
  int dim =  mean.getDimension();
  HyperPointSet points(dim);

  for (int i = 0; i < dim; i++){

    double max = getBinning().getMax(i);
    double min = getBinning().getMin(i);
    
    double sigma = widths.at(i);

    HyperPoint low (mean);
    HyperPoint high(mean);
    low .at(i) -= sigma*numSigma;
    high.at(i) += sigma*numSigma;
    
    if (low .at(i) < min) low .at(i) = min + (max-min)*0.00001;
    if (high.at(i) > max) high.at(i) = max - (max-min)*0.00001;

    points.push_back(low );
    points.push_back(high);
  }
  
  return points;

}

double HyperBinningHistogram::gaussianKernal(const HyperPoint& point, const HyperPoint& sigmas) const{
  
  int nBins = getBinning().getNumBins();
  
  HyperPointSet points = makePointsAtGaussianExtremes(point, sigmas, 3.0);
  std::vector<int> binNumbers = getBinning().getBinNumsContainingPoints(points);
  int nSelectedBins = binNumbers.size();
  
  _nIntegrationsWtrick  += nSelectedBins;
  _nIntegrationsWOtrick += nBins;
  //std::cout << "Total of " << nBins << " nbins, but have selected only " << nSelectedBins << " for kernal." << std::endl; 

  double sumW = 0.0;
  double sum  = 0.0;

  for (int i = 0; i < nSelectedBins; i++){
    double integral = intgrateGaussianOverBin( point, sigmas, binNumbers.at(i) );
    double val      = getBinContent(binNumbers.at(i));
    sumW += integral;
    sum  += val*integral;
  }
  
  if (sumW == 0.0) {
    std::cout << "Sum of integrals is :" << sumW << std::endl;
    std::cout << "POINT: "; point.print();
    std::cout << "SIGMA: "; sigmas.print();
  }
  return sum/sumW;

}

std::vector<int> HyperBinningHistogram::findNHighestContributingKernalBins(const HyperPoint& point, const HyperPoint& sigmas, int n) const{
  
  int nBins = getBinning().getNumBins();

  double* integrals = new double [nBins];
  int   * index     = new int    [nBins];

  for (int i = 0; i < nBins; i++){
    double integral = intgrateGaussianOverBin(point, sigmas, i);
    integrals[i] = integral;
    index    [i] = i;
  }  

  TMath::Sort(nBins, integrals, index, true);

  std::vector<int> nearest;

  for (int i = 0; i < n; i++){
    nearest.push_back( index[i] );
  }

  delete integrals;
  delete index;

  return nearest;

}

void HyperBinningHistogram::reweightDatasetWithAdaptiveGaussianKernal(HyperPointSet& points, double smoothing) const{

  int npoints = points.size();

  for (int i = 0; i < npoints; i++){
    
    //SAM::printInterationStatus(i, npoints);

    HyperPoint& point = points.at(i);

    double val = this->adaptiveGaussianKernal(point, smoothing);

    int nW = point.numWeights();

    for (int w = 0; w < nW; w++){
      double oldW = point.getWeight(w);
      double newW = oldW*val;
      point.setWeight(w, newW);
    }

    if (nW == 0) point.addWeight(val);



  }

}

*/


