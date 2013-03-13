#include "http_event.hh"
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;	
		
	void Request_msg::debug() const
	{
		std::cout << "Url is: "<< _url << std::endl;
		std::cout << "Method is: " << _method << std::endl;
		std::cout << "Version is: "<< _version << std::endl;
		std::cout << "Arguments are:" << std::endl;
		for(map<string, string>::const_iterator i1 = _url_args.begin(); i1 != _url_args.end();++i1)
			std::cout << i1->first << " = " << i1->second << std::endl;
		std::cout << "Post data is:" << std::endl;
		for(map<string, string>::const_iterator i1 = _post_data.begin(); i1 != _post_data.end();++i1)
			std::cout << i1->first << " = " << i1->second << std::endl;
	}	
	
	Http_request_event::Http_request_event(const Request_msg& msg) 
		: Event(static_get_name()), _req_msg(msg)
	{
		
	}
	
	const Request_msg& Http_request_event::get_request() const
	{
		return _req_msg;
	}
	
	Http_response_event::Http_response_event(const std::string& res) 
		: Event(static_get_name()), _response(res)
	{ }
	
	const std::string& Http_response_event::get_response() const
	{
		return _response;
	}

};