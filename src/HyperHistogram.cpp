#include "HyperHistogram.h"
#include "HyperBinningPainter1D.h"
#include "HyperBinningPainter2D.h"


/**
The most basic constructor - just pass anything that is derrived from BinningBase
*/
HyperHistogram::HyperHistogram(const BinningBase& binning) :
  HistogramBase(binning.getNumBins()),
  _binning(binning.clone())
{
  WELCOME_LOG << "Good day from the HyperHistogram() Constructor"; 
  setFuncLimits( getLimits() );
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
HyperHistogram::HyperHistogram(
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
  _binning(0)
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
  
  HyperHistogram* hist = binnningMaker->getHyperBinningHistogram(); 
  
  *this = *hist;
  delete hist;
  delete binnningMaker;

  //This inherets from a HyperFunction. Although non-essential, it's useful for
  //the function to have some limits for it's domain.
  setFuncLimits( getLimits() );

}

HyperHistogram& HyperHistogram::operator=(const HyperHistogram& other){

  HistogramBase::operator=(other);
  HyperFunction::operator=(other);

  _binning = other._binning->clone();
  return *this;

}


/**
  Load a HyperHistogram from filename. If
*/
HyperHistogram::HyperHistogram(TString filename, TString option) :
  HistogramBase(0),
  _binning(0)
{
  WELCOME_LOG << "Good day from the HyperHistogram() Constructor";

  if (option.Contains("Empty")){
    loadEmpty(filename, option);
  }
  else{
    load(filename, option);
  }

  //This inherets from a HyperFunction. Although non-essential, it's useful for
  //the function to have some limits for it's domain.
  setFuncLimits( getLimits() );
}

/**
Load an array of HyperHistograms from different files and merge them into a 
memory resident HyperBinning
*/
HyperHistogram::HyperHistogram(std::vector<TString> filename) :
  HistogramBase(0),
  _binning(0)
{
  WELCOME_LOG << "Good day from the HyperHistogram() Constructor";
  
  int nFiles = filename.size();
  if (nFiles == 0){
    ERROR_LOG << "The list of filenames you provided to HyperHistogram is empty" << std::endl;
  }

  INFO_LOG << "Loading HyperHistogram at: " << filename.at(0) << std::endl;
  load(filename.at(0), "MEMRES");

  for (int i = 1; i < nFiles; i++){
    INFO_LOG << "Loading and merging HyperHistogram at: " << filename.at(i) << std::endl;
    merge(filename.at(i));
  }
  
  //This inherets from a HyperFunction. Although non-essential, it's useful for
  //the function to have some limits for it's domain.
  setFuncLimits( getLimits() );

}

/**
Load an array of HyperHistograms from different files and merge them into a 
disk resident HyperBinning stored at 'targetFilename'
*/
HyperHistogram::HyperHistogram(TString targetFilename, std::vector<TString> filename) :
  HistogramBase(0),
  _binning(0)
{

  WELCOME_LOG << "Good day from the HyperHistogram() Constructor";
  
  int nFiles = filename.size();
  if (nFiles == 0){
    ERROR_LOG << "The list of filenames you provided to HyperHistogram is empty" << std::endl;
  }

  //Getting the binning type from the first file
  TString binningType = getBinningType( filename.at(0) );

  INFO_LOG << "Creating HyperHistogram at: " << targetFilename << " with binning type " << binningType <<std::endl;
  loadEmpty(targetFilename, "DISKRES", binningType );

  int nResBins = estimateCapacity(filename, binningType);
  INFO_LOG << "I estimate there will be " <<  nResBins << " in total - resizing the Histogram accordingly" << std::endl;

  for (int i = 0; i < nFiles; i++){
    INFO_LOG << "Loading and merging HyperHistogram at: " << filename.at(i) << std::endl;
    merge(filename.at(i));
  }
  
  //This inherets from a HyperFunction. Although non-essential, it's useful for
  //the function to have some limits for it's domain.
  setFuncLimits( getLimits() );  
}


int HyperHistogram::estimateCapacity(std::vector<TString> filename, TString binningType){

  int nbins = 0;
  int nvols = 0;

  for (unsigned i = 0; i < filename.size(); i++){
    TFile* file = new TFile(filename.at(i), "READ");
    if (file == 0){
      ERROR_LOG << "HyperHistogram::estimateCapacity - " << filename.at(i) << " does not exist" << std::endl; 
      return 0;
    }
    TTree* hist = dynamic_cast<TTree*>( file->Get(binningType) );
    TTree* base = dynamic_cast<TTree*>( file->Get("HistogramBase") );
    if (hist == 0){
      ERROR_LOG << "HyperHistogram::estimateCapacity - " << filename.at(i) << " does not contain tree " << binningType << std::endl; 
      return 0;
    }
    if (base == 0){
      ERROR_LOG << "HyperHistogram::estimateCapacity - " << filename.at(i) << " does not contain tree HistogramBase" << std::endl; 
      return 0;
    }  
    nbins += base->GetEntries();
    nvols += hist->GetEntries();
    file->Close();
  }
  
  reserveCapacity(nbins);
  _binning->reserveCapacity(nvols);

  return nbins;

}


HyperHistogram::HyperHistogram(const HyperHistogram& other) :
  HistogramBase(other),
  HyperFunction(other),
  _binning( other._binning->clone() )
{

}


/**
Private constructor
*/
HyperHistogram::HyperHistogram() :
  HistogramBase(0),
  _binning(0)
{
  WELCOME_LOG << "Good day from the HyperHistogram() Constructor";
}


void HyperHistogram::setNames( HyperName names ){ 
  _binning->setNames(names); 
}
/**< Set the HyperName (mainly used for axis labels)*/

HyperName HyperHistogram::getNames() const {
  return _binning->getNames();
}      
/**< Get the HyperName (mainly used for axis labels)*/




/**
Fill the HyperHistogram with a HyperPoint and aspecified weight
*/
int HyperHistogram::fill(const HyperPoint& coords, double weight){

  int binNumber = _binning->getBinNum(coords);
  this->fillBase(binNumber, weight);
  return binNumber;
}

/**
Fill the HyperHistogram with a HyperPoint. If the 
HyperPoint has a weight, use it.
*/
int HyperHistogram::fill(const HyperPoint& coords){

  int binNumber = _binning->getBinNum(coords);
  this->fillBase(binNumber, coords.getWeight(0));
  return binNumber;
}

/**
Get the bin content where the given HyperPoint lies
*/
double HyperHistogram::getVal(const HyperPoint& point) const{ 
  
  int binNumber = _binning->getBinNum(point);
  return this->getBinContent(binNumber);

}

/**
Get the bin content for each HyperPoint in a HyperPointSet
*/
std::vector<double> HyperHistogram::getVal(const HyperPointSet& points) const{ 
  
  int nPoints = points.size();

  std::vector<int> binNums  = _binning->getBinNum(points);

  std::vector<double> conts(nPoints);
  
  for (int i = 0; i < nPoints; i++){
    conts.at(i) = this->getBinContent( binNums.at(i) );
  }

  return conts;

}



/**
Add a HyperPointSet to the HyperHistogram - if any of
the HyperPoints are weighted, they will be used.
*/
void HyperHistogram::fill(const HyperPointSet& points){

  for(unsigned i = 0; i < points.size(); i++){
    fill(points.at(i), points.at(i).getWeight());
  }

}


/**
Get the limits of the histogram
*/
HyperCuboid HyperHistogram::getLimits() const{
  return _binning->getLimits();
}


/**
Merge two HyperHistograms
*/
void HyperHistogram::merge( const HistogramBase& other ){
  
  //INFO_LOG << "Starting HyperHistogram::merge" <<std::endl;

  const HyperHistogram* histOther = dynamic_cast<const HyperHistogram*>(&other);

  if (histOther == 0){
    ERROR_LOG << "The object passed to HyperHistogram::merge is not of type ";
    ERROR_LOG << "HyperHistogram, so cannot merge";
    return;    
  }

  _binning->mergeBinnings( histOther->getBinning() );
  HistogramBase::merge( other );
  
  //I do this so that getLimits doesn't have to be 
  //called (which will update the cache). Will speed
  //things up I hope

  HyperCuboid a = getFuncLimits();
  HyperCuboid b = histOther->getFuncLimits();

  HyperVolume vol( _binning->getDimension() );
  if (a.getDimension() != 0) vol.push_back(a);
  if (b.getDimension() != 0) vol.push_back(b);

  setFuncLimits( vol.getLimits() );

  //INFO_LOG << "Ending HyperHistogram::merge" <<std::endl;


}

/**
Merge this histogram with another in a file
*/
void HyperHistogram::merge( TString filenameother ){

  HyperHistogram other(filenameother, "DISK");
  merge( other );

}


/**
Set the bin contents of the histogram using parsed function.
Will set bin errors to zero and use bin centers for evaluating
function
*/
void HyperHistogram::setContentsFromFunc(const HyperFunction& func){
  
  int nbins = getNBins();
  
  for (int i = 0; i < nbins; i++){
    HyperPoint binCenter = _binning->getBinHyperVolume(i).getAverageCenter();
    double funcVal = func.getVal(binCenter);
    setBinContent(i, funcVal);
    setBinError  (i, 0  );
  }
  

}




/**
*/
void HyperHistogram::mergeBinsWithSameContent(){
  

  if ( _binning->getBinningType() != "HyperBinning" ){
    ERROR_LOG << "It is only possible to merge bins when using HyperBinning. Doing nothing." << std::endl;
    return;
  }
  
  

  const HyperBinning& hyperBinning = dynamic_cast<const HyperBinning&>( getBinning() );

  std::map<int, bool> volumeKept;
  for (int i = 0; i < hyperBinning.getNumHyperVolumes(); i++){  
    volumeKept[i] = true;
  }
  
  //Loop over all HyperVolumes and see if there are any linked bins.
  //If there are, see if these linked bins are actually bins, and not
  //just part of the binning hierarchy. If they are actually bins,
  //See if they al have the same bin content. If they do, mark them 
  //to be removed. 

  for (int i = 0; i < hyperBinning.getNumHyperVolumes(); i++){

    std::vector<int> linkedVols = hyperBinning.getLinkedHyperVolumes(i);
    if (linkedVols.size() == 0) continue;
    
    bool linksLeadToBins = true;

    for (unsigned j = 0; j < linkedVols.size(); j++){
      int volNum = linkedVols.at(j);
      if (hyperBinning.getLinkedHyperVolumes(volNum).size() != 0){
        linksLeadToBins = false;
        break;
      } 
    }
    
    if (linksLeadToBins == false) continue;
    
    double binCont0 = -99999.9;
    
    bool binsHaveSameContent = true;

    for (unsigned j = 0; j < linkedVols.size(); j++){
      int volNum = linkedVols.at(j);
      int binNum = hyperBinning.getBinNum(volNum);
      double binContent = getBinContent(binNum);
      if (j == 0) binCont0 = binContent;

      if (binContent != binCont0){
        binsHaveSameContent = false;
        break;
      }

    } 

    if (binsHaveSameContent == false) continue;
    
    for (unsigned j = 0; j < linkedVols.size(); j++){
      int volNum = linkedVols.at(j);      
      volumeKept[volNum] = false;
    } 

 
  }
  
  //Make a map of the old volume numbers to the new ones (once the removed bins have
  //actually been removed).

  std::map<int, int> oldToNewVolumeNum;
  int newVolNum = 0;  

  for (int i = 0; i < hyperBinning.getNumHyperVolumes(); i++){
    bool exists = volumeKept[i];

    if (exists == true){
      oldToNewVolumeNum[i] = newVolNum;
      newVolNum++;
    }
    else{
      oldToNewVolumeNum[i] = -1;
    }

  }
  
  //Create a new HyperVolumeBinning with the bins removed
  HyperBinning* binningNew;
  

  if (hyperBinning.isDiskResident() == true){
    binningNew = new HyperBinningDiskRes();
    binningNew->load( hyperBinning.filename().ReplaceAll(".root", "_temp.root"), "RECREATE" );
  }
  else{
    binningNew = new HyperBinningMemRes();
  }

  INFO_LOG << "Created a new HyperBinning for the reduced binning" << std::endl;
  
  int count = 0;

  for (int i = 0; i < hyperBinning.getNumHyperVolumes(); i++){
    HyperVolume vol = hyperBinning.getHyperVolume(i);
    
    int newVolNum = oldToNewVolumeNum[i];

    if (newVolNum == -1) continue;
    
    if (newVolNum != count){
      ERROR_LOG << "Something has gone wrong in mergeBinsWithSameContent()" << std::endl;
    }

    
    std::vector<int> linkedVols = hyperBinning.getLinkedHyperVolumes( i );
    std::vector<int> newLinkedVols;

    int nLinked = 0;
    for (unsigned j = 0; j < linkedVols.size(); j++){
      int linkVolNum    = linkedVols.at(j);
      int newLinkVolNum = oldToNewVolumeNum[linkVolNum];

      if (newLinkVolNum != -1){
        newLinkedVols.push_back(newLinkVolNum);
        nLinked++;
      }


    }
    
    binningNew->addHyperVolume(vol, newLinkedVols);


    if (nLinked == 1){
      INFO_LOG << "This should never be one" << std::endl;
    }

    count++;
  }
  
  

  std::vector<int> primVolNums = hyperBinning.getPrimaryVolumeNumbers();
  
  for (unsigned i = 0; i < primVolNums.size(); i++){
    int oldVolNum = primVolNums.at(i);
    int newVolNum = oldToNewVolumeNum[oldVolNum];
    binningNew->addPrimaryVolumeNumber(newVolNum);
  }

  INFO_LOG << "Filled the new binning with reduced bins" << std::endl;


  HyperHistogram newHist(*binningNew);

  INFO_LOG << "Made a new hitogram which will clone the binning" << std::endl;

  newHist.setContentsFromFunc(*this);

  INFO_LOG << "You have managed to remove " << hyperBinning.getNumBins() - binningNew->getNumBins() << " bins with the same content" << std::endl;
  
  if ( hyperBinning.getNumBins() - binningNew->getNumBins() > 0 ) newHist.mergeBinsWithSameContent();
  
  if (hyperBinning.isDiskResident() == true){
    TString filenm = hyperBinning.filename();
    delete _binning;
    _binning = 0;
    newHist.save( filenm );

    load(filenm, "DISK");
  }
  else{
    *this = newHist;
  }  
  


  delete binningNew;


}

/**
Draw the HyperHistogram - the drawing class
used depends on the dimensionality of the data.
This just plots the raw bin contents, not the 
frequency density.
*/
void HyperHistogram::draw(TString path){
  
  if (_binning->getDimension() == 1){
    HyperBinningPainter1D painter(this);
    painter.draw(path);  
  }
  else if (_binning->getDimension() == 2){

    HyperBinningPainter2D painter(this);
    painter.draw(path);
  } 
  else{
    HyperBinningPainter   painter(this);
    painter.draw(path);
  }

}

/**
Draw the frequency density of the HyperHistogram 
- the drawing class used depends on the dimensionality of the data.
*/
void HyperHistogram::drawDensity(TString path){
  
  if (_binning->getDimension() == 1){
    HyperBinningPainter1D painter(this);
    painter.useDensity(true);
    painter.draw(path);  
  }
  else if (_binning->getDimension() == 2){
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
Print all info about the HyperHistogram
*/
void HyperHistogram::printFull() const{

  for(int i = 0; i < _binning->getNumBins(); i++){
    INFO_LOG << "Bin Content " << i << ": " << _binContents[i] << "      SumW2: " << _sumW2[i];
    _binning->getBinHyperVolume(i).getHyperCuboid(0).print();
  }

  INFO_LOG << "Overflow: " << _binContents[_nBins] << std::endl;

}


/**
 \todo remember how this works
*/
void HyperHistogram::project(TH1D* histogram, const HyperCuboid& cuboid, double content, int dimension) const{

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
void HyperHistogram::project(TH1D* histogram, const HyperVolume& hyperVolume, double content, int dimension) const{

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
HyperHistogram HyperHistogram::slice(std::vector<int> sliceDims, std::vector<double> sliceVals) const{
  
  int nStartingDims = _binning->getDimension();
  int nSliceDims    = sliceDims.size();
  int nEndDims      = nStartingDims - nSliceDims;

  HyperBinningMemRes temp;
  
  HyperPoint point(nStartingDims);
  for (int i = 0; i < nSliceDims; i++) point.at(sliceDims.at(i)) = sliceVals.at(i);

  std::vector<double> binContents;
  std::vector<double> binErrors  ;
  
  for (int i = 0; i < getNBins(); i++){
    

    HyperVolume vol       = _binning->getBinHyperVolume(i);

    HyperVolume slicedVol = vol.slice(point, sliceDims);

    if (slicedVol.size() == 0) continue;

    temp.addHyperVolume(slicedVol);
    
    binContents.push_back( getBinContent(i) );
    binErrors  .push_back( getBinError  (i) );
  }  
  
  //std::cout << "Now setting bin contents" << std::endl;

  HyperHistogram slicedHist(temp);

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
      //std::cout << "names.at(" << count << ") = _binning->getNames().at(" << i << ")" << std::endl;
      names.at(count) = _binning->getNames().at(i); count++; 
    }
  }  

  slicedHist.setNames(names);
  
  slicedHist.setMin( getMin() );
  slicedHist.setMax( getMax() );

  //std::cout << "Done" << std::endl;


  return slicedHist;

}

/**
 \todo remember how this works
*/
HyperHistogram HyperHistogram::slice(int dim, double val) const{

  std::vector<int>    sliceDims;
  std::vector<double> sliceVals;
  sliceDims.push_back(dim);
  sliceVals.push_back(val);

  return slice(sliceDims, sliceVals);

}

int HyperHistogram::getDimension() const{

  if (_binning == 0){
    ERROR_LOG << "HyperHistogram::getDimension - cannot get dimension, binning not set." << std::endl;
    return 0;
  }
  return _binning->getDimension();

}


void HyperHistogram::draw2DSlice(TString path, int sliceDimX, int sliceDimY, const HyperPoint& slicePoint) const{
  
  //std::cout << "draw2DSlice(" << path << ", " << sliceDimX << ", " << sliceDimY << ", " << slicePoint << ")" << std::endl;

  std::vector<int   > _sliceDims;
  std::vector<double> _sliceVals;

  for (int i = 0; i < slicePoint.getDimension(); i++){
    if (i == sliceDimX) continue;
    if (i == sliceDimY) continue;
    _sliceDims.push_back( i                );
    _sliceVals.push_back( slicePoint.at(i) );
  }

  HyperHistogram sliceHist = slice( _sliceDims, _sliceVals );
  sliceHist.draw(path);

}

void HyperHistogram::drawRandom2DSlice(TString path, TRandom* random) const{

  int dim = getDimension();

  if (dim < 3){
    ERROR_LOG << "Why would you take a 2D slice of something with less than 3 dim." << std::endl;
    return;
  }

  int slicedimx = random->Integer(dim);
  int slicedimy = random->Integer(dim);
  while( slicedimx == slicedimy ){
    slicedimy = random->Integer(dim);
  }

  HyperPoint slicepoint = getLimits().getRandomPoint(random);

  draw2DSlice(path, slicedimx, slicedimy, slicepoint);

}

void HyperHistogram::draw2DSliceSet(TString path, int sliceDimX, int sliceDimY, int sliceSetDim, int nSlices, const HyperPoint& slicePoint) const{

  HyperPoint slicePointCp(slicePoint);

  double min = _binning->getMin(sliceSetDim);
  double max = _binning->getMax(sliceSetDim);
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

void HyperHistogram::draw2DSliceSet(TString path, int sliceDimX, int sliceDimY, int nSlices, const HyperPoint& slicePoint) const{
  

  
  for (int i = 0; i < slicePoint.getDimension(); i++){

    if (i == sliceDimX) continue;
    if (i == sliceDimY) continue;
    
    TString thsPath = path;
    thsPath += "_scanDim";
    thsPath += i;

    draw2DSliceSet(thsPath, sliceDimX, sliceDimY, i, nSlices, slicePoint);

  }
  

}

void HyperHistogram::draw2DSliceSet(TString path, int nSlices, const HyperPoint& slicePoint) const{
  

  
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
TH1D HyperHistogram::project(int dim, int bins, TString name) const{
  
  double lowEdge  = _binning->getMin(dim);
  double highEdge = _binning->getMax(dim);

  TH1D projection(name, name, bins, lowEdge, highEdge);
  projection.GetXaxis()->SetTitle(_binning->getNames().at(dim));

  for(int i = 0; i < _binning->getNumBins(); i++){
    project(&projection, _binning->getBinHyperVolume(i), this->getBinContent(i), dim);
  }
  
  for (int i = 1; i <= projection.GetNbinsX(); i++){
    projection.SetBinError(i, 0.0);
  }

  return projection;

}

/**
 \todo remember how this works
*/
void HyperHistogram::drawProjection(TString path, int dim, int bins) const{
  
  TH1D projection = project(dim, bins);
  RootPlotter1D plotter(&projection, 300, 300);
  plotter.setMin(0.0);
  plotter.plot(path);

}

/**
 \todo remember how this works
*/
void HyperHistogram::drawAllProjections(TString path, int bins) const{

  for(int i = 0; i < _binning->getDimension(); i++){
    TString thisPath = path + "_"; thisPath += i;
    drawProjection(thisPath, i, bins);
  }

}

/**
 \todo remember how this works
*/
void HyperHistogram::compareProjection    (TString path, int dim, const HyperHistogram& other, int bins) const{
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
void HyperHistogram::compareAllProjections(TString path, const HyperHistogram& other, int bins) const{
  for(int i = 0; i < _binning->getDimension(); i++){
    TString thisPath = path + "_"; thisPath += i;
    compareProjection(thisPath, i, other, bins);
  }  
}

/**
Save the HyperHistogram to a TFile
*/
void HyperHistogram::save(TString filename){

  TFile* file = new TFile(filename, "RECREATE");

  if (file == 0){
    ERROR_LOG << "Could not open TFile in HyperHistogram::save(" << filename << ")";
    return;
  }

  //save the bin contents
  this->saveBase();
  //save the binning
  _binning->save();

  file->Write();
  file->Close();

}

/**
Save the HyperHistogram to a .txt file
*/
void HyperHistogram::saveToTxtFile(TString filename) const{
  
  if ( _binning->getBinningType() != "HyperBinning" ){
    ERROR_LOG << "It is only possible to saveToTxtFile when using HyperBinning. Doing nothing." << std::endl;
    return;
  }
  
  const HyperBinning& hyperBinning = dynamic_cast<const HyperBinning&>( getBinning() );

  int nVolumes = hyperBinning.getNumHyperVolumes();
  
  std::ofstream myfile;
  myfile.open (filename);

  for (int i = 0; i < nVolumes; i++ ){
    HyperVolume vol  = hyperBinning.getHyperVolume(i);
    HyperCuboid cube = vol.getHyperCuboid(0);
    int binNumber = hyperBinning.getBinNum(i); 
    double content = -1.0;
    double error   = -1.0;

    bool isPrimary = hyperBinning.isPrimaryVolume(i);
    bool isBin     = false;
    
    std::vector<int> linkedBins = hyperBinning.getLinkedHyperVolumes(i);

    if (binNumber != -1){
      content = getBinContent(binNumber);
      error   = getBinError  (binNumber);
      isBin   = true;
    }
    
    myfile << std::setw(4) << std::left;
    if (isPrimary) myfile << "P"; 
    if (isBin    ) myfile << "B"; 
    if (!isBin   ) myfile << "V";
    
    int width = vol.getDimension()*10 + 10;

    myfile <<  std::setw(width) << std::left << cube.getLowCorner() << std::setw(width) << std::left << cube.getHighCorner();
    
    if (isBin){
      myfile << std::setw(10) << std::left <<  content << std::setw(10) << std::left <<  error;
    }

    if (!isBin){
      for (unsigned j = 0; j < linkedBins.size(); j++){
        myfile << std::setw(10) << std::left <<  linkedBins.at(j);
      }
    }
    
    myfile << std::endl;
  }

  myfile.close();


}

/**
Get binning type from file
*/
TString HyperHistogram::getBinningType(TString filename){

  TFile* file = new TFile(filename, "READ");

  if (file == 0){
    ERROR_LOG << "Could not open TFile in HyperBinning::load(" << filename << ")";
    return "";
  }

  TTree* tree  = (TTree*)file->Get("HyperBinning");

  if (tree != 0){
    file->Close();
    return "HyperBinning";
  }

  file->Close();
  return "";

}

/**
Load the HyperHistogram from a TFile
*/
void HyperHistogram::load(TString filename, TString option){

  //If loading from a file, we first need to figure out what 
  //type of binning is saved in that file. 
  
  TString binningType = getBinningType(filename);
 
  if (binningType.Contains("HyperBinning")){
    
    if (_binning != 0) {
      delete _binning;
      _binning = 0;
    }

    if (option.Contains("DISK")) _binning = new HyperBinningDiskRes();
    else{
      _binning = new HyperBinningMemRes();
    }

  }

  if (binningType == ""){
    ERROR_LOG << "HyperHistogram::load - I could not find any binning scheme in this file" << std::endl;
  }

  _binning->load(filename, "READ");
  this->loadBase(filename);

}

void HyperHistogram::loadEmpty(TString filename, TString option, TString binningType){

  if (binningType.Contains("HyperBinning")){

    if (option.Contains("DISK")) _binning = new HyperBinningDiskRes();
    else{
      _binning = new HyperBinningMemRes();
    }

  }
  if (binningType == ""){
    ERROR_LOG << "HyperHistogram::loadEmpty - I could not find any binning scheme in this file" << std::endl;
  }  

  _binning->load(filename, "RECREATE");
  this->resetBinContents(0);

}
/**
Get the volume of a HyperVolume bin
*/
double HyperHistogram::getBinVolume(int bin) const{
  return _binning->getBinHyperVolume(bin).volume();
}

/**
Destructor
*/
HyperHistogram::~HyperHistogram(){

  bool diskResidentBinning = 0;
  TString filename = "";

  if (_binning != 0){
    diskResidentBinning = _binning->isDiskResident();
    filename            = _binning->filename();
    delete _binning;
    _binning = 0;
  }
  
  if (diskResidentBinning){

    TFile *file = new TFile(filename, "update");
    std::string object_to_remove="HistogramBase";
    gDirectory->Delete(object_to_remove.c_str());
    
    saveBase();

    file->Close();
  }

  GOODBYE_LOG << "Goodbye from the HyperHistogram() Constructor"; 
}


/*
void HyperHistogram::printOptimisationStatistics(){

  INFO_LOG << "With no optimisation we would perform " << _nIntegrationsWOtrick << " integrations";
  INFO_LOG << "In fact we only performed " << _nIntegrationsWtrick << " integrations";
  INFO_LOG << "This used approximately " << (_nIntegrationsWtrick/_nIntegrationsWOtrick)*100.0 << "% of time";

}

HyperPoint HyperHistogram::findAdaptiveSigma(const HyperPoint& point, const HyperPoint& sigmas) const{
  
  int nBins = _binning->getNumBins();
  int dim   = _binning->getDimension();

  HyperPoint binWidthSum(dim, 0.0);
  double     weightSum = 0.0;
  
  HyperPointSet points = makePointsAtGaussianExtremes(point, sigmas, 3.0);
  std::vector<int> binNumbers = _binning->getBinNumsContainingPoints(points);
  int nSelectedBins = binNumbers.size();
  
  _nIntegrationsWtrick  += nSelectedBins;
  _nIntegrationsWOtrick += nBins;


  for (int i = 0; i < nSelectedBins; i++){
    double integral = intgrateGaussianOverBin(point, sigmas, binNumbers.at(i));
    HyperPoint width(dim, 0.0);
    for (int j = 0; j < dim; j++) width.at(j) = _binning->getBinHyperVolume(binNumbers.at(i)).getMax(j) - _binning->getBinHyperVolume(binNumbers.at(i)).getMin(j);
    width = width*integral;
    binWidthSum = binWidthSum + width;
    weightSum   += integral;
  }  
  
  double dimensionalityScale = pow(1.4, dim) - 1.0; 

  //makes the ratio of weights between 1.0 and 2.0 sigma approximatly equal.
  //I hope this means the 'smoothing' is approximatly the same for any dimensonality.

  return binWidthSum/(weightSum*dimensionalityScale);

}

double HyperHistogram::adaptiveGaussianKernal(const HyperPoint& point, double smoothing ) const{
  
  int dim = point.getDimension();
  
  double dimensionalityScale = pow(1.4, dim) - 1.0;
  HyperPoint initalSigma = _binning->getAverageBinWidth()*(1.0/dimensionalityScale);
  
  //initalSigma.print();

  HyperPoint sigma = findAdaptiveSigma(point, initalSigma );
  sigma = sigma * smoothing;
  
  //sigma.print();

  return gaussianKernal(point, sigma );


}


double HyperHistogram::intgrateGaussianOverHyperCuboid(const HyperPoint& mean, const HyperPoint& sigmas, const HyperCuboid& cuboid) const{
  
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

double HyperHistogram::intgrateGaussianOverHyperVolume(const HyperPoint& point, const HyperPoint& sigmas, const HyperVolume& volume) const{
  
  double nCuboids = volume.getHyperCuboids().size();
  
  double multiInt = 0.0;

  for (int i = 0; i < nCuboids; i++){
    multiInt += intgrateGaussianOverHyperCuboid(point, sigmas, volume.getHyperCuboid(i));
  }

  return multiInt;

}

double HyperHistogram::intgrateGaussianOverBin(const HyperPoint& point, const HyperPoint& sigmas, int bin) const{
  
  return intgrateGaussianOverHyperVolume( point, sigmas, _binning->getBinHyperVolume(bin) );
}

HyperPointSet HyperHistogram::makePointsAtGaussianExtremes(const HyperPoint& mean, const HyperPoint& widths, double numSigma) const{
  
  int dim =  mean.getDimension();
  HyperPointSet points(dim);

  for (int i = 0; i < dim; i++){

    double max = _binning->getMax(i);
    double min = _binning->getMin(i);
    
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

double HyperHistogram::gaussianKernal(const HyperPoint& point, const HyperPoint& sigmas) const{
  
  int nBins = _binning->getNumBins();
  
  HyperPointSet points = makePointsAtGaussianExtremes(point, sigmas, 3.0);
  std::vector<int> binNumbers = _binning->getBinNumsContainingPoints(points);
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

std::vector<int> HyperHistogram::findNHighestContributingKernalBins(const HyperPoint& point, const HyperPoint& sigmas, int n) const{
  
  int nBins = _binning->getNumBins();

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

void HyperHistogram::reweightDatasetWithAdaptiveGaussianKernal(HyperPointSet& points, double smoothing) const{

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

