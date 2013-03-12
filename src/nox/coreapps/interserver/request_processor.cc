#include "request_processor.hh"

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
	 
	static Vlog_module lg("request_processor");

	void Request_processor::postResponse(std::string r)
	{
		post(new Http_response_event(r));
	}
	
	Disposition Request_processor::handle_request(const Event& e)
	{
		const Http_request_event& me = assert_cast<const Http_request_event&>(e); //dynamic_cast<Http_request_event&>(const_cast<Event&>(e));
		lg.dbg("Request_perocessor: Handling RESTfull request");
		me.get_request().debug();
		
		// working here now
		
		//while(true)
		//{
			if( me.get_request()._url == "/switches" /*&& me.get_request()._method == "GET"*/)
			{
				lg.dbg("Request for DPIDs info");
				
				std::string response;
				response += "DPIDs of connected switches are:\n";
				std::vector<datapathid>::const_iterator i1 = _dpids.begin();
				std::vector<datapathid>::const_iterator i2 = _dpids.end();
				if(i1 == i2)
					response += "there are no connected switches\n";
				else
					for(i1; i1 != i2; ++i1)
					{
						response += i1->string();
						response += '\n';
					} 
				
				postResponse(response);
				
				return CONTINUE;
			}
			
			// factory calls
		//}
		
		return CONTINUE;
	}
	
	Disposition Request_processor::datapath_join_handler(const Event& e)
	{
		const Datapath_join_event& info = assert_cast<const Datapath_join_event&>(e);
		_dpids.push_back(info.dpid);
		
		return CONTINUE;
	}
	
	Disposition Request_processor::datapath_leave_handler(const Event& e)
	{
		const Datapath_leave_event& info = assert_cast<const Datapath_leave_event&>(e);
		
		_dpids.erase( std::find(_dpids.begin(),_dpids.end(),info.datapath_id) );
		
		return CONTINUE;
	}
	
	void Request_processor::install()
	{
		//nox::post_event(new Http_request_event(Request_msg())); // works good
		register_handler<Http_request_event>
			(boost::bind(&Request_processor::handle_request, this, _1));
		
		register_handler<Datapath_join_event>
			(boost::bind(&Request_processor::datapath_join_handler, this, _1));
			
		register_handler<Datapath_leave_event>
			(boost::bind(&Request_processor::datapath_leave_handler, this, _1));
	}
	
	void Request_processor::getInstance(const container::Context* ctxt, 
			      vigil::Request_processor*& scpa) 
	{
		scpa = dynamic_cast<Request_processor*>
				(ctxt->get_by_interface(container::Interface_description
					(typeid(Request_processor).name())));
	}
			 
	REGISTER_COMPONENT(vigil::container::
		     Simple_component_factory<vigil::Request_processor>, 
		     vigil::Request_processor);
};
