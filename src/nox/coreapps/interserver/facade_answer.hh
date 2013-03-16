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
  };
  
};

  
#endif
