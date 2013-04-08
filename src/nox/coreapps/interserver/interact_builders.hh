#ifndef INTERACT_BUILDERS_HH__
#define INTERACT_BUILDERS_HH__

#include <boost/noncopyable.hpp>
#include "interactor.hh"
#include <string>
#include "openflow/openflow.h"
#include "flowmod.hh"

namespace vigil
{
  using namespace vigil::container; 

  typedef std::string builder_name;
	
  class Match_fields_builder : boost::noncopyable
  {
	private:
		arguments_list _match_tags;
		ofl_match_header *_m_header;
	public:  
		Match_fields_builder();
		~Match_fields_builder();
		
		ofl_match_header* construct_match(const request_arguments& );
		const arguments_list& get_match_args() const;
		
  };
  
  class Action_fields_builder : boost::noncopyable
  {
	private:
		arguments_list _action_tags;
	public:
		Action_fields_builder();
		~Action_fields_builder();
		
		Actions* construct_action(const request_arguments&);
		const arguments_list& get_action_args() const;
  };

  class Instruction_fields_builder : boost::noncopyable
  {
	private:
		arguments_list _instr_tags;
		Action_fields_builder _actionBuilder;
	public:  
		Instruction_fields_builder();
		~Instruction_fields_builder();
		
		Instruction* construct_instruction(const request_arguments& );
		const arguments_list& get_instr_args() const;
  };
  
	
  class Inter_sw_config : public Interactor 
  {
	public:
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_sw_config() {};
  };
  
  class Inter_sw_config_setter : public Interactor
  {
	public:
		Inter_sw_config_setter();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name();
		
		virtual ~Inter_sw_config_setter() {};
  };
  
  class Inter_features_request : public Interactor
  {
	  public:
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_features_request() {};
  };
  
  // not works beacause of OFPMP_TABLE_FEATURES reply event error
  class Inter_table_features : public Interactor
  {
	  public:
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_table_features() {};
  };

  class Inter_desc : public Interactor
  {
	  public:
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_desc() {};
  };
  
  class Inter_table_stats : public Interactor
  {
	  public:
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_table_stats() {};
  };
  
  class Inter_flow_info : public Interactor
  {
	public:
		Inter_flow_info();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_flow_info() {};
  };
  
  class Inter_flow_mod : public Interactor
  {
	public:
		Inter_flow_mod();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_flow_mod() {};
  };
  
};

  
#endif
