#ifndef INTERACTOR_HH__
#define INTERACTOR_HH__

#include "netinet++/datapathid.hh"
//#include "ofp-msg-event.hh"
#include "assert.hh"
#include "vlog.hh"
#include "nox.hh"

#include <boost/shared_ptr.hpp>
//#include <boost/any.hpp>
#include <boost/noncopyable.hpp>
#include <vector>
#include <string>
#include "component.hh"
#include "json-util.hh"
#include <map>

namespace vigil
{
  using namespace vigil::json;
  using namespace vigil::container;  
  /*
  template<typename T>
  class SimpleSingleton : boost::noncopyable
  {
	public:
		static T& Instance()
		{
			static T _obj;
			return _obj;
		}
	private:
		SimpleSingleton();
		~SimpleSingleton();
  };*/
  
  typedef std::map<std::string, std::string> request_arguments;
  
  // abstract base class for all components in interact request scheme
  class Interactor
  {
	public:
		// which arg-s must be in request?
		virtual std::vector<std::string> arg_requires() = 0;
		
		virtual  struct ofl_msg_header * request_msg_creator(const request_arguments& ) = 0;
		// returns true if it is modify OpenFlow request 
		virtual bool is_modify() const = 0;
		
		virtual const char* get_name() const
		{ 
			return typeid(*this).name() ; 
		};
		
		virtual ~Interactor() {};
	
  };
  
  template< class AbstractObj, typename Identifier, typename Creator >
  class cl_factory : boost::noncopyable
  {
	private:
		typedef std::map<Identifier, Creator> save_struct;
		save_struct _assoc;
	public:
		bool register_object(const Identifier& key, Creator creator)
		{
			return  _assoc.insert(typename save_struct::value_type(key,creator) ).second;
		}
		
		bool unregister_object(const Identifier& key)
		{
			return _assoc.erase(key) == 1;
		} 
		
		boost::shared_ptr<AbstractObj> create_object(const Identifier& key)
		{
			typename save_struct::iterator i;
			
			if((i = _assoc.find(key)) == _assoc.end() )
				return boost::shared_ptr<AbstractObj> ();

			return boost::shared_ptr<AbstractObj> ( (i->second)() );
		}
		
		void Debug() const 
		{
			typename save_struct::const_iterator i1 = _assoc.begin();
			typename save_struct::const_iterator i2 = _assoc.end();
			for(; i1 != i2; ++i1)
				std::cout << i1->first << std::endl;
		}
  };
  
  //typedef SimpleSingleton< cl_factory<Interactor, const char*, Interactor* (*)()> > InterFactory;
  
  template <typename T>
  class InteractorCreator
  {
	public:
	  
	  static Interactor* create()
	  {
		  T* obj = new T;
		  // check type T is derrived from Interactor
		  return dynamic_cast<Interactor*>(obj);
	  }
  };
  
};

  
#endif
