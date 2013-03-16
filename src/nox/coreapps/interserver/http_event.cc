#include "http_event.hh"
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;	
		
	void Request_msg::debug() const
	{
		std::cout << "Url is: "<< _url << std::endl;
		std::cout << "POST data is: " << _post_data << std::endl;
	//	std::cout << "Version is: "<< _version << std::endl;
	//	std::cout << "Method is: "<< _method << std::endl
	}	
	
	void Request_msg::clear()
	{
		//this->_method.clear();
		this->_post_data.clear();
		this->_url.clear();
		//this->_version.clear();
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