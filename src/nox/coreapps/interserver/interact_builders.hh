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
  
  class Inter_flow_agr_info : public Interactor
  {
	public:
		Inter_flow_agr_info();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_flow_agr_info() {};
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
  
  class Inter_port_stats: public Interactor
  {
	public:
		Inter_port_stats();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_port_stats() {};
  };
  
  class Inter_port_desc: public Interactor
  {
	public:
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_port_desc() {};
  };
  
  class Inter_queue_stats: public Interactor
  {
	public:
		Inter_queue_stats();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_queue_stats() {};
  };
  // not work because of bad_port...?
  class Inter_queue_config: public Interactor
  {
	public:
		Inter_queue_config();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_queue_config() {};
  };
  
  class Inter_group_stats: public Interactor
  {
	public:
		Inter_group_stats();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_group_stats() {};
  };
  
  class Inter_group_desc: public Interactor
  {
	public:
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_group_desc() {};
  };
  
  class Inter_group_features: public Interactor
  {
	public:
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_group_features() {};
  };

  class Inter_meter_stats: public Interactor
  {
	public:
		Inter_meter_stats();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_meter_stats() {};
  };  

  class Inter_meter_config: public Interactor
  {
	public:
		Inter_meter_config();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_meter_config() {};
  }; 
  
  class Inter_meter_features: public Interactor
  {
	public:
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_meter_features() {};
  };
  
  class Inter_group_mod: public Interactor
  {
	public:
		Inter_group_mod();
		
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
		
		virtual ~Inter_group_mod() {};
  };
};

#endif
