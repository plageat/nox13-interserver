#ifndef MSG_RESOLVER_HH__
#define MSG_RESOLVER_HH__

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <map>
#include "component.hh"
#include "interactor.hh"

namespace vigil
{
  using namespace vigil::container; 
  
  typedef std::vector<std::string> id_to_type;
  
  class Msg_resolver : public Component
  {
	  private:
		cl_factory<Interactor, std::string , boost::function< Interactor* () > > _interactsFact;
		id_to_type _res_ids;
	  
		boost::shared_ptr<Interactor> _temp_ptr;
	  public:
		
		Msg_resolver(const Context* c, const json_object* node) : Component(c) 
		{  }	
	
		~Msg_resolver() {};
	
		void configure(const Configuration* c);
	
		void install();

		static void getInstance(const container::Context* ctxt, 
									vigil::Msg_resolver*& scpa);
									
		const id_to_type& get_type_saver() const;
		// msg_resolver main logic
		
		bool init_interactor(const std::string&);	// must be call first!
		std::vector<std::string> give_arguments() const;
		std::vector<std::string> give_additional_args() const;
		bool is_modify() const;
		int resolve_request(const datapathid& did, const boost::shared_ptr<json_object>&);	// must be call last!
		
  };
  
};

  
#endif
