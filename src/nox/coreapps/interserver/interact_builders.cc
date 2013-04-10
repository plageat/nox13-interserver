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
#include "json-util.hh"	
	
#define FIND_AVALUE(args, s)  ( (json::get_dict_value( (args), (s) )->get_string(true) ).c_str() )
#define EXISTS_AKEY(args, s) ( ( json::get_dict_value( (args), (s) ) ) != NULL )	
#define FIND_IF_EXISTS_ALL(args,s,key,value) ( ( EXISTS_AKEY( (args) ,(s)) )  &&  \
		( (value) = FIND_AVALUE( (args), (s) ), (key) = (s), true ) )
#define FIND_IF_EXISTS(args,s,value) ( ( EXISTS_AKEY( (args) ,(s)) )  &&  \
		( (value) = FIND_AVALUE( (args), (s) ), true ) )

namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
 
	static Vlog_module lg("inter_builders");
	
	
	typedef std::map<std::string,std::pair<int,int> > match_hash;
	
	namespace
	{
		arguments_list construct_action_tags_ext()
		{
			arguments_list action_tags;
			
			BOOST_FOREACH(const match_hash::value_type& p, fields)
			{
				switch(p.second.first)
				{
					case OXM_OF_IPV4_SRC:
					case OXM_OF_IPV4_DST:
					{
						action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_Ipv4) );
						action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_Ipv4) );
						break;
					};
					case OXM_OF_ETH_SRC:
					case OXM_OF_ETH_DST:
					{
						action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_MAC) );
						action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_MAC) );
						break;
					}
					case OXM_OF_IPV6_SRC:
					case OXM_OF_IPV6_DST:
					case OXM_OF_IPV6_ND_TARGET:
					case OXM_OF_IPV6_ND_TLL:
					case OXM_OF_IPV6_ND_SLL:
					{
						action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_Ipv6) );
						action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_Ipv6) );
						break;
					}
					default:
					{
						action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_Num) );
						action_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_Num) );
						break;
					}
				};
			}
			
			return action_tags;
		}
	}
	
	arguments_list Action_fields_builder::_action_tags = boost::assign::map_list_of ("actions_set",e_String)("act_out_port",e_String)
						("act_queue_id",e_Num)("act_group_id",e_Num)("tag_type",e_String)
						("ethertype",e_Num)("ttl_action",e_String)("ttl",e_Num)("ttl_type",e_String)
						("action_type",e_String);
	
	arguments_list Action_fields_builder::_action_tags_ext = construct_action_tags_ext();
	
	
	arguments_list Action_fields_builder::get_action_args()
	{
		arguments_list ret_val = _action_tags;
		ret_val.insert( ret_val.end(), _action_tags_ext.begin(), _action_tags_ext.end() );
		
		return _action_tags;
	}
	
	Actions* Action_fields_builder::construct_action(const json_object* args, Actions* acts)
	{
		std::cout << "Entering construct actions..."<< std::endl;
		
		std::string key,value,value_mask;

		if( FIND_IF_EXISTS(args,"action_type",value) == false )
			throw std::runtime_error("missing action_type field\n");
			
		if( value == "output" )
		{
			if( FIND_IF_EXISTS(args,"out_port",value) == false )
				throw std::runtime_error("missing out_port argument in action output!\n");
			
			uint32_t port = 0;
			
			if( value == "in_port" )
					port = OFPP_IN_PORT;
			else if( value == "table" )
					port = OFPP_TABLE;
			else if( value == "normal" )
					port = OFPP_NORMAL;
			else if( value == "flood" )
					port = OFPP_FLOOD;
			else if( value == "controller" )
					port = OFPP_CONTROLLER;
			else if( value == "local" )
					port = OFPP_LOCAL;
			else
				port = (uint32_t) atoi( value.c_str() );
			
			acts->CreateOutput(port);
			std::cout << "Creating output action to port: OK!" <<std::endl;
					
			return acts;
		}
		
		if( value == "set_queue" )
		{
			if( FIND_IF_EXISTS(args,"queue_id",value) == false )
				throw std::runtime_error("missing queue_id argument in action set_queue!\n");
					
			uint32_t q_id = (uint32_t) atoi( value.c_str() );
			
			acts->CreateSetQueue(q_id);
					
			return acts;
		}
		
		if( value == "group" )
		{
			if( FIND_IF_EXISTS(args,"group_id",value) == false )
				throw std::runtime_error("missing act_group_id argument in action group!\n");
					
			uint32_t group = (uint32_t) atoi( value.c_str() );
			
			acts->CreateGroupAction(group);
					
			return acts;
		}
		
		if( value == "push" )
		{
			std::string tt,et;
			
			if( FIND_IF_EXISTS(args,"tag_type",tt) == false )
				throw std::runtime_error("missing tag_type argument in action push!\n");
					
			if( FIND_IF_EXISTS(args,"ethertype",et) == false )
				throw std::runtime_error("missing ethertype argument in action push!\n");
			
			enum ofp_action_type type; 
			uint16_t ethertype = (uint16_t) atoi( et.c_str() );
			
			if( tt == "vlan")
				type = OFPAT_PUSH_VLAN;
			else if(tt == "mpls")
				type = OFPAT_PUSH_MPLS;
			else if(tt == "pbb")
				type = OFPAT_PUSH_PBB;
			else 
				throw std::runtime_error("invalid tag_type in action push!\n");
			
			acts->CreatePushAction(type,ethertype);
			
			std::cout << "Creating push action : OK!" <<std::endl;
			
			return acts;
		}
		
		if( value == "pop" )
		{
			std::string tt;
			
			if( FIND_IF_EXISTS(args,"tag_type",tt) == false )
				throw std::runtime_error("missing tag_type argument in action pop!\n");
			
			if( tt == "vlan")
				acts->CreatePopVlan();
			else if(tt == "mpls")
			{
				std::string et;
				if( FIND_IF_EXISTS(args,"ethertype",et) == false )
					throw std::runtime_error("missing ethertype argument in action pop!\n");
			
				uint16_t ethertype = (uint16_t) atoi( et.c_str() );
				acts->CreatePopMpls(ethertype);
			}
			else 
				throw std::runtime_error("invalid tag_type in action pop!\n");

			return acts;
		}
		
		if( value == "change_ttl" )
		{
			std::string ta;
			
			if( FIND_IF_EXISTS(args,"ttl_action",ta) == false )
				throw std::runtime_error("missing ttl_action argument in action change ttl!\n");
			
			if( ta == "set")
			{
				std::string ttl_str;
				
				if( FIND_IF_EXISTS(args,"ttl",ttl_str) ==  false )
					throw std::runtime_error("missing ttl argument in action change ttl!\n");
				
				uint8_t ttl = (uint8_t) atoi( ttl_str.c_str() );
				
				std::string ttl_type_str;
				if( FIND_IF_EXISTS(args,"ttl_type",ttl_type_str) == false )
					throw std::runtime_error("missing ttl_type argument in action change ttl!\n");
				
				if( ttl_type_str == "ip" )
					acts->CreateSetNwTTL(ttl);
				else if ( ttl_type_str == "mpls" )
					acts->CreateSetMplsTTL(ttl);
				else
					throw std::runtime_error("invalid ttl_type in action chage ttl!\n");
				
				return acts;
			}
			else if(ta == "decrement")
			{
				enum ofp_action_type type;
				
				std::string ttl_type_str;
				if( FIND_IF_EXISTS(args,"ttl_type",ttl_type_str) == false )
					throw std::runtime_error("missing ttl_type argument in action change ttl!\n");
				
				if( ttl_type_str == "ip" )
					type = OFPAT_DEC_NW_TTL;
				else if ( ttl_type_str == "mpls" )
					type = OFPAT_DEC_MPLS_TTL;
				else
					throw std::runtime_error("invalid ttl_type in action change ttl!\n");

				acts->CreateDecTTL(type);
				
				return acts;
			}
			else if (ta == "copy")
			{
				enum ofp_action_type type;
				
				std::string ttl_type_str;
				if( FIND_IF_EXISTS(args,"ttl_type",ttl_type_str) == false )
					throw std::runtime_error("missing ttl_type argument in action change ttl!\n");
				
				if( ttl_type_str == "in" )
					type = OFPAT_COPY_TTL_IN;
				else if (ttl_type_str == "out")
					type = OFPAT_COPY_TTL_OUT;
				else
					throw std::runtime_error("invalid ttl_type in action change ttl!\n");

				acts->CreateCopyTTL(type);
					
				return acts;	
			}
			else
				throw std::runtime_error("invalid ttl_action!\n");
		}
		
		
		if( value == "set_field" )
		{
			// :-( 
			
			if( FIND_IF_EXISTS_ALL(args,"ipv4_src",key,value) || 
				FIND_IF_EXISTS_ALL(args,"ipv4_dst",key,value) )
			{
				struct in_addr* addr = new in_addr;
				inet_pton(AF_INET, value.c_str(), addr);
				acts->CreateSetField(key, addr);
					
				return acts;
			}
			if( FIND_IF_EXISTS_ALL(args, "ipv6_src", key, value) ||
				FIND_IF_EXISTS_ALL(args, "ipv6_dst" ,key, value) ||
				FIND_IF_EXISTS_ALL(args, "ipv6_nd_target", key, value) ||
				FIND_IF_EXISTS_ALL(args, "ipv6_nd_tll", key, value) ||
				FIND_IF_EXISTS_ALL(args, "ipv6_nd_sll", key,value) )
			{
				struct in6_addr* addr = new in6_addr;
				inet_pton(AF_INET6, value.c_str(), addr);
				acts->CreateSetField(key, addr);
				
				return acts;
			}
			if( FIND_IF_EXISTS_ALL(args, "eth_dst", key, value) |\
				FIND_IF_EXISTS_ALL(args, "eth_src", key, value) )
			{	
				ethernetaddr* addr = new  ethernetaddr(value);
				acts->CreateSetField(key, addr->octet);
				return acts;
			}
			
			BOOST_FOREACH(const arguments_list::value_type& p, _action_tags_ext)
			{
				if( FIND_IF_EXISTS_ALL(args, p.first, key, value) == false )
					continue;
				else
				{
					match_hash::const_iterator it = fields.find( key );
					
					switch(it->second.second)
					{
						case 1:
						{
							uint8_t* value_b = (uint8_t*)xmalloc( sizeof (uint8_t) ) ;
							*value_b = (uint8_t)atoi( value.c_str() );
							acts->CreateSetField(key, value_b);
						
							return acts;
						}
						case 2:
						{
							uint16_t* value_b = (uint16_t*)xmalloc( sizeof (uint16_t ) ); 
							*value_b = (uint16_t)atoi( value.c_str() );
							acts->CreateSetField(key, value_b);
							
							return acts;
						}
						case 4:
						{
							uint32_t *value_b = (uint32_t*)xmalloc( sizeof (uint32_t ) ); 
							*value_b = (uint32_t)atoi( value.c_str() );
							acts->CreateSetField(key, value_b);
						
							return acts;
						}
						case 8:
						{
							uint64_t* value_b = (uint64_t*)xmalloc( sizeof (uint64_t ) );  
							*value_b = (uint64_t)atoll( value.c_str() );
							acts->CreateSetField(key, value_b);
							
							return acts;
						}
					};
				}
			}
		}
		// empty acts = drop action
		
		return acts;
	}
	
	//=============================================================================

	arguments_list Instruction_fields_builder::_instr_tags = boost::assign::map_list_of ("instr_type",e_String)
									("goto_id", e_Num) ("metadata",e_Num) ("metadata_mask",e_Num)("meter",e_Num)
									("action_set",e_String);
			
	arguments_list Instruction_fields_builder::get_instr_args()
	{
		// todo
		// instructions includes action tags as them part of instructions
		arguments_list ret_val = Action_fields_builder::get_action_args();
		ret_val.insert( ret_val.begin(), _instr_tags.begin(), _instr_tags.end() );
		
		return ret_val;
	}
	
	
	Instruction* Instruction_fields_builder::construct_instruction(const json_object* args, Instruction* instr)
	{
		std::cout << "Entering construct instructions..."<< std::endl;
		
		std::string value,value_mask;
		
		// required argument, must be set in builder constructor
		if( FIND_IF_EXISTS(args,"instr_type",value) == false)
			throw std::runtime_error("missing instr_type field\n");
	
		if(value == "goto_table" )
		{
			std::cout << "Entering construct goto_table instruction..."<< std::endl;

			if( FIND_IF_EXISTS(args,"goto_id",value) == false )
				throw std::runtime_error("missing goto_id argument\n");
		
			uint8_t id = (uint8_t) atoi( value.c_str() );
			
			instr->CreateGoToTable(id);
			
			return instr;
		}
		
		if(value == "write_metadata")
		{
			std::cout << "Entering construct write_metadata instruction..."<< std::endl;

			if( FIND_IF_EXISTS(args,"metadata",value) == false )
				throw std::runtime_error("missing metadata field");
				
			uint64_t md = (uint64_t) atoll( value.c_str() );
			
			if( FIND_IF_EXISTS(args,"metadata_mask",value_mask) == false )
				instr->CreateWriteMetadata(md);
			else
			{
				uint64_t md_mask = (uint64_t) atoll( value_mask.c_str() );
				instr->CreateWriteMetadata(md,md_mask);
			}
			
			return instr;
		}
		
		if( value == "write_actions" )
		{
			std::cout<< "Calling apply actions" << std::endl;
			
			Actions *ac = new Actions();
			if( FIND_IF_EXISTS(args,"action_set",value) )
			{
				ssize_t len = value.size();
				json_object* a = new json_object((const uint8_t*)value.c_str(),len);
				json_array* ar = (json_array*)a->object;
				BOOST_FOREACH(const json_array::value_type& p, *ar)
				{
					ac =  Action_fields_builder::construct_action(p,ac);
				}
			}
			instr->CreateWrite( ac );
			
			return instr;
		}
		
		if( value == "apply_actions" )
		{
			std::cout<< "Calling apply actions" << std::endl;
			
			Actions *ac = new Actions();
			if( FIND_IF_EXISTS(args,"action_set",value) )
			{
				ssize_t len = value.size();
				json_object* a = new json_object((const uint8_t*)value.c_str(),len);
				json_array* ar = (json_array*)a->object;
				BOOST_FOREACH(const json_array::value_type& p, *ar)
				{
					ac =  Action_fields_builder::construct_action(p,ac);
				}
			}
			instr->CreateApply( ac );
			
			return instr;
		}
		
		if( value == "clear_actions" )
		{
			instr->CreateClearActions();
			
			return instr;
		}
		
		if( value == "meter")
		{
			if( FIND_IF_EXISTS(args,"meter",value) == false )
				throw std::runtime_error("missing meter field");
				
			uint32_t id = (uint32_t) atoi( value.c_str() );
			
			instr->CreateMeterInstruction(id);
			
			return instr;
		}
	
		return instr;
	}
	
	//=============================================================================
	
	arguments_list Bucket_fields_builder::_bucket_tags = boost::assign::map_list_of ("weight",e_Num)
														("watch_port",e_Num) ("watch_group",e_Num);
	
	arguments_list Bucket_fields_builder::get_bucket_args()
	{
		// need optimization!
		arguments_list ret_val = Action_fields_builder::get_action_args();
		ret_val.insert( ret_val.begin(), _bucket_tags.begin(), _bucket_tags.end() );
		
		return ret_val;
	}
	
	struct ofl_bucket * Bucket_fields_builder::construct_bucket(const json_object* args,struct ofl_bucket * buck)
	{
		std::string value;
		
		uint16_t weight;
		uint32_t watch_port;
		uint32_t watch_group;
		
		if( FIND_IF_EXISTS(args,"weight", value) == false )
			weight = 0;
		else
			weight = (uint16_t)atoi( value.c_str() );	
		if( FIND_IF_EXISTS(args,"watch_port", value) == false )
			watch_port = 0;
		else
			watch_port = (uint32_t)atoi( value.c_str() );
		if( FIND_IF_EXISTS(args,"watch_group", value) == false)
			watch_group = 0;
		else
			watch_group = (uint32_t)atoi( value.c_str() );
			
		Actions *ac = new Actions();
		if( FIND_IF_EXISTS(args,"action_set",value) )
		{
			ssize_t len = value.size();
			json_object* a = new json_object((const uint8_t*)value.c_str(),len);
			json_array* ar = (json_array*)a->object;
			BOOST_FOREACH(const json_array::value_type& p, *ar)
			{
				ac =  Action_fields_builder::construct_action(p,ac);
			}
		}
		
		buck->actions = ac->acts;
		buck->actions_num = ac->act_num;
		buck->watch_group = watch_group;
		buck->watch_port = watch_port;
		buck->weight = weight;
		
		return buck;
	}
	
	//=============================================================================
	
	arguments_list Meter_band_builder::_band_tags = boost::assign::map_list_of ("meter_type",e_String)
				("rate",e_Num) ("burst_size",e_Num) ("prec_level",e_Num);
	
	arguments_list Meter_band_builder::get_band_args()
	{
		return _band_tags;
	}
	
	struct ofl_meter_band_header * Meter_band_builder::construct_band(const json_object* args)
	{
		std::string value;
		
		struct ofl_meter_band_header *band = NULL;
	
		uint16_t meter_type;
		uint32_t rate;
		uint32_t burst_size;
		uint8_t prec_level; 

		if( FIND_IF_EXISTS(args,"rate",value) )
			rate = (uint32_t)atoi( value.c_str() );
		else
			std::runtime_error("missing rate field\n");
			
		if( FIND_IF_EXISTS(args, "burst_size", value) )
			burst_size = (uint32_t)atoi( value.c_str() );
		else
			std::runtime_error("missing burst_size field\n");
	
		if( FIND_IF_EXISTS(args, "meter_type", value) == false)
			std::runtime_error("missing meter_type field\n");

		if(value  == "drop")
				meter_type = OFPMBT_DROP;
		else if(value == "dscp_remark")
				meter_type = OFPMBT_DSCP_REMARK;
		else
			std::runtime_error("Invalid meter_type field!\n");
	
		switch(meter_type)
		{
			case OFPMBT_DROP:
			{
				band = (struct ofl_meter_band_header*) xmalloc( sizeof (ofl_meter_band_drop) );
				break;
			}
			case OFPMBT_DSCP_REMARK:
			{
				if( FIND_IF_EXISTS(args,"prec_level",value) )
					prec_level = (uint8_t)atoi( value.c_str() );
				else
					std::runtime_error("missing prec_level field\n");
					
				band = (struct ofl_meter_band_header*) xmalloc( sizeof (ofl_meter_band_dscp_remark) );
				
				((struct ofl_meter_band_dscp_remark*)band)->prec_level = prec_level;
				break;
			}
		};
		
		band->burst_size = burst_size;
		band->rate = rate;
		band->type = meter_type;
		
		return band;
	}
	
	//=============================================================================
	
	namespace
	{
		arguments_list construct_match_tags_ext()
		{
			arguments_list match_tags;
			
		
			BOOST_FOREACH(const match_hash::value_type& p, fields)
			{
				switch(p.second.first)
				{
					case OXM_OF_IPV4_SRC:
					case OXM_OF_IPV4_DST:
					case OXM_OF_ETH_SRC:
					case OXM_OF_ETH_DST:
					case OXM_OF_IPV6_SRC:
					case OXM_OF_IPV6_DST:
					case OXM_OF_IPV6_ND_TARGET:
					case OXM_OF_IPV6_ND_TLL:
					case OXM_OF_IPV6_ND_SLL:
									{ break; }
					/*
					case OXM_OF_METADATA:
					{
						match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_String) );
						match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_String) );
						break;
					}*/
					default:
					{
						match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_Num) );
						//match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_Num) );
						break;
					}
				};
			}
			
			return match_tags;
		}
		
		arguments_list construct_match_tags()
		{
			arguments_list match_tags;
		
			BOOST_FOREACH(const match_hash::value_type& p, fields)
			{
				switch(p.second.first)
				{
					case OXM_OF_IPV4_SRC:
					case OXM_OF_IPV4_DST:
					{
						match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_Ipv4) );
						match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_Ipv4) );
						break;
					};
					case OXM_OF_ETH_SRC:
					case OXM_OF_ETH_DST:
					{
						match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_MAC) );
						match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_MAC) );
						break;
					}
					case OXM_OF_IPV6_SRC:
					case OXM_OF_IPV6_DST:
					case OXM_OF_IPV6_ND_TARGET:
					case OXM_OF_IPV6_ND_TLL:
					case OXM_OF_IPV6_ND_SLL:
					{
						match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_Ipv6) );
						match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_Ipv6) );
						break;
					}
					default:
					{
					//	match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first,e_Num) );
						match_tags.push_back( std::pair<std::string, enum e_arg_type_t>(p.first + "_mask",e_Num) );
						break;
					}
				};
				
			}
			
			return match_tags;
		}
	}
	
	arguments_list Match_fields_builder::_match_tags = construct_match_tags();
	arguments_list Match_fields_builder::_match_tags_ext = construct_match_tags_ext();
	
	arguments_list Match_fields_builder::get_match_args()
	{
		// so slowooooooooowly...todo
		arguments_list ret_val;

		ret_val.insert(ret_val.end(), _match_tags.begin(),_match_tags.end());
		ret_val.insert(ret_val.end(), _match_tags_ext.begin(),_match_tags_ext.end());
		
		return ret_val;
	}
	
	Flow* Match_fields_builder::construct_match(const json_object* args, Flow* f)
	{
		std::cout << "Entering match constructor..." << std::endl;
		// expecting checked arguments
		//Flow *f;
		//if(m)
		//	f = new Flow((const struct ofl_match*)m);	// this SUPER! bug i thing, look into constructor
		//else
		//	f = new Flow();
		std::string key,value, value_mask;
		// we have problems with maskeble matches..
		if( FIND_IF_EXISTS_ALL(args, "ipv4_src", key, value) || 
			FIND_IF_EXISTS_ALL(args, "ipv4_dst", key, value) )
		{
			struct in_addr addr;
			inet_pton(AF_INET,value.c_str(),&addr);
				
			if( FIND_IF_EXISTS(args, key + "_mask",value_mask)  == false )
				f->Add_Field(key, addr);
			else
			{
				std::cout <<"Working masked ip match builder..." << std::endl;
				
				struct in_addr addr_mask;
				inet_pton(AF_INET, value_mask.c_str() ,&addr_mask);
				//std::cout << "Values in hmap : " << f->match.match_fields.n << std::endl;
				f->Add_Field(key, addr, addr_mask);
				//std::cout << "Values in hmap : " << f->match.match_fields.n << std::endl;
				//std::cout << ofl_structs_match_to_string((ofl_match_header*)&f->match,NULL);
			}
		}
		if( FIND_IF_EXISTS_ALL(args,"ipv6_src",key,value) ||
			FIND_IF_EXISTS_ALL(args,"ipv6_dst",key,value) ||
			FIND_IF_EXISTS_ALL(args,"ipv6_nd_target",key,value) ||
			FIND_IF_EXISTS_ALL(args,"ipv6_nd_tll",key,value) ||
			FIND_IF_EXISTS_ALL(args,"ipv6_nd_sll",key,value) )
		{
			struct in6_addr addr;
			inet_pton(AF_INET6,value.c_str(),&addr);
					
			if( FIND_IF_EXISTS(args,value + "_mask",value_mask) == false )
				f->Add_Field(key,addr);
			else
			{
				struct in6_addr addr_mask;
				inet_pton(AF_INET6,value_mask.c_str(),&addr_mask);
				f->Add_Field(key,addr,addr_mask);
			}
		}
		if( FIND_IF_EXISTS_ALL(args,"eth_dst",key,value) || 
			FIND_IF_EXISTS_ALL(args,"eth_src",key,value) )
		{
			if( FIND_IF_EXISTS(args,value + "_mask",value_mask) == false  )
				f->Add_Field(key,value);
			else
				f->Add_Field(key,value,value_mask);
		}
		// default
		// for non adress arguments: ports and which are simple numbers
		// need determine size correctly for call Add_fields method
		
		BOOST_FOREACH(const arguments_list::value_type p, _match_tags_ext)
		{
			if( FIND_IF_EXISTS_ALL(args,p.first,key,value) )
			{
				match_hash::const_iterator it = fields.find( key );
				
				switch(it->second.second)
				{
					case 1:
					{
						uint8_t value_b = (uint8_t)atoi(value.c_str());
						
						if(  FIND_IF_EXISTS(args,value + "_mask",value_mask) == false  )
							f->Add_Field(key,value_b);
						else
						{
							uint8_t value_mask_b = (uint8_t)atoi(value_mask.c_str());
							f->Add_Field(key,value_b,value_mask_b);
						}
						
						break;
					}
					case 2:
					{
						uint16_t value_b = (uint16_t)atoi(value.c_str());
						
						if( FIND_IF_EXISTS(args,value + "_mask",value_mask) == false  )
							f->Add_Field(key,value_b);
						else
						{
							std::cout << "Constructing 2 byte match "<< std::endl;
							uint16_t value_mask_b = (uint16_t)atoi(value_mask.c_str());
							f->Add_Field(key,value_b,value_mask_b);
						}
						
						break;
					}
					case 4:
					{
						uint32_t value_b = (uint32_t)atoi(value.c_str());
						
						if( FIND_IF_EXISTS(args,value + "_mask",value_mask) == false )
							f->Add_Field(key,value_b);
						else
						{
							uint32_t value_mask_b = (uint32_t)atoi(value_mask.c_str());
							f->Add_Field(key,value_b,value_mask_b);
						}
						
						break;
					}
					case 8:
					{
						std::cout << "Constructing match 8 byte: " << value << std::endl;
						uint64_t value_b = (uint64_t)atoll(value.c_str());
						
						if( FIND_IF_EXISTS(args,value + "_mask",value_mask) == false )
							f->Add_Field(key,value_b);
						else
						{
							uint64_t value_mask_b = (uint64_t)atoll(value_mask.c_str());
							f->Add_Field(key,value_b,value_mask_b);
						}
						
						break;
					}
				};
			}
				
		}
		
		return f;
	}
	
	
	//===========================================================
	
	struct ofl_msg_header * Inter_sw_config::request_msg_creator(const interact_args& args)
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
	
	struct ofl_msg_header * Inter_sw_config_setter::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		struct ofl_msg_set_config*  msg = new ofl_msg_set_config;
		msg->header.type = OFPT_SET_CONFIG;
		
		struct ofl_config  *config = new ofl_config;
		
		config->flags = (uint16_t) atoi( FIND_AVALUE(args.get(),"flags") );
		config->miss_send_len = (uint16_t) atoi ( FIND_AVALUE(args.get(),"max_len") );
		
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
	
	struct ofl_msg_header * Inter_features_request::request_msg_creator(const interact_args& args)
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
		
	struct ofl_msg_header * Inter_table_features::request_msg_creator(const interact_args& args)
	{
		struct ofl_msg_multipart_request_table_features*  msg = new ofl_msg_multipart_request_table_features;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_TABLE_FEATURES;
		// todo
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
	
	struct ofl_msg_header * Inter_desc::request_msg_creator(const interact_args& args)
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
	
	struct ofl_msg_header * Inter_table_stats::request_msg_creator(const interact_args& args)
	{
		struct ofl_msg_multipart_request_header*  msg = new ofl_msg_multipart_request_header;
		msg->header.type = OFPT_MULTIPART_REQUEST;
		msg->type = OFPMP_TABLE;
		
		return (ofl_msg_header*)msg;
	}
	//===========================================
	
	namespace 
	{
		struct ofl_msg_header* flow_information_req_creator(const interact_args& args,enum ofp_multipart_types type)
		{
			//check_args(args);
			std::string value;
	
			struct ofl_msg_multipart_request_flow*  msg = new ofl_msg_multipart_request_flow;
			msg->header.header.type = OFPT_MULTIPART_REQUEST;
			msg->header.type = type;
	
			if( FIND_IF_EXISTS(args.get(),"table_id",value) )
				msg->table_id = (uint8_t)atoi( value.c_str() );
			else
				msg->table_id = OFPTT_ALL;
			
			if( EXISTS_AKEY(args.get(),"out_port") )
				msg->out_port = (uint32_t)atoi( FIND_AVALUE(args.get(),"out_port") );
			else
				msg->out_port = OFPP_ANY;
			if( EXISTS_AKEY(args.get(),"out_group") )
				msg->out_group = (uint32_t)atoi( FIND_AVALUE(args.get(),"out_group") );
			else	
				msg->out_group = OFPG_ANY;
			if( EXISTS_AKEY(args.get(),"cookie") )
				msg->cookie = (uint64_t)atoll( FIND_AVALUE(args.get(),"cookie") );
			else
				msg->cookie = 0;
			if(	EXISTS_AKEY(args.get(),"cookie_mask")  )
				msg->cookie_mask = (uint64_t)atoll( FIND_AVALUE(args.get(),"cookie_mask") );
			else
				msg->cookie_mask = 0;
			
			Flow *f = new Flow();
			// flow info + match set causes switch working error
			// if switch has values flows for respond ; but sometimes occures
			if ( FIND_IF_EXISTS(args.get(),"match_set",value) )
			{
				ssize_t len = value.size();
				json_object* a = new json_object((const uint8_t*)value.c_str(),len);
				json_array* ar = (json_array*)a->object;
				
				msg->match = NULL;
				
				BOOST_FOREACH(const json_array::value_type& p, *ar)
				{
					f =  Match_fields_builder::construct_match(p,f);
				}
				msg->match = (struct ofl_match_header*)&(f->match);
			}
			else
			{
				msg->match = (struct ofl_match_header*)&(f->match);
			}
			
			return (ofl_msg_header*)msg;
		}
	}
	
	builder_name Inter_flow_info::name()
	{		
		return "flow_info";
	}
	
	bool Inter_flow_info::is_modify() const
	{
		return false;
	}
	
	struct ofl_msg_header * Inter_flow_info::request_msg_creator(const interact_args& args)
	{		
		//check_args(args);
		return flow_information_req_creator(args,OFPMP_FLOW);
	}
	
	Inter_flow_info::Inter_flow_info()
	{
		_addit_args = Match_fields_builder::get_match_args(); 
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("match_set"),e_Num) );
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
	
	struct ofl_msg_header * Inter_flow_agr_info::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		return flow_information_req_creator(args, OFPMP_AGGREGATE);
	}
	
	Inter_flow_agr_info::Inter_flow_agr_info()
	{
		_addit_args = Match_fields_builder::get_match_args(); 
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
	
	struct ofl_msg_header * Inter_port_stats::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		std::string port;
		
		struct ofl_msg_multipart_request_port*  msg = new ofl_msg_multipart_request_port;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_PORT_STATS;
	
		if( FIND_IF_EXISTS(args.get(), "port", port) == false )
			msg->port_no = OFPP_ANY;
		else
			msg->port_no = (uint32_t)atoi( port.c_str() );

	
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
	
	struct ofl_msg_header * Inter_queue_stats::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		std::string value;
		
		struct ofl_msg_multipart_request_queue*  msg = new ofl_msg_multipart_request_queue;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_QUEUE;
	
		if( FIND_IF_EXISTS(args.get(), "port", value ) == false )
			msg->port_no = OFPP_ANY;
		else
			msg->port_no = (uint32_t)atoi( value.c_str() );
		if( FIND_IF_EXISTS(args.get(), "queue_id", value ) == false )
			msg->queue_id = OFPQ_ALL;
		else
			msg->queue_id = (uint32_t)atoi( value.c_str() );

	
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
	
	struct ofl_msg_header * Inter_group_stats::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		std::string group_id;
		
		struct ofl_msg_multipart_request_group*  msg = new ofl_msg_multipart_request_group;
		msg->header.header.type = OFPT_MULTIPART_REQUEST;
		msg->header.type = OFPMP_GROUP;
	
		if( FIND_IF_EXISTS(args.get(), "group_id", group_id ) == false )
			msg->group_id = OFPG_ALL;
		else
			msg->group_id = (uint32_t)atoi( group_id.c_str() );

	
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
	
	struct ofl_msg_header * Inter_port_desc::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
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
	
	struct ofl_msg_header * Inter_group_desc::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
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
	
	struct ofl_msg_header * Inter_group_features::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
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
	
	struct ofl_msg_header * Inter_queue_config::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		std::string port;
		
		struct ofl_msg_queue_get_config_request*  msg = new ofl_msg_queue_get_config_request;
		msg->header.type= OFPT_QUEUE_GET_CONFIG_REQUEST;

		if( FIND_IF_EXISTS(args.get(), "port", port ) == false )
			msg->port = OFPP_ANY;
		else
			msg->port = (uint32_t)atoi( port.c_str() );
		
		return (ofl_msg_header*)msg;
	}
	
	//======================================
	
	namespace 
	{
		struct ofl_msg_header *	create_meter_request(const interact_args& args,enum ofp_multipart_types type )
		{
			std::string meter_id;
		
			struct ofl_msg_multipart_meter_request*  msg = new ofl_msg_multipart_meter_request;
			msg->header.header.type = OFPT_MULTIPART_REQUEST;
			msg->header.type = type;

			if( FIND_IF_EXISTS(args.get(),"meter_id",meter_id) == false )
				msg->meter_id = OFPM_ALL;
			else
				msg->meter_id = (uint32_t)atoi( meter_id.c_str() );
			
			return (ofl_msg_header*)msg;
		}
	}
	
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
	
	struct ofl_msg_header * Inter_meter_stats::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		return create_meter_request(args,OFPMP_METER);
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
	
	struct ofl_msg_header * Inter_meter_config::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		return create_meter_request(args,OFPMP_METER_CONFIG);
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
	
	struct ofl_msg_header * Inter_meter_features::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
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
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("instruction_set"),e_String) );
		
		_addit_args = Instruction_fields_builder::get_instr_args();
		arguments_list match_args = Match_fields_builder::get_match_args();
		_addit_args.insert(_addit_args.end(),match_args.begin(), match_args.end());
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("match_set"),e_String) );
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
	
	struct ofl_msg_header* Inter_flow_mod::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		std::string value;
		
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
		
		if( FIND_IF_EXISTS(args.get(),"cookie",value) == false)
			cookie = 0;
		else
			cookie = (uint64_t)atoll( value.c_str() );
		if( FIND_IF_EXISTS(args.get(),"cookie_mask",value) == false )
			cookie_mask = 0;
		else
			cookie_mask = (uint64_t)atoll( value.c_str() );	
		if( FIND_IF_EXISTS(args.get(),"table_id",value) == false )
			table_id = OFPTT_ALL;
		else
			table_id = (uint8_t)atoi(value.c_str());
		
		std::string command_type = FIND_AVALUE(args.get(),"command");
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
		
		if( FIND_IF_EXISTS(args.get(),"idle_timeout",value) == false )
			idle_timeout = 0;
		else
			idle_timeout = (uint16_t)atoi(value.c_str());
		if( FIND_IF_EXISTS(args.get(),"hard_timeout",value) == false )
			hard_timeout = 0;
		else
			hard_timeout = (uint16_t)atoi(value.c_str());
		if( FIND_IF_EXISTS(args.get(),"out_port",value) == false )
			out_port = OFPP_ANY;
		else
			out_port = (uint32_t)atoi(value.c_str());
		if( FIND_IF_EXISTS(args.get(),"out_group",value) == false)
			out_group = OFPG_ANY;
		else	
			out_group = (uint32_t)atoi(value.c_str());
		if( FIND_IF_EXISTS(args.get(),"flags",value) == false)
			flags = 0;
		else	
			flags = (uint16_t)atoi(value.c_str());
		if( FIND_IF_EXISTS(args.get(),"priority",value) == false)
			priority = 0;
		else	
			priority = (uint16_t)atoi(value.c_str());
		if( FIND_IF_EXISTS(args.get(),"buffer_id",value) == false)
			buffer_id = 0;
		else	
			buffer_id = (uint32_t)atoi( value.c_str() );
		
		FlowMod *mod = new FlowMod(cookie,cookie_mask, table_id,command, idle_timeout, hard_timeout, priority,
							buffer_id, out_port, out_group , flags);
		// match construction
		Flow *f = new Flow();
		if ( FIND_IF_EXISTS(args.get(),"match_set",value) )
		{
			ssize_t len = value.size();
			json_object* a = new json_object((const uint8_t*)value.c_str(),len);
			json_array* ar = (json_array*)a->object;
			BOOST_FOREACH(const json_array::value_type& p, *ar)
			{
				f =  Match_fields_builder::construct_match(p,f);
			}
		}
		mod->AddMatch(&(f->match));
		// instruction construction
		Instruction *i = new Instruction();
		if ( FIND_IF_EXISTS(args.get(),"instruction_set",value) )
		{
			ssize_t len = value.size();
			json_object* a = new json_object((const uint8_t*)value.c_str(),len);
			json_array* ar = (json_array*)a->object;
			BOOST_FOREACH(const json_array::value_type& p, *ar)
			{
				i =  Instruction_fields_builder::construct_instruction(p,i);
			}
		}
		mod->AddInstructions(i);

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
		_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("group_id"),e_Num) );
		
		_addit_args = Bucket_fields_builder::get_bucket_args();
		
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("buckets"),e_Num) );
		
	}
	
	struct ofl_msg_header* Inter_group_mod::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		std::string value;
		
		struct ofl_msg_group_mod *msg = new ofl_msg_group_mod;
		
		enum ofp_group_mod_command command;
		uint8_t type;
		uint32_t group_id;
		
		value = FIND_AVALUE(args.get(),"command"); 
		
		if( value  == "add")
			command = OFPGC_ADD;
		else if( value == "modify")
				command = OFPGC_MODIFY;
		else if( value == "delete")
				command = OFPGC_DELETE;
		else
			std::runtime_error("Invalid command, must be add, modify or delete!\n");
			
		value = FIND_AVALUE( args.get(), "group_type" ); 
		
		if( value  == "all" )
			type = OFPGT_ALL;
		else if( value == "select" )
				type = OFPGT_SELECT;
		else if( value == "indirect" )
				type = OFPGT_INDIRECT;
		else if( value == "fast_failover" )
				type = OFPGT_FF;
		else
			std::runtime_error("Invalid group_type!\n");
		
		group_id = (uint32_t)atoi( FIND_AVALUE(args.get(),"group_id") );
			
		// buckets construction
		struct ofl_bucket **buckets = (struct ofl_bucket**)xmalloc( sizeof(struct ofl_bucket*) );
		int buckets_num = 0;
		
		if ( FIND_IF_EXISTS(args.get(),"buckets",value) )
		{
			ssize_t len = value.size();
			json_object* a = new json_object((const uint8_t*)value.c_str(),len);
			json_array* ar = (json_array*)a->object;
			BOOST_FOREACH(const json_array::value_type& p, *ar)
			{
				struct ofl_bucket* bucket = (struct ofl_bucket*)xmalloc( sizeof(struct ofl_bucket) ); 
				bucket =  Bucket_fields_builder::construct_bucket(p,bucket);
				if(buckets_num)
					buckets = (struct ofl_bucket**) xrealloc(buckets, sizeof(struct ofl_bucket *) * (buckets_num + 1));
				buckets[ buckets_num++ ] = bucket;
			}
		}
		
		msg->header.type = OFPT_GROUP_MOD;
		msg->buckets = buckets;
		msg->buckets_num = buckets_num;
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
	
	struct ofl_msg_header* Inter_port_mod::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		std::string value;
		
		struct ofl_msg_port_mod *msg = new ofl_msg_port_mod;
	
		uint32_t advertise = (uint32_t)atoi( FIND_AVALUE(args.get(),"advertise") );
		uint32_t port_no = (uint32_t)atoi( FIND_AVALUE(args.get(),"port") );
		uint32_t config = (uint32_t)atoi( FIND_AVALUE(args.get(),"config") );
		uint32_t mask;
		
		if( FIND_IF_EXISTS(args.get(), "mask", value) == false )
			mask = 0xffffffff;
		else
			mask = (uint32_t)atoi( value.c_str() );
		
		int len = ETH_ADDR_LEN;
		ethernetaddr addr = ethernetaddr( FIND_AVALUE( args.get(), "hw_addr" ) );
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
		
		_addit_args.push_back( std::pair<std::string, enum e_arg_type_t>(std::string("meter_bands"),e_String) );
	}
	
	struct ofl_msg_header* Inter_meter_mod::request_msg_creator(const interact_args& args)
	{
		//check_args(args);
		
		std::string value;
		
		struct ofl_msg_meter_mod *msg = new ofl_msg_meter_mod;
	
		uint16_t command;
		uint16_t flags;
		//uint16_t meter_type;
		uint32_t meter_id;
		//uint32_t rate;
		//uint32_t burst_size;
		//uint8_t prec_level; 
		
		value = FIND_AVALUE( args.get(), "command" );
		
		if(value  == "add")
				command = OFPMC_ADD;
		else if(value == "modify")
				command = OFPMC_MODIFY;
		else if(value == "delete")
				command = OFPMC_DELETE;
		else
			std::runtime_error("Invalid command!\n");
			
		flags =  (uint16_t)atoi( FIND_AVALUE(args.get(),"flags") );
		//rate = (uint32_t)atoi( args.find("rate")->second.c_str() );
		//burst_size = (uint32_t)atoi( args.find("burst_size")->second.c_str() );
		
		value = FIND_AVALUE( args.get(), "meter_id" );
		
		if(value  == "slowpath")
				meter_id = OFPM_SLOWPATH;
		else if(value == "controller")
				meter_id = OFPM_CONTROLLER;
		else if(value == "all")
				meter_id = OFPM_ALL;
		else
			meter_id = (uint32_t)atoi( value.c_str() ); // todo
		
		// meter bands construction
		struct ofl_meter_band_header ** bands = (struct ofl_meter_band_header**)xmalloc(sizeof (struct ofl_meter_band_header*));
		int bands_num = 0;
		
		if ( FIND_IF_EXISTS(args.get(),"meter_bands",value) )
		{
			ssize_t len = value.size();
			json_object* a = new json_object((const uint8_t*)value.c_str(),len);
			json_array* ar = (json_array*)a->object;
			BOOST_FOREACH(const json_array::value_type& p, *ar)
			{
				struct ofl_meter_band_header* band; 
				band =  Meter_band_builder::construct_band( p);
				if(bands_num)
					bands = (struct ofl_meter_band_header**)
										xrealloc(bands, sizeof(struct ofl_meter_band_header*) * (bands_num + 1));
				bands[ bands_num++ ] = band;
			}
		}
		
		msg->bands = bands;
		msg->command = command;
		msg->header.type = OFPT_METER_MOD;
		msg->flags = flags;
		msg->meter_bands_num = bands_num;
		msg->meter_id = meter_id;
		
		return (ofl_msg_header*)msg;
	}
};
