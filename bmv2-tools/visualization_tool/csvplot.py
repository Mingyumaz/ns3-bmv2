import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

#Set font format
plt.rcParams['font.family'] = 'sans-serif'

# init parameters
tc0_filePath = "/home/p4/ns3dev/trace-data/n0r_tc-qsize.txt"
tc1_filePath = "/home/p4/ns3dev/trace-data/n1r_tc-qsize.txt"
dev_filePath = "/home/p4/ns3dev/trace-data/dev-qsize.txt"
time_tolerance = '0.05s'

#import data from csv file and preprocessing
df_tc0 = pd.read_csv(tc0_filePath, sep=",", header=None)
df_tc0.columns = ['time','tc0_queue_length']
df_tc0['time'] = pd.to_datetime(df_tc0['time'], unit="s", origin=pd.Timestamp('2023-01-01'))

df_tc1 = pd.read_csv(tc1_filePath, sep=",", header=None)
df_tc1.columns = ['time','tc1_queue_length']
df_tc1['time'] = pd.to_datetime(df_tc1['time'], unit="s", origin=pd.Timestamp('2023-01-01'))

df_dev = pd.read_csv(dev_filePath, sep=",", header=None)
df_dev.columns = ['time','dev_queue_length']
df_dev['time'] = pd.to_datetime(df_dev['time'], unit="s", origin=pd.Timestamp('2023-01-01'))

# merge the data into one data frame
df_temp = pd.merge_asof(df_tc0, df_tc1, on = 'time', tolerance=pd.Timedelta(time_tolerance))
df = pd.merge_asof(df_temp, df_dev, on = 'time', tolerance=pd.Timedelta(time_tolerance))
df.fillna(0)
print('The processed dataframe: ' + '-'*20 +'\n', df)

# plot the figure
df.plot(x = 'time', y = ['tc0_queue_length', 'tc1_queue_length', 'dev_queue_length'], title="tc and dev queue length in Bytes")
plt.show() 