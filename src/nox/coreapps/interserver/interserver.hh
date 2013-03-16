#ifndef INTERSERVER_HH__
#define INTERSERVER_HH__

// boost wave bug, 6 hours of my life...
#undef _GLIBCXX_CONCEPT_CHECKS
/** Server port number 
 */
#define INTERSERVER_PORT "4695"

#define POST_BUFFER_SIZE 1024

#include "http_event.hh"
#include <boost/noncopyable.hpp>
#include <map>
#include <boost/network/protocol/http/server.hpp>
#include "component.hh"
#include "threads/cooperative.hh"

namespace vigil
{
  using namespace vigil::container;
  using namespace boost;
  namespace http = boost::network::http;
  
  /*<< Defines the server. >>*/
  struct http_handler;
  typedef http::server<http_handler> server;
  
  struct http_handler
  {
    void operator() (server::request const &request, server::response &) ;

    void log(...);
  };
  
  class Interserver : public Component, Co_thread
  {
	friend struct http_handler;

	private:
		std::string _port;
		boost::shared_ptr<server> _server;
	public:
	  Interserver(const Context* c, const json_object* node)
			: Component(c),_port(INTERSERVER_PORT)
	  {
	  }
	  
	  ~Interserver();
	  /** Configure component
     * Register events..
     * @param config configuration
     */
	void configure(const Configuration* config);

    /** Start component.
     */
    void install();
	/** Get instance of messenger_core (for python)
     * @param ctxt context
     * @param scpa reference to return instance with
     */
    static void getInstance(const container::Context* ctxt, 
			    vigil::Interserver*& scpa);
	
	void listen_state();
	
	Disposition response_handler(const Event&);
	
	private:
	// used to interact with request handler
	// ???
	static struct Request_process_Info
	{
		volatile bool _accept_post;
		Request_msg _request_msg;
		volatile bool _accept_response;
		std::string _response;
	}	_rp_info;
	
  };
	

};

  
#endif
