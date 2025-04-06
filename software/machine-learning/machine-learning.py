"""
Machine learning for OBDADS project

Sample rate of 50 ms - 20 Hz
Run every 10 seconds ()
Grab a minute of data

1. Startup
2. Train a new model on the last 10 minutes of data
3. Get the last minute of data (20*60 = 1200 rows)
4. Normalize the data

"""

import pandas as pd
import numpy as np
import sqlite3
import matplotlib.pyplot as plt

import tensorflow as tf
from keras._tf_keras import keras
from keras._tf_keras.keras.models import Sequential
from keras._tf_keras.keras.layers import LSTM, Dense, RepeatVector, TimeDistributed
from sklearn.preprocessing import MinMaxScaler

# Determine the number of samples
SAMPLE_PERIOD = 50  # ms
samples_per_second = 1000 / SAMPLE_PERIOD
samples_in_minute = int(samples_per_second * 60)
samples_in_10_minutes = int(samples_per_second * 60 * 10)

# Define hyperparameters
SEQ_LENGTH = 40
THRESHOLD = 0.01

# Define the features
features = ['ENGINE_RPM', 'VEHICLE_SPEED', 'THROTTLE', 'ENGINE_LOAD']


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
    LSTM(64, activation='relu', input_shape=(SEQ_LENGTH, len(features)), return_sequences=True),
    LSTM(32, activation='relu', return_sequences=False),
    RepeatVector(SEQ_LENGTH),
    LSTM(32, activation='relu', return_sequences=True),
    LSTM(64, activation='relu', return_sequences=True),
    TimeDistributed(Dense(len(features)))
])

# Compile the model
model.compile(optimizer='adam', loss='mse', metrics=['accuracy'])


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

# Train the model
early_stopping = keras.callbacks.EarlyStopping(patience=2, restore_best_weights=True)
model.fit(X_train, X_train, epochs=100, batch_size=64, validation_split=0.1, callbacks=[early_stopping])

# Save the model
model.save('model.keras')

# Plot the loss
plt.plot(model.history.history['loss'], label='loss')
plt.plot(model.history.history['val_loss'], label='val_loss')
plt.legend()
plt.show()
plt.savefig('loss.png')

# Get the last minute of data
print(samples_in_minute)
data_last_minute = data_scaled[-samples_in_minute:]

# Create the sequence
data_last_minute = create_sequences(data_last_minute, SEQ_LENGTH)


# Detect anomalies
reconstructed_data = model.predict(data_last_minute)
reconstruction_error = np.mean(np.abs(reconstructed_data - data_last_minute), axis=-1)

# Get the reconstruction error for each feature
avg_reconstruction_error = np.mean(reconstruction_error, axis=0)


# Print the reconstruction error for each feature
# for i, feature in enumerate(features):
#     print(f'{feature}: {avg_reconstruction_error[i]}')

# print(f'Reconstruction error: {avg_reconstruction_error}')


# Connect to database

try:
    conn = sqlite3.connect('test.db')
    cursor = conn.cursor()
    print("Connected to SQLite")
except sqlite3.Error as e:
    print(f"Error connecting to SQLite: {e}")
    
# Create a table for the data
cursor.execute('''
    CREATE TABLE IF NOT EXISTS ml (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        start_time TEXT NOT NULL,
        description TEXT NOT NULL
    )
''')

# Print whether there is an anomaly for each stat
for i, feature in enumerate(features):
    if avg_reconstruction_error[i] > THRESHOLD:
        print(f'Anomaly detected in {feature} with error {avg_reconstruction_error[i]}')
        
        # Construct the message
        description = f'Anomaly detected in {feature} with error {avg_reconstruction_error[i]}'
        
        # Insert the data into the database
        cursor.execute('''
            INSERT INTO ml (start_time, description)
            VALUES (datetime("now"), ?)
        ''', (description,))
        
        conn.commit()
        print(f'Inserted anomaly into database: {description}')
    else:
        print(f'No anomaly detected in {feature} with error {avg_reconstruction_error[i]}')
