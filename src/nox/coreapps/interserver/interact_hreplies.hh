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
		std::string extract_features( struct ofl_msg_features_reply *repl) const;
		std::string extract_capabilities( uint32_t capabilities) const;
		
  };
  
  class Switch_table_features
  {
	public:
		static std::string to_string(struct ofl_msg_multipart_reply_table_features * repl);
  };
  
  class Switch_desc
  {
	public:
		static std::string to_string(struct ofl_msg_reply_desc * repl);
  };
  
  class Switch_table_stats
  {
		std::string read_stats(ofl_table_stats * s) const;
	public:
		std::string to_string(struct ofl_msg_multipart_reply_table * repl) const;
  };
  
  class Switch_flow_info
  {
	 // std::string parse_flags(uint16_t flags) const;
	  std::string read_stats(struct ofl_flow_stats* s) const;
	public:
		std::string to_string(struct ofl_msg_multipart_reply_flow * repl) const;
  };
  
};

  
#endif
