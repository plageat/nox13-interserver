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
	
	struct ofl_msg_header * Inter_sw_config::request_msg_creator(const request_arguments& )
	{
		struct ofl_msg_header*  msg = new ofl_msg_header;
		msg->type = OFPT_GET_CONFIG_REQUEST;
		
		return msg;
	}
	
	enum ofp_type Inter_sw_config::name()
	{
		return OFPT_GET_CONFIG_REQUEST;
	}
	
	/*
	Disposition Inter_sw_config::handle_request(const Event& e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_get_config_reply *repl = (struct ofl_msg_get_config_reply *)**pi.msg;
		
		std::stringstream sbuf;
		
		sbuf << "Config flags are:\n";
		
		if(OFPC_FRAG_NORMAL == repl->config->flags)
			sbuf << "no special handling for fragments\n";
		else
		{
			sbuf << "drop fragments: " << (repl->config->flags & OFPC_FRAG_DROP ? "yes" : "no") << std::endl;
			sbuf << "reassemble: " << (repl->config->flags & OFPC_FRAG_REASM ? "yes" : "no") << std::endl;
		}
		
		sbuf << "Max bytes of datapaths packet sending to the controller: ";
		
		if( OFPCML_NO_BUFFER == repl->config->miss_send_len )
			sbuf << "no buffering, send all" << std::endl;
		else
			sbuf << (unsigned int)repl->config->miss_send_len << std::endl;
		
		response = sbuf.str();
		
		post(new Http_response_event(response));
		
		return CONTINUE;
	}*/
		
		//return send_openflow_msg(did, (struct ofl_msg_header *)msg, 1/*xid*/, false/*block*/);
};
