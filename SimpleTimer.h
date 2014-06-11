#include <functional>
//TIMER
class SimpleTimer : public SimpleThread {
	int ticktime; //milliseconds
	virtual DWORD Run();

public:
	std::function<void()> TickFunc; //USAGE: timer->TickFunc=std::bind(&Class::func,classPtr);
	inline void SetTickTime(int a) { ticktime=a; }	 //milliseconds
};
