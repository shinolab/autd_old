//
//  holo_gain.cpp
//  autd3
//
//  Created by Seki Inoue on 7/6/16.
//
//

#include <iostream>
#include <string>
#include <map>
#include <complex>
#include <vector>
#include <boost/assert.hpp>
#include "privdef.hpp"
#include "autd3.hpp"
#include <boost/random.hpp>
#include <Eigen/Eigen>
#define _USE_MATH_DEFINES
#include <math.h>

#define REPEAT_SDP (10)
#define LAMBDA_SDP (0.8)

namespace hologainimpl {
    // TODO: use cache and interpolate. don't use exp
    std::complex<float> transfer(Eigen::Vector3f trans_pos, Eigen::Vector3f trans_norm, Eigen::Vector3f target_pos) {
        Eigen::Vector3f diff = trans_pos-target_pos;
        float dist = diff.norm();
        float cos = diff.dot(trans_norm)/dist/trans_norm.norm();
        
        //1.11   & 1.06  &  0.24 &  -0.12  &  0.035
        float directivity = 1.11 *sqrt((2*0+1)/(4*M_PI))*1 + 1.06*sqrt((2*1+1)/(4*M_PI))*cos + 0.24*sqrt((2*2+1)/(4*M_PI))/2*(3*cos*cos-1)  -0.12*sqrt((2*3+1)/(4*M_PI))/2*cos*(5*cos*cos-3) + 0.035*sqrt((2*4+1)/(4*M_PI))/8*(35*cos*cos*cos*cos-30*cos*cos+3);
        
        std::complex<float> g = directivity/dist*exp(std::complex<float>(-dist*1.15e-4, -2*M_PI/ULTRASOUND_WAVELENGTH*dist));
        return g;
    }
    
    void removeRow(Eigen::MatrixXcf& matrix, unsigned int rowToRemove)
    {
        unsigned int numRows = matrix.rows()-1;
        unsigned int numCols = matrix.cols();
        
        if( rowToRemove < numRows )
            matrix.block(rowToRemove,0,numRows-rowToRemove,numCols) = matrix.block(rowToRemove+1,0,numRows-rowToRemove,numCols);
        
        matrix.conservativeResize(numRows,numCols);
    }
    
    void removeColumn(Eigen::MatrixXcf& matrix, unsigned int colToRemove)
    {
        unsigned int numRows = matrix.rows();
        unsigned int numCols = matrix.cols()-1;
        
        if( colToRemove < numCols )
            matrix.block(0,colToRemove,numRows,numCols-colToRemove) = matrix.block(0,colToRemove+1,numRows,numCols-colToRemove);
        
        matrix.conservativeResize(numRows,numCols);
    }
}

autd::GainPtr autd::HoloGainSdp::Create(Eigen::MatrixX3f foci, Eigen::VectorXf amp) {
    std::shared_ptr<HoloGainSdp> ptr = std::shared_ptr<HoloGainSdp>(new HoloGainSdp());
    ptr->_foci = foci;
    ptr->_amp = amp;
    return ptr;
}

void autd::HoloGainSdp::build() {
    if (this->built()) return;
    if (this->geometry() == nullptr) BOOST_ASSERT_MSG(false, "Geometry is required to build Gain");

    //const double pinvtoler=1.e-7;
    const float alpha = 1e-3;
    
    const int M = (int)_foci.rows();
    const int N = (int)this->geometry()->numTransducers();
    
    Eigen::MatrixXcf P = Eigen::MatrixXcf::Zero(M, M);
    Eigen::VectorXcf p = Eigen::VectorXcf::Zero(M);
    Eigen::MatrixXcf B = Eigen::MatrixXcf(M,N);
    Eigen::VectorXcf q = Eigen::VectorXcf(N);
    
    boost::random::mt19937 rng( static_cast<unsigned int>(time(0)) );
    boost::random::uniform_real_distribution<> range(0,1);
    boost::random::variate_generator< boost::random::mt19937, boost::random::uniform_real_distribution<> > mt(rng, range);
    
    for (int i = 0; i < M; i++) {
        p(i) = _amp(i)*exp(std::complex<float>(0,2*M_PI*mt()));
        P(i,i) = _amp(i);
        
        Eigen::Vector3f q = _foci.row(i);
        for (int j = 0; j < N; j++) {
            B(i,j) = hologainimpl::transfer(
                        this->geometry()->position(j),
                        this->geometry()->direction(j),
                        q);
        }
    }
    
    Eigen::JacobiSVD<Eigen::MatrixXcf> svd(B, Eigen::ComputeThinU | Eigen::ComputeThinV);
    Eigen::JacobiSVD<Eigen::MatrixXcf>::SingularValuesType singularValues_inv= svd.singularValues();
    for ( long i=0; i < singularValues_inv.size(); ++i) {
        singularValues_inv(i) =  singularValues_inv(i) / (singularValues_inv(i) *singularValues_inv(i)  + alpha*alpha);
    }
    Eigen::MatrixXcf pinvB = (svd.matrixV()*singularValues_inv.asDiagonal()*svd.matrixU().adjoint());
    
    Eigen::MatrixXcf MM = P*(Eigen::MatrixXcf::Identity(M, M) - B*pinvB)*P;
    Eigen::MatrixXcf X = Eigen::MatrixXcf::Identity(M, M);
    for (int i = 0; i < REPEAT_SDP; i++) {
        unsigned int ii = mt()*M;
        
        Eigen::MatrixXcf Xc = X;
        hologainimpl::removeRow(Xc, ii);
        hologainimpl::removeColumn(Xc, ii);
        Eigen::VectorXcf MMc = MM.col(ii);
        MMc.block(ii,0,MMc.rows()-1-ii,1) = MMc.block(ii+1,0,MMc.rows()-1-ii,1);
        MMc.conservativeResize(MMc.rows()-1, 1);
        
        Eigen::VectorXcf x = Xc*MMc;
        std::complex<float> gamma = x.adjoint()*MMc;
        if (gamma.real()>1e-9) {
            x = -x*sqrt(LAMBDA_SDP/gamma.real());
            X.block(ii,0,1,ii) = x.block(0,0,ii,1).adjoint().eval();
            X.block(ii,ii+1, 1, M-ii-1) = x.block(ii,0,M-1-ii,1).adjoint().eval();
            X.block(0,ii,ii,1) = x.block(0,0,ii,1).eval();
            X.block(ii+1,ii, M-ii-1, 1) = x.block(ii,0,M-1-ii,1).eval();
        }else {
            X.block(ii,0,1,ii) = Eigen::VectorXcf::Zero(ii).adjoint();
            X.block(ii,ii+1, 1, M-ii-1) = Eigen::VectorXcf::Zero(M-1).adjoint();
            X.block(0,ii,ii,1) = Eigen::VectorXcf::Zero(ii);
            X.block(ii+1,ii, M-ii-1, 1) = Eigen::VectorXcf::Zero(M-1);
        }
        
    }
    
    Eigen::ComplexEigenSolver<Eigen::MatrixXcf> ces(X);
    Eigen::VectorXcf evs = ces.eigenvalues();
    float abseiv = 0;
    int idx = 0;
    for (int j = 0; j < evs.rows(); j++) {
        float eiv = abs(evs(j));
        if (abseiv < eiv) {
            abseiv = eiv;
            idx = j;
        }
    }
    
    Eigen::VectorXcf u = ces.eigenvectors().col(idx);
    q = pinvB*P*u;
    
    this->_data.clear();
    const int ndevice = this->geometry()->numDevices();
    for (int i = 0; i < ndevice; i++) {
        this->_data[this->geometry()->deviceIdForDeviceIdx(i)].resize(NUM_TRANS_IN_UNIT);
    }
    
    //float maxCoeff = sqrt(q.cwiseAbs2().maxCoeff());
    for (int j = 0; j < N; j++) {
        float famp = 1.0;//abs(q(j))/maxCoeff;
        float fphase = arg(q(j)) / (2*M_PI) + 0.5;
        uint8_t amp = famp*255, phase = (1-fphase)*255;
        this->_data[this->geometry()->deviceIdForTransIdx(j)][j%NUM_TRANS_IN_UNIT] = ((uint16_t)amp << 8) + phase;
    }
}
