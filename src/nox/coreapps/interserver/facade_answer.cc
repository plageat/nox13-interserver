#include "facade_answer.hh"

#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/bind.hpp>
#include <algorithm>
#include "nox.hh"
#include <map>
#include <stdexcept>
#include "interact_event.hh"
#include "ofp-msg-event.hh"
#include "interact_hreplies.hh"
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
	 
	static Vlog_module lg("facade_answer");
	
	
	Disposition Facade_answer::handle_sw_config_reply(const Event& e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_get_config_reply *repl = (struct ofl_msg_get_config_reply *)**pi.msg;
		
		response = Switch_get_config_reply::to_string(repl);
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	void Facade_answer::install()
	{
		//to add all answers from switch 
		register_handler(Ofp_msg_event::get_name(OFPT_GET_CONFIG_REPLY), 
							boost::bind(&Facade_answer::handle_sw_config_reply, this, _1) );
	}
	
	void Facade_answer::getInstance(const container::Context* ctxt, 
			      vigil::Facade_answer*& scpa) 
	{
		scpa = dynamic_cast<Facade_answer*>
				(ctxt->get_by_interface(container::Interface_description
					(typeid(Facade_answer).name())));
	}
	
	void Facade_answer::acceptResponse(std::string answ)
	{
		post( new Interact_event(answ) );
	}
		 
	REGISTER_COMPONENT(vigil::container::
		     Simple_component_factory<vigil::Facade_answer>, 
		     vigil::Facade_answer);
};
