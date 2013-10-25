#include "myclass.h"

// define the static here

int			MyClassNiftyCounter::nifty_count_	= 0;
MyClass*	MyClassNiftyCounter::p_				= 0;

///////////

MyClassNiftyCounter::MyClassNiftyCounter()
{
	++nifty_count_;
}

MyClassNiftyCounter::~MyClassNiftyCounter()
{
	if ( 0 == --nifty_count_ && p_ )
	{
		delete	p_;
	}
}

MyClass&	MyClassNiftyCounter::Get_Instance()
{
	if ( ! p_ )
	{
		p_	= new MyClass();
	}

	return	*p_;
}


