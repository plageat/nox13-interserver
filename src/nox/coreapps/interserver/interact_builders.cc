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
	
	enum ofp_type Inter_sw_config::name()
	{
		return OFPT_GET_CONFIG_REQUEST;
	}

};
