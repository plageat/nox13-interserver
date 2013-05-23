#include "interact_answer.hh"

#include <sstream>
#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/bind.hpp>
#include <algorithm>
#include "nox.hh"
#include <map>
#include <stdexcept>
#include "interact_reply_event.hh"
#include "ofp-msg-event.hh"

	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
	 
	static Vlog_module lg("facade_answer");
	
	
	Disposition Interact_answer::handle_sw_config_reply(const Event& e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_get_config_reply *repl = (struct ofl_msg_get_config_reply *)**pi.msg;
		
		response = ofl_structs_config_to_string(repl->config);
		response += '\n';
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_features_reply(const Event& e)
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
	Disposition Interact_answer::handle_table_features(const Event& e)
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
	
	Disposition Interact_answer::handle_desc(const Event &e)
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
	
	Disposition Interact_answer::handle_table_stats(const Event &e)
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
	
	Disposition Interact_answer::handle_flow_info(const Event&e)
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
	
	Disposition Interact_answer::handle_agr_flow_info(const Event& e)
	{
		std::stringstream sbuf;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_aggregate *repl = (struct ofl_msg_multipart_reply_aggregate *)**pi.msg;
		// todo
		sbuf << "Byte count: " <<  (int)repl->byte_count << std::endl;
		sbuf << "Packet count: " <<  (int)repl->packet_count << std::endl;
		sbuf << "Flow count: " <<  (int)repl->flow_count << std::endl;
		
		acceptResponse( sbuf.str() );

		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_port_stats(const Event&e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_port *repl = (struct ofl_msg_multipart_reply_port *)**pi.msg;
	
		for(int i = 0;  i < repl->stats_num; ++i)
		{
			response += ofl_structs_port_stats_to_string(repl->stats[i]);
			response += "\n\n";
		}
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_port_desc(const Event&e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_port_desc *repl = (struct ofl_msg_multipart_reply_port_desc *)**pi.msg;
	
		for(int i = 0;  i < repl->stats_num; ++i)
		{
			response += ofl_structs_port_to_string(repl->stats[i]);
			response += "\n\n";
		}
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_queue_stats(const Event&e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_queue *repl = (struct ofl_msg_multipart_reply_queue *)**pi.msg;

		for(int i = 0;  i < repl->stats_num; ++i)
		{
			response += ofl_structs_queue_stats_to_string(repl->stats[i]);
			response += "\n\n";
		}
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_group_stats(const Event&e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_group *repl = (struct ofl_msg_multipart_reply_group *)**pi.msg;

		for(int i = 0;  i < repl->stats_num; ++i)
		{
			response += ofl_structs_group_stats_to_string(repl->stats[i]);
			response += "\n\n";
		}
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_group_desc(const Event&e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_group_desc *repl = (struct ofl_msg_multipart_reply_group_desc *)**pi.msg;

		for(int i = 0;  i < repl->stats_num; ++i)
		{
			response += ofl_structs_group_desc_stats_to_string(repl->stats[i],NULL);
			response += "\n\n";
		}
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_group_features(const Event&e)
	{
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_group_features *repl = 
											(struct ofl_msg_multipart_reply_group_features *)**pi.msg;
		
		std::string response = ofl_msg_to_string((struct ofl_msg_header*)repl,NULL );
		
		response += "\n\n";
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_queue_config(const Event&e)
	{
		std::stringstream sbuf;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_queue_get_config_reply *repl = 
											(struct ofl_msg_queue_get_config_reply *)**pi.msg;
		
		sbuf << "Port to be queried";
		sbuf << (int)repl->port  << std::endl; // fix this,must be not int
		
		for(int i = 0;  i < repl->queues_num; ++i)
		{
			sbuf << ofl_structs_queue_to_string(repl->queues[i]);
			sbuf << "\n\n";
		}
		
		acceptResponse(sbuf.str());
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_meter_stats(const Event&e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_meter *repl = 
											(struct ofl_msg_multipart_reply_meter *)**pi.msg;
		
		for(int i = 0;  i < repl->stats_num; ++i)
		{
			response += ofl_structs_meter_stats_to_string(repl->stats[i]);
			response += "\n\n";
		}
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_meter_config(const Event&e)
	{
		std::string response;
		
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_meter_conf *repl = 
											(struct ofl_msg_multipart_reply_meter_conf *)**pi.msg;

		for(int i = 0;  i < repl->stats_num; ++i)
		{
			response += ofl_structs_meter_config_to_string(repl->stats[i]);
			response += "\n\n";
		}
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_meter_features(const Event&e)
	{
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		struct ofl_msg_multipart_reply_meter_features *repl = 
											(struct ofl_msg_multipart_reply_meter_features *)**pi.msg;
		
		std::string response = ofl_structs_meter_features_to_string(repl->features);
		
		response += "\n\n";
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	Disposition Interact_answer::handle_error(const Event& e)
	{
		const Ofp_msg_event& pi = assert_cast<const Ofp_msg_event&>(e);
		
		struct ofl_msg_error *repl = (struct ofl_msg_error *)**pi.msg;
		
		std::string response = ofl_msg_to_string((struct ofl_msg_header*)repl,NULL );

		response += "\n";
		
		acceptResponse(response);
		
		return CONTINUE;
	}
	
	void Interact_answer::install()
	{
		//to add all answers from switch 
		register_handler(Ofp_msg_event::get_name(OFPT_GET_CONFIG_REPLY), 
							boost::bind(&Interact_answer::handle_sw_config_reply, this, _1) );
		register_handler(Ofp_msg_event::get_name(OFPT_FEATURES_REPLY), 
							boost::bind(&Interact_answer::handle_features_reply, this, _1) );
		
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_TABLE_FEATURES), 
							boost::bind(&Interact_answer::handle_desc, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_DESC), 
							boost::bind(&Interact_answer::handle_desc, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_TABLE), 
							boost::bind(&Interact_answer::handle_table_stats, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_FLOW), 
							boost::bind(&Interact_answer::handle_flow_info, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_AGGREGATE), 
							boost::bind(&Interact_answer::handle_agr_flow_info, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_PORT_STATS), 
							boost::bind(&Interact_answer::handle_port_stats, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_PORT_DESC), 
							boost::bind(&Interact_answer::handle_port_desc, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_QUEUE), 
							boost::bind(&Interact_answer::handle_queue_stats, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_GROUP), 
							boost::bind(&Interact_answer::handle_group_stats, this, _1) );
		
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_GROUP_DESC), 
							boost::bind(&Interact_answer::handle_group_desc, this, _1) );
		
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_GROUP_FEATURES), 
							boost::bind(&Interact_answer::handle_group_features, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_METER), 
							boost::bind(&Interact_answer::handle_meter_stats, this, _1) );
							
		register_handler(Ofp_msg_event::get_stats_name(OFPMP_METER_CONFIG), 
							boost::bind(&Interact_answer::handle_meter_config, this, _1) );
		// currently not supported	by switch?			
		//register_handler(Ofp_msg_event::get_stats_name(OFPMP_METER_FEATURES), 
			//				boost::bind(&Interact_answer::handle_meter_features, this, _1) );
			
		register_handler(Ofp_msg_event::get_name(OFPT_QUEUE_GET_CONFIG_REPLY), 
							boost::bind(&Interact_answer::handle_queue_config, this, _1) );
		
		register_handler(Ofp_msg_event::get_name(OFPT_ERROR), 
							boost::bind(&Interact_answer::handle_error, this, _1) );
	}
	
	void Interact_answer::getInstance(const container::Context* ctxt, 
			      vigil::Interact_answer*& scpa) 
	{
		scpa = dynamic_cast<Interact_answer*>
				(ctxt->get_by_interface(container::Interface_description
					(typeid(Interact_answer).name())));
	}
	
	void Interact_answer::acceptResponse(const std::string& answ)
	{
		post( new Interact_reply_event(answ) );
	}
		 
	REGISTER_COMPONENT(vigil::container::
		     Simple_component_factory<vigil::Interact_answer>, 
		     vigil::Interact_answer);
};
