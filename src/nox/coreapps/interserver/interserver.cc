#include "interserver.hh"

#include "assert.hh"
#include "vlog.hh"
#include "hash_map.hh"
#include <boost/bind.hpp>
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;
	 
	static Vlog_module lg("interserver");
	static Vlog_module lg_lib("microhttpd");
	
	static void request_completed(void *cls, MHD_Connection * con, void **con_cls, MHD_RequestTerminationCode code)
	{
		//lg_lib.dbg("!!!!!!!!!!!!Calling Request completed");
		Http_connection_info* con_inf = (Http_connection_info*)*con_cls;
		if(con_inf != NULL)
			delete con_inf;
		con_inf = NULL;
		*con_cls = NULL;
	}
	
	static int send_back(MHD_Connection* con, const std::string& data, int status_code)
	{
		
		MHD_Response * response;
		response = MHD_create_response_from_data(data.size(),(void*)data.c_str(),MHD_NO,MHD_YES);
		if(!response)
			return MHD_NO;
		//lg_lib.dbg("!!!!!!!!!!!!Calling libmicrohttpd");
		//lg_lib.dbg(data.c_str());
		int ret_val = MHD_queue_response(con,status_code,response);
		MHD_destroy_response(response);
		
		return ret_val;
	}
	
	static int get_url_args(void *cls, MHD_ValueKind kind, const char *key , const char* value)
	{
		lg_lib.dbg("getting url arguments...%s",key);
		map<string, string> * url_args = static_cast<map<string, string> *>(cls);

		if (url_args->find(key) == url_args->end()) 
		{
			if (!value)
				(*url_args)[key] = "";
			else 
				(*url_args)[key] = value;
		}
		return MHD_YES;
	}
	
	static int get_post_data(void *cls, enum MHD_ValueKind kind, const char *key,
									const char *filename, const char* content_type, 
									const char* transfer_encoding, const char *data,
									long long unsigned int off, unsigned int size)
	{
		map<string, string> * post_data = static_cast<map<string, string> *>(cls);

		if (post_data->find(key) == post_data->end()) 
		{
			if (!data)
			{
				(*post_data)[key] = "";
			}	
			else 
			{
				(*post_data)[key] = std::string(data,size);
			}
		}
		return MHD_YES;
	}
		// correct static issue
	 int url_handler (void *cls,
			struct MHD_Connection *connection,
				const char *url,
					const char *method,
						const char *version,
							const char *upload_data, unsigned int *upload_data_size, void **ptr)
	{
		lg_lib.dbg("Calling url_handler succesfully");
		
		Http_connection_info *con_inf = NULL;

		Interserver::Request_process_Info *post_info = &Interserver::_rp_info;  //Interserver::Request_process_Info *)cls;//&(((Interserver*)cls)->_rp_info);
		
		if(*ptr == NULL)	// incoming 
		{
			con_inf = new Http_connection_info;
			if(con_inf == NULL)
				return MHD_NO;
			if(strcmp(method,"GET") == 0)
				con_inf->_connection_type = e_READ;
			else	// for POST, PUT and DELETE the same
			{
				con_inf->_connection_type = e_MODIFY; 
				con_inf->_pp = MHD_create_post_processor(connection, POST_BUFFER_SIZE ,
																	get_post_data, (void*) &post_info->_request_msg._post_data);
				if(con_inf->_pp == NULL)
				{
					lg_lib.dbg("Error of post processor");
					return MHD_NO;
				}	
			}
			
			*ptr = (void*)con_inf;
			
			return MHD_YES;
		}
		// connection exist
		if(strcmp(method,"GET") != 0)
		{
			con_inf = (Http_connection_info*)*ptr;
			if(*upload_data_size != 0)
				MHD_post_process(con_inf->_pp, upload_data, *upload_data_size);
			*upload_data_size = 0;
		}
		
		post_info->_request_msg._url = url;
		post_info->_request_msg._method = method;
		post_info->_request_msg._version = version;
		
		MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND, get_url_args, &post_info->_request_msg._url_args);
		//MHD_get_connection_values (connection, MHD_GET_ARGUMENT_KIND, get_url_args, &post_info->_request_msg._url_args);
		//post_info->_request_msg.debug();

		post_info->_accept_post = true;
		// give some data and response to requester

		while(post_info->_accept_response != true); // fail
		//lg_lib.dbg("Good, accept responce handled!!!");
		
		post_info->_accept_response = false;
		
		std::string s = post_info->_response; 
		//return 
		return send_back(connection,s,MHD_HTTP_OK); // bag or new version change
		
		post_info->_request_msg.clear();
		
		return MHD_YES;
	}
	
	Http_connection_info::Http_connection_info() : _connection_type(e_READ),_pp(NULL)
	{
	}
	
	Http_connection_info::~Http_connection_info()
	{
		if(_pp != NULL)
			MHD_destroy_post_processor(_pp);
		_pp = NULL;
	}
	
	//_________________________________________________________________________
	
	Interserver::Request_process_Info Interserver::_rp_info = {false,Request_msg(),false,std::string()};
	
	Interserver::~Interserver()
	{
		MHD_stop_daemon(_daemon);
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
			_port = (unsigned short) atoi(i->second.c_str());
	}
	
	void Interserver::install()
	{
		this->_daemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG | MHD_USE_POLL,
			_port, 0, 0, &url_handler,
			(void*)NULL, MHD_OPTION_NOTIFY_COMPLETED, request_completed, NULL, MHD_OPTION_END);
		if(this->_daemon == 0)
			lg.dbg("Cannot start url handler, some problems...");
			
		start(boost::bind(&Interserver::run, this));
	}
	
	void Interserver::getInstance(const container::Context* ctxt, 
			      vigil::Interserver*& scpa) 
	{
		scpa = dynamic_cast<Interserver*>
		(ctxt->get_by_interface(container::Interface_description
					(typeid(Interserver).name())));
	}
	
	void Interserver::run()
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