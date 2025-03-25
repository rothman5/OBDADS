
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

import tensorflow as tf
from keras._tf_keras.keras.models import Sequential
from keras._tf_keras.keras.layers import LSTM, Dense, RepeatVector, TimeDistributed
from sklearn.preprocessing import MinMaxScaler

features = ['ENGINE_RPM', 'VEHICLE_SPEED', 'THROTTLE', 'ENGINE_LOAD']


# Graph the last minute of data
df = pd.read_csv('live1.csv')

# TEMPORARY: Clean up data
df.columns = df.columns.str.replace('(', '')
df.columns = df.columns.str.replace(')', '')
df.columns = df.columns.str.strip()

# Clean up data``
df.replace('', np.nan, inplace=True)
df.replace('NaN', np.nan, inplace=True)

# TEMPORARY: Select relevant columns
data = df[features].values

# Graph the data
plt.plot(data)
plt.legend(features)
plt.show()

# Save the graph
plt.savefig('data.png')
print('Data graph saved to data.png')