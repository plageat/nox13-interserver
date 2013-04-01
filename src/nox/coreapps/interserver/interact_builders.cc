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
	
	std::vector<std::string> Inter_sw_config::additional_args()
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
	
	builder_name Inter_sw_config::name()
	{
		return "get_sw_config";  
	}

	//======================================
	builder_name Inter_sw_config_setter::name()
	{		
		return "set_sw_config";  
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
	
	std::vector<std::string> Inter_sw_config_setter::additional_args()
	{
		return std::vector<std::string> ();
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
	//=========================================
	builder_name Inter_features_request::name()
	{		
		return "features_request";
	}
	
	bool Inter_features_request::is_modify() const
	{
		return false;
	}
	
	std::vector<std::string> Inter_features_request::arg_requires()
	{
		std::vector<std::string> args;
		
		return args;
	}
	
	std::vector<std::string> Inter_features_request::additional_args()
	{
		return std::vector<std::string> ();
	}
	
	struct ofl_msg_header * Inter_features_request::request_msg_creator(const request_arguments& args)
	{
		struct ofl_msg_header*  msg = new ofl_msg_header;
		msg->type = OFPT_FEATURES_REQUEST;
		
		return msg;
	}
	//===========================================
	
	builder_name Inter_table_features::name()
	{		
		return "table_features";
	}
	
	bool Inter_table_features::is_modify() const
	{
		return false;
	}
	
	std::vector<std::string> Inter_table_features::arg_requires()
	{
		std::vector<std::string> args;
		
		return args;
	}
	
	std::vector<std::string> Inter_table_features::additional_args()
	{
		return std::vector<std::string> ();
	}
	
	struct ofl_msg_header * Inter_table_features::request_msg_creator(const request_arguments& args)
	{
		struct ofl_msg_multipart_request_table_features*  msg = new ofl_msg_multipart_request_table_features;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_TABLE_FEATURES;
		msg->tables_num = 0;
		msg->table_features = 0;
		
		return (ofl_msg_header*)msg;
	}
	
	//===========================================
	
	builder_name Inter_desc::name()
	{		
		return "switch_description";
	}
	
	bool Inter_desc::is_modify() const
	{
		return false;
	}
	
	std::vector<std::string> Inter_desc::arg_requires()
	{
		std::vector<std::string> args;
		
		return args;
	}
	
	std::vector<std::string> Inter_desc::additional_args()
	{
		return std::vector<std::string> ();
	}
	
	struct ofl_msg_header * Inter_desc::request_msg_creator(const request_arguments& args)
	{
		struct ofl_msg_multipart_request_header*  msg = new ofl_msg_multipart_request_header;
		msg->header.type = OFPT_MULTIPART_REQUEST;
		msg->type = OFPMP_DESC;
		
		return (ofl_msg_header*)msg;
	}
	//===========================================
	
	builder_name Inter_table_stats::name()
	{		
		return "table_stats";
	}
	
	bool Inter_table_stats::is_modify() const
	{
		return false;
	}
	
	std::vector<std::string> Inter_table_stats::arg_requires()
	{
		std::vector<std::string> args;
		
		return args;
	}
	
	std::vector<std::string> Inter_table_stats::additional_args()
	{
		return std::vector<std::string> ();
	}
	
	struct ofl_msg_header * Inter_table_stats::request_msg_creator(const request_arguments& args)
	{
		struct ofl_msg_multipart_request_header*  msg = new ofl_msg_multipart_request_header;
		msg->header.type = OFPT_MULTIPART_REQUEST;
		msg->type = OFPMP_TABLE;
		
		return (ofl_msg_header*)msg;
	}
};
