# Machine learning for OBDADS project

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
import os
import sys
import time
import datetime

import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense, RepeatVector, TimeDistributed
from sklearn.preprocessing import MinMaxScaler

# Import the csv file
df = pd.read_csv('live1.csv')

# Clean up data
# Remove '()' from the column names
df.columns = df.columns.str.replace('(', '')
df.columns = df.columns.str.replace(')', '')
df.columns = df.columns.str.strip()

# Replace all the empty strings with NaN
df.replace('', np.nan, inplace=True)

# Replace all 'Nan' with np.nan
df.replace('NaN', np.nan, inplace=True)

# Select relevant columns
features = ['ENGINE_RPM', 'VEHICLE_SPEED', 'THROTTLE', 'ENGINE_LOAD']
data = df[features].values

# Normalize data
scaler = MinMaxScaler()
data_scaled = scaler.fit_transform(data)

# Create sequences
SEQ_LENGTH = 10  # 10-second window
def create_sequences(data, seq_length):
    sequences = []
    for i in range(len(data) - seq_length):
        sequences.append(data[i:i + seq_length])
    return np.array(sequences)

X_train = create_sequences(data_scaled, SEQ_LENGTH)

# Define LSTM Autoencoder
model = Sequential([
    LSTM(64, activation='relu', input_shape=(SEQ_LENGTH, len(features)), return_sequences=True),
    LSTM(32, activation='relu', return_sequences=False),
    RepeatVector(SEQ_LENGTH),
    LSTM(32, activation='relu', return_sequences=True),
    LSTM(64, activation='relu', return_sequences=True),
    TimeDistributed(Dense(len(features)))
])

model.compile(optimizer='adam', loss='mse')
model.summary()

# Train the model
model.fit(X_train, X_train, epochs=50, batch_size=32, validation_split=0.1)

# Get reconstruction error for anomaly detection
X_pred = model.predict(X_train)
reconstruction_errors = np.mean(np.abs(X_pred - X_train), axis=(1, 2))

# Set threshold for anomaly detection
threshold = np.percentile(reconstruction_errors, 95)  # Top 5% as anomalies
anomalies = reconstruction_errors > threshold

# Add anomaly labels to original data
df['Anomaly'] = np.concatenate(([False] * SEQ_LENGTH, anomalies))

# Save results
df.to_csv("anomalies_detected.csv", index=False)

# Plot anomalies
plt.plot(df['ENGINE_RPM'], color='blue')
plt.plot(df['VEHICLE_SPEED'], color='red')
plt.plot(df['THROTTLE'], color='green')
plt.plot(df['ENGINE_LOAD'], color='orange')
plt.scatter(df.index[df['Anomaly']], df['ENGINE_RPM'][df['Anomaly']], color='black')
plt.scatter(df.index[df['Anomaly']], df['VEHICLE_SPEED'][df['Anomaly']], color='black')
plt.scatter(df.index[df['Anomaly']], df['THROTTLE'][df['Anomaly']], color='black')
plt.scatter(df.index[df['Anomaly']], df['ENGINE_LOAD'][df['Anomaly']], color='black')

# Add legend
plt.legend(['ENGINE_RPM', 'VEHICLE_SPEED', 'THROTTLE', 'ENGINE_LOAD', 'Anomaly'])

# Add title
plt.title('Anomalies Detected')

# Add labels
plt.xlabel('Time')
plt.ylabel('Value')

# Rotate x-axis labels
plt.xticks(rotation=45)

# Show plot
plt.show() 

# Save plot
plt.savefig('anomalies.png')

# Save model
model.save('anomaly_detection_model.keras')

