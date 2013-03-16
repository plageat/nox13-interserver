#ifndef INTERACT_EVENT_HH__
#define INTERACT_EVENT_HH__

#include <map>
#include "component.hh"


namespace vigil
{
  using namespace vigil::container; 
  
  // generats when replyes from switch has been processed and ready for sending to client 
  class Interact_event : public Event 
  {
	private:  
	  std::string _reply;
	public:
	  Interact_event(const std::string& msg);
	  ~Interact_event() { };

    Interact_event() : Event(static_get_name()) 
    { }
    /** Static name required in NOX.
     */
    static const Event_name static_get_name() 
    {
      return "Interact_event";
    }

	const std::string& get_reply() const;
  };  
  
};


  
#endif
