import matplotlib.pyplot as plt
import pandas as pd
import numpy as np

#Set font format
plt.rcParams['font.family'] = 'sans-serif'

# init parameters
filePath = "/home/p4/ns3dev/trace-data/tc-qsize.txt"

#import data from csv file
df = pd.read_csv(filePath, sep=",", header=None)
df.columns = ['time','queue_length']

# gen figure
df.plot(x = 'time', y = 'queue_length', title=filePath)
plt.show()