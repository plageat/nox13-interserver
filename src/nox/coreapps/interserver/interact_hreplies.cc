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
	
	
};
