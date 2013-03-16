#include "interserver.hh"

#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/bind.hpp>
#include <stdlib.h>
#include <stdio.h>	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
	 
	static Vlog_module lg("interserver");
	static Vlog_module lg_lib("cpp-netlib");
	
	void http_handler::log(...)
	{
		// empty
	}
	
	void http_handler::operator() (server::request const &request, server::response &response) 
	{
		lg_lib.dbg("Working url handler");
		
		Interserver::Request_process_Info *post_info = &Interserver::_rp_info;
		
		post_info->_request_msg._url = destination(request);
		post_info->_request_msg._post_data = body(request);
		
		post_info->_accept_post = true;
		
		while(post_info->_accept_response != true);
		
		post_info->_accept_response = false;
		
		post_info->_request_msg.clear();
		response = server::response::stock_reply(
            server::response::ok, post_info->_response);
	}
	
	//_________________________________________________________________________
	
	Interserver::Request_process_Info Interserver::_rp_info = {false,Request_msg(),false,std::string()};
	
	Interserver::~Interserver()
	{
		_server->stop();
	}
	
	void Interserver::configure(const Configuration* config)
	{
		register_event(Http_request_event::static_get_name());
		register_event(Http_response_event::static_get_name());
		
		register_handler<Http_response_event>
			(boost::bind(&Interserver::response_handler, this, _1));
		
		 //Get commandline arguments
		const hash_map<string, string> argmap = config->get_arguments_list();
		hash_map<string, string>::const_iterator i;
		i = argmap.find("port");
		if (i != argmap.end())
			_port = i->second.c_str();
	}
	
	void Interserver::install()
	{
		http_handler handler;
		_server.reset(new server("0.0.0.0", _port, handler) );

		boost::thread t(boost::bind(&server::run,_server.get() ));
		t.detach();
		
		start(boost::bind(&Interserver::listen_state, this));
	}
	
	void Interserver::getInstance(const container::Context* ctxt, 
			      vigil::Interserver*& scpa) 
	{
		scpa = dynamic_cast<Interserver*>
		(ctxt->get_by_interface(container::Interface_description
					(typeid(Interserver).name())));
	}
	
	void Interserver::listen_state()
	{
		// request processing here?
		struct timeval wait;	// some recoding?
		wait.tv_sec = 0;
		wait.tv_usec = 1;
		while(true)
		{
			co_timer_wait(wait,NULL);
			co_block();
			if(_rp_info._accept_post == true)
			{
				//lg.dbg("cycle is working");
				_rp_info._accept_post = false;
				post(new Http_request_event(_rp_info._request_msg));
			}
		}
	}
	
	Disposition Interserver::response_handler(const Event& e)
	{
		const Http_response_event& me = assert_cast<const Http_response_event&>(e); //dynamic_cast<Http_response_event&>(const_cast<Event&>(e));
		this->_rp_info._response = me.get_response();
		this->_rp_info._accept_response = true;
	
		lg.dbg("Interserver::response_handler working:");
		std::cout << "***Responsed data is: \n" <<  me.get_response() << std::endl;
		
		return CONTINUE;
	}
	
	REGISTER_COMPONENT(vigil::container::
		     Simple_component_factory<vigil::Interserver>, 
		     vigil::Interserver);

};