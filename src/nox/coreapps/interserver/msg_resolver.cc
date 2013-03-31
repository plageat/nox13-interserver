#include "msg_resolver.hh"

#include "interact_builders.hh"
#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/bind.hpp>
#include <algorithm>
#include <stdexcept>
#include "nox.hh"
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
	 
	static Vlog_module lg("msg_resolver");
	
	void Msg_resolver::configure(const Configuration* c)
	{
	}
	
	void Msg_resolver::install()
	{
		boost::function< Interactor* () > f;
		// to register 
		f = boost::bind(&InteractorCreator<Inter_sw_config>::create);
		this->_interactsFact.register_object(Inter_sw_config::name(), f);
		
		f = boost::bind(&InteractorCreator<Inter_sw_config_setter>::create);
		this->_interactsFact.register_object(Inter_sw_config_setter::name(), f);
	
	}
	
	void Msg_resolver::getInstance(const container::Context* ctxt, 
			      vigil::Msg_resolver*& scpa) 
	{
		scpa = dynamic_cast<Msg_resolver*>
				(ctxt->get_by_interface(container::Interface_description
					(typeid(Msg_resolver).name())));
	}
	
	bool Msg_resolver::init_interactor(enum ofp_type tp)
	{
		this->_temp_ptr = _interactsFact.create_object(tp);
		
		if (_temp_ptr.get() == 0)
			return false;
			
		return true;
	}
	
	std::vector<std::string> Msg_resolver::give_arguments() const
	{
		if (_temp_ptr)
			return this->_temp_ptr->arg_requires();
		else
			throw std::logic_error("Must call init_interactor first!");
	}
	
	bool Msg_resolver::is_modify() const
	{
		if (_temp_ptr)
			return this->_temp_ptr->is_modify();
		else
			throw std::logic_error("Must call init_interactor first!");	
	}
	
	int Msg_resolver::resolve_request(const datapathid& did, const request_arguments& args)
	{
		if (_temp_ptr)
		{
			ofl_msg_header * msg = this->_temp_ptr->request_msg_creator(args);
			// clear temp pointer
			_temp_ptr.reset();
			return send_openflow_msg(did, (struct ofl_msg_header *)msg, 1/*xid*/, false/*block*/);
		}
		else
			throw std::logic_error("Must call init_interactor first!");
			
	}
	
	
		 
	REGISTER_COMPONENT(vigil::container::
		     Simple_component_factory<vigil::Msg_resolver>, 
		     vigil::Msg_resolver);
			 
};
