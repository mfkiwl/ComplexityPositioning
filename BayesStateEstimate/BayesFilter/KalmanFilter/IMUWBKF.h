//
// Created by steve on 18-1-27.
//

#ifndef COMPLEXITYPOSITIONING_IMUWBKF_H
#define COMPLEXITYPOSITIONING_IMUWBKF_H

#include <sophus/so3.h>
#include <sophus/se3.h>

#include "KalmanFilterBase.h"
#include "KalmanFilterBase.cpp"

#include <Eigen/Dense>
#include <Eigen/Geometry>

namespace BSE {
    enum StateTransactionMethodType {
        NormalRotation = 0
    };

    enum MeasurementMethodType {
        NormalUwbMeasuremnt = 0,
        NormalZeroVeclotiMeasurement = 1
    };

    /**
     * system state: x y z vx vy vz wx wy wz
     * @tparam UWBNumber
     * @tparam time_interval
     */
    class IMUWBKFBase :
            public KalmanFilterBase {
    public:
        IMUWBKFBase(
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
                         Eigen::Vector3d acc(input.block(0, 0, 3, 1));
                         Eigen::Vector3d gyr(input.block(3, 0, 3, 1) * time_interval_);
                         if (IS_DEBUG) {
                             std::cout << "before imu:" << (rotate_q_ * acc).transpose() << std::endl;
                         }


                         if (gyr.norm() > 1e-18) {
                             Eigen::Quaterniond tmp_q = Eigen::AngleAxisd(gyr(0), Eigen::Vector3d::UnitX())
                                                        *
                                                        Eigen::AngleAxisd(gyr(1), Eigen::Vector3d::UnitY())
                                                        * Eigen::AngleAxisd(gyr(2),
                                                                            Eigen::Vector3d::UnitZ());
//                             rotate_q_ =  tmp_q * rotate_q_;
                             tmp_q.normalize();
                             rotate_q_ = rotate_q_ * tmp_q;
                             rotate_q_.normalize();

                         }


                         auto euler_func = [&](Eigen::Vector3d angle) {
//                             return rotate_q_
                             auto tmp_q = Eigen::AngleAxisd(angle(0), Eigen::Vector3d::UnitX())
                                          *
                                          Eigen::AngleAxisd(angle(1), Eigen::Vector3d::UnitY())
                                          * Eigen::AngleAxisd(angle(2),
                                                              Eigen::Vector3d::UnitZ());
//                             tmp_q = tmp_q * rotate_q_;
                             tmp_q = rotate_q_ * tmp_q;
                             tmp_q.normalize();
                             return tmp_q.toRotationMatrix().eulerAngles(0, 1, 2);
                         };
                         Eigen::Vector3d gravity_g(0, 0, local_g_);
                         Eigen::Vector3d linear_acc = rotate_q_ * acc + gravity_g;
                         if (IS_DEBUG) {
                             std::cout << "acc in navigation frame:" << (rotate_q_ * acc).transpose();
                             std::cout << "linear_acc:" << linear_acc.transpose() << std::endl;
                         }

                         auto converted_input = input;
                         converted_input.block(0, 0, 3, 1) = linear_acc;
                         converted_input.block(3, 0, 3, 1) = gyr;

//                state.block
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
                         B_.block(0, 0, 3, 3) =
                                 Eigen::Matrix3d::Identity() * 0.5 * time_interval_ *
                                 time_interval_;
                         B_.block(3, 0, 3, 3) = Eigen::Matrix3d::Identity() * time_interval_;
//                         B_.block(3,0,3,3) =
//                         B_.block()

                         double step_len_rate = 0.1;
                         for (int i(0); i < 3; ++i) {
                             auto t_angle = gyr;
                             t_angle(i) = gyr(i) * (1 + step_len_rate);
//                             B_.block(6,3+i,3,1) = (euler_func(t_angle)-euler_func(gyr))/(t_angle(i)-gyr(i));
                         }

                         state = A_ * state + B_ * converted_input;
                         state.block(6, 0, 3, 1) = rotate_q_.toRotationMatrix().eulerAngles(0, 1, 2);

                         if (IS_DEBUG) {
                             std::cout << "state trans P:" << state_prob << std::endl;
                             std::cout << " A * P * A^ T " << A_ * state_prob * A_.transpose() << std::endl;
                             std::cout << "B * cove * B.transpose() " << B_ * cov_input * B_.transpose() << std::endl;
                         }
                         // unconverted value
//                         B_
//                         cov_input.block(3,0,3,1) = (rotate_q_.toRotationMatrix()) * cov_input.block(3,0,3,1);
                         B_.block(3,0,3,3) = rotate_q_.toRotationMatrix() * time_interval_;
                         state_prob = A_ * state_prob * A_.transpose() + B_ * cov_input * B_.transpose();
                         if (std::isnan(state_prob.sum())) {
                             std::cout << "state prob is naa: " << state_prob << std::endl;
                         }


                         return;
                     })});


            MeasurementEquationMap.insert({MeasurementMethodType::NormalZeroVeclotiMeasurement, ([&](
                    Eigen::MatrixXd &state,
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
                    std::cout << " H * P * H^T + cosv:" << H_ * state_prob * H_.transpose().eval() + cov_m << std::endl;
                    std::cout << "inverse " << (H_ * state_prob * H_.transpose().eval() + cov_m).inverse() << std::endl;

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
//                if(state_prob.norm()>10000){
//                    state_prob/=100.0;
//                }
                if (std::isnan(state_prob.sum()) || std::isinf(state_prob.sum())) {
                    std::cout << "state prob has nan" << std::endl;
                }

                /*
                 * update state
                 */
                Eigen::MatrixXd tdx = K_ * (m - state.block(3, 0, 3, 1));

                state += tdx;

                Eigen::Quaterniond delta_q = Eigen::AngleAxisd(tdx(3), Eigen::Vector3d::UnitX())
                                             * Eigen::AngleAxisd(tdx(4), Eigen::Vector3d::UnitY())
                                             * Eigen::AngleAxisd(tdx(5), Eigen::Vector3d::UnitZ());
                if (std::isnan(state.sum())) {
                    std::cout << "some error " << std::endl;
                }

//                rotate_q_ =  rotate_q_ * delta_q;
//                rotate_q_ = delta_q * rotate_q_;
//                state.block(3,0,3,1) = rotate_q_.toRotationMatrix().eulerAngles(0,1,2);

                rotate_q_.normalize();

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
//            f_u = imu_data.col(0).mean();
//            f_v = imu_data.col(1).mean();
//            f_w = imu_data.col(2).mean();
            Eigen::Vector3d acc = imu_data.block(0, 0, imu_data.rows(), 3).colwise().mean();
            auto g = acc.norm();
            local_g_ = g;

            double roll = std::atan(f_v / f_w);
            double pitch = -std::asin(f_u /
                                      std::sqrt(f_u * f_u + f_v * f_v + f_w * f_w));

            double min_roll(0.0), min_pitch(0.0);
            double error = 10000.0;

            auto g_error = [g, acc](double roll, double pitch, double yaw) -> double {

                auto rotate_matrix = (Eigen::AngleAxisd(roll, Eigen::Vector3d::UnitX())
                                      * Eigen::AngleAxisd(pitch, Eigen::Vector3d::UnitY())
                                      * Eigen::AngleAxisd(yaw, Eigen::Vector3d::UnitZ()));
//                std::cout << typeid(rotate_matrix) << std::endl;
                return std::abs(g + (rotate_matrix * acc)(2));
            };
            auto ge(0.0);
//

            /**
             * find initial euler angle through optimization.
             */
            double tr(0.0);
            double tp(0.0);
            double step_len = 0.05;
            double update_rate = 0.5;
            int iter_counter = 0;
            double current_error(g_error(tr, tp, initial_ori));
            while (current_error > 1e-5 && iter_counter < 3000) {

                iter_counter++;
//                tr +=(g_error)
                double delta_tr = (g_error(tr + step_len, tp, initial_ori) - current_error) / step_len;
                double delta_tp = (g_error(tr, tp + step_len, initial_ori) - current_error) / step_len;
                tr -= delta_tr * update_rate;
                tp -= delta_tp * update_rate;
                if (update_rate > 0.0000001) {
                    update_rate *= 0.9;
                }

                current_error = g_error(tr, tp, initial_ori);
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
            min_roll = tr;
            min_pitch = tp;


//            std::cout << state_
            state_.block(0, 0, 3, 1) = initial_pos;
            state_.block(3, 0, 3, 1).setZero();
            state_.block(6, 0, 3, 1) = Eigen::Vector3d(min_roll, min_pitch, initial_ori);

            rotate_q_ = (Eigen::AngleAxisd(min_roll, Eigen::Vector3d::UnitX())
                         * Eigen::AngleAxisd(min_pitch, Eigen::Vector3d::UnitY())
                         * Eigen::AngleAxisd(initial_ori, Eigen::Vector3d::UnitZ()));
            std::cout << "value angle:" << state_.block(6, 0, 3, 1).transpose() << std::endl;
            std::cout << "eular angle:" << rotate_q_.toRotationMatrix().eulerAngles(0, 1, 2).transpose()
                      << std::endl;
            std::cout << "before acc:" << acc.transpose() << std::endl;
            std::cout << "after acc:" << (rotate_q_ * acc).transpose() << std::endl;


        }

        /**
         * Auxiliary function convert euler angle to rotation matrix.
         * @param pitch
         * @param roll
         * @param yaw
         * @return
         */
        inline Eigen::Matrix3d Ang2RotMatrix(double pitch, double roll, double yaw) {
            double cp = std::cos(pitch);
            double sp = std::sin(pitch);

            double cr = std::cos(roll);
            double sr = std::sin(roll);

            double cy = std::cos(yaw);
            double sy = std::sin(yaw);

            Eigen::Matrix3d C;
            C << cp * cy, (sr * sp * cy) - (cr * sy), (cr * sp * cy + sr * sy),
                    cp * sy, (sr * sp * sy) + (cr * cy), (cr * sp * sy) - (sr * cy),
                    -sp, sr * cp, cr * cp;
            return C;
        }

        double getTime_interval_() const {
            return time_interval_;
        }

        void setTime_interval_(double time_interval_) {
            IMUWBKFBase::time_interval_ = time_interval_;
        }

        const Eigen::Quaterniond &getRotate_q() const {
            return rotate_q_;
        }

        void setRotate_q(const Eigen::Quaterniond &rotate_q) {
            IMUWBKFBase::rotate_q_ = rotate_q;
        }

    protected:
        double time_interval_ = 0.005;

        Eigen::Quaterniond rotate_q_ = Eigen::Quaterniond().setIdentity();

        double local_g_ = 9.81;


    };

}


#endif //COMPLEXITYPOSITIONING_IMUWBKF_H
