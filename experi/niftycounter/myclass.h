#ifndef	MYCLASS_H
#define	MYCLASS_H

#include <iostream>

class	MyClassNiftyCounter;

class MyClass
{
	friend	MyClassNiftyCounter;	// give friendship


	MyClass()
	{
		std::cout << "MyClass CTOR\n" << std::flush;
	};

	MyClass( const MyClass & );        // undefined
	MyClass& operator=( MyClass );

public:

	// made public because VC++ 6 has a bug
	~MyClass()
	{
		std::cout << "MyClass DTOR\n" << std::flush;
	};

	void  Method()
	{
		std::cout << "Method called\n" << std::flush;
	}
};


class MyClassNiftyCounter
{
public:

	MyClassNiftyCounter();

	~MyClassNiftyCounter();

	static	MyClass&	Get_Instance();

private:

	static	int		nifty_count_;

	static	MyClass	*p_;
};


namespace
{
	// a counter per cpp file including this header

	// require the static because of another VC++ bug
	// annoymous namespace doesn't work

	static	MyClassNiftyCounter		local_counter_;
}


#endif	// MYCLASS_H
