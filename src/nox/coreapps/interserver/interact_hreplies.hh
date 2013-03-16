#ifndef INTERACTS_HREPLIES_HH__
#define INTERACTS_HREPLIES_HH__

#include "component.hh"
#include <boost/noncopyable.hpp>
//#include <map>

namespace vigil
{
  using namespace vigil::container; 
  
  class Switch_get_config_reply 
  {
	public:
		static std::string to_string( struct ofl_msg_get_config_reply *repl);
  };
  
};

  
#endif
