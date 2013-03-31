#ifndef INTERACT_BUILDERS_HH__
#define INTERACT_BUILDERS_HH__

#include "interactor.hh"
#include <string>
#include <utility>

namespace vigil
{
  using namespace vigil::container; 

  typedef std::pair<std::string, enum ofp_type> builder_name;
	
  class Inter_sw_config : public Interactor 
  {
	public:
	
		std::vector<std::string> arg_requires();
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
  };
  
  class Inter_sw_config_setter : public Interactor
  {
	public:
		std::vector<std::string> arg_requires();
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name();
  };
  
  class Inter_features_request : public Interactor
  {
	  public:
		std::vector<std::string> arg_requires();
		struct ofl_msg_header * request_msg_creator(const request_arguments& );
		bool is_modify() const;
		static builder_name name(); 
  };
};

  
#endif
