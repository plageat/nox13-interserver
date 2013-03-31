#include "request_processor.hh"

#include "interact_event.hh"
//#include "inter_sw_config.hh"
#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/bind.hpp>
#include <algorithm>
#include "nox.hh"
#include "json-util.hh"
#include <map>
#include <stdexcept>
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
	 
	static Vlog_module lg("request_processor");

	void Request_processor::postResponse(Return_msg r)
	{
		post(new Http_response_event(r));
	}
	
	datapathid Request_processor::interpret_dpid(json_object* jobj)
	{
		json_object* t = json::get_dict_value(jobj,std::string("dpid"));
		if(t == NULL)
			throw http_request_error("POST data must have dpid field!\n",e_bad_request);
			
		std::string str_t = t->get_string(true);
		
		int id = 0;
		id = atoi(str_t.c_str());
		if(!id)
			throw http_request_error("POST data containes invalid dpid value\n",e_bad_request);
		
		delete t;
		
		return datapathid::from_host(id);
	}
	
	typedef std::map< std::string, enum ofp_type> type_saver;
	// this function analize type of request
	enum ofp_type Request_processor::interpret_type(json_object* jobj)
	{
		// replacing switch / case construction 
		static type_saver type_hash;
		static bool init = false;
		if(!init )
		{
			// to add key and value for type			
			type_hash.insert(type_saver::value_type(
								std::string("get_sw_config"),OFPT_GET_CONFIG_REQUEST) );
			type_hash.insert(type_saver::value_type(
								std::string("set_sw_config"),OFPT_SET_CONFIG) );
			init = true;
		}

		json_object* t = json::get_dict_value(jobj,std::string("type"));
		if(t == NULL)
			throw http_request_error("POST data must have type field!\n",e_bad_request);
			
		std::string str_t = t->get_string(true);
		
		delete t;
		
		type_saver::iterator i;
		
		if((i = type_hash.find(str_t)) == type_hash.end() )
			throw http_request_error("POST data containes invalid type field\n",e_bad_request);

		return i->second;
	}
	
	request_arguments Request_processor::find_args(json_object* jobj ,const std::vector<std::string>& keys)
	{
		request_arguments args;
		json_object* t = NULL;
		std::string th_msg;	// for exception
		std::string value;
		
		std::vector<std::string>::const_iterator i1 = keys.begin();
		std::vector<std::string>::const_iterator i2 = keys.end();

		for(i1; i1 != i2; ++i1)
		{
			t = json::get_dict_value(jobj,*i1);
			 
			if(t == NULL)
			{
				th_msg += *i1;
				th_msg += '\n';
				continue;
			}
			value = t->get_string(true);
			args[*i1] = value;
		}
		
		if(!th_msg.empty())
		{
			std::string s("Missing next fields in POST request:\n");
			s += th_msg;
			throw http_request_error( s.c_str() ,e_bad_request);
		}
	
		return args;
	}
	// if dpid is not present , generates exception
	void Request_processor::dpid_check(const datapathid& id)
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
	
		const Http_request_event& me = assert_cast<const Http_request_event&>(e);
		
		lg.dbg("Handling RESTfull request");
		
		me.get_request().debug();
		
		//std::string response;
		
		if( me.get_request()._url == "/switches/info" )
		{
			try
			{
				enum ofp_type tp;
				if(me.get_request()._post_data.size() == 0)
					throw http_request_error("POST data not found!\n",e_not_found);
			
				std::string post_data = me.get_request()._post_data;
				ssize_t len = post_data.size();
			
				json_object* a = new json_object((const uint8_t*)post_data.c_str(),len);
				if(a->type == json_object::JSONT_NULL)
					throw http_request_error("Invalid POST data format, must be JSON\n",e_bad_request);
				
				Msg_resolver* reslv;
			
				this->resolve(reslv);
				
				tp = interpret_type(a);
				// step 1
				bool inited = reslv->init_interactor(tp);
				if(!inited)
					http_request_error("Requested type is not curretly supported by Interserver\n" ,e_not_supported);
				// step 2
				std::vector<std::string> keys = reslv->give_arguments();
				request_arguments args;
				if(keys.size() != 0)
					args = find_args(a,keys);
				// step 3
				bool mod_req = reslv->is_modify();
				// step 4
				datapathid did = interpret_dpid(a);
				dpid_check(did);
				int sendGood = reslv->resolve_request(did,args);
				if(sendGood != 0)
					throw http_request_error("nox_core sending interactor msg error\n",e_internal_server_error);
				// send reply now if modify request 
				if(mod_req)
					postResponse( Return_msg(std::string("Accepted\n"),e_ok) );
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
		/*
		if( me.get_request()._url == "/switches" && me.get_request()._method == "GET")
		{
			lg.dbg("Request for DPIDs info");
			
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
		*/
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
		//nox::post_event(new Http_request_event(Request_msg())); // works good
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
