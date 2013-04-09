#ifndef FACADE_ANSWER_HH__
#define FACADE_ANSWER_HH__

#include <boost/noncopyable.hpp>
#include "component.hh"

namespace vigil
{
  using namespace vigil::container; 
  
  class Facade_answer : public Component
  {
	  public:
		Facade_answer(const Context* c, const json_object* node) : Component(c) 
		{  }	
	
		~Facade_answer() {};
	
		void configure(const Configuration* c) { };
	
		void install();

		static void getInstance(const container::Context* ctxt, 
									vigil::Facade_answer*& scpa);
		// send special event, when response is complited for sending to client
		void acceptResponse(std::string);
		
		//here to add handlers for switch answers
		Disposition handle_sw_config_reply(const Event& e);
		
		Disposition handle_features_reply(const Event& e);
		
		Disposition handle_table_features(const Event&);
		
		Disposition handle_desc(const Event&);
		
		Disposition handle_table_stats(const Event&);
		
		Disposition handle_flow_info(const Event&);
		
		Disposition handle_agr_flow_info(const Event&);
		
		Disposition handle_port_stats(const Event&);
		
		Disposition handle_port_desc(const Event&);
		
		Disposition handle_queue_stats(const Event&);
		
		Disposition handle_group_stats(const Event&);
		
		Disposition handle_group_desc(const Event&);
		
		Disposition handle_group_features(const Event&);
		
		Disposition handle_meter_stats(const Event&);
		
		Disposition handle_meter_config(const Event&);
		
		Disposition handle_meter_features(const Event&);
		
		Disposition handle_queue_config(const Event&);
  };
  
};

  
#endif
