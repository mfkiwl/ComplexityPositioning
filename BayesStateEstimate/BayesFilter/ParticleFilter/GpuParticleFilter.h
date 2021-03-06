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
// Created by steve on 18-3-7.
//


#include <boost/compute/function.hpp>
#include <boost/compute/system.hpp>
#include <boost/compute/algorithm/count_if.hpp>
#include <boost/compute/container/vector.hpp>
#include <boost/compute/iterator/buffer_iterator.hpp>
#include <boost/compute/random/default_random_engine.hpp>
#include <boost/compute/types/fundamental.hpp>

namespace compute=boost::compute;

namespace BSE {


    template<typename T, int StateSize>
    class GpuParticleFilter {
    public:
        GpuParticleFilter(int particle_num_,
                          Eigen::Matrix<T,1,StateSize> ini_state_){
            compute::device gpu = compute::system::default_device();

            compute::context context(gpu);
            compute::command_queue queue(
                    context, gpu,compute::command_queue::enable_profiling
            );



        }

    private:
        int particle_num_ = (1000);

        compute::vector<Eigen::Matrix<T,1,StateSize>> particle_states_;
        compute::vector<T> probability_;

    };
}


