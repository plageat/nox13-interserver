#ifndef HTTP_EVENT_HH__
#define HTTP_EVENT_HH__

#include <boost/noncopyable.hpp>
#include <map>
#include "component.hh"


namespace vigil
{
  using namespace vigil::container; 
  
  struct Request_msg
  {
	std::string _url;
	std::string _method;
	std::string _version;
	std::string _upload_data;
	std::map<std::string,std::string> _url_args;
	std::map<std::string,std::string> _post_data;
	
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
  
  class Http_response_event : public Event
  {
	private:
	  std::string _response;
	public:
	  Http_response_event(const std::string& res);
	  ~Http_response_event() {};
	  Http_response_event() : Event(static_get_name())
	  { }
	  static const Event_name static_get_name()
	  {
		  return "Http_response_event";
	  }
	  
	  const std::string& get_response() const;
  };
  
};


  
#endif
