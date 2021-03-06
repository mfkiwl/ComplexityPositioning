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
// Created by steve on 11/13/18.
//

#ifndef COMPLEXITYPOSITIONING_DUALFEETCONSTRAINT_HPP
#define COMPLEXITYPOSITIONING_DUALFEETCONSTRAINT_HPP


#include <gtsam/base/Lie.h>
#include <gtsam/nonlinear/NonlinearFactor.h>
#include <gtsam/geometry/Pose3.h>

namespace gtsam {
	struct DualFeetConstraint : public NoiseModelFactor2<Pose3, Pose3> {

		typedef boost::shared_ptr<DualFeetConstraint> shared_ptr;


		double threshold_;

		DualFeetConstraint(Key key1, Key key2, double threshold = 1.0, double sigma = 10000) :
				NoiseModelFactor2<Pose3, Pose3>(noiseModel::Isotropic::Sigma(1,0.1), key1, key2),
				threshold_(threshold) {

		}

		~DualFeetConstraint() {
			threshold_ = 1000.0;
		}


		/**
		 * @brief Function called by optimizer
		 * @param x
		 * @param H
		 * @return
		 */
		Vector unwhitenedError(const Values &x, boost::optional<std::vector<Matrix> &> H = boost::none) const {
			if (this->active(x)) {
				const Pose3 &x1 = x.at<Pose3>(keys_[0]);
				const Pose3 &x2 = x.at<Pose3>(keys_[1]);
				if (H) {
					std::cout << "evalue unwhitened error" << std::endl;
					return evaluateError(x1, x2, (*H)[0], (*H)[1]);
				} else {
					return evaluateError(x1, x2);
				}
			} else {
				return Vector::Zero(this->dim());
			}
		}

		/**
		 * @brief return true when distance between two pose lager than threshold_.
		 * @param v
		 * @return
		 */
		bool active(const Values &v) const {
			const Pose3 &x1 = v.at<Pose3>(keys_[0]);
			const Pose3 &x2 = v.at<Pose3>(keys_[1]);

			if (x1.range(x2) < threshold_) {
				return false;
			} else {
				return true;
			}
		}

		Vector evaluateError(const Pose3 &x1, const Pose3 &x2,
		                     boost::optional<Matrix &> H1 = boost::none,
		                     boost::optional<Matrix &> H2 = boost::none) const {
			double range_value = x1.range(x2,H1,H2);
			return (Vector(1) << range_value).finished();


		}


	private:
		/** Serialization function */
		friend class boost::serialization::access;

		template<class ARCHIVE>
		void serialize(ARCHIVE &ar, const unsigned int /*version*/) {
			ar & boost::serialization::make_nvp("NoiseModelFactor2",
			                                    boost::serialization::base_object<Base>(*this));
			ar & BOOST_SERIALIZATION_NVP(threshold_);
		}


	};
}

#endif //COMPLEXITYPOSITIONING_DUALFEETCONSTRAINT_HPP
