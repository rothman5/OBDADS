"""
Common variables for the machine learning
"""

import numpy as np
import pandas as pd
import numpy as np
import sqlite3
import matplotlib.pyplot as plt

import tensorflow as tf
from keras._tf_keras import keras
from keras._tf_keras.keras.models import Sequential
from keras._tf_keras.keras.layers import LSTM, Dense, RepeatVector, TimeDistributed
from sklearn.preprocessing import MinMaxScaler

# The features in the model (columns in the DB)
features = ['ENGINE_RPM', 'VEHICLE_SPEED', 'THROTTLE', 'ENGINE_LOAD']

# The name of the model file to save
model_name = 'model.keras'

# Determine the number of samples
SAMPLE_PERIOD = 50  # ms
samples_per_second = 1000 / SAMPLE_PERIOD
samples_in_minute = int(samples_per_second * 60)
samples_in_10_minutes = int(samples_per_second * 60 * 10)

# Define hyperparameters
SEQ_LENGTH = 40
THRESHOLD = 0.01

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

