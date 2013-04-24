#include "request_processor.hh"

#include "interact_event.hh"
#include <algorithm>
#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/bind.hpp>
#include <algorithm>
#include "nox.hh"
#include "json-util.hh"
#include <map>
#include <stdexcept>
#include <boost/thread.hpp>
	
#define WAIT_FOR_REQ 2
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
	 
	static Vlog_module lg("request_processor");
	
	void Request_processor::auto_set_reply()
	{
		struct timeval wait = {0 , 1};
		struct timeval wait_sleep = {WAIT_FOR_REQ, 0};
		while(true)
		{
			co_sleep(wait);
			
			if(_auto_mutex == false)
				continue;
			
			co_sleep(wait_sleep);

			if (_reply_state == true)
				postResponse( Return_msg(std::string("Accepted\n"),e_ok) );
				
			_auto_mutex = false;
		}
	}
	
	void Request_processor::postResponse(Return_msg r)
	{
		if(_reply_state == false)
			return;
		_reply_state = false;
		
		post(new Http_response_event(r));
	}
	
	bool Request_processor::interpret_checkerror(const json_object* jobj) const
	{
		json_object* t = json::get_dict_value(jobj,std::string("checkerror"));
		if(t == NULL)
			return false;
		
		if( t->get_string(true) == "yes" )
			return true;
		
		return false;
	}
	
	datapathid Request_processor::interpret_dpid(const json_object* jobj) const
	{
		json_object* t = json::get_dict_value(jobj,std::string("dpid"));
		if(t == NULL)
			throw http_request_error("POST data must have dpid field!\n",e_bad_request);
			
		std::string str_t = t->get_string(true);
		
		int id = 0;
		id = atoi(str_t.c_str());
		if(!id)
			throw http_request_error("POST data containes invalid dpid value\n",e_bad_request);
		
		return datapathid::from_host(id);
	}
	
	typedef std::vector< std::string> type_saver;
	// this function analize type of request
	std::string Request_processor::interpret_type(const json_object* jobj) const
	{
		// replacing switch / case construction 
		Msg_resolver* reslv = NULL;
		static type_saver type_hash = (this->resolve(reslv), reslv->get_type_saver()) ;

		json_object* t = json::get_dict_value(jobj,std::string("type"));
		if(t == NULL)
			throw http_request_error("POST data must have type field!\n",e_bad_request);
			
		std::string str_t = t->get_string(true);
		
		type_saver::iterator i;
		
		if((i = std::find(type_hash.begin(),type_hash.end(),str_t)) == type_hash.end() )
			throw http_request_error("POST data containes invalid type field\n",e_bad_request);

		return *i;
	}
	
	// if dpid is not present , generates exception
	void Request_processor::dpid_check(const datapathid& id) const
	{
		if ( _dpids.end() == std::find(_dpids.begin(),_dpids.end(),id) )
		{
			std::string th_msg("Switch with datapathid ");
			th_msg += id.string();
			th_msg += " is not exist\n";
			throw http_request_error(th_msg.c_str(),e_not_found);
		}
	}
	
	Disposition Request_processor::handle_request(const Event& e)
	{
	
		_reply_state = true;
		
		const Http_request_event& me = assert_cast<const Http_request_event&>(e);
		
		//lg.dbg("Handling RESTfull request...");
		
		//me.get_request().debug();
		
		if( me.get_request()._url == "/switches/info" )
		{
			try
			{
				std::string tp;
				if(me.get_request()._post_data.size() == 0)
					throw http_request_error("POST data not found!\n",e_not_found);
			
				std::string post_data = me.get_request()._post_data;
				ssize_t len = post_data.size();
			
				boost::shared_ptr<json_object> a( new json_object((const uint8_t*)post_data.c_str(),len) );
				if(a->type == json_object::JSONT_NULL)
					throw http_request_error("Invalid POST data format, must be JSON\n",e_bad_request);
				
				Msg_resolver* reslv;
			
				this->resolve(reslv);
				
				tp = interpret_type(a.get());
				// step 1
				bool inited = reslv->init_interactor(tp);
				if(!inited)
					http_request_error("Requested type is not curretly supported by Interserver\n" ,e_not_supported);
				// step 2
				bool mod_req = reslv->is_modify();
				// step 3
				datapathid did = interpret_dpid(a.get());
				dpid_check(did);
				int sendGood = reslv->resolve_request(did,a);
				if(sendGood != 0)
				{
					throw http_request_error("nox_core sending interactor msg error\n",e_internal_server_error);
				}
				// send reply now if modify request 
				if( interpret_checkerror( a.get() ) == false)
				{
					if(mod_req)
						postResponse( Return_msg(std::string("Accepted\n"),e_ok) );
				}
				else
				{
					if(mod_req)
					{
						_auto_mutex = true;
					}
					std::cout << "handle request after..." << std::endl;
				}
			}
			catch(http_request_error& e)
			{
				postResponse(e.construct_return());
			}
			catch(std::exception &e)
			{
				postResponse( Return_msg(std::string(e.what()),e_internal_server_error) );
			}
			
		}
		else
			postResponse( Return_msg(std::string("Invalid path for NOX interserver source\n"),e_not_implemented ));

		return CONTINUE;
	}
	
	Disposition Request_processor::handle_reply(const Event& e)
	{
		const Interact_event& pi = assert_cast<const Interact_event&>(e);
		
		postResponse( Return_msg(pi.get_reply(),e_ok ) );
		
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
	
	void Request_processor::configure(const Configuration* c)
	{
		register_event(Http_request_event::static_get_name());
		register_event(Http_response_event::static_get_name());
	}
	
	void Request_processor::install()
	{
		start(boost::bind(&Request_processor::auto_set_reply, this));
		
		register_event(Interact_event::static_get_name());
		
		register_handler<Http_request_event>
			(boost::bind(&Request_processor::handle_request, this, _1));
			
		register_handler<Interact_event>
			(boost::bind(&Request_processor::handle_reply,this, _1));
		
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
