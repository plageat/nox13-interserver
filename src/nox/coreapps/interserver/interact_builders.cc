#include "interact_builders.hh"

#include "http_event.hh"
#include "netinet++/datapathid.hh"
#include "ofp-msg-event.hh"
#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/bind.hpp>
#include <algorithm>
#include "nox.hh"
#include <sstream>	
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
 
	static Vlog_module lg("inter_sw_config");
	
	std::vector<std::string> Inter_sw_config::arg_requires()
	{
		return std::vector<std::string> ();
	}
	
	struct ofl_msg_header * Inter_sw_config::request_msg_creator(const request_arguments& args)
	{
		struct ofl_msg_header*  msg = new ofl_msg_header;
		msg->type = OFPT_GET_CONFIG_REQUEST;
		
		return msg;
	}
	
	bool Inter_sw_config::is_modify() const
	{
		return false;
	}
	
	enum ofp_type Inter_sw_config::name()
	{
		return OFPT_GET_CONFIG_REQUEST;
	}
	//======================================
	enum ofp_type Inter_sw_config_setter::name()
	{
		return OFPT_SET_CONFIG;
	}
	
	bool Inter_sw_config_setter::is_modify() const
	{
		return true;
	}
	
	std::vector<std::string> Inter_sw_config_setter::arg_requires()
	{
		std::vector<std::string> args;
		args.push_back("flags");
		args.push_back("max_len");
		
		return args;
	}
	
	struct ofl_msg_header * Inter_sw_config_setter::request_msg_creator(const request_arguments& args)
	{
		struct ofl_msg_set_config*  msg = new ofl_msg_set_config;
		msg->header.type = OFPT_SET_CONFIG;
		
		struct ofl_config  *config = new ofl_config;
	
		config->flags = (uint16_t) atoi( args.find("flags")->second.c_str() );
		config->miss_send_len = (uint16_t) atoi ( args.find("max_len")->second.c_str() );
		
		msg->config = config;
		
		return (ofl_msg_header *)msg;
	}
};
