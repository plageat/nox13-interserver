#ifndef INTERACTOR_HH__
#define INTERACTOR_HH__

#include "netinet++/datapathid.hh"

#include "assert.hh"
#include "vlog.hh"
#include "nox.hh"

#include "netinet++/ethernetaddr.hh"
#include <boost/foreach.hpp>
#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/noncopyable.hpp>
#include <vector>
#include <string>
#include "component.hh"
#include "json-util.hh"
#include <map>

#include <algorithm>

#define FIND_AVALUE(args, s)  ( (json::get_dict_value( (args), (s) )->get_string(true) ).c_str() )
#define EXISTS_AKEY(args, s) ( ( json::get_dict_value( (args), (s) ) ) != NULL )	
#define FIND_IF_EXISTS_ALL(args,s,key,value) ( ( EXISTS_AKEY( (args) ,(s)) )  &&  \
		( (value) = FIND_AVALUE( (args), (s) ), (key) = (s), true ) )
#define FIND_IF_EXISTS(args,s,value) ( ( EXISTS_AKEY( (args) ,(s)) )  &&  \
		( (value) = FIND_AVALUE( (args), (s) ), true ) )

#define REGISTER_ARG(tag_list, key, type) ( (tag_list).push_back \
		( std::pair<std::string, enum e_arg_type_t>( (key) , (type) ) ) )

namespace vigil
{
  using namespace vigil::json;
  using namespace vigil::container;  
  
  typedef std::map<std::string, std::string> request_arguments;
  
  enum e_arg_type_t
  {
		e_String,
		e_Num,
		e_Ipv4,
		e_Ipv6,
		e_MAC,
		e_JArray
  };

  typedef std::map<std::string, std::string> request_arguments;
  typedef std::vector< std::pair<std::string, enum e_arg_type_t> > arguments_list;
  typedef boost::shared_ptr<json_object> interact_args;
  // abstract base class for all components in interact request scheme
  class Interactor
  {
	protected:
		arguments_list _args;
		//arguments_list _addit_args;
		
	public:
		// which arg-s must be in request?
		
		const arguments_list& arg_requires() const
		{
			return _args;
		};
		
		virtual  struct ofl_msg_header * request_msg_creator(const boost::shared_ptr<json_object>& ) = 0;
		// must return true if it is modify OpenFlow request 
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
