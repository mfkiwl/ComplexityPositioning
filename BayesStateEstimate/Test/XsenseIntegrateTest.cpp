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
// Created by steve on 18-3-8.
//



#include <iostream>
#include <fstream>
#include <thread>

#include <Eigen/Dense>
#include <Eigen/Geometry>
//#include <AWF.h>dd


#include "AWF.h"

#include "../BayesFilter/KalmanFilter/IMUWBKF.h"
#include "../BayesFilter/KalmanFilter/IMUWBKF.cpp"

#include "../AuxiliaryTool/UwbTools.h"
#include "../AuxiliaryTool/UwbTools.cpp"

#include "../AuxiliaryTool/ImuTools.h"
#include "../AuxiliaryTool/ImuTools.cpp"


namespace plt = matplotlibcpp;


int main(int argc, char *argv[]) {

    std::cout.precision(30);

    std::string file_name = "/home/steve/Data/XsensData/line-high.csv";
//    std::string file_name = "/home/steve/Data/XsensData/line-low.csv";
//    std::string file_name = "/home/steve/Data/XsensData/two round high.csv";

    AWF::FileReader imu_file(file_name);

    Eigen::MatrixXd imu_data = imu_file.extractDoulbeMatrix(",");

    Eigen::MatrixXd process_noise_matrix =
            Eigen::MatrixXd::Identity(6, 6);
    process_noise_matrix.block(0, 0, 3, 3) *= 0.1;
    process_noise_matrix.block(3, 3, 3, 3) *= (0.1 * M_PI / 180.0);


    Eigen::MatrixXd initial_prob_matrix = Eigen::MatrixXd::Identity(9, 9);
    initial_prob_matrix.block(0, 0, 3, 3) *= 0.001;
    initial_prob_matrix.block(3, 3, 3, 3) *= 0.001;
    initial_prob_matrix.block(6, 6, 3, 3) *= 0.001 * (M_PI / 180.0);


    auto filter = BSE::IMUWBKFSimple(initial_prob_matrix);
    filter.setTime_interval_(0.01);
    filter.initial_state(imu_data.block(0, 1, 50, 6), 0.0);


    auto imu_tool = BSE::ImuTools();

    std::vector<std::vector<double>> trace = {{},
                                              {},
                                              {}};




    for (int i(0); i < imu_data.rows(); ++i) {
        filter.StateTransaction(imu_data.block(i, 1, 1, 6).transpose(),
                                process_noise_matrix,
                                BSE::StateTransactionMethodType::NormalRotation);

        auto state_x = filter.getTransformMatrix();

        for (int j(0); j < 3; ++j) {
            trace[j].push_back(state_x(j, 3));
        }
    }



    plt::plot(trace[0], trace[1], "-+");
    plt::title(file_name);
    plt::grid(true);
    plt::show();


}
