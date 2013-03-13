#ifndef INTER_SW_CONFIG_HH__
#define INTER_SW_CONFIG_HH__

//#include <map>
#include "interactor.hh"
//#include "component.hh"

namespace vigil
{
  using namespace vigil::container; 
  
  class Inter_sw_config : public Interactor 
  {
	public:
	
		// find event for replying
		//Disposition handle_request(const Event& e);
		
		Inter_sw_config(const Context* c, const json_object* node) : Interactor(c,node) 
		{  }	
	
		~Inter_sw_config() {};
	
		void configure(const Configuration* c) { };
	
		void install();

		static void getInstance(const container::Context* ctxt, 
									vigil::Inter_sw_config*& scpa);
									
		const std::string& response(const args_cont&); 
  };
  
};

  
#endif
