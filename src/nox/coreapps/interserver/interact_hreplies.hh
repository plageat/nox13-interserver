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
  
  class Switch_features_reply 
  {
	public:
		std::string extract_features( struct ofl_msg_features_reply *repl);
		std::string extract_capabilities( uint32_t capabilities);
		
  };
  
  class Switch_table_features
  {
	public:
		std::string to_string(struct ofl_msg_multipart_reply_table_features * repl);
  };
  
  class Switch_desc
  {
	public:
		static std::string to_string(struct ofl_msg_reply_desc * repl);
  };
};

  
#endif
