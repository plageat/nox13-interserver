#include "interact_builders.hh"

#include "http_event.hh"
#include "netinet++/ethernetaddr.hh"
#include "netinet++/datapathid.hh"
#include "ofp-msg-event.hh"
#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/foreach.hpp>
#include <boost/bind.hpp>
#include <algorithm>
#include "nox.hh"
#include <sstream>	
#include "flow.hh"
#include <stdexcept>
#include "boost/assign.hpp"
	
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
 
	static Vlog_module lg("inter_sw_config");
	
	namespace 
	{
		Match_fields_builder matchBuilder;
		Action_fields_builder actionBuilder;
		Instruction_fields_builder instrBuilder;
	
	}
	
	typedef std::map<std::string,std::pair<int,int> > matchs_hash;
	
	Action_fields_builder::Action_fields_builder()
	{
		_action_tags = boost::assign::map_list_of ("action_type",e_String)("act_out_port",e_String)
						("act_queue_id",e_Num)("act_group_id",e_Num)("tag_type",e_String)
						("ethertype",e_Num)("ttl_action",e_String)("ttl",e_Num)("ttl_type",e_String);
	
		std::string tag;
		BOOST_FOREACH(const matchs_hash::value_type& p, fields)
		{
			tag.clear();
			tag += "act_";
			tag += p.first;
			
			switch(p.second.first)
			{
				case OXM_OF_IPV4_SRC:
				case OXM_OF_IPV4_DST:
				{
					_action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(tag,e_Ipv4) );
					break;
				};
				case OXM_OF_METADATA:
				{
					_action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(tag,e_String) );
					break;
				}
				case OXM_OF_ETH_SRC:
				case OXM_OF_ETH_DST:
				{
					_action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(tag,e_MAC) );
					break;
				}
				case OXM_OF_IPV6_SRC:
				case OXM_OF_IPV6_DST:
				case OXM_OF_IPV6_ND_TARGET:
				case OXM_OF_IPV6_ND_TLL:
				case OXM_OF_IPV6_ND_SLL:
				{
					_action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(tag,e_Ipv6) );
					break;
				}
				default:
				{
					_action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(tag,e_Num) );
					break;
				}
			};
			
		}
	}
	
	Action_fields_builder::~Action_fields_builder()
	{
		
	}
	
	const arguments_list& Action_fields_builder::get_action_args() const
	{
		return _action_tags;
	}
	
	Actions* Action_fields_builder::construct_action(const request_arguments& args)
	{
		// this logic for installing one action from one transaction
		Actions * acts = new Actions();
	
		request_arguments::const_iterator i = args.find("action_type");
		if(i == args.end())
			throw std::runtime_error("missing action_type field\n");
		
		if(i->second == "output" )
		{
			request_arguments::const_iterator a_out_port = args.find("act_out_port");
			if( a_out_port == args.end() )
				throw std::runtime_error("missing act_out_port argument!\n");
			
			uint32_t port = 0;
			
			if(a_out_port->second == "in_port" )
				port = OFPP_IN_PORT;
			else if(a_out_port->second == "table")
				port = OFPP_TABLE;
			else if(a_out_port->second == "normal")
				port = OFPP_NORMAL;
			else if(a_out_port->second == "flood")
				port = OFPP_FLOOD;
			else if(a_out_port->second == "controller")
				port = OFPP_CONTROLLER;
			else if(a_out_port->second == "local")
				port = OFPP_LOCAL;
			else
				port = (uint32_t) atoi( a_out_port->second.c_str() );
			
			acts->CreateOutput(port);
			std::cout << "Create output action to port OK!" <<std::endl;
			return acts;
		}
		
		if(i->second == "set_queue" )
		{
			request_arguments::const_iterator queue_id = args.find("act_queue_id");
			if( queue_id == args.end() )
				throw std::runtime_error("missing act_queue_id argument!\n");
			uint32_t q_id = (uint32_t) atoi( queue_id->second.c_str() );
			
			acts->CreateSetQueue(q_id);
			
			return acts;
		}
		
		if(i->second == "group" )
		{
			request_arguments::const_iterator i = args.find("act_group_id");
			if( i == args.end() )
				throw std::runtime_error("missing act_group_id argument!\n");
			uint32_t group = (uint32_t) atoi( i->second.c_str() );
			
			acts->CreateGroupAction(group);
			
			return acts;
		}
		
		if(i->second == "push" )
		{
			request_arguments::const_iterator i = args.find("tag_type");
			if( i == args.end() )
				throw std::runtime_error("missing tag_type argument!\n");
			request_arguments::const_iterator j = args.find("ethertype");
			if( j == args.end() )
				throw std::runtime_error("missing ethertype argument!\n");
			
			enum ofp_action_type type; 
			uint16_t ethertype = (uint16_t) atoi( j->second.c_str() );
			
			if( i->second == "vlan")
				type = OFPAT_PUSH_VLAN;
			else if(i->second == "mpls")
				type = OFPAT_PUSH_MPLS;
			else if(i->second == "pbb")
				type = OFPAT_PUSH_PBB;
			else 
				throw std::runtime_error("invalid tag_type!\n");
			
			acts->CreatePushAction(type,ethertype);
			
			return acts;
		}
		
		if(i->second == "pop" )
		{
			request_arguments::const_iterator i = args.find("tag_type");
			if( i == args.end() )
				throw std::runtime_error("missing tag_type argument!\n");
			
			if( i->second == "vlan")
				acts->CreatePopVlan();
			else if(i->second == "mpls")
			{
				request_arguments::const_iterator j = args.find("ethertype");
				if( j == args.end() )
					throw std::runtime_error("missing ethertype argument!\n");
			
				uint16_t ethertype = (uint16_t) atoi( j->second.c_str() );
				acts->CreatePopMpls(ethertype);
			}
			else 
				throw std::runtime_error("invalid tag_type!\n");

			return acts;
		}

		if(i->second == "set_field")
		{
			// :-( 
			BOOST_FOREACH(const request_arguments::value_type& p, args)
			{
				if( p.first == "act_ipv4_src" || 
					p.first == "act_ipv4_dst")
				{
					struct in_addr addr;
					inet_pton(AF_INET,p.second.c_str(),&addr);
					acts->CreateSetField(p.first.substr(4),&addr);
					
					continue;
				}
				if( p.first == "act_ipv6_src" ||
					p.first == "act_ipv6_dst" ||
					p.first == "act_ipv6_nd_target" ||
					p.first == "act_ipv6_nd_tll" ||
					p.first == "act_ipv6_nd_sll")
				{
					struct in6_addr addr;
					inet_pton(AF_INET6,p.second.c_str(),&addr);
					acts->CreateSetField(p.first.substr(4),&addr);
				
					continue;
				}
				if(p.first == "act_eth_dst" ||
					p.first == "act_eth_src")
				{	//works? need to check
					ethernetaddr addr = ethernetaddr(p.second);
					acts->CreateSetField(p.first.substr(4),addr.octet);
				
					continue;
				}
				std::string for_size = p.first.substr(4); // delete prefix "act_"
				matchs_hash::const_iterator it = fields.find( for_size );
				if( it == fields.end() )
					continue;
				else
				{
					switch(it->second.second)
					{
						case 1:
						{
							uint8_t value = (uint8_t)atoi(p.second.c_str());
							acts->CreateSetField(for_size,&value);
						
							break;
						}
						case 2:
						{
							uint16_t value = (uint16_t)atoi(p.second.c_str());
							acts->CreateSetField(for_size,&value);
						
							break;
						}
						case 4:
						{
							uint32_t value = (uint32_t)atoi(p.second.c_str());
							acts->CreateSetField(for_size,&value);
						
							break;
						}
						case 8:
						{
							uint64_t value = (uint64_t)atoll(p.second.c_str());
							acts->CreateSetField(for_size,&value);
							
							continue;
						}
					};
				}
			}
			
			return acts;
		}
		if(i->second == "change_ttl")
		{
			request_arguments::const_iterator i = args.find("ttl_action");
			if( i == args.end() )
				throw std::runtime_error("missing ttl_action argument!\n");
			
			if( i->second == "set")
			{
				request_arguments::const_iterator j = args.find("ttl");
				if( j == args.end() )
					throw std::runtime_error("missing ttl argument!\n");
				
				uint8_t ttl = (uint8_t) atoi( j->second.c_str() );
				
				request_arguments::const_iterator c = args.find("ttl_type");
				if( c == args.end() )
					throw std::runtime_error("missing ttl_type argument!\n");
				
				if( c->second == "ip" )
					acts->CreateSetNwTTL(ttl);
				else if (c->second == "mpls")
					acts->CreateSetMplsTTL(ttl);
				else
					throw std::runtime_error("invalid ttl_type!\n");
			}
			else if(i->second == "decrement")
			{
				enum ofp_action_type type;
				
				request_arguments::const_iterator j = args.find("ttl_type");
				if( j == args.end() )
					throw std::runtime_error("missing ttl_type argument!\n");
				
				if( j->second == "ip" )
					type = OFPAT_DEC_NW_TTL;
				else if (j->second == "mpls")
					type = OFPAT_DEC_MPLS_TTL;
				else
					throw std::runtime_error("invalid ttl_type!\n");

				acts->CreateDecTTL(type);
			}
	
			else if (i->second == "copy")
			{
				enum ofp_action_type type;
				
				request_arguments::const_iterator j = args.find("ttl_type");
				if( j == args.end() )
					throw std::runtime_error("missing ttl_type argument!\n");
				
				if( j->second == "in" )
					type = OFPAT_COPY_TTL_IN;
				else if (j->second == "out")
					type = OFPAT_COPY_TTL_OUT;
				else
					throw std::runtime_error("invalid ttl_type!\n");

				acts->CreateCopyTTL(type);
			}
			else
				throw std::runtime_error("invalid ttl_action!\n");

			return acts;
		}
		
		// empty acts = drop action
		return acts;
	}
	
	const arguments_list& Instruction_fields_builder::get_instr_args() const
	{
		return _instr_tags;
	}
	
	Instruction_fields_builder::Instruction_fields_builder()
	{
		// possible arguments for instruction
		
		_instr_tags = boost::assign::map_list_of ("goto_id", e_Num) ("metadata",e_Num) ("metadata_mask",e_Num)
				("meter",e_Num);
		// instructions includes action tags as them part of instructions
		_instr_tags.insert(_instr_tags.begin(), _actionBuilder.get_action_args().begin(),
												_actionBuilder.get_action_args().end() );
	}
	
	Instruction_fields_builder::~Instruction_fields_builder()
	{
		
	}
	
	Instruction* Instruction_fields_builder::construct_instruction(const request_arguments& args)
	{
		Instruction * instr = new Instruction();
		
		// required argument, must be set in builder constructor
		request_arguments::const_iterator i = args.find("instr_type");
		if(i == args.end())
			throw std::runtime_error("missing instr_type field");
	
		if(i->second == "goto_table" )
		{
			request_arguments::const_iterator table_id = args.find("goto_id");
			if( table_id == args.end() )
				throw std::runtime_error("missing go to table id, goto_id argument not found!\n");
			uint8_t id = (uint8_t) atoi( table_id->second.c_str() );
			
			instr->CreateGoToTable(id);
			
			return instr;
		}
		
		if(i->second == "write_metadata")
		{
			request_arguments::const_iterator metadata = args.find("metadata");
			if( metadata == args.end() )
				throw std::runtime_error("missing metadata field");
				
			uint64_t md = (uint64_t) atoll( metadata->second.c_str() );
			
			request_arguments::const_iterator metadata_mask = args.find("metadata_mask");
			
			if( metadata_mask == args.end() )
				instr->CreateWriteMetadata(md);
			else
			{
				uint64_t md_mask = (uint64_t) atoll( metadata_mask->second.c_str() );
				instr->CreateWriteMetadata(md,md_mask);
			}
			
			return instr;
		}
		
		if( i->second == "write_actions" )
		{
			instr->CreateWrite( actionBuilder.construct_action(args) );
			
			return instr;
		}
		
		if( i->second == "apply_actions" )
		{
			instr->CreateApply( actionBuilder.construct_action(args) );
			
			return instr;
		}
		
		if( i->second == "clear_actions" )
		{
			instr->CreateClearActions();
			
			return instr;
		}
		
		if( i->second == "meter")
		{
			request_arguments::const_iterator meter = args.find("meter");
			if( meter == args.end() )
				throw std::runtime_error("missing meter field");
				
			uint32_t id = (uint32_t) atoi( meter->second.c_str() );
			
			instr->CreateMeterInstruction(id);
			
			return instr;
		}
		
		return instr;
	}
	
	Match_fields_builder::Match_fields_builder() : _m_header(NULL)
	{
		BOOST_FOREACH(const matchs_hash::value_type& p, fields)
		{
			switch(p.second.first)
			{
				case OXM_OF_IPV4_SRC:
				case OXM_OF_IPV4_DST:
				{
					_match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_Ipv4) );
					_match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_Ipv4) );
					break;
				};
				case OXM_OF_METADATA:
				{
					_match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_String) );
					_match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_String) );
					break;
				}
				case OXM_OF_ETH_SRC:
				case OXM_OF_ETH_DST:
				{
					_match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_MAC) );
					_match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_MAC) );
					break;
				}
				case OXM_OF_IPV6_SRC:
				case OXM_OF_IPV6_DST:
				case OXM_OF_IPV6_ND_TARGET:
				case OXM_OF_IPV6_ND_TLL:
				case OXM_OF_IPV6_ND_SLL:
				{
					_match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_Ipv6) );
					_match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_Ipv6) );
					break;
				}
				default:
				{
					_match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_Num) );
					_match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_Num) );
					break;
				}
			};
			
		}
	}
	
	Match_fields_builder::~Match_fields_builder()
	{
		
	}
	
	const arguments_list& Match_fields_builder::get_match_args() const
	{
		return _match_tags;
	}
	
	ofl_match_header* Match_fields_builder::construct_match(const request_arguments& args)
	{
		// expected checking arguments
		_m_header = NULL;
		
		Flow *f = new Flow;
		request_arguments::const_iterator i;
		// we have problems with maskeble mathes..
		BOOST_FOREACH(const request_arguments::value_type& p, args)
		{
			if( p.first == "ipv4_src" || 
				p.first == "ipv4_dst")
			{
				struct in_addr addr;
				inet_pton(AF_INET,p.second.c_str(),&addr);
				
				if( (i = args.find(p.first + "_mask") ) == args.end() )
					f->Add_Field(p.first,addr);
				else
				{
					std::cout <<"Working masked ip match builder..." << std::endl;
					std::cout << "Address is: " << p.second<< std::endl;
					std::cout << "Mask is: " << i->second << std::endl;
					
					struct in_addr addr_mask;
					inet_pton(AF_INET,i->second.c_str(),&addr_mask);
					f->Add_Field(p.first,addr,addr_mask);
				}
				continue;
			}
			if( p.first == "ipv6_src" ||
				p.first == "ipv6_dst" ||
				p.first == "ipv6_nd_target" ||
				p.first == "ipv6_nd_tll" ||
				p.first == "ipv6_nd_sll")
				{
					struct in6_addr addr;
					inet_pton(AF_INET6,p.second.c_str(),&addr);
					
					if( (i = args.find(p.first + "_mask") ) == args.end() )
						f->Add_Field(p.first,addr);
					else
					{
						struct in6_addr addr_mask;
						inet_pton(AF_INET6,i->second.c_str(),&addr_mask);
						f->Add_Field(p.first,addr,addr_mask);
					}
				
					continue;
				}
			if(p.first == "eth_dst" ||
				p.first == "eth_src")
			{
				
				if( (i = args.find(p.first + "_mask") ) == args.end() )
					f->Add_Field(p.first,p.second);
				else
					f->Add_Field(p.first,p.second,i->second);
				continue;
			}
			// default
			// for non adress arguments: ports and which are simple numbers
			// need determine size correctly for call Add_fields method
			matchs_hash::const_iterator it = fields.find( p.first );
			if( it == fields.end() )
				continue;
			else
			{
				switch(it->second.second)
				{
					case 1:
					{
						uint8_t value = (uint8_t)atoi(p.second.c_str());
						
						if( (i = args.find(p.first + "_mask") ) == args.end() )
							f->Add_Field(p.first,value);
						else
						{
							uint8_t value_mask = (uint8_t)atoi(i->second.c_str());
							f->Add_Field(p.first,value,value_mask);
						}
						
						break;
					}
					case 2:
					{
						uint16_t value = (uint16_t)atoi(p.second.c_str());
						
						if( (i = args.find(p.first + "_mask") ) == args.end() )
							f->Add_Field(p.first,value);
						else
						{
							uint16_t value_mask = (uint16_t)atoi(i->second.c_str());
							f->Add_Field(p.first,value,value_mask);
						}
						
						break;
					}
					case 4:
					{
						uint32_t value = (uint32_t)atoi(p.second.c_str());
						
						if( (i = args.find(p.first + "_mask") ) == args.end() )
							f->Add_Field(p.first,value);
						else
						{
							uint32_t value_mask = (uint32_t)atoi(i->second.c_str());
							f->Add_Field(p.first,value,value_mask);
						}
						
						break;
					}
					case 8:
					{
						uint64_t value = (uint64_t)atoll(p.second.c_str());
						
						if( (i = args.find(p.first + "_mask") ) == args.end() )
							f->Add_Field(p.first,value);
						else
						{
							uint64_t value_mask = (uint64_t)atoll(i->second.c_str());
							f->Add_Field(p.first,value,value_mask);
						}
						
						break;
					}
				};
			}
		}
		
		
		_m_header = (struct ofl_match_header*)&(f->match);
		return _m_header;
	}
	
	
	//===========================================================
	
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
	
	Inter_sw_config_setter::Inter_sw_config_setter()
	{
		_args.push_back(std::pair<std::string, enum e_arg_type_t>(std::string("flags"),e_Num) );
		_args.push_back(std::pair<std::string, enum e_arg_type_t>(std::string("max_len"),e_Num) );
	}
	
	struct ofl_msg_header * Inter_sw_config_setter::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
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
	
	struct ofl_msg_header * Inter_table_stats::request_msg_creator(const request_arguments& args)
	{
		struct ofl_msg_multipart_request_header*  msg = new ofl_msg_multipart_request_header;
		msg->header.type = OFPT_MULTIPART_REQUEST;
		msg->type = OFPMP_TABLE;
		
		return (ofl_msg_header*)msg;
	}
	//===========================================
	
	builder_name Inter_flow_info::name()
	{		
		return "flow_info";
	}
	
	bool Inter_flow_info::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_flow_info::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_request_flow*  msg = new ofl_msg_multipart_request_flow;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_FLOW;
	
		if( (i = args.find("table_id")) == args.end() )
			msg->table_id = OFPTT_ALL;
		else
			msg->table_id = (uint8_t)atoi(i->second.c_str());
		if( (i = args.find("out_port") ) == args.end() )
			msg->out_port = OFPP_ANY;
		else
			msg->out_port = (uint32_t)atoi(i->second.c_str());
		if( (i = args.find("out_group")) == args.end() )
			msg->out_group = OFPG_ANY;
		else	
			msg->out_group = (uint32_t)atoi(i->second.c_str());
		if( (i = args.find("cookie")) == args.end() )
			msg->cookie = 0;
		else
			msg->cookie = (uint64_t)atoll(i->second.c_str());
		if( (i = args.find("cookie_mask")) == args.end() )
			msg->cookie_mask = 0;
		else
			msg->cookie_mask = (uint64_t)atoll(i->second.c_str());

		msg->match =  matchBuilder.construct_match(args);
	
		return (ofl_msg_header*)msg;
	}
	
	Inter_flow_info::Inter_flow_info()
	{
		_addit_args = matchBuilder.get_match_args(); 
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("table_id"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("out_port"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("out_group"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("cookie"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("cookie_mask"),e_Num) );
	}
		//===========================================
	
	builder_name Inter_flow_agr_info::name()
	{		
		return "agr_flow_info";
	}
	
	bool Inter_flow_agr_info::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_flow_agr_info::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_request_flow*  msg = new ofl_msg_multipart_request_flow;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_AGGREGATE;
	
		if( (i = args.find("table_id")) == args.end() )
			msg->table_id = OFPTT_ALL;
		else
			msg->table_id = (uint8_t)atoi(i->second.c_str());
		if( (i = args.find("out_port") ) == args.end() )
			msg->out_port = OFPP_ANY;
		else
			msg->out_port = (uint32_t)atoi(i->second.c_str());
		if( (i = args.find("out_group")) == args.end() )
			msg->out_group = OFPG_ANY;
		else	
			msg->out_group = (uint32_t)atoi(i->second.c_str());
		if( (i = args.find("cookie")) == args.end() )
			msg->cookie = 0;
		else
			msg->cookie = (uint64_t)atoll(i->second.c_str());
		if( (i = args.find("cookie_mask")) == args.end() )
			msg->cookie_mask = 0;
		else
			msg->cookie_mask = (uint64_t)atoll(i->second.c_str());

		msg->match =  matchBuilder.construct_match(args);
	
		return (ofl_msg_header*)msg;
	}
	
	Inter_flow_agr_info::Inter_flow_agr_info()
	{
		_addit_args = matchBuilder.get_match_args(); 
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("table_id"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("out_port"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("out_group"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("cookie"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("cookie_mask"),e_Num) );
	}
	//=======================================
	builder_name Inter_port_stats::name()
	{		
		return "port_stats";
	}
	
	bool Inter_port_stats::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_port_stats::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_request_port*  msg = new ofl_msg_multipart_request_port;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_PORT_STATS;
	
		if( (i = args.find("port")) == args.end() )
			msg->port_no = OFPP_ANY;
		else
			msg->port_no = (uint32_t)atoi(i->second.c_str());

	
		return (ofl_msg_header*)msg;
	}
	
	Inter_port_stats::Inter_port_stats()
	{ 
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("port"),e_Num) );
	}
	//=======================================
	builder_name Inter_queue_stats::name()
	{		
		return "queue_stats";
	}
	
	bool Inter_queue_stats::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_queue_stats::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_request_queue*  msg = new ofl_msg_multipart_request_queue;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_QUEUE;
	
		if( (i = args.find("port")) == args.end() )
			msg->port_no = OFPP_ANY;
		else
			msg->port_no = (uint32_t)atoi(i->second.c_str());
		if( (i = args.find("queue_id")) == args.end() )
			msg->queue_id = OFPQ_ALL;
		else
			msg->queue_id = (uint32_t)atoi(i->second.c_str());

	
		return (ofl_msg_header*)msg;
	}
	
	Inter_queue_stats::Inter_queue_stats()
	{ 
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("port"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("queue_id"),e_Num) );
	}
	//=======================================
	builder_name Inter_group_stats::name()
	{		
		return "group_stats";
	}
	
	bool Inter_group_stats::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_group_stats::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_request_group*  msg = new ofl_msg_multipart_request_group;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_GROUP;
	
		if( (i = args.find("group_id")) == args.end() )
			msg->group_id = OFPG_ALL;
		else
			msg->group_id = (uint32_t)atoi(i->second.c_str());

	
		return (ofl_msg_header*)msg;
	}
	
	Inter_group_stats::Inter_group_stats()
	{ 
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("group_id"),e_Num) );
	}
	//======================================
	builder_name Inter_port_desc::name()
	{		
		return "port_desc";
	}
	
	bool Inter_port_desc::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_port_desc::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_request_header*  msg = new ofl_msg_multipart_request_header;
		msg->header.type = OFPT_MULTIPART_REQUEST;
		msg->type = OFPMP_PORT_DESC;

		return (ofl_msg_header*)msg;
	}
	//======================================
	builder_name Inter_group_desc::name()
	{		
		return "group_desc";
	}
	
	bool Inter_group_desc::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_group_desc::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_request_header*  msg = new ofl_msg_multipart_request_header;
		msg->header.type = OFPT_MULTIPART_REQUEST;
		msg->type = OFPMP_GROUP_DESC;

		return (ofl_msg_header*)msg;
	}
	//======================================
	builder_name Inter_group_features::name()
	{		
		return "group_features";
	}
	
	bool Inter_group_features::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_group_features::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_request_header*  msg = new ofl_msg_multipart_request_header;
		msg->header.type = OFPT_MULTIPART_REQUEST;
		msg->type = OFPMP_GROUP_FEATURES;

		return (ofl_msg_header*)msg;
	}
		//======================================
	Inter_queue_config::Inter_queue_config()
	{
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("port"),e_Num) );	
	}
	
	builder_name Inter_queue_config::name()
	{		
		return "queue_config";
	}
	
	bool Inter_queue_config::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_queue_config::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_queue_get_config_request*  msg = new ofl_msg_queue_get_config_request;
		msg->header.type= OFPT_QUEUE_GET_CONFIG_REQUEST;

		if( (i = args.find("port") ) == args.end() )
			msg->port = OFPP_ANY;
		else
			msg->port = (uint32_t)atoi( i->second.c_str() );
		
		return (ofl_msg_header*)msg;
	}
	
	//======================================
	Inter_meter_stats::Inter_meter_stats()
	{
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("meter_id"),e_Num) );	
	}
	
	builder_name Inter_meter_stats::name()
	{		
		return "meter_stats";
	}
	
	bool Inter_meter_stats::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_meter_stats::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_meter_request*  msg = new ofl_msg_multipart_meter_request;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_METER;

		if( (i = args.find("meter_id") ) == args.end() )
			msg->meter_id = OFPM_ALL;
		else
			msg->meter_id = (uint32_t)atoi( i->second.c_str() );
		
		return (ofl_msg_header*)msg;
	}
		//======================================
	Inter_meter_config::Inter_meter_config()
	{
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("meter_id"),e_Num) );	
	}
	
	builder_name Inter_meter_config::name()
	{		
		return "meter_config";
	}
	
	bool Inter_meter_config::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_meter_config::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_meter_request*  msg = new ofl_msg_multipart_meter_request;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_METER_CONFIG;

		if( (i = args.find("meter_id") ) == args.end() )
			msg->meter_id = OFPM_ALL;
		else
			msg->meter_id = (uint32_t)atoi( i->second.c_str() );
		
		return (ofl_msg_header*)msg;
	}
		//======================================
	builder_name Inter_meter_features::name()
	{		
		return "meter_features";
	}
	
	bool Inter_meter_features::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_meter_features::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_multipart_request_header*  msg = new ofl_msg_multipart_request_header;
		msg->header.type = OFPT_MULTIPART_REQUEST;
		msg->type = OFPMP_METER_FEATURES;

		return (ofl_msg_header*)msg;
	}
	//======================================
	
	builder_name Inter_flow_mod::name() 
	{
		return "flow_mod";
	}
	
	bool Inter_flow_mod::is_modify() const
	{
		return true;
	}
	
	Inter_flow_mod::Inter_flow_mod()
	{
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("command"),e_String) );
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("instr_type"),e_String) );
		
		_addit_args = instrBuilder.get_instr_args();
		_addit_args.insert(_addit_args.end(),matchBuilder.get_match_args().begin(),
											 matchBuilder.get_match_args().end());
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("cookie"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("cookie_mask"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("table_id"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("idle_timeout"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("hard_timeout"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("priority"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("buffer_id"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("flags"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("out_port"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("out_group"),e_Num) );
	}
	
	struct ofl_msg_header* Inter_flow_mod::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		uint64_t cookie;
		uint64_t cookie_mask;
		uint8_t table_id;
		enum ofp_flow_mod_command command;
		uint16_t idle_timeout;
		uint16_t hard_timeout;
		uint32_t out_port;
		uint16_t flags;
		uint16_t priority;
		uint32_t buffer_id;
		uint32_t out_group;
		
		if( (i = args.find("cookie") ) == args.end() )
			cookie = 0;
		else
			cookie = (uint64_t)atoll( i->second.c_str() );
		if( (i = args.find("cookie_mask") ) == args.end() )
			cookie_mask = 0;
		else
			cookie_mask = (uint64_t)atoll( i->second.c_str() );	
		if( (i = args.find("table_id") ) == args.end() )
			table_id = OFPTT_ALL;
		else
			table_id = (uint8_t)atoi(i->second.c_str());
		
		std::string command_type = args.find("command")->second;
		if(command_type  == "add")
			command = OFPFC_ADD;
		else if(command_type == "modify")
				command = OFPFC_MODIFY;
		else if(command_type == "modify_strict")
				command = OFPFC_MODIFY_STRICT;
		else if(command_type == "delete")
				command = OFPFC_DELETE;
		else if(command_type == "delete_strict")
				command = OFPFC_DELETE_STRICT;
		else
			command = OFPFC_DELETE;
		
		if( (i = args.find("idle_timeout") ) == args.end() )
			idle_timeout = 0;
		else
			idle_timeout = (uint16_t)atoi(i->second.c_str());
		if( (i = args.find("hard_timeout") ) == args.end() )
			hard_timeout = 0;
		else
			hard_timeout = (uint16_t)atoi(i->second.c_str());
		if( (i = args.find("out_port") ) == args.end() )
			out_port = OFPP_ANY;
		else
			out_port = (uint32_t)atoi(i->second.c_str());
		if( (i = args.find("out_group") ) == args.end())
			out_group = OFPG_ANY;
		else	
			out_group = (uint32_t)atoi(i->second.c_str());
		if( (i = args.find("flags") ) == args.end())
			flags = 0;
		else	
			flags = (uint16_t)atoi(i->second.c_str());
		if( (i = args.find("priority") ) == args.end())
			priority = 0;
		else	
			priority = (uint16_t)atoi(i->second.c_str());
		if( (i = args.find("buffer_id") ) == args.end())
			buffer_id = 0;
		else	
			buffer_id = (uint32_t)atoi(i->second.c_str());
		
		FlowMod *mod = new FlowMod(cookie,cookie_mask, table_id,command, idle_timeout, hard_timeout, priority,
							buffer_id, out_port, out_group , flags);
		mod->AddMatch( (struct ofl_match*)matchBuilder.construct_match(args) );
		mod->AddInstructions( instrBuilder.construct_instruction(args) );

		return (ofl_msg_header*)&mod->fm_msg;
	}
		//======================================
	
	builder_name Inter_group_mod::name() 
	{
		return "group_mod";
	}
	
	bool Inter_group_mod::is_modify() const
	{
		return true;
	}
	
	Inter_group_mod::Inter_group_mod()
	{
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("command"),e_String) );
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("group_type"),e_String) );
		
		_addit_args = actionBuilder.get_action_args();
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("weight"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("watch_port"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("watch_group"),e_Num) );
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("group_id"),e_Num) );
	}
	
	struct ofl_msg_header* Inter_group_mod::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_group_mod *msg = new ofl_msg_group_mod;
		struct ofl_bucket **bucket = new ofl_bucket*[1];
		bucket[0] = new ofl_bucket;
		
		enum ofp_group_mod_command command;
		uint8_t type;
		uint32_t group_id;
		uint16_t weight;
		uint32_t watch_port;
		uint32_t watch_group;
		
		std::string value = args.find("command")->second;
		
		if(value  == "add")
			command = OFPGC_ADD;
		else if(value == "modify")
				command = OFPGC_MODIFY;
		else if(value == "delete")
				command = OFPGC_DELETE;
		else
			std::runtime_error("Invalid command, must be add, modify or delete!\n");
			
		value = args.find("group_type")->second;
		
		if(value  == "all")
			type = OFPGT_ALL;
		else if(value == "select")
				type = OFPGT_SELECT;
		else if(value == "indirect")
				type = OFPGT_INDIRECT;
		else if(value == "fast_failover")
				type = OFPGT_FF;
		else
			std::runtime_error("Invalid group_type!\n");
		
		if( (i = args.find("group_id") ) == args.end() )
			group_id = 0;
		else
			group_id = (uint32_t)atoi( i->second.c_str() );
		if( (i = args.find("weight") ) == args.end() )
			weight = 0;
		else
			weight = (uint16_t)atoi( i->second.c_str() );	
		if( (i = args.find("watch_port") ) == args.end() )
			watch_port = 0;
		else
			watch_port = (uint32_t)atoi(i->second.c_str());
		if( (i = args.find("watch_group") ) == args.end() )
			watch_group = 0;
		else
			watch_group = (uint32_t)atoi(i->second.c_str());

		Actions * acts = actionBuilder.construct_action(args);
		(*bucket)->actions = acts->acts;
		(*bucket)->actions_num = acts->act_num;
		(*bucket)->weight = weight;
		(*bucket)->watch_port = watch_port;
		(*bucket)->watch_group = watch_group;
		
		msg->header.type = OFPT_GROUP_MOD;
		msg->buckets = bucket;
		msg->buckets_num = 1;
		msg->command = command;
		msg->type = type;
		msg->group_id = group_id;

		return (ofl_msg_header*)msg;
	}
	//======================================
	
	builder_name Inter_port_mod::name() 
	{
		return "port_mod";
	}
	
	bool Inter_port_mod::is_modify() const
	{
		return true;
	}
	
	Inter_port_mod::Inter_port_mod()
	{
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("port"),e_Num) );
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("hw_addr"),e_MAC) );
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("config"),e_Num) );
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("advertise"),e_Num) );
		
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("mask"),e_Num) );

	}
	
	struct ofl_msg_header* Inter_port_mod::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_port_mod *msg = new ofl_msg_port_mod;
	
		uint32_t advertise = (uint32_t)atoi( args.find("advertise")->second.c_str() );
		uint32_t port_no = (uint32_t)atoi( args.find("port")->second.c_str() );
		uint32_t config = (uint32_t)atoi( args.find("config")->second.c_str() );
		uint32_t mask;
		
		if( (i = args.find("mask") ) == args.end() )
			mask = 0xffffffff;
		else
			mask = (uint32_t)atoi( i->second.c_str() );
		
		int len = ETH_ADDR_LEN;
		ethernetaddr addr = ethernetaddr(args.find("hw_addr")->second);
		memcpy(msg->hw_addr,addr.octet,len);
		
		msg->header.type = OFPT_PORT_MOD;
		msg->advertise = advertise;
		msg->port_no = port_no;
		msg->config = config;
		msg->mask = mask;
		
		return (ofl_msg_header*)msg;
	}
	//======================================
	
	builder_name Inter_meter_mod::name() 
	{
		return "meter_mod";
	}
	
	bool Inter_meter_mod::is_modify() const
	{
		return true;
	}
	
	Inter_meter_mod::Inter_meter_mod()
	{
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("command"),e_String) );
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("flags"),e_Num) );
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("meter_id"),e_String) );
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("meter_type"),e_String) );
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("rate"),e_Num) );
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("burst_size"),e_Num) );
		
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("prec_level"),e_Num) );

	}
	
	struct ofl_msg_header* Inter_meter_mod::request_msg_creator(const request_arguments& args)
	{
		check_args(args);
		
		request_arguments::const_iterator i;
		
		struct ofl_msg_meter_mod *msg = new ofl_msg_meter_mod;
	
		uint16_t command;
		uint16_t flags;
		uint16_t meter_type;
		uint32_t meter_id;
		uint32_t rate;
		uint32_t burst_size;
		uint8_t prec_level; 
		
		std::string value = args.find("command")->second;
		
		if(value  == "add")
				command = OFPMC_ADD;
		else if(value == "modify")
				command = OFPMC_MODIFY;
		else if(value == "delete")
				command = OFPMC_DELETE;
		else
			std::runtime_error("Invalid command!\n");
			
		flags =  (uint16_t)atoi( args.find("flags")->second.c_str() );
		rate = (uint32_t)atoi( args.find("rate")->second.c_str() );
		burst_size = (uint32_t)atoi( args.find("burst_size")->second.c_str() );
		
		value = args.find("meter_id")->second;
		
		if(value  == "slowpath")
				meter_id = OFPM_SLOWPATH;
		else if(value == "controller")
				meter_id = OFPM_CONTROLLER;
		else if(value == "all")
				meter_id = OFPM_ALL;
		else
			meter_id = (uint32_t)atoi( value.c_str() ); // todo
			
		value = args.find("meter_type")->second;
		
		if(value  == "drop")
				meter_type = OFPMBT_DROP;
		else if(value == "dscp_remark")
				meter_type = OFPMBT_DSCP_REMARK;
		// future planing
		//else if(value == "experementer")
				//type = OFPMBT_EXPERIMENTER;
		else
			std::runtime_error("Invalid meter_type field!\n");
		
		if( (i = args.find("prec_level") ) == args.end() )
			prec_level = 0;
		else
			prec_level = (uint8_t)atoi( i->second.c_str() );
		
		struct ofl_meter_band_header ** band = new ofl_meter_band_header*[1];
	
		switch(meter_type)
		{
			case OFPMBT_DROP:
			{
				band[0] = (ofl_meter_band_header*) new ofl_meter_band_drop;
				break;
			}
			case OFPMBT_DSCP_REMARK:
			{
				band[0] = (ofl_meter_band_header*) new ofl_meter_band_dscp_remark;
				((struct ofl_meter_band_dscp_remark*)band[0])->prec_level = prec_level;
				break;
			}
		};
		band[0]->burst_size = burst_size;
		band[0]->rate = rate;
		band[0]->type = meter_type;
		
		msg->bands = band;
		msg->command = command;
		msg->header.type = OFPT_METER_MOD;
		msg->flags = flags;
		msg->meter_bands_num = 1;
		msg->meter_id = meter_id;
		
		return (ofl_msg_header*)msg;
	}
};
