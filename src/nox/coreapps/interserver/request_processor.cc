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
		
	//	delete t;
		
		return datapathid::from_host(id);
	}
	
	typedef std::vector< std::string> type_saver;
	// this function analize type of request
	std::string Request_processor::interpret_type(json_object* jobj)
	{
		// replacing switch / case construction 
		Msg_resolver* reslv = NULL;
		static type_saver type_hash = (this->resolve(reslv), reslv->get_type_saver()) ;

		json_object* t = json::get_dict_value(jobj,std::string("type"));
		if(t == NULL)
			throw http_request_error("POST data must have type field!\n",e_bad_request);
			
		std::string str_t = t->get_string(true);
		
		//delete t;
		
		type_saver::iterator i;
		
		if((i = std::find(type_hash.begin(),type_hash.end(),str_t)) == type_hash.end() )
			throw http_request_error("POST data containes invalid type field\n",e_bad_request);

		return *i;
	}
	/*
	request_arguments Request_processor::find_args(json_object* jobj ,const std::vector<std::string>& keys,
																		const std::vector<std::string>& addit_keys,
																		const std::string& prev_mark)
	{
		static int syn;
		if(prev_mark == "")
			syn = 0;
		
		int this_level = syn;

		
		Interact_marker marker;
		request_arguments args;
		json_object* t = NULL; 
		std::string value;
		std::string key,tmp;
		int mark_future = syn;
		
		std::vector<std::string> all_keys = keys;
		all_keys.insert(all_keys.end(),addit_keys.begin(),addit_keys.end());
		
		std::vector<std::string>::const_iterator i1 = all_keys.begin();
		std::vector<std::string>::const_iterator i2 = all_keys.end();	
		for(i1; i1 != i2; ++i1)
		{
			//std::cout <<  "Current iterator value is "<< *i1<< std::endl;
			// if processed skip
			if( args.find(*i1) != args.end() )
				continue;
			
			//std::cout <<  "Current iterator value is "<< *i1<< std::endl;
			//std::cout << jobj->get_string(true) << std::endl;
			t = json::get_dict_value(jobj,*i1);

			if( t == NULL )
				continue;
					
			value = t->get_string(true);
			// check if value is JSON object
			std::cout <<  "Check if value is JSON: "<< value << std::endl;
			ssize_t len = value.size();
			json_object* a = new json_object((const uint8_t*)value.c_str(),len);
			if(a->type == json_object::JSONT_NULL)
			{
				std::cout << "Adding next info: " <<  *i1 << " : " << value << std::endl;
				key = marker.construct_marked(*i1,this_level,prev_mark);
				args[key] = value; // if not JSON, add to ret list
			}
			else 
			{
				std::cout << "***Is JSON" << std::endl;
				
				tmp = marker.get_marker(this_level,prev_mark);
				key = marker.construct_marked(*i1,syn + 1,tmp);
				
				args[key] = "ok"; // or somelike
				
				//Checking if array
				if(a->type == json_object::JSONT_ARRAY)
				{
					lg.dbg("Is array, special handling...");
					json_array *ar = (json_array*)a->object;
					json_array::iterator i1 = ar->begin();
					json_array::iterator i2 = ar->end();
					for(i1; i1 != i2; ++i1)
					{
						++syn;
						request_arguments args2 = find_args(*i1,keys,addit_keys,tmp);
						args.insert(args2.begin(),args2.end());
					}
				}
				else
				{
					++syn;
					request_arguments args2 = find_args(a,keys,addit_keys,tmp);
					args.insert(args2.begin(),args2.end());
				}
			}
			delete a;
		}

		return args;
	}*/
	
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
