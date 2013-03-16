#include "interact_event.hh"
	
namespace vigil
{	
	using namespace vigil::container;    	
	
	Interact_event::Interact_event(const std::string& msg) 
		: Event(static_get_name()), _reply(msg)
	{
		
	}
	
	const std::string& Interact_event::get_reply() const
	{
		return this->_reply;
	}

};