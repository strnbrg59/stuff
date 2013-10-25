#include "MyClass.h"

class Dangerous
{
public:
	Dangerous()  {}
	~Dangerous()
	{
		// change to nifty counter instead

		MyClassNiftyCounter::Get_Instance().Method();  // dangerous
	}
};

void Dangerous_Function()
{
	static Dangerous danger;    // undefined order of destruction
}
