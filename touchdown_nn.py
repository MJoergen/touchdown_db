#!/usr/bin/env python3

# This uses tensorflow (the keras API) to train a neural network to predict the 
# game theoretic value of any position.

# The data set consists of approx 750 thousand samples for the 4x4 touchdown board game.
# The idea is to vary the topology of the neural network (specifically the number
# of neurons in the hidden layer) to see how the accuracy varies.
# I've got the following results (using 50 epochs and a batch size of 1024:
# Topology | Params | Training acc | Test acc
#        4 |   142  |  0.9738      |  0.9215
#        8 |   282  |  0.9675      |  0.9133
#       16 |   562  |  0.9834      |  0.9523
#       32 |  1122  |  0.9871      |  0.9627
#       64 |  2242  |  0.9901      |  0.9755
#      128 |  4482  |  0.9923      |  0.9798
#      256 |  8962  |  0.9952      |  0.9837
#      512 | 17922  |  0.9974      |  0.9905 - could perhaps be run longer than 50 epochs.
#    64,16 |  3186  |  0.9923      |  0.9763
#    64,32 |  4258  |  0.9925      |  0.9804
#    64,64 |  6402  |  0.9951      |  0.9826
#  128,128 | 20994  |  0.9980      |  0.9882 - could perhaps be run longer than 50 epochs.
#  128,128 | 20994  |  0.9999      |  0.9938 - after 250 epochs with a batch size of 512.
#    64,64 |  6402  |  0.9993      |  0.9987 - after 250 epochs with a batch size of 128.
# 64,64,64 | 10562  |  0.9996      |  0.9933 - after 250 epochs with a batch size of 128.
#    32,32 |  2178  |  0.9957      |  0.9862 - after 250 epochs with a batch size of 128.
# 32,32,32 |  3234  |  0.9967      |  0.9879 - after 250 epochs with a batch size of 128.
#  128,128 | 20994  |  0.9998      |  0.9513 - after 250 epochs with a batch size of 128.

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

train_size = 740000  # Use this number of data rows for training
test_size  =  10000  # Use this number of data rows for testing

input_data_train  = input_data[:train_size]
output_data_train = output_data[:train_size]
input_data_test   = input_data[train_size:train_size+test_size]
output_data_test  = output_data[train_size:train_size+test_size]

print("Training data:")
print(input_data_train.shape)
print(output_data_train.shape)
print("Test data:")
print(input_data_test.shape)
print(output_data_test.shape)

##########################################################
# SEQUENTIAL MODEL
##########################################################

# Start construction of the Keras Sequential model.
model = Sequential()

# Add an input layer which is similar to a feed_dict in TensorFlow.
# Note that the input-shape must be a tuple containing the image-size.
model.add(InputLayer(input_shape=(input_width,)))

# First fully-connected / dense layer with ReLU-activation.
model.add(Dense(128, activation='relu'))

# Second fully-connected / dense layer with ReLU-activation.
model.add(Dense(128, activation='relu'))

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


model.fit(x=input_data_train,
          y=output_data_train,
          epochs=250, batch_size=128)



##########################################################
# EVALUATION
##########################################################

print('Evaluating test data')
result = model.evaluate(x=input_data_test,
                        y=output_data_test)

print('')

for name, value in zip(model.metrics_names, result):
    print(name, value)

print('')

print("{0}: {1:.2%}".format(model.metrics_names[1], result[1]))


##########################################################
# Examples of mis-classified images
##########################################################


#y_pred = model.predict(x=input_data_test)

#cls_pred = np.argmax(y_pred, axis=1)


