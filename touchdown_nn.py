#!/usr/bin/env python3

# This uses tensorflow (the keras API) to train a neural network to predict the 
# game theoretic value of any position.

# The data set consists of approx 750 thousand samples for the 4x4 touchdown board game.
# The idea is to vary the topology of the neural network (specifically the number
# of neurons in the hidden layers) to see how the accuracy varies.
#
# I've got the following results (using 250 epochs and a batch size of 512:
# Topology | Params | Loss    | Accuracy | Time
# 32,32    |  2178  | 0.0202  | 0.9915   |
# 64,64    |  6402  | 0.0071  | 0.9972   |
# 128,128  | 20994  | 0.00087 | 0.9998   |
# 256,128  | 41602  | 0.00035 | 0.9999   | 56 min

epochs = 250
batch_size = 512

hidden1 = 256
hidden2 = 128

import gzip
import numpy as np
import tensorflow as tf

from tensorflow.python.keras.models import Sequential
from tensorflow.python.keras.layers import InputLayer, Input
from tensorflow.python.keras.layers import Reshape, MaxPooling2D
from tensorflow.python.keras.layers import Conv2D, Dense, Flatten


input_width = 32    # Number of input features
output_width = 2    # Number of output classes

with gzip.open("touchdown.txt.gz", 'rb') as f:
    input_data = np.loadtxt(f, usecols=range(input_width))

with gzip.open("touchdown.txt.gz", 'rb') as f:
    output_data = np.loadtxt(f, usecols=range(input_width, input_width + output_width))


print("Total data available:")
print(input_data.shape)
print(output_data.shape)

##########################################################
# SEQUENTIAL MODEL
##########################################################

# Start construction of the Keras Sequential model.
model = Sequential()

# Add an input layer which is similar to a feed_dict in TensorFlow.
# Note that the input-shape must be a tuple containing the image-size.
model.add(InputLayer(input_shape=(input_width,)))

# First fully-connected / dense layer with ReLU-activation.
model.add(Dense(hidden1, activation='relu'))

# Second fully-connected / dense layer with ReLU-activation.
model.add(Dense(hidden2, activation='relu'))

# Last fully-connected / dense layer with softmax-activation
# for use in classification.
model.add(Dense(output_width, activation='softmax'))

##########################################################
# MODEL COMPILATION
##########################################################

from tensorflow.python.keras.optimizers import Adam

optimizer = Adam(lr=1e-3)

model.compile(optimizer=optimizer,
              loss='categorical_crossentropy',
              metrics=['accuracy'])

model.summary()


##########################################################
# TRAINING
##########################################################


model.fit(x=input_data,
          y=output_data,
          epochs=epochs, batch_size=batch_size)


