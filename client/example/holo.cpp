/*
 *  holo.cpp
 *  holo
 *
 *  Created by Seki Inoue on 7/6/16.
 *  Copyright © 2016 Hapis Lab. All rights reserved.
 *
 */

#define _USE_MATH_DEFINES
#include <math.h>
#include <thread>
#include "autd3.hpp"

using namespace std;

int main(int argc, char const *argv[])
{
    autd::Controller autd;
	autd.Open(autd::LinkType::ETHERCAT); // Connect to AUTD Server at local
    if (!autd.isOpen()) return ENXIO;
    
    autd.geometry()->AddDevice(Eigen::Vector3f::Zero(), Eigen::Vector3f::Zero()); // Place a first board at origin
    autd.geometry()->AddDevice(Eigen::Vector3f(182, -10, 0), Eigen::Vector3f(M_PI, 0, 0)); // Place a second board

	// Make Hologram
    Eigen::MatrixX3f holo(2,3);
    holo << 160, 70, 200,
             20, 70, 200;

    Eigen::VectorXf amp(2);
    amp << 1, 1;
    
    autd.AppendGainSync(autd::HoloGainSdp::Create(holo, amp));
    autd.AppendModulation(autd::SineModulation::Create(150)); // 150Hz AM
	std::cout << "press any key to continue..." << std::endl;
	getchar();
    std::cout << "disconnecting..." << std::endl;
    autd.Close();
	return 0;
}
