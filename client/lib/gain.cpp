//
//  gain.cpp
//  autd3
//
//  Created by Seki Inoue on 6/1/16.
//
//

#include <iostream>
#include <string>
#include <map>
#include <vector>
#include <boost/assert.hpp>
#include "privdef.hpp"
#include "autd3.hpp"


autd::GainPtr autd::Gain::Create() {
    return GainPtr(new Gain());
}

autd::Gain::Gain() {
    this->_built = false;
    this->_geometry = GeometryPtr(nullptr);
}

void autd::Gain::build() {
    if (this->built()) return;
    if (this->geometry().get() == nullptr) BOOST_ASSERT_MSG(false, "Geometry is required to build Gain");
    
    for (int i = 0; i < this->geometry()->numDevices(); i++) {
        this->_data[this->geometry()->deviceIdForDeviceIdx(i)] = std::vector<uint16_t>(NUM_TRANS_IN_UNIT, 0x0000);
    }
}

bool autd::Gain::built() {
    return this->_built;
}

void autd::Gain::SetGeometry(const autd::GeometryPtr &geometry) {
    this->_geometry = geometry;
}

autd::GeometryPtr autd::Gain::geometry() {
    return this->_geometry;
}

autd::GainPtr autd::PlaneWaveGain::Create(Eigen::Vector3f direction) {
    std::shared_ptr<PlaneWaveGain> ptr = std::shared_ptr<PlaneWaveGain>(new PlaneWaveGain());
    ptr->_direction = direction;
    return ptr;
}

void autd::PlaneWaveGain::build() {
    if (this->built()) return;
    if (this->geometry() == nullptr) BOOST_ASSERT_MSG(false, "Geometry is required to build Gain");

    this->_data.clear();
    const int ndevice = this->geometry()->numDevices();
    for (int i = 0; i < ndevice; i++) {
        this->_data[this->geometry()->deviceIdForDeviceIdx(i)].resize(NUM_TRANS_IN_UNIT);
    }

    const int ntrans = this->geometry()->numTransducers();
    for (int i = 0; i < ntrans; i++) {
        this->_data[this->geometry()->deviceIdForTransIdx(i)][i%NUM_TRANS_IN_UNIT] = 0xFF00;
    }

    this->_built = true;
}

autd::GainPtr autd::FocalPointGain::Create(Eigen::Vector3f point) {
    std::shared_ptr<FocalPointGain> ptr = std::shared_ptr<FocalPointGain>(new FocalPointGain());
    ptr->_point = point;
    ptr->_geometry = GeometryPtr(nullptr);
    return ptr;
}

void autd::FocalPointGain::build() {
    if (this->built()) return;
    if (this->geometry() == nullptr) BOOST_ASSERT_MSG(false, "Geometry is required to build Gain");

    this->_data.clear();
    const int ndevice = this->geometry()->numDevices();
    for (int i = 0; i < ndevice; i++) {
        this->_data[this->geometry()->deviceIdForDeviceIdx(i)].resize(NUM_TRANS_IN_UNIT);
    }
    const int ntrans = this->geometry()->numTransducers();
    for (int i = 0; i < ntrans; i++) {
        Eigen::Vector3f trp = this->geometry()->position(i);
        float dist = (trp-this->_point).norm();
        float fphase = fmodf(dist, ULTRASOUND_WAVELENGTH)/ULTRASOUND_WAVELENGTH;
        uint8_t amp = 0xff;
        uint8_t phase = round(255.0*(1-fphase));
        this->_data[this->geometry()->deviceIdForTransIdx(i)][i%NUM_TRANS_IN_UNIT] = ((uint16_t)amp << 8) + phase;
    }

    this->_built = true;
}

autd::GainPtr autd::TransducerTestGain::Create(int idx, int amp, int phase) {
    std::shared_ptr<TransducerTestGain> ptr = std::shared_ptr<TransducerTestGain>(new TransducerTestGain());
    ptr->_xdcr_idx = idx;
    ptr->_amp = amp;
    ptr->_phase = phase;
    return ptr;
}

void autd::TransducerTestGain::build() {
    if (this->built()) return;
    if (this->geometry() == nullptr) BOOST_ASSERT_MSG(false, "Geometry is required to build Gain");

    this->_data.clear();
    const int ndevice = this->geometry()->numDevices();
    for (int i = 0; i < ndevice; i++) {
        this->_data[this->geometry()->deviceIdForDeviceIdx(i)].resize(NUM_TRANS_IN_UNIT);
    }
    this->_data[this->geometry()->deviceIdForTransIdx(_xdcr_idx)][_xdcr_idx%NUM_TRANS_IN_UNIT] = (_amp << 8) + (_phase);
    this->_data[0][0] = 0xff00;
    this->_built = true;
}
