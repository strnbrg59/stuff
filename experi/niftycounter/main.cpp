#include "MyClass.h"

#include <iostream>
using	namespace std;

extern void Dangerous_Function();

int main()
{
	cout << "Main INIT" << endl;

	Dangerous_Function();

	// changed to nifty counter instead

	MyClassNiftyCounter::Get_Instance().Method();

	cout << "Main EXIT" << endl;
	return 0;
}
