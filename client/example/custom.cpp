/*
 *  custom.cpp
 *  custom
 *
 *  Created by Seki Inoue on 5/18/16.
 *  Copyright © 2016 Hapis Lab. All rights reserved.
 *
 */

#define _USE_MATH_DEFINES
#include <math.h>
#include <thread>
#include "autd3.hpp"

#ifdef WIN32
#include <signal.h>
#endif

using namespace std;

volatile sig_atomic_t sigflag = 0;

class MyGain : public autd::Gain {
public:
    void build() {
        // Call super method to allocate a buffer for gain data.
        // The size of buffer is automatically determined by the context.
        autd::Gain::build();
        
        // Set amp and phase of each transducer.
		// We drive only 100 transducers here.
        for (int device_id = 0; device_id < this->_data.size(); device_id++) {
            for (int transducer_id = 0; transducer_id < this->_data[device_id].size(); transducer_id++) {
                uint8_t amp = 255;
                uint8_t phase = 0;
				if (transducer_id > 68 && (transducer_id % 18 >= 15 || transducer_id <= 6))
					this->_data[device_id][transducer_id] = ((uint16_t)amp << 8) + phase;
				else
					this->_data[device_id][transducer_id] = 0xFFFF;
            }
        }
        
    };
};

class MyModulation : public autd::Modulation {
public:
    MyModulation() {
		float duration = 0.5;
		float freq = 300;
        int buffer_size = this->samplingFrequency()*duration;
        this->buffer.resize(buffer_size, 0);
        for (int i = 0; i < buffer_size; i++) {
			float tamp = 127.5f + 127.5f*cosf(2.0*M_PI*(i) * freq / this->samplingFrequency());
			this->buffer[i] = floor(fmin(fmaxf(tamp, 0.0f), 255.0f));
        }
        this->loop = false;
    }
};

int main(int argc, char const *argv[])
{
    signal(SIGINT, [](int signum) {
        sigflag = 1;
    });
    
    autd::Controller autd;
	autd.Open(autd::LinkType::ETHERCAT);
    if (!autd.isOpen()) return ENXIO;
    
	
	autd.geometry()->AddDevice(Eigen::Vector3f(0, 0, 0), Eigen::Vector3f(0, 0, 0));

    autd::GainPtr gain = autd::GainPtr(new MyGain());
    autd::ModulationPtr mod = autd::ModulationPtr(new MyModulation());
    
	autd.AppendGain(gain);
	autd.AppendModulation(autd::SineModulation::Create(50));
	std::cout << "press any key to continue..." << std::endl;
	getchar();
	autd.AppendGain(autd::FocalPointGain::Create(Eigen::Vector3f(90, 70, 200)));
	while (!sigflag) {
		autd.SetSilentMode(!autd.silentMode());

		// play custom modulaion and then play static pressure
		autd.AppendModulation(mod);
		autd.AppendModulation(autd::SineModulation::Create(100));
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		// stop ultrasound for 1000 msec
		autd.AppendModulation(autd::Modulation::Create(0));
		std::this_thread::sleep_for(std::chrono::milliseconds(1000));
	}
    std::cout << "disconnecting..." << std::endl;
    autd.Close();
	return 0;
}
