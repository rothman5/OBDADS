"""
Machine learning for OBDADS project

Run every 10 seconds
Grab a minute of data

1. Startup
2. Train a new model on the last 10 minutes of data
3. Get the last minute of data (600 seconds)
4. Normalize the data

"""

import pandas as pd
import numpy as np
import matplotlib.pyplot as plt

import tensorflow as tf
from tensorflow.keras.models import Sequential
from tensorflow.keras.layers import LSTM, Dense, RepeatVector, TimeDistributed
from sklearn.preprocessing import MinMaxScaler

# Define the features
features = ['ENGINE_RPM', 'VEHICLE_SPEED', 'THROTTLE', 'ENGINE_LOAD']
SEQ_LENGTH = 40  # 10-second window
THRESHOLD = 0.1

# Create sequences


def create_sequences(data, seq_length):
    """
    Splits the input data into overlapping sequences of a specified length.

        data (array-like): The input data to be split into sequences.
        seq_length (int): The length of each sequence.

        numpy.ndarray: A NumPy array containing the generated sequences.
    """
    sequences = []
    for i in range(len(data) - seq_length):
        sequences.append(data[i:i + seq_length])
    return np.array(sequences)


# Define the model
model = Sequential([
    LSTM(128, activation='relu', input_shape=(SEQ_LENGTH, len(features)), return_sequences=True),
    LSTM(64, activation='relu', return_sequences=False),
    RepeatVector(SEQ_LENGTH),
    LSTM(64, activation='relu', return_sequences=True),
    LSTM(128, activation='relu', return_sequences=True),
    TimeDistributed(Dense(len(features)))
])

# Compile the model
model.compile(optimizer='adam', loss='mse')


# Read the last 10 minutes of data from influxdb

# TEMPORARY: Use a CSV file for nowr
df = pd.read_csv('live1.csv')

# TEMPORARY: Clean up data
df.columns = df.columns.str.replace('(', '')
df.columns = df.columns.str.replace(')', '')
df.columns = df.columns.str.strip()

# Clean up data
df.replace('', np.nan, inplace=True)
df.replace('NaN', np.nan, inplace=True)

# TEMPORARY: Select relevant columns
data = df[features].values

# Normalize the data
scaler = MinMaxScaler()
data_scaled = scaler.fit_transform(data)

# Create the sequences for training
X_train = create_sequences(data_scaled, SEQ_LENGTH)

# Train the model for 100 epochs or until the loss is less than 0.01


class EarlyStoppingByLoss(tf.keras.callbacks.Callback):
    def __init__(self, monitor='loss', value=0.01):
        super().__init__()
        self.monitor = monitor
        self.value = value

    def on_epoch_end(self, epoch, logs=None):
        current = logs.get(self.monitor)
        if current is not None and current < self.value:
            print(f"\nStopping training as {self.monitor} has reached below {self.value}")
            self.model.stop_training = True


early_stopping = EarlyStoppingByLoss(monitor='loss', value=0.01)

model.fit(X_train, X_train, epochs=100, batch_size=64, validation_split=0.1, callbacks=[early_stopping])

# Save the model
model.save('model.keras')

# Plot the loss
plt.plot(model.history.history['loss'], label='loss')
plt.plot(model.history.history['val_loss'], label='val_loss')
plt.legend()
plt.show()

# Get the last minute of data
data_last_minute = data_scaled[-600:]

# Create the sequence
data_last_minute = create_sequences(data_last_minute, SEQ_LENGTH)


# Detect anomalies
reconstructed_data = model.predict(data_last_minute)
reconstruction_error = np.mean(np.abs(reconstructed_data - data_last_minute), axis=-1)

# Get the reconstruction error for each feature
avg_reconstruction_error = np.mean(reconstruction_error, axis=0)


# Print the reconstruction error for each feature
for i, feature in enumerate(features):
    print(f'{feature}: {avg_reconstruction_error[i]}')

# print(f'Reconstruction error: {avg_reconstruction_error}')

# Print whether there is an anomaly for each stat
for i, feature in enumerate(features):
    if avg_reconstruction_error[i] > THRESHOLD:
        print(f'Anomaly detected in {feature} with error {avg_reconstruction_error[i]}')
    else:
        print(f'No anomaly detected in {feature} with error {avg_reconstruction_error[i]}')
