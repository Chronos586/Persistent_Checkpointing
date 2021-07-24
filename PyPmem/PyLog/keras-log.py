import smdebuglog.tensorflow as smd
#import smdebug.tensorflow as smd
from tensorflow.keras.losses import SparseCategoricalCrossentropy
from tensorflow.keras import datasets, layers, models
from tensorflow.keras.datasets import cifar10
from tensorflow.keras.callbacks import Callback
from tensorflow.keras.optimizers import Adam
import os
import time

def main():
    #hook = smd.KerasHook.create_from_json_file()
    hook = smd.KerasHook(
        out_dir="./tmp",
        # Information on default collections https://github.com/awslabs/sagemaker-debugger/blob/master/docs/api.md#default-collections-saved
        include_collections=["weights"],
        save_config=smd.SaveConfig(save_interval=500)
    )
    (X_train, y_train), (X_valid, y_valid) = cifar10.load_data()

    X_train, X_valid = X_train / 255.0, X_valid / 255.0

    model = models.Sequential()
    model.add(layers.Conv2D(32, (3, 3), activation='relu', input_shape=(32, 32, 3)))
    model.add(layers.MaxPooling2D((2, 2)))
    model.add(layers.Conv2D(64, (3, 3), activation='relu'))
    model.add(layers.MaxPooling2D((2, 2)))
    model.add(layers.Conv2D(64, (3, 3), activation='relu'))
    model.add(layers.Flatten())
    model.add(layers.Dense(64, activation='relu'))
    model.add(layers.Dense(10))
    optimizer = Adam()
    optimizer = hook.wrap_optimizer(optimizer)
    hook.register_model(model)
    model.compile(optimizer=optimizer,loss=SparseCategoricalCrossentropy(from_logits=True),metrics=['accuracy'])
    batch_size = 128
    #batch_size = 256
    #batch_size = 512
    epochs = 2

    # Add the hook as a callback
    # Set hook.set_mode to set tensors to be stored in different phases of training job, such as TRAIN and EVAL

    t0 =  time.time()
    model.fit(X_train, y_train, batch_size=batch_size,epochs=epochs, callbacks=[hook])
    t1 =  time.time()

    print("Batch Size: {}\nEpoch Size: {}".format(batch_size,epochs))
    print("Training Time: {}".format(t1-t0))

if __name__ == "__main__":
    main()
