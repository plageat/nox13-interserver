#include "interact_hreplies.hh"

#include <algorithm>
#include "nox.hh"
//#include <map>
//#include <stdexcept>
#include <sstream>	
	
namespace vigil
{	
	using namespace vigil::container;    
	 
	std::string Switch_get_config_reply::to_string( struct ofl_msg_get_config_reply *repl)
	{
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
		
		return sbuf.str();
	}
	
	std::string Switch_features_reply::extract_features( struct ofl_msg_features_reply *repl)
	{
		std::stringstream sbuf;
		
		sbuf << "Datapathid is:	" << repl->datapath_id << std::endl;
		sbuf << "Max packet buffered at once: " << (uint32_t)repl->n_buffers << std::endl;
		sbuf << "Number of tables supported: "<< (uint32_t)repl->n_tables << std::endl;
		sbuf << "Auxiliary ID: " << (uint32_t)repl->auxiliary_id << std::endl;
		
		return sbuf.str();
		
	}
	
	std::string Switch_features_reply::extract_capabilities(uint32_t capabilities)
	{
		std::stringstream sbuf;
		
		sbuf << "Supported capabilities are:\n";
		
		sbuf << "flow statistics: " << (capabilities & OFPC_FLOW_STATS ? "yes" : "no") << std::endl;
		sbuf << "table statistics: " << (capabilities & OFPC_TABLE_STATS ? "yes" : "no") << std::endl;
		sbuf << "port statistics: " << (capabilities & OFPC_PORT_STATS ? "yes" : "no") << std::endl;
		sbuf << "group statistics: " << (capabilities & OFPC_GROUP_STATS ? "yes" : "no") << std::endl;
		sbuf << "reasemble ip fragments: " << (capabilities & OFPC_IP_REASM ? "yes" : "no") << std::endl;
		sbuf << "queue statistics: " << (capabilities & OFPC_QUEUE_STATS ? "yes" : "no") << std::endl;
		sbuf << "block looping ports: " << (capabilities & OFPC_PORT_BLOCKED ? "yes" : "no") << std::endl;
		
		return sbuf.str();
	}
};
