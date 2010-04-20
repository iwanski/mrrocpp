#include <iostream>

#include "Agent.hh"
#include "DataBuffer.hh"

class Reader : public Agent {
private:
	DataBuffer<int> IntBuffer;
	DataBuffer<double> DoubleBuffer;
public:

	Reader(const std::string & name) : Agent(name),
		IntBuffer("integer buffer"),
		DoubleBuffer("double buffer")
	{
		registerBuffer(IntBuffer);
		registerBuffer(DoubleBuffer);
		listBuffers();
	}

	bool step() {
		// Wait() << IntBuffer & DoubleBuffer;
		AndBufferContainer cont1 = (IntBuffer && DoubleBuffer);
		AndBufferContainer cont2 = cont1; //DoubleBuffer;
		AndBufferContainer cont3 = (cont1 && cont2);

		OrBufferContainer or1(cont1);// = DoubleBuffer || IntBuffer;
		cont1.print(); std::cout << std::endl;
		Wait(DoubleBuffer);

//		cont3
		int i = Get<int>("integer buffer");
		double d = Get<double>("double buffer");
		std::cout << "Reader: " << i << "," << d << std::endl;
		// Send() << IntBuffer & DoubleBuffer; //?
		return true;
	}
};