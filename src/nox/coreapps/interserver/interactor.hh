#ifndef INTERACTOR_HH__
#define INTERACTOR_HH__

#include "netinet++/datapathid.hh"
//#include "ofp-msg-event.hh"
#include "assert.hh"
#include "vlog.hh"
#include "nox.hh"

#include "netinet++/ethernetaddr.hh"
#include <boost/foreach.hpp>
#include <utility>
#include <boost/shared_ptr.hpp>
//#include <boost/any.hpp>
#include <boost/noncopyable.hpp>
#include <vector>
#include <string>
#include "component.hh"
#include "json-util.hh"
#include <map>
#include <boost/lexical_cast.hpp>
#include <algorithm>

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
  
  enum e_arg_type_t
  {
		e_String,
		e_Num,
		e_Ipv4,
		e_Ipv6,
		e_MAC
  };
  namespace 
  {
	  // need optimization, mb regex or something like
  	bool is_valid_number(const std::string& s)
	{
		try
		{
			boost::lexical_cast<int>(s);
			
			return true;
		}
		catch(...)
		{
			return false;
		}
	}
	
	bool is_valid_ipv4(const std::string& s)
	{
		struct in_addr addr;
		int ret = inet_pton(AF_INET,s.c_str(),&addr);
			
		return ret == 1;
	}
	
	bool is_valid_ipv6(const std::string& s)
	{
		struct in6_addr addr;
		int ret = inet_pton(AF_INET6,s.c_str(),&addr);
		
		return ret == 1;
	}
	
	bool is_valid_eth(const std::string& s)
	{
		try
		{
			ethernetaddr(s);
			return true;
		}
		catch(bad_ethernetaddr_cast& e)
		{
			return false;
		}
	}
	
  }
  typedef std::map<std::string, std::string> request_arguments;
  typedef std::vector< std::pair<std::string, enum e_arg_type_t> > arguments_list;
  // abstract base class for all components in interact request scheme
  class Interactor
  {
	protected:
		arguments_list _args;
		arguments_list _addit_args;
		/*
		virtual void check_args(const request_arguments& a)
		{
			//function for optimization
			std::string value;
			arguments_list l;
			request_arguments::const_iterator i;
			
			l.reserve(_args.size() + _addit_args.size() );
			l.insert( l.end(),_args.begin(),_args.end() );
			l.insert( l.end(),_addit_args.begin(),_addit_args.end() );
			
			std::pair< std::string, enum e_arg_type_t > p;

			BOOST_FOREACH(p, l)
			{
				i = a.find(p.first);
				if ( i == a.end() )
					continue;
					
				value = i->second;
				
				switch(p.second)
				{
					case e_Num:
					{
						if ( is_valid_number(value) == false )
						{
							std::string msg = p.first;
							msg += " has invalid value!\n";
							throw std::invalid_argument(msg);
						}
						break;
					};
					case e_Ipv4:
					{
						if ( is_valid_ipv4(value) == false )
						{
							std::string msg = p.first;
							msg += " has invalid ip4 address!\n";
							throw std::invalid_argument(msg);
						}
						break;
					};
					case e_Ipv6:
					{
						if ( is_valid_ipv6(value) == false )
						{
							std::string msg = p.first;
							msg += " has invalid ip6 address!\n";
							throw std::invalid_argument(msg);
						}
						break;
					};
					case e_MAC:
					{
						if ( is_valid_eth(value) == false )
						{
							std::string msg = p.first;
							msg += " has invalid MAC address!\n";
							throw std::invalid_argument(msg);
						}
						
						break;
					}
					
					default:
						break;
				};
			}
			
		}*/
		
	public:
		// which arg-s must be in request?
		/*
		virtual std::vector<std::string> arg_requires() 
		{
			std::vector<std::string> a;
			std::pair< std::string, enum e_arg_type_t > p;
			
			BOOST_FOREACH(p, _args)
			{
				a.push_back(p.first);
			}
			
			return a;
		};*/
		// additional optional arguments
		/*
		virtual std::vector<std::string> additional_args()
		{
			std::vector<std::string> a;
			std::pair< std::string, enum e_arg_type_t > p;
			
			BOOST_FOREACH(p, _addit_args)
			{
				a.push_back(p.first);
			}
			
			return a;
		};*/
		
		virtual  struct ofl_msg_header * request_msg_creator(const boost::shared_ptr<json_object>& ) = 0;
		// must return true if it is modify OpenFlow request 
		virtual bool is_modify() const = 0;
		
		virtual const char* get_name() const
		{ 
			return typeid(*this).name() ; 
		};
		
		virtual ~Interactor() {};
	
  };
  
  typedef boost::shared_ptr<json_object> interact_args;
  
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
