#include "interact_reply_event.hh"
	
namespace vigil
{	
	using namespace vigil::container;    	
	
	Interact_reply_event::Interact_reply_event(const std::string& msg) 
		: Event(static_get_name()), _reply(msg)
	{
		
	}
	
	const std::string& Interact_reply_event::get_reply() const
	{
		return this->_reply;
	}

};