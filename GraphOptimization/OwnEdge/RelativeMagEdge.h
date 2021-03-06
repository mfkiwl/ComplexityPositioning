/** 
                   _ooOoo_ 
                  o8888888o 
                  88" . "88 
                  (| -_- |) 
                  O\  =  /O 
               ____/`---'\____ 
             .'  \\|     |//  `. 
            /  \\|||  :  |||//  \ 
           /  _||||| -:- |||||-  \ 
           |   | \\\  -  /// |   | 
           | \_|  ''\---/''  |   | 
           \  .-\__  `-`  ___/-. / 
         ___`. .'  /--.--\  `. . __ 
      ."" '<  `.___\_<|>_/___.'  >'"". 
     | | :  `- \`.;`\ _ /`;.`/ - ` : | | 
     \  \ `-.   \_ __\ /__ _/   .-` /  / 
======`-.____`-.___\_____/___.-`____.-'====== 
                   `=---=' 
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ 
         佛祖保佑       永无BUG 
*/
//
// Created by steve on 17-11-6.
//

#ifndef QUICKFUSING_RELATIVEMAGEDGE_H
#define QUICKFUSING_RELATIVEMAGEDGE_H

#include "g2o/core/sparse_optimizer.h"
#include "g2o/core/block_solver.h"
#include "g2o/core/factory.h"
#include "g2o/core/optimization_algorithm_levenberg.h"
#include "g2o/solvers/csparse/linear_solver_csparse.h"
#include "g2o/types/slam3d/types_slam3d.h"
#include "g2o/types/slam3d_addons/types_slam3d_addons.h"
#include <sophus/se3.h>
#include <sophus/so3.h>


class RelativeMagEdge :
        public g2o::BaseBinaryEdge<3, Eigen::Vector3d, g2o::VertexSE3, g2o::VertexSE3> {
public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW;

    Eigen::Vector3d src_mag_;
    Eigen::Vector3d target_mag_;

    RelativeMagEdge(const Eigen::Vector3d &src_mag,
                    const Eigen::Vector3d &target_mag);

    virtual bool read(std::istream &is);

    virtual bool write(std::ostream &os) const;

    void computeError();

    void setMeasurement(const Eigen::Vector3d &m) {
        _measurement = m;
    }

    virtual bool getMeasurementData(double *d) const {
        Eigen::Map<Eigen::Vector3d> v(d);
        v = _measurement;
        return true;
    }

    virtual int measurementDimension() const {
        return 3;
    }

    virtual bool setMeasurementFromState();

    virtual double initialEstimatePossible(
            const g2o::OptimizableGraph::VertexSet &/*from*/,
            g2o::OptimizableGraph::Vertex */*to*/) {
//        //TODO:
//        std::cout << __FILE__ << __FUNCTION__
//                  << __LINE__ << "this function not implement" << std::endl;
        return 1.0;
    }


    virtual void initialEstimate(const g2o::OptimizableGraph::VertexSet &from,
                                 g2o::OptimizableGraph::Vertex *to);


    double sigma_ = 1.0;

    bool setSigma(double sigma) {
        if (sigma > 0) {
            sigma_ = sigma;
            return true;
        } else {
            return false;
        }

    }

    double getSigma() {
        return sigma_;
    }

    inline double logNormalPdf(double x, double miu, double sigma) {
        double para1((x - miu) * (x - miu) / 2 / sigma / sigma);
        double para2(1 / std::sqrt(2 * sigma * sigma * M_PI));

        return std::log(para2 + 1e-10) / para1;
    }

    inline double NormalPdf(double x,
                            double miu,
                            double sigma) {
//    std::cout << "dis :" << x << " range:" << miu << std::endl;
        double para1((x - miu) * (x - miu) / 2 / sigma / sigma);
        double para2(1 / std::sqrt(2 * sigma * sigma * M_PI));
        return para2 * std::exp(-para1);
    }

};


#endif //QUICKFUSING_RELATIVEMAGEDGE_H
