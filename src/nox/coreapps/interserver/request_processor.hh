#ifndef REQUEST_PROCESSOR_HH__
#define REQUEST_PROCESSOR_HH__

#undef _GLIBCXX_CONCEPT_CHECKS

#include "http_event.hh"
#include <boost/noncopyable.hpp>
#include <map>
#include <boost/thread/mutex.hpp>
#include "component.hh"
#include "datapath-join.hh"
#include "datapath-leave.hh"
#include "msg_resolver.hh"
#include "threads/cooperative.hh"

namespace vigil
{
  using namespace vigil::container; 
  
  class Request_processor : public Component, Co_thread
  {
	private:
	    bool _auto_mutex;
	    bool _reply_state;
		std::vector<datapathid> _dpids;
	  public:
		// handler for http incoming
		Disposition handle_request(const Event& e);
		// handler for interact reply
		Disposition handle_reply(const Event& e);
		// something like datapathmem
		Disposition datapath_join_handler(const Event& e);
		Disposition datapath_leave_handler(const Event& e);
		void dpid_check(const datapathid& id) const;
		
		void postResponse(const Return_msg&);
		
		Request_processor(const Context* c, const json_object* node) : Component(c) 
		{  
			_auto_mutex = false;
			_reply_state = false;
		}	
	
		~Request_processor() {};
	
		void configure(const Configuration* c);
	
		void install();

		static void getInstance(const container::Context* ctxt, 
									vigil::Request_processor*& scpa);
		
		bool interpret_checkerror(const json_object*) const;
		std::string interpret_type(const json_object*) const;	
		datapathid interpret_dpid(const json_object*) const;	
		
		void auto_set_reply();
  };
  
};

  
#endif
