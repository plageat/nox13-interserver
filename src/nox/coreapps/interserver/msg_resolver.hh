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
  
  class Msg_resolver : public Component
  {
	  private:
		cl_factory<Interactor, enum ofp_type , boost::function< Interactor* () > > _interactsFact;
	  
		boost::shared_ptr<Interactor> _temp_ptr;
	  public:
		
		Msg_resolver(const Context* c, const json_object* node) : Component(c) 
		{  }	
	
		~Msg_resolver() {};
	
		void configure(const Configuration* c);
	
		void install();

		static void getInstance(const container::Context* ctxt, 
									vigil::Msg_resolver*& scpa);
		// msg_resolver main logic
		
		bool init_interactor(enum ofp_type);	// must be call first!
		std::vector<std::string> give_arguments() const;
		bool is_modify() const;
		int resolve_request(const datapathid& did, const request_arguments&);	// must be call last!
		
  };
  
};

  
#endif
