import cma
import os

dim = 8
x = dim * [0.0]
sigma = 0.1

# objective function.
def objective_function(x):
    cmd = './player --configuration'
    for i in x:
    	cmd += ' %f' %i
    pipe = os.popen(cmd, 'r')
    return 1.0 / (1.0 + float(pipe.read()))

res = cma.fmin(objective_function, x, sigma)
