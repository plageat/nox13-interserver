#include "interserver.hh"


#include "shutdown-event.hh"
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
		
		post_info->_http_sync.lock();	
		post_info->_accept_post = true;		
		
		boost::system_time timeout = boost::get_system_time() + boost::posix_time::seconds(10);
		
		bool ret = post_info->_http_sync.timed_lock(timeout); 
		std::string res;
		if( ret == true)	
			res = post_info->_response;
		else
			res = "Datapathid not responses\n";

		post_info->_http_sync.unlock();	
		post_info->_request_msg.clear();
		response = server::response::stock_reply(
            server::response::ok, res);
			
		lg_lib.dbg("Closing handler");
	}
	
	//_________________________________________________________________________
	
	Interserver::Request_process_Info Interserver::_rp_info = {false,Request_msg(),std::string()};
	
	Interserver::~Interserver()
	{
		_server->stop();
	}
	
	Disposition Interserver::shutdown_handler(const Event&)
	{
		_server->stop();
		return CONTINUE;
	}
	
	void Interserver::configure(const Configuration* config)
	{
		register_event(Http_request_event::static_get_name());
		register_event(Http_response_event::static_get_name());
		
		register_handler<Http_response_event>
			(boost::bind(&Interserver::response_handler, this, _1));
		register_handler<Shutdown_event> 
                                  (boost::bind(&Interserver::shutdown_handler, this, _1));
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
		
		_server.reset(new server(boost::network::http::_address= "0.0.0.0",
							boost::network::http::_port=_port, 
							boost::network::http::_handler=handler, 
							boost::network::http::_reuse_address=true) );
		
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
		struct timeval wait;	// some recoding?
		wait.tv_sec = 0;
		wait.tv_usec = 1;
		while(true)
		{
			co_timer_wait(wait,NULL);
			co_block();
			if(_rp_info._accept_post == true)
			{
				_rp_info._accept_post = false;
				post(new Http_request_event(_rp_info._request_msg));
			}
		}
	}
	
	Disposition Interserver::response_handler(const Event& e)
	{
		const Http_response_event& me = assert_cast<const Http_response_event&>(e);
		this->_rp_info._response = me.get_response();
		this->_rp_info._http_sync.unlock();
		
		//lg.dbg("Interserver::response_handler working:");
		//std::cout << "***Responsed data is: \n" <<  me.get_response() << std::endl;
		
		return CONTINUE;
	}
	
	REGISTER_COMPONENT(vigil::container::
		     Simple_component_factory<vigil::Interserver>, 
		     vigil::Interserver);

};