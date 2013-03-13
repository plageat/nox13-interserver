#include "inter_sw_config.hh"

#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/bind.hpp>
#include <algorithm>
#include "nox.hh"
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
	 
	static Vlog_module lg("inter_sw_config");
	
	void Inter_sw_config::install()
	{
		//register_handler<>
		//	(boost::bind(&Request_processor::handle_request, this, _1));
	}
	
	void Inter_sw_config::getInstance(const container::Context* ctxt, 
			      vigil::Inter_sw_config*& scpa) 
	{
		scpa = dynamic_cast<Inter_sw_config*>
				(ctxt->get_by_interface(container::Interface_description
					(typeid(Inter_sw_config).name())));
	}
	
	const std::string& Inter_sw_config::response(const args_cont& args)
	{
		// some ligic
		_response += "Inter sw config returns response";
		return _response;
	}
			 
	REGISTER_COMPONENT(vigil::container::
		     Simple_component_factory<vigil::Inter_sw_config>, 
		     vigil::Inter_sw_config);
};
