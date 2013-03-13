#ifndef INTERSERVER_HH__
#define INTERSERVER_HH__

/** Server port number 
 */
#define INTERSERVER_PORT 4695

#define POST_BUFFER_SIZE 1024

enum e_Connection_type 
{
	e_READ,	// GET
	e_MODIFY // POST or PUT or something to modify data
};

#include "http_event.hh"
#include <boost/noncopyable.hpp>
#include <map>
#include <microhttpd.h>
#include "component.hh"
#include "threads/cooperative.hh"
#include "datapath-join.hh"
#include "datapath-leave.hh"

namespace vigil
{
  using namespace vigil::container; 
  
  struct Http_connection_info 
  {
	  enum e_Connection_type _connection_type;
	  MHD_PostProcessor * _pp;
	  
	  Http_connection_info();
	  ~Http_connection_info();
  };
  
  extern int url_handler (void *cls,
			struct MHD_Connection *connection,
				const char *url,
					const char *method,
						const char *version,
							const char *upload_data, unsigned int *upload_data_size, void **ptr);
  
  class Interserver : public Component, Co_thread
  {
	  friend int url_handler (void *cls,
			struct MHD_Connection *connection,
				const char *url,
					const char *method,
						const char *version,
							const char *upload_data, unsigned int *upload_data_size, void **ptr);
	private:
		unsigned short _port;
		MHD_Daemon *_daemon;
		
	public:
	  Interserver(const Context* c, const json_object* node)
			: Component(c),_port(INTERSERVER_PORT),_daemon(NULL)
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
	
	void run();
	
	Disposition response_handler(const Event&);
	
	private:
	// used to interact with request handler
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
