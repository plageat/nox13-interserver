#include "facade_answer.hh"

#include <sstream>
#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/bind.hpp>
#include <algorithm>
#include "nox.hh"
#include <map>
#include <stdexcept>
#include "interact_event.hh"
#include "ofp-msg-event.hh"

	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
	 
	static Vlog_module lg("facade_answer");
	
	
	Disposition Facade_answer::handle_sw_config_reply(const Event& e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_get_config_reply *repl = (struct ofl_msg_get_config_reply *)**pi.msg;
		
		response = ofl_structs_config_to_string(repl->config);
		response += '\n';
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Facade_answer::handle_features_reply(const Event& e)
	{
		std::stringstream sbuf;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_features_reply *repl = (struct ofl_msg_features_reply *)**pi.msg;
		
		sbuf << "Datapathid is:	" << repl->datapath_id << std::endl;
		sbuf << "Max packet buffered at once: " << (uint32_t)repl->n_buffers << std::endl;
		sbuf << "Number of tables supported: "<< (uint32_t)repl->n_tables << std::endl;
		sbuf << "Auxiliary ID: " << (uint32_t)repl->auxiliary_id << std::endl;
		
		sbuf << "Supported capabilities are:\n";
		
		sbuf << "flow statistics: " << (repl->capabilities & OFPC_FLOW_STATS ? "yes" : "no") << std::endl;
		sbuf << "table statistics: " << (repl->capabilities & OFPC_TABLE_STATS ? "yes" : "no") << std::endl;
		sbuf << "port statistics: " << (repl->capabilities & OFPC_PORT_STATS ? "yes" : "no") << std::endl;
		sbuf << "group statistics: " << (repl->capabilities & OFPC_GROUP_STATS ? "yes" : "no") << std::endl;
		sbuf << "reasemble ip fragments: " << (repl->capabilities & OFPC_IP_REASM ? "yes" : "no") << std::endl;
		sbuf << "queue statistics: " << (repl->capabilities & OFPC_QUEUE_STATS ? "yes" : "no") << std::endl;
		sbuf << "block looping ports: " << (repl->capabilities & OFPC_PORT_BLOCKED ? "yes" : "no") << std::endl;
		
		acceptResponse(sbuf.str());
		
		return CONTINUE;
	}
	// problen of multiply calls
	Disposition Facade_answer::handle_table_features(const Event& e)
	{
		lg.dbg("I must call 1 times!");
		
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_table_features *repl = (struct ofl_msg_multipart_reply_table_features *)**pi.msg;
		
		for(size_t i = 0; i < repl->tables_num; ++i)
			response += ofl_structs_table_features_to_string(repl->table_features[i]);
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Facade_answer::handle_desc(const Event &e)
	{
		std::stringstream sbuf;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_reply_desc *repl = (struct ofl_msg_reply_desc *)**pi.msg;
		
		sbuf << "Manufacturer:	" << repl->mfr_desc << endl;
		sbuf << "Hardware:	" << repl->hw_desc << endl;
		sbuf << "Software:	" << repl->sw_desc << endl;
		sbuf << "Serial number:	" << repl->serial_num << endl;
		sbuf << "Datapath:	" << repl->dp_desc << endl; 
		
		acceptResponse( sbuf.str() );
		
		return CONTINUE;
	}
	
	Disposition Facade_answer::handle_table_stats(const Event &e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_table *repl = (struct ofl_msg_multipart_reply_table *)**pi.msg;
		
		for(int i = 0;  i < repl->stats_num; ++i)
		{
			response += ofl_structs_table_stats_to_string(repl->stats[i]);
			response += "\n\n";
		}
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Facade_answer::handle_flow_info(const Event&e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_flow *repl = (struct ofl_msg_multipart_reply_flow *)**pi.msg;
	
		for(int i = 0;  i < repl->stats_num; ++i)
		{
			response += ofl_structs_flow_stats_to_string(repl->stats[i],NULL);
			response += "\n\n";
		}
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	void Facade_answer::install()
	{
		//to add all answers from switch 
		register_handler(Ofp_msg_event::get_name(OFPT_GET_CONFIG_REPLY), 
							boost::bind(&Facade_answer::handle_sw_config_reply, this, _1) );
		register_handler(Ofp_msg_event::get_name(OFPT_FEATURES_REPLY), 
							boost::bind(&Facade_answer::handle_features_reply, this, _1) );
		
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_TABLE_FEATURES), 
							boost::bind(&Facade_answer::handle_desc, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_DESC), 
							boost::bind(&Facade_answer::handle_desc, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_TABLE), 
							boost::bind(&Facade_answer::handle_table_stats, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_FLOW), 
							boost::bind(&Facade_answer::handle_flow_info, this, _1) );
	}
	
	void Facade_answer::getInstance(const container::Context* ctxt, 
			      vigil::Facade_answer*& scpa) 
	{
		scpa = dynamic_cast<Facade_answer*>
				(ctxt->get_by_interface(container::Interface_description
					(typeid(Facade_answer).name())));
	}
	
	void Facade_answer::acceptResponse(std::string answ)
	{
		post( new Interact_event(answ) );
	}
		 
	REGISTER_COMPONENT(vigil::container::
		     Simple_component_factory<vigil::Facade_answer>, 
		     vigil::Facade_answer);
};
