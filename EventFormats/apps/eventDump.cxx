#include "EventFormats/DAQFormats.hpp"
#include <unistd.h>
#include "EventFormats/TLBDataFragment.hpp"
#include "EventFormats/TLBMonitoringFragment.hpp"
#include "EventFormats/DigitizerDataFragment.hpp"

using namespace DAQFormats;

static void usage() {
   std::cout<<"Usage: eventDump [-f] [-d] [-n nEventsMax] <filename>"<<std::endl;
   exit(1);
}

int main(int argc, char **argv) {

  // argument parsing
  if(argc<2) usage();

  bool showFragments=false;
  bool showData=false;
  bool showTLB=false;
  bool showTRB=false;
  bool showDigitizer=false;
  int nEventsMax = -1;
  int opt;
  while ( (opt = getopt(argc, argv, "fd:n:")) != -1 ) {  
    switch ( opt ) {
    case 'f':
      std::cout<<"DumpingFragments ... "<<std::endl;
      showFragments = true;
      break;
    case 'd':
      std::cout<<"DumpingData ..."<<std::endl;
      if(optarg==NULL)
        std::cout<<"DumpingData : NULL"<<std::endl;
      else
        std::cout<<"DumpingData : "<<optarg<<std::endl;
      showFragments = true;
      showData = true;
      if(optarg==NULL){
        std::cout<<"No systems specified - printing all"<<std::endl;
        showTLB=true;
        showTRB=true;
        showDigitizer=true;
      }
      else{
        std::string systems = optarg;
        std::cout<<"DumpingData select systems : "<<systems<<std::endl;
        if(systems.find("TLB")==0){
          showTLB=true;
        }
        if(systems.find("TRB")==0){
          showTRB=true;
        }
        if(systems.find("Digitizer")==0){
          showDigitizer=true;
        }
      }
      std::cout<<"DumpingData TLB       : "<<showTLB<<std::endl;
      std::cout<<"DumpingData TRB       : "<<showTRB<<std::endl;
      std::cout<<"DumpingData Digitizer : "<<showDigitizer<<std::endl;
      break;
    case 'n':
      std::cout<<"Specifying Nvents : "<<optarg<<std::endl;
      nEventsMax = std::atoi(optarg);
      break;
    case ':':
      std::cout<<"Missing optopt : "<<optopt<<std::endl;
      break;
    case '?':  // unknown option...
      usage();
      break;
    }
  }

  
  
  std::string filename(argv[optind]);
  std::ifstream in(filename, std::ios::binary);
  if (!in.is_open()){
    std::cout << "ERROR: can't open file "<<filename<<std::endl;
    return 1;
  }
  
  int nEventsRead=0;
  
  while(in.good() and in.peek()!=EOF) {
    try {
      EventFull event(in);
      std::cout<<event<<std::endl;
      if (showFragments) {
      for(const auto &id :event.getFragmentIDs()) {
        const EventFragment* frag=event.find_fragment(id);
        std::cout<<*frag<<std::endl;
        if (showData) {
          switch (frag->source_id()&0xFFFF0000) {
          case TriggerSourceID:
            if(showData && showTLB){
              if (event.event_tag() == PhysicsTag ) {
                TLBDataFragment tlb_data_frag = TLBDataFragment(frag->payload<const uint32_t*>(), frag->payload_size());
                std::cout<<"TLB data fragment:"<<std::endl;
                std::cout<<tlb_data_frag<<std::endl;
              }
              else if (event.event_tag() == TLBMonitoringTag ) {
                TLBMonitoringFragment tlb_mon_frag = TLBMonitoringFragment(frag->payload<const uint32_t*>(), frag->payload_size());
                std::cout<<"TLB monitoring fragment:"<<std::endl;
                std::cout<<tlb_mon_frag<<std::endl;
              }
            }
            break;
          case TrackerSourceID: //FIXME put in specific 
          case PMTSourceID:
            if(showData && showDigitizer){
              if (event.event_tag() == PhysicsTag ) {
                DigitizerDataFragment digitizer_data_frag = DigitizerDataFragment(frag->payload<const uint32_t*>(), frag->payload_size());
                std::cout<<"Digitizer data fragment:"<<std::endl;
                std::cout<<digitizer_data_frag<<std::endl;
              }
            }
            break;
          default:
            const uint32_t* payload=frag->payload<const uint32_t *>();
            unsigned int ii=0;
            for(;ii<frag->payload_size()/4;ii++) {
          if (ii%8==0) std::cout<<" ";
          std::cout<<" 0x"<<std::setw(8)<<std::hex<<std::setfill('0')<<payload[ii];
          if (ii%8==7) std::cout<<std::endl;
              }
              if (ii%8!=0) std::cout<<std::endl;
              std::cout<<std::dec<<std::setfill(' ');
              break;
            }
          }
        }	
      }
    } catch (EFormatException &e) {
      std::cout<<"Problem while reading file - "<<e.what()<<std::endl;
      return 1;
    }
    
    // read up to nEventsMax if specified
    nEventsRead++;
    if(nEventsMax!=-1 && nEventsRead>=nEventsMax){
      std::cout<<"Finished reading specified number of events : "<<nEventsMax<<std::endl;
      break;
    }
    
  }
}