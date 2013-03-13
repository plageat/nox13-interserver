#ifndef REQUEST_PROCESSOR_HH__
#define REQUEST_PROCESSOR_HH__

#include "http_event.hh"
#include <boost/noncopyable.hpp>
#include <map>
#include "component.hh"
#include "datapath-join.hh"
#include "datapath-leave.hh"

namespace vigil
{
  using namespace vigil::container; 
  
  class Request_processor : public Component
  {
	  private:
		std::vector<datapathid> _dpids;
	  public:
	
		Disposition handle_request(const Event& e);
		// something like datapathmem
		Disposition datapath_join_handler(const Event& e);
		Disposition datapath_leave_handler(const Event& e);
		
		void postResponse(std::string);
		
		Request_processor(const Context* c, const json_object* node) : Component(c) 
		{  }	
	
		~Request_processor() {};
	
		void configure(const Configuration* c) { };
	
		void install();

		static void getInstance(const container::Context* ctxt, 
									vigil::Request_processor*& scpa);
  };
  
};

  
#endif
