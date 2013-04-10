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
	
	const id_to_type& Msg_resolver::get_type_saver() const
	{
		return _res_ids;
	}
	
	void Msg_resolver::install()
	{
		boost::function< Interactor* () > f;
		// to register 
		f = boost::bind(&InteractorCreator<Inter_sw_config>::create);
		this->_interactsFact.register_object(Inter_sw_config::name(), f);
		_res_ids.push_back(Inter_sw_config::name());
		
		f = boost::bind(&InteractorCreator<Inter_sw_config_setter>::create);
		this->_interactsFact.register_object(Inter_sw_config_setter::name(), f);
		_res_ids.push_back(Inter_sw_config_setter::name());
		
		f = boost::bind(&InteractorCreator<Inter_features_request>::create);
		this->_interactsFact.register_object(Inter_features_request::name(), f);
		_res_ids.push_back(Inter_features_request::name());
	
		f = boost::bind(&InteractorCreator<Inter_table_features>::create);
		this->_interactsFact.register_object(Inter_table_features::name(), f);
		_res_ids.push_back(Inter_table_features::name());
		
		f = boost::bind(&InteractorCreator<Inter_desc>::create);
		this->_interactsFact.register_object(Inter_desc::name(), f);
		_res_ids.push_back(Inter_desc::name());
		
		f = boost::bind(&InteractorCreator<Inter_table_stats>::create);
		this->_interactsFact.register_object(Inter_table_stats::name(), f);
		_res_ids.push_back(Inter_table_stats::name());
		
		f = boost::bind(&InteractorCreator<Inter_flow_info>::create);
		this->_interactsFact.register_object(Inter_flow_info::name(), f);
		_res_ids.push_back(Inter_flow_info::name());
		
		f = boost::bind(&InteractorCreator<Inter_flow_mod>::create);
		this->_interactsFact.register_object(Inter_flow_mod::name(), f);
		_res_ids.push_back(Inter_flow_mod::name());
		
		f = boost::bind(&InteractorCreator<Inter_flow_agr_info>::create);
		this->_interactsFact.register_object(Inter_flow_agr_info::name(), f);
		_res_ids.push_back(Inter_flow_agr_info::name());
		
		f = boost::bind(&InteractorCreator<Inter_port_stats>::create);
		this->_interactsFact.register_object(Inter_port_stats::name(), f);
		_res_ids.push_back(Inter_port_stats::name());
		
		f = boost::bind(&InteractorCreator<Inter_port_desc>::create);
		this->_interactsFact.register_object(Inter_port_desc::name(), f);
		_res_ids.push_back(Inter_port_desc::name());
		
		f = boost::bind(&InteractorCreator<Inter_queue_stats>::create);
		this->_interactsFact.register_object(Inter_queue_stats::name(), f);
		_res_ids.push_back(Inter_queue_stats::name());
		
		f = boost::bind(&InteractorCreator<Inter_group_stats>::create);
		this->_interactsFact.register_object(Inter_group_stats::name(), f);
		_res_ids.push_back(Inter_group_stats::name());
		
		f = boost::bind(&InteractorCreator<Inter_group_desc>::create);
		this->_interactsFact.register_object(Inter_group_desc::name(), f);
		_res_ids.push_back(Inter_group_desc::name());
		
		f = boost::bind(&InteractorCreator<Inter_group_features>::create);
		this->_interactsFact.register_object(Inter_group_features::name(), f);
		_res_ids.push_back(Inter_group_features::name());
	
		f = boost::bind(&InteractorCreator<Inter_meter_stats>::create);
		this->_interactsFact.register_object(Inter_meter_stats::name(), f);
		_res_ids.push_back(Inter_meter_stats::name());
		
		f = boost::bind(&InteractorCreator<Inter_meter_config>::create);
		this->_interactsFact.register_object(Inter_meter_config::name(), f);
		_res_ids.push_back(Inter_meter_config::name());
		
		f = boost::bind(&InteractorCreator<Inter_meter_features>::create);
		this->_interactsFact.register_object(Inter_meter_features::name(), f);
		_res_ids.push_back(Inter_meter_features::name());
		
		f = boost::bind(&InteractorCreator<Inter_queue_config>::create);
		this->_interactsFact.register_object(Inter_queue_config::name(), f);
		_res_ids.push_back(Inter_queue_config::name());
		
		f = boost::bind(&InteractorCreator<Inter_group_mod>::create);
		this->_interactsFact.register_object(Inter_group_mod::name(), f);
		_res_ids.push_back(Inter_group_mod::name());
		/*
		f = boost::bind(&InteractorCreator<Inter_port_mod>::create);
		this->_interactsFact.register_object(Inter_port_mod::name(), f);
		_res_ids.push_back(Inter_port_mod::name());
		
		f = boost::bind(&InteractorCreator<Inter_meter_mod>::create);
		this->_interactsFact.register_object(Inter_meter_mod::name(), f);
		_res_ids.push_back(Inter_meter_mod::name());*/
	}
	
	void Msg_resolver::getInstance(const container::Context* ctxt, 
			      vigil::Msg_resolver*& scpa) 
	{
		scpa = dynamic_cast<Msg_resolver*>
				(ctxt->get_by_interface(container::Interface_description
					(typeid(Msg_resolver).name())));
	}
	
	bool Msg_resolver::init_interactor(const std::string& tp)
	{
		this->_temp_ptr = _interactsFact.create_object(tp);
		
		if (_temp_ptr.get() == 0)
			return false;
			
		return true;
	}
	
	bool Msg_resolver::is_modify() const
	{
		if (_temp_ptr)
			return this->_temp_ptr->is_modify();
		else
			throw std::logic_error("Must call init_interactor first!");	
	}
	
	int Msg_resolver::resolve_request(const datapathid& did, const boost::shared_ptr<json_object>& args)
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
