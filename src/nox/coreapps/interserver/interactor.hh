#ifndef INTERACTOR_HH__
#define INTERACTOR_HH__

#include <boost/any.hpp>
#include <vector>
#include <string>
#include "component.hh"

namespace vigil
{
  using namespace vigil::container; 
  using boost::any_cast;
  
  typedef std::vector<boost::any> args_cont;
  
  // abstract base class for all components in interact scheme
  class Interactor : public Component
  {
	protected:
		args_cont _args;	// argumenst for concreate interactor
		std::string _response;
	public:
	
		Interactor(const Context* c, const json_object* node) : Component(c) 
		{ } 
		
		virtual const std::string& response(const args_cont&) = 0;
		
		virtual ~Interactor() {};
  };
  
};

  
#endif
