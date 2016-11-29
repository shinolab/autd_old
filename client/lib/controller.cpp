/*
 *  autd3.cpp
 *  autd3
 *
 *  Created by Seki Inoue on 5/13/16.
 *  Copyright © 2016 Hapis Lab. All rights reserved.
 *
 */

#include <iostream>
#include <string>
#include <algorithm>
#include <vector>
#include <queue>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <chrono>
#include <boost/algorithm/string.hpp>
#include <boost/assert.hpp>
#include "autd3.hpp"
#include "privdef.hpp"
#include "ethercat_link.hpp"

class autd::Controller::impl {
 public:
    GeometryPtr _geometry;
    std::shared_ptr<internal::Link> _link;
    std::queue<GainPtr> _build_q;
    std::queue<GainPtr> _send_gain_q;
    std::queue<ModulationPtr> _send_mod_q;

    std::thread _build_thr;
    std::thread _send_thr;
    std::condition_variable _build_cond;
    std::condition_variable _send_cond;
    std::mutex _build_mtx;
    std::mutex _send_mtx;

    int8_t frequency_shift;
    bool silentMode;

    ~impl();
    bool isOpen();
    void Close();

    void InitPipeline();
    void AppendGain(const GainPtr gain);
    void AppendGainSync(const GainPtr gain);
    void AppendModulation(const ModulationPtr mod);
    void AppendModulationSync(const autd::ModulationPtr mod);
    void FlushBuffer();
    std::unique_ptr<uint8_t[]> MakeBody(GainPtr gain, ModulationPtr mod, size_t *size);
};

autd::Controller::impl::~impl() {
    if (std::this_thread::get_id() != this->_build_thr.get_id() && this->_build_thr.joinable()) this->_build_thr.join();
    if (std::this_thread::get_id() != this->_send_thr.get_id() && this->_send_thr.joinable()) this->_send_thr.join();
}

void autd::Controller::impl::InitPipeline() {
    srand(0);
    // pipeline step #1
    this->_build_thr = std::thread([&] {
        while (this->isOpen()) {
            GainPtr gain;
            // wait for gain
            {
                std::unique_lock<std::mutex> lk(_build_mtx);
                _build_cond.wait(lk, [&]{
                    return _build_q.size() || !this->isOpen();
                });
                if (_build_q.size()) {
                    gain = _build_q.front();
                    _build_q.pop();
                }
            }

            // build gain
            if (gain != nullptr && !gain->built()) gain->build();

            // pass gain to next pipeline stage
            {
                std::unique_lock<std::mutex> lk(_send_mtx);
                _send_gain_q.push(gain);
                _send_cond.notify_all();
            }
        }
    });

    // pipeline step #2
    this->_send_thr = std::thread([&] {
        try {
            while (this->isOpen()) {
                GainPtr gain;
                ModulationPtr mod;

                // wait for inputs
                {
                    std::unique_lock<std::mutex> lk(_send_mtx);
                    _send_cond.wait(lk, [&]{
                        return _send_gain_q.size() || _send_mod_q.size() || !this->isOpen();
                    });
                    if (_send_gain_q.size())
                        gain = _send_gain_q.front();
                    if (_send_mod_q.size())
                        mod = _send_mod_q.front();
                }
#ifdef DEBUG
                auto start = std::chrono::steady_clock::now();
#endif

                size_t body_size = 0;
                std::unique_ptr<uint8_t[]> body = MakeBody(gain, mod, &body_size);
                if (this->_link->isOpen()) this->_link->Send(body_size, std::move(body));
#ifdef DEBUG
                auto end = std::chrono::steady_clock::now();
                std::cout << std::chrono::duration <double, std::milli> (end-start).count() << " ms" << std::endl;
#endif
                // remove elements
                std::unique_lock<std::mutex> lk(_send_mtx);
                if (gain.get() != nullptr)
                    _send_gain_q.pop();
                if (mod.get() != nullptr && mod->buffer.size() <= mod->sent) {
                    mod->sent = 0;
                    _send_mod_q.pop();
                }

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        } catch (int errnum) {
            this->Close();
            std::cerr << "Link closed." << std::endl;
        }
    });
}

void autd::Controller::impl::AppendGain(const autd::GainPtr gain) {
    {
        std::unique_lock<std::mutex> lk(_build_mtx);
        _build_q.push(gain);
    }
    _build_cond.notify_all();
}

void autd::Controller::impl::AppendGainSync(const autd::GainPtr gain) {
    try {
        gain->SetGeometry(this->_geometry);
        if (!gain->built()) gain->build();
        size_t body_size = 0;
        std::unique_ptr<uint8_t[]> body = this->MakeBody(gain, ModulationPtr(nullptr), &body_size);
        if (this->isOpen()) this->_link->Send(body_size, std::move(body));
    }
    catch (int errnum) {
        this->_link->Close();
        std::cerr << "Link closed." << std::endl;
    }
}

void autd::Controller::impl::AppendModulation(const autd::ModulationPtr mod) {
    std::unique_lock<std::mutex> lk(_send_mtx);
    _send_mod_q.push(mod);
    _send_cond.notify_all();
}

void autd::Controller::impl::AppendModulationSync(const autd::ModulationPtr mod) {
    try {
        if (this->isOpen()) {
            while (mod->buffer.size() > mod->sent) {
                size_t body_size = 0;
                std::unique_ptr<uint8_t[]> body = this->MakeBody(autd::GainPtr(nullptr), mod, &body_size);
                this->_link->Send(body_size, std::move(body));
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
            mod->sent = 0;
        }
    }
    catch (int errnum) {
        this->Close();
        std::cerr << "Link closed." << std::endl;
    }
}


void autd::Controller::impl::FlushBuffer() {
    std::unique_lock<std::mutex> lk0(_send_mtx);
    std::unique_lock<std::mutex> lk1(_build_mtx);
    std::queue<GainPtr>().swap(_build_q);
    std::queue<GainPtr>().swap(_send_gain_q);
    std::queue<ModulationPtr>().swap(_send_mod_q);
}

std::unique_ptr<uint8_t[]> autd::Controller::impl::MakeBody(GainPtr gain, ModulationPtr mod, size_t *size) {
    int num_devices = (gain != nullptr)?gain->geometry()->numDevices():0;

    *size = sizeof(RxGlobalHeader)+sizeof(uint16_t)*NUM_TRANS_IN_UNIT*num_devices;
    std::unique_ptr<uint8_t[]> body(new uint8_t[*size]);

    RxGlobalHeader *header = reinterpret_cast<RxGlobalHeader*>(&body[0]);
    header->msg_id = static_cast<uint8_t>(rand()%256); // NOLINT
    header->control_flags = 0;
    header->mod_size = 0;

	if (this->silentMode) header->control_flags |= SILENT;

    if (mod != nullptr) {
        int mod_size = std::max(0, std::min(static_cast<int>(mod->buffer.size()-mod->sent), MOD_FRAME_SIZE));
        header->mod_size = mod_size;
        if (mod->sent == 0) header->control_flags |= MOD_BEGIN;
        if (mod->loop && mod->sent == 0) header->control_flags |= LOOP_BEGIN;
        if (mod->loop && mod->sent+mod_size >= mod->buffer.size()) header->control_flags |= LOOP_END;
        header->frequency_shift = this->_geometry->_freq_shift;

        memcpy(header->mod, &mod->buffer[mod->sent], mod_size);
        mod->sent += mod_size;
    }

    uint8_t *cursor = &body[0] + sizeof(RxGlobalHeader)/sizeof(body[0]);
    if (gain != nullptr) {
        for (int i = 0; i < gain->geometry()->numDevices(); i++) {
            int deviceId = gain->geometry()->deviceIdForDeviceIdx(i);
            size_t byteSize = NUM_TRANS_IN_UNIT*sizeof(uint16_t);
            memcpy(cursor, &gain->_data[deviceId][0], byteSize);
            cursor += byteSize/sizeof(body[0]);
        }
    }

    return body;
}

bool autd::Controller::impl::isOpen() {
    return this->_link.get() && this->_link->isOpen();
}

void autd::Controller::impl::Close() {
    if (this->isOpen()) {
        this->AppendGainSync(NullGain::Create());
        this->_link->Close();
        this->FlushBuffer();
        this->_build_cond.notify_all();
        if (std::this_thread::get_id() != this->_build_thr.get_id() && this->_build_thr.joinable()) this->_build_thr.join();
        this->_send_cond.notify_all();
        if (std::this_thread::get_id() != this->_send_thr.get_id() && this->_send_thr.joinable()) this->_send_thr.join();
        this->_link = std::shared_ptr<internal::EthercatLink>(nullptr);
    }
}

autd::Controller::Controller() {
    this->_pimpl = std::unique_ptr<impl>(new impl);
    this->_pimpl->_geometry = GeometryPtr(new Geometry());
    this->_pimpl->frequency_shift = -3;
    this->_pimpl->silentMode = true;
}

autd::Controller::~Controller() {
    this->Close();
}

void autd::Controller::Open(autd::LinkType type, std::string location) {
    this->Close();

    switch (type) {
        case LinkType::ETHERCAT: {
            // TODO(volunteer): a smarter localhost detection
            if (location == "" ||
                location.find("localhost") == 0 ||
                location.find("0.0.0.0")   == 0 ||
                location.find("127.0.0.1") == 0 ) {
                this->_pimpl->_link = std::shared_ptr<internal::LocalEthercatLink>(new internal::LocalEthercatLink());
            } else {
                this->_pimpl->_link = std::shared_ptr<internal::EthercatLink>(new internal::EthercatLink());
            }
            this->_pimpl->_link->Open(location);
            break;
        }
        default:
            BOOST_ASSERT_MSG(false, "This link type is not implemented yet.");
            break;
    }

    if (this->_pimpl->_link->isOpen())
        this->_pimpl->InitPipeline();
    else
        this->Close();
}

bool autd::Controller::isOpen() {
    return this->_pimpl->isOpen();
}

void autd::Controller::Close() {
    this->_pimpl->Close();
}

void autd::Controller::AppendGain(const GainPtr &gain) {
    gain->SetGeometry(this->_pimpl->_geometry);
    this->_pimpl->AppendGain(gain);
}

void autd::Controller::AppendGainSync(const GainPtr &gain) {
    gain->SetGeometry(this->_pimpl->_geometry);
    this->_pimpl->AppendGainSync(gain);
}

void autd::Controller::AppendModulation(const ModulationPtr &modulation) {
    this->_pimpl->AppendModulation(modulation);
}

void autd::Controller::AppendModulationSync(const ModulationPtr &modulation) {
    this->_pimpl->AppendModulationSync(modulation);
}

void autd::Controller::Flush() {
    this->_pimpl->FlushBuffer();
}

autd::Controller& autd::Controller::operator<<(const float coef) {
    ModulationPtr mod = Modulation::Create();
    mod->buffer.push_back(coef);
    mod->loop = true;
    this->AppendModulation(mod);
    return *this;
}

autd::Controller& autd::Controller::operator<<(const ModulationPtr &coef) {
    this->AppendModulation(coef);
    return *this;
}

autd::Controller& autd::Controller::operator<<(const GainPtr &gain) {
    this->AppendGain(gain);
    return *this;
}

autd::GeometryPtr autd::Controller::geometry() {
    return this->_pimpl->_geometry;
}

void autd::Controller::SetGeometry(const GeometryPtr &geometry) {
    this->_pimpl->_geometry = geometry;
}

size_t autd::Controller::remainingInBuffer() {
    return this->_pimpl->_send_gain_q.size() + this->_pimpl->_send_mod_q.size() + this->_pimpl->_build_q.size();
}

void autd::Controller::SetSilentMode(bool silent) {
    this->_pimpl->silentMode = silent;
}

bool autd::Controller::silentMode() {
    return this->_pimpl->silentMode;
}
