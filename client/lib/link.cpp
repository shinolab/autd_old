//
//  link.cpp
//  autd3
//
//  Created by Seki Inoue on 6/17/16.
//
//

#include <stdio.h>
#include "link.hpp"

std::vector<uint16_t>& autd::internal::Link::accessGainData(GainPtr &gain, const int deviceId) {
    return gain->_data[deviceId];
}

int& autd::internal::Link::accessSent(ModulationPtr &mod) {
    return mod->sent;
}