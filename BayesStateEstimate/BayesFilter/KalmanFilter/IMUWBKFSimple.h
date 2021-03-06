//
// Created by steve on 18-1-27.
//

#ifndef COMPLEXITYPOSITIONING_IMUWBKF_H
#define COMPLEXITYPOSITIONING_IMUWBKF_H

//#include <sophus/so3.h>
//#include <sophus/se3.h>
#include  <sophus/so3.hpp>
#include <sophus/se3.hpp>

#include "KalmanFilterBase.h"
#include "KalmanFilterBase.cpp"


#include "AuxiliaryTool/AccMeasurementFunction.h"

#include <Eigen/Dense>
#include <Eigen/Geometry>

namespace BSE {
    enum StateTransactionMethodType {
        NormalRotation = 0
    };

    enum MeasurementMethodType {
        NormalUwbMeasuremnt = 0,
        NormalZeroVeclotiMeasurement = 1,
        NormalAngleConstraint = 2
    };

    /**
     * system state: x y z vx vy vz wx wy wz
     * @tparam UWBNumber
     * @tparam time_interval
     */
    class IMUWBKFSimple :
            public KalmanFilterBase {
    public:
        IMUWBKFSimple(
                const Eigen::MatrixXd &initial_probability_vec) :
                KalmanFilterBase(
                        initial_probability_vec) {



            /**
             * define state transaction equation
             */
            StateTransactionEquationMap.insert(
                    {StateTransactionMethodType::NormalRotation,
                     ([&](Eigen::MatrixXd &state,
                          Eigen::MatrixXd &state_prob,
                          const Eigen::MatrixXd &input,
                          const Eigen::MatrixXd &cov_input) {
                         Eigen::Vector3d acc(input.block(0,
                         		0,
                         		3,
                         		1));
                         Eigen::Vector3d gyr(input.block(3, 0,
                         		3, 1) * time_interval_);
                         if (IS_DEBUG) {
                             std::cout << "before rotate update imu:"
                                       << (rotate_q_ * acc).transpose() << std::endl;
                         }

//                         rotate_q_ = st

                         if (gyr.norm() > 1e-8) {
                             Eigen::Quaterniond tmp_q =
                                     Eigen::AngleAxisd(gyr(0), Eigen::Vector3d::UnitX())
                                     * Eigen::AngleAxisd(gyr(1), Eigen::Vector3d::UnitY())
                                     * Eigen::AngleAxisd(gyr(2), Eigen::Vector3d::UnitZ());
//                             rotate_q_ =  tmp_q * rotate_q_;
//                             tmp_q.normalize();
                             rotate_q_ = rotate_q_ * tmp_q;
//                             rotate_q_ = tmp_q * rotate_q_;
//                             rotate_q_ = tmp_q * rotate_q_;
//                             rotate_q_ = rotate_q_ * tmp_q.inverse();
//                             rotate_q_ = tmp_q * rotate_q_;
                             rotate_q_.normalize();

                         }


                         Eigen::Vector3d gravity_g(0, 0, local_g_);
                         Eigen::Vector3d linear_acc = rotate_q_.toRotationMatrix() * acc + gravity_g;
                         if (IS_DEBUG) {
                             std::cout << "acc in navigation frame:" << (rotate_q_ * acc).transpose();
                             std::cout << "\nlinear_acc:" << linear_acc.transpose() << std::endl;
                         }

                         auto converted_input = input;
                         converted_input.block(0, 0, 3, 1) = linear_acc;
                         converted_input.block(3, 0, 3, 1) = gyr;


                         Eigen::MatrixXd A_ = Eigen::MatrixXd::Zero(9, 9);
                         // x y z
                         A_.block(0, 0, 3, 3) = Eigen::Matrix3d::Identity();
                         A_.block(0, 3, 3, 3) = Eigen::Matrix3d::Identity() * time_interval_;
                         // vx vy vz
                         A_.block(3, 3, 3, 3) = Eigen::Matrix3d::Identity();
                         // wx wy wz
                         A_.block(6, 6, 3, 3) = Eigen::Matrix3d::Identity();

                         Eigen::MatrixXd B_ = Eigen::MatrixXd::Zero(9, 6);
                         //x =
//                         B_.block(0, 0, 3, 3) =
//                                 Eigen::Matrix3d::Identity() * 0.5 * time_interval_ *
//                                 time_interval_;
                         B_.block(3, 0, 3, 3) = Eigen::Matrix3d::Identity() * time_interval_;
                         B_.block(6, 3, 3, 3) = Eigen::Matrix3d::Identity() * time_interval_;


                         state = A_ * state + B_ * converted_input;
                         state.block(6, 0, 3, 1) = rotate_q_.toRotationMatrix()
                                 .eulerAngles(0, 1, 2);

                         if (IS_DEBUG) {
                             std::cout << "state trans P:"
                                       << state_prob << std::endl;
                             std::cout << " A * P * A^ T "
                                       << A_ * state_prob * A_.transpose() << std::endl;
                             std::cout << "B * cove * B.transpose() "
                                       << B_ * cov_input * B_.transpose() << std::endl;
                         }
                         // unconverted value
//                         B_.block(6, 0, 3, 3) = rotate_q_.toRotationMatrix() ;//* time_interval_;
                         state_prob = A_ * state_prob * A_.transpose() +
                                      B_ * cov_input * B_.transpose();
                         if (std::isnan(state_prob.sum())) {
                             std::cout << "state prob is naa: " << state_prob << std::endl;
                         }


                         return;
                     })});


            /**
             * Measurement function.
             */


            /**
             * zero-velocity constraint function (without angle constraint).
             */
            MeasurementEquationMap.insert(
                    {MeasurementMethodType::NormalZeroVeclotiMeasurement,
                     ([&](Eigen::MatrixXd &state,
                          Eigen::MatrixXd &state_prob,
                          const Eigen::MatrixXd &m,
                          const Eigen::MatrixXd &cov_m,
                          Eigen::MatrixXd &dx

                     ) {

                         H_ = Eigen::MatrixXd::Zero(3, 9);
                         H_.block(0, 3, 3, 3) = Eigen::Matrix3d::Identity() * 1.0;
                         if (IS_DEBUG) {
                             std::cout << H_ << std::endl;
                             std::cout << " p * H^T :" << state_prob * H_.transpose().eval() << std::endl;
                             std::cout << " H * P * H^T + cosv:" << H_ * state_prob * H_.transpose().eval() + cov_m
                                       << std::endl;
                             std::cout << "inverse " << (H_ * state_prob * H_.transpose().eval() + cov_m).inverse()
                                       << std::endl;

                         }

                         K_ = (state_prob * H_.transpose().eval()) *
                              (H_ * state_prob * H_.transpose().eval() + cov_m).inverse();
                         if (std::isnan(K_.sum()) || std::isinf(K_.sum())) {
                             std::cout << "K is nana" << std::endl;
                         }

                         /*
                          * update probability
                          */
                         state_prob = (Eigen::Matrix<double, 9, 9>::Identity() - K_ * H_) * state_prob;
                         state_prob = (state_prob + state_prob.transpose().eval()) * 0.5;
                         if (state_prob.norm() > 10000) {
                             std::cout << __FILE__
                                       << ":"
                                       << __LINE__
                                       << " Error state prob is too big"
                                       << std::endl;
                             state_prob /= 100.0;
                         }
                         if (std::isnan(state_prob.sum()) || std::isinf(state_prob.sum())) {
                             std::cout << "state prob has nan" << std::endl;
                         }

                         /*
                          * update state
                          */
                         Eigen::MatrixXd tdx = K_ * (m - state.block(3, 0, 3, 1));

                         state += tdx;


                         Eigen::Quaterniond delta_q = Eigen::AngleAxisd(tdx(6), Eigen::Vector3d::UnitX())
                                                      * Eigen::AngleAxisd(tdx(7), Eigen::Vector3d::UnitY())
                                                      * Eigen::AngleAxisd(tdx(8), Eigen::Vector3d::UnitZ());
                         if (std::isnan(state.sum())) {
                             std::cout << "some error " << std::endl;
                         }

                         Eigen::Matrix3d rotation_m(rotate_q_.toRotationMatrix());
                         Eigen::Matrix3d omega = Eigen::Matrix3d::Zero();
                         omega << 0.0, tdx(8), -tdx(7),
                                 -tdx(8), 0.0, tdx(6),
                                 tdx(7), -tdx(6), 0.0;
                         omega *= -1.0;
//                         rotation_m = (2.0 * Eigen::Matrix3d::Identity() + omega) *
//                                      (2.0 * Eigen::Matrix3d::Identity() - omega).inverse()
//                                      * rotation_m;
                         rotation_m = (Eigen::Matrix3d::Identity() - omega) * rotation_m;

//                         rotate_q_ = delta_q.inverse() * rotate_q_;
                         rotate_q_ = Eigen::Quaterniond(rotation_m);
                         return;
                     })});


            /**
             *
             */
            MeasurementEquationMap.insert(
                    {MeasurementMethodType::NormalUwbMeasuremnt,
                     ([&](Eigen::MatrixXd &state,
                          Eigen::MatrixXd &state_prob,
                          const Eigen::MatrixXd &m,
                          const Eigen::MatrixXd &cov_m,
                          Eigen::MatrixXd &dx
                     ) {
                         Eigen::Vector3d b = m.block(0, 0, 3, 1);
                         Eigen::Matrix<double, 1, 1> z;
                         z(0) = m(3);
                         Eigen::Matrix<double, 1, 1> y;
                         y(0) = (state.block(0, 0, 3, 1) - b).norm();


                         H_.resize(1, 9);
                         H_.setZero();
                         H_.block(0, 0, 1, 3) = (state.block(0, 0, 3, 1) - b).transpose() / y(0);

                         K_ = (state_prob * H_.transpose().eval()) *
                              (H_ * state_prob * H_.transpose().eval() + cov_m).inverse();

                         dx = K_ * (z - y);

                         state.block(0, 0, 6, 1) += dx.block(0, 0, 6, 1);
                         Sophus::SO3d r = Sophus::SO3d::exp(state.block(6, 0, 3, 1));
                         r = r * Sophus::SO3d::exp(dx.block(6, 0, 3, 1));
                         state.block(6, 0, 3, 1) = r.log();
                         rotate_q_ = r.unit_quaternion();


//                std::cout << "dx:" << dx.transpose() << std::endl;

                         state_prob = (Eigen::Matrix<double, 9, 9>::Identity() - K_ * H_) * state_prob;


                         return;
                     })});


            /**
             *  measurement here is not the measurement value in kalman filter frame!!!
             *  m here should be acc of this moment.
             *
             */
            MeasurementEquationMap.insert(
                    {MeasurementMethodType::NormalAngleConstraint,
                     ([&](Eigen::MatrixXd &state,
                          Eigen::MatrixXd &state_prob,
                          const Eigen::MatrixXd &m,
                          const Eigen::MatrixXd &cov_m,
                          Eigen::MatrixXd &dx
                     ) {
                         Eigen::Vector3d tmp_acc = m;
//                         local_g_ = tmp_acc.norm();
//                         std::cout << "local g :" << local_g_ << std::endl;
//                         std::cout <<"linear acc:"
//                                   << (rotate_q_.toRotationMatrix() * m).transpose() << std::endl;
//                         state.block(6,0,3,1)  = rotate_q_.toRotationMatrix().eulerAngles(0,1,2);
                         auto the_y = [tmp_acc](Eigen::Matrix<double, 9, 1> u) -> Eigen::Vector3d {
                             Eigen::Matrix<double, 3, 1> w = u.block(6, 0, 3, 1);
                             Eigen::Quaterniond tmp_q = Eigen::AngleAxisd(w(0), Eigen::Vector3d::UnitX())
                                                        * Eigen::AngleAxisd(w(1), Eigen::Vector3d::UnitY())
                                                        * Eigen::AngleAxisd(w(2), Eigen::Vector3d::UnitZ());

//                             return tmp_q.inverse() * Eigen::Vector3d(0, 0, tmp_acc.norm());
                             return tmp_q.inverse() * Eigen::Vector3d(0, 0, 9.837);
                         };


                         state.block(6, 0, 3, 1) = rotate_q_.toRotationMatrix().eulerAngles(0, 1, 2);
//                dx =
                         H_ = Eigen::Matrix<double, 3, 9>::Zero();
                         double epsilon = 1e-10;
                         auto src_value = the_y(state);
                         Eigen::Matrix<double, 9, 1> offset;
                         offset.setZero();
                         for (int i(0); i < 9; ++i) {
                             offset(i) += epsilon;
                             H_.block(0, i, 3, 1) = (the_y(state + offset) -
                                                     src_value) / epsilon;
                             offset(i) -= epsilon;
                         }


                         K_ = (state_prob * H_.transpose().eval()) *
                              (H_ * state_prob * H_.transpose() + cov_m).inverse();

                         dx = K_ * (tmp_acc - the_y(state_));

                         std::cout << "===========================================================" << std::endl;
                         std::cout << "oldacc:" << (rotate_q_ * tmp_acc).transpose() << std::endl;

                         Eigen::Quaterniond tmp_q =
                                 Eigen::AngleAxisd(dx(6), Eigen::Vector3d::UnitX())
                                 * Eigen::AngleAxisd(dx(7), Eigen::Vector3d::UnitY())
                                 * Eigen::AngleAxisd(dx(8), Eigen::Vector3d::UnitZ());
                         rotate_q_ = tmp_q * rotate_q_;

                         rotate_q_.normalize();
                         state.block(6, 0, 3, 1) =
                                 rotate_q_.toRotationMatrix().eulerAngles(0, 1, 2);

                         std::cout << "   acc:" << tmp_acc.transpose()
                                   << "acc norm:" << tmp_acc.norm() << std::endl;
                         std::cout << "newacc:" << (rotate_q_ * tmp_acc).transpose() << std::endl;
                         std::cout << "===========================================================" << std::endl;

                         state.block(0, 0, 6, 1) += dx.block(0, 0, 6, 1);

                         //////////////////////////
//                         Eigen::Matrix3d rotation_m(rotate_q_.toRotationMatrix());
//                         Eigen::Matrix3d omega = Eigen::Matrix3d::Zero();
//                         omega << 0.0, dx(2 + 6), -dx(1 + 6),
//                                 -dx(2 + 6), 0.0, dx(0 + 6),
//                                 dx(1 + 6), -dx(0 + 6), 0.0;
//                         omega *= -1.0;
//                         rotation_m = (2.0 * Eigen::Matrix3d::Identity() + omega) *
//                                      (2.0 * Eigen::Matrix3d::Identity() - omega).inverse()
//                                      * rotation_m;
//                         rotation_m = (Eigen::Matrix3d::Identity() - omega) * rotation_m;

//                         rotate_q_ = delta_q.inverse() * rotate_q_;
//                         if (rotation_m.eulerAngles(0, 1, 2).norm() > 0.5) {
//                             rotate_q_.normalize();
//                             std::cout << "nomr:" << rotation_m.eulerAngles(0, 1, 2).transpose()
//                                       << " diff:" << (tmp_acc - the_y(state_)).transpose()
//                                       << "acc:" << tmp_acc
//                                       << "acc norm:" << tmp_acc.norm()<< std::endl;
//
//                         } else {
//
//                             rotate_q_ = Eigen::Quaterniond(rotation_m);
//                             rotate_q_.normalize();
//                         }
//                         state.block(6, 0, 3, 1) = rotate_q_.toRotationMatrix().eulerAngles(0, 1, 2);

                         return;
                     })});
        }

        /**
         * initial own pose.
         * @param imu_data
         * @param initial_ori
         * @param initial_pos
         * @return
         */
        bool initial_state(Eigen::MatrixXd imu_data,
                           double initial_ori = 0.0,
                           Eigen::Vector3d initial_pos = Eigen::Vector3d(0, 0, 0)
        ) {
            long double f_u(0.0), f_v(0.0), f_w(0.0);
            Eigen::Vector3d acc = imu_data.block(0, 0, imu_data.rows(), 3).colwise().mean();
            auto g = acc.norm();
            local_g_ = -1.0 * g;


            auto g_error = [&, &g, acc](double roll, double pitch, double yaw) -> double {

                auto rotate_matrix = (Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX())
                                      * Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY())
                                      * Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()));
                return std::abs(std::abs(g) * 1.0 * local_g_ / std::abs(local_g_) + (rotate_matrix * acc)(2));
            };
            auto ge(0.0);
//

            /**
             * find initial euler angle through optimization.
             */
            double tr = 0.0;
            double tp = 0.0;

            double step_len = 0.000005;
            double update_rate = 0.5;
            int iter_counter = 0;
            double current_error(g_error(tr, tp, initial_ori));
            while (current_error > 1e-7 && iter_counter < 30000) {

                iter_counter++;
//                tr +=(g_error)
                // compute gradient
                double delta_tr = (g_error(tr + step_len, tp, initial_ori) - current_error) / step_len;
                double delta_tp = (g_error(tr, tp + step_len, initial_ori) - current_error) / step_len;
                if (std::isnan(delta_tp) || std::isnan(delta_tr)) {
                    delta_tp = 0.0001;
                    delta_tr = 0.0001;

                    continue;
                }
                // update state.
                tr -= delta_tr * update_rate;
                tp -= delta_tp * update_rate;

                while (tr > M_PI + 0.01) {
                    tr -= 2.0 * M_PI;
                }
                while (tr < -M_PI - 0.01) {

                    tr += 2.0 * M_PI;
                }

                while (tp > M_PI + 0.01) {
                    tp -= 2.0 * M_PI;
                }

                while (tp < -M_PI - 0.01) {
                    tp += 2.0 * M_PI;
                }
                /*
                 * Reduce the learning rate.
                 */
                if (update_rate > 0.00001) {
                    update_rate *= 0.99;
                }


                current_error = g_error(tr, tp, initial_ori);
                if (IS_DEBUG) {
                    std::cout << iter_counter
                              << ":"
                              << current_error
                              << "{"
                              << tr
                              << ":"
                              << tp
                              << "}"
                              << std::endl;
                }

            }


            state_.block(0, 0, 3, 1) = initial_pos;
            state_.block(3, 0, 3, 1).setZero();
            state_.block(6, 0, 3, 1) = Eigen::Vector3d(tr, tp, initial_ori);

            rotate_q_ = (Eigen::AngleAxisd(tr, Eigen::Vector3d::UnitX())
                         * Eigen::AngleAxisd(tp, Eigen::Vector3d::UnitY())
                         * Eigen::AngleAxisd(initial_ori, Eigen::Vector3d::UnitZ()));

            std::cout << "value angle:" << state_.block(6, 0, 3, 1).transpose() << std::endl;
            std::cout << "eular angle:" << rotate_q_.toRotationMatrix().eulerAngles(0, 1, 2).transpose()
                      << std::endl;
            std::cout << "before acc:" << acc.transpose() << std::endl;
            std::cout << "after acc:" << (rotate_q_ * acc).transpose() << std::endl;


        }


        double getTime_interval_() const {
            return time_interval_;
        }

        void setTime_interval_(double time_interval_) {
            IMUWBKFSimple::time_interval_ = time_interval_;
        }

        const Eigen::Quaterniond &getRotate_q() const {
            return rotate_q_;
        }

        void setRotate_q(const Eigen::Quaterniond &rotate_q) {
            IMUWBKFSimple::rotate_q_ = rotate_q;
        }

        double getLocal_g_() const {
            return local_g_;
        }

        void setLocal_g_(double local_g_) {
            IMUWBKFSimple::local_g_ = local_g_;
        }


        /**
         * Get current transform matrix.
         * @return
         */
        Eigen::Isometry3d getTransformMatrix() {
            Eigen::Isometry3d trans_matrix = Eigen::Isometry3d::Identity();
            trans_matrix.matrix().block(0, 0, 3, 3) = rotate_q_.toRotationMatrix().eval() * 1.0;
            trans_matrix.matrix().block(0, 3, 3, 1) = state_.block(0, 0, 3, 1);

            return trans_matrix;

        }

    protected:
        double time_interval_ = 0.005;

        Eigen::Quaterniond rotate_q_ = Eigen::Quaterniond().setIdentity();

        double local_g_ = -9.81;


    };

}


#endif //COMPLEXITYPOSITIONING_IMUWBKF_H
