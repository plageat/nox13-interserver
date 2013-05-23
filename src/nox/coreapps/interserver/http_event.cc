#include "http_event.hh"
	
namespace vigil
{	
	using namespace vigil::container;    
	using namespace std;	
		
	void Request_msg::debug() const
	{
		std::cout << "Url is: "<< _url << std::endl;
		std::cout << "POST data is: " << _data << std::endl;
	//	std::cout << "Version is: "<< _version << std::endl;
	//	std::cout << "Method is: "<< _method << std::endl
	}	
	
	void Request_msg::clear()
	{
		//this->_method.clear();
		this->_data.clear();
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
	
	Return_msg::Return_msg()
	{ }
	
	Return_msg::Return_msg(const std::string& response, enum http_status_type type)
		: _type(type), _response(response)
	{ }
	
	
	Http_response_event::Http_response_event(const std::string& res, enum http_status_type type) 
		: Event(static_get_name()),_status(res,type)
	{ }
	
	Http_response_event::Http_response_event(const Return_msg& s)
		: Event(static_get_name()),_status(s) 
	{ }
	
	const Return_msg& Http_response_event::get_response() const
	{
		return _status;
	}

	http_request_error::http_request_error(const std::string& msg, enum http_status_type type)
		: std::runtime_error(msg), _type(type)
	{ }	
	Return_msg http_request_error::construct_return() const
	{
		Return_msg m;
		m._response = this->what();
		m._type = _type;
		
		return m;
	}

	http_request_error::~http_request_error() throw()
 	{
		
	}
};	