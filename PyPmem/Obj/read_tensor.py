from smdebug.trials import create_trial

trial = create_trial("./tmp")
tens_list = trial._tensors_for_step(step=0)
tname = 'conv2d/weights/conv2d/kernel:0'
tensor = trial.tensor(tname)
print(trial.tensor_names)
print(tens_list)
print(tensor.value(step_num=0))
