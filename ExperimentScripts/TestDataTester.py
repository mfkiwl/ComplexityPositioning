# Created by steve on 18-2-2 下午3:16
'''
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
'''
import numpy as np
import scipy as sp
from matplotlib import  pyplot as plt


class DataLoder():

    '''
    Load data, and provide some test function.
    '''
    def __init__(self, data_dir):
        if not data_dir[-1] is '/':
            data_dir = data_dir + '/'
        self.left_imu_data = np.loadtxt(data_dir+'LEFT_FOOT.data',delimiter=',')
        self.right_imu_data = np.loadtxt(data_dir+'RIGHT_FOOT.data',delimiter=',')
        self.head_imu_data = np.loadtxt(data_dir + 'HEAD.data', delimiter=',')
        self.uwb_data = np.loadtxt(data_dir+'uwb_result.csv', delimiter=',')
        self.beacon_set = np.loadtxt(data_dir + 'beaconSet.csv', delimiter=',')






if __name__ == '__main__':
    imu_data = np.loadtxt('/home/steve/Data/FusingLocationData/0014/LEFT_FOOT.data',
                          delimiter=',')

