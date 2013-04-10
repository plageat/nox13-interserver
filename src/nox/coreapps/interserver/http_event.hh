#ifndef HTTP_EVENT_HH__
#define HTTP_EVENT_HH__

#include <exception>
#include <boost/noncopyable.hpp>
#include <map>
#include "component.hh"


namespace vigil
{
  using namespace vigil::container; 
  
  struct Request_msg
  {
	std::string _url;
	//std::string _method;
	//std::string _version;
	std::string _post_data;
	
	void debug() const;
	
	void clear();
  };
  
  class Http_request_event : public Event 
  {
	private:  
	  Request_msg _req_msg;
	public:
	  Http_request_event(const Request_msg& msg);
	  ~Http_request_event() { };

    Http_request_event() : Event(static_get_name()) 
    { }
    /** Static name required in NOX.
     */
    static const Event_name static_get_name() 
    {
      return "Http_request_event";
    }

	const Request_msg& get_request() const;
  };  
  
  // some of http status codes
  enum http_status_type 
  {
	  e_ok = 200,
	  e_created = 201,
	  e_not_modified = 304,
	  e_bad_request = 400,
	  e_not_found = 404,
	  e_not_supported = 405,
	  e_internal_server_error = 500,
	  e_not_implemented = 501,
	  e_bad_gateway = 502,
	  e_service_unavailable = 503
   } ;
   
   struct Return_msg
   {
	   http_status_type _type;
	   std::string _response;
	   
	   Return_msg();
	   Return_msg(const std::string&, enum http_status_type );
   } ;
  
  class Http_response_event : public Event
  {
	private:
		struct Return_msg _status;
	public:
	  Http_response_event(const std::string& res, enum http_status_type type=e_ok);
	  Http_response_event(const Return_msg&);
	  ~Http_response_event() {};
	  Http_response_event() : Event(static_get_name())
	  { }
	  static const Event_name static_get_name()
	  {
		  return "Http_response_event";
	  }
	  
	  const Return_msg& get_response() const;
  };
  
  
  class http_request_error : public std::runtime_error
  {
	enum http_status_type _type;
	
	public:
		http_request_error(const std::string&, enum http_status_type );
		
		Return_msg construct_return() const;
		
		virtual ~http_request_error() throw();
  };
  
};


  
#endif
