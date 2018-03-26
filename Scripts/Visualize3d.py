# Created by steve on 18-3-6 下午7:40
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

import matplotlib.pyplot as plt
from mpl_toolkits.mplot3d import Axes3D
import os

import numpy as np

import pyqtgraph as pg




def plot_file(ax_handle, file_name, flag='+-'):
    data = np.loadtxt(file_name, delimiter=',')
    ax_handle.plot(data[:, 0], data[:, 1], data[:, 2], flag, label=file_name)
    pg.plot(data[:,0],data[:,1])




if __name__ == '__main__':
    # dir_name = './' # graph Optimization result
    dir_name = '/home/steve/Code/ComplexityPositioning/XsenseResult/'  # xsense imu test result

    fig = plt.figure()
    ax = fig.add_subplot(1, 1, 1, projection='3d')
    pg.mkQApp()


    for file_name in os.listdir(dir_name):
        print(file_name)
        file_name = dir_name + file_name

        if '.csv' in file_name:
            # if 'graph' in file_name:
            #     plot_file(ax, file_name)
            # if 'uwb' in file_name:
            #     plot_file(ax, file_name, '*')
            plot_file(ax, file_name)

    ax.legend()
    ax.grid()

    # pg.plot()


    plt.show()
