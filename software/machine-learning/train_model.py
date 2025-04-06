"""
Code to train a model for the anomaly detection task using a recurrent neural network (RNN) with LSTM layers.
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

from common import features, SEQ_LENGTH, THRESHOLD, create_sequences, samples_in_10_minutes

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

# Read the last 10 minutes of data from sqlite
conn = sqlite3.connect('test.db')
cursor = conn.cursor()

# Read the last 10 minutes of data from the data table
cursor.execute('SELECT * FROM data ORDER BY timestamp DESC LIMIT ?', (samples_in_10_minutes,))

# Select the relevant columns
rows = cursor.fetchall()
data = pd.DataFrame(rows, columns=['timestamp'] + features)
data = data[features].values

# Close the cursor and connection
cursor.close()
conn.close()

# Normalize the data
scaler = MinMaxScaler()
data = scaler.fit_transform(data)

# Create sequences
X_train = create_sequences(data, SEQ_LENGTH)

# Train the model
early_stopping = keras.callbacks.EarlyStopping(patience=2, restore_best_weights=True)
model.fit(X_train, X_train, epochs=100, batch_size=64, validation_split=0.1, callbacks=[early_stopping])

# Save the model
model.save('model.keras')
print('Model saved to model.keras')