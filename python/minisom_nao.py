from __future__ import division

from numpy import genfromtxt, zeros, product, setdiff1d, arange, where, set_printoptions, unravel_index, tanh
from parameters import param
from random import choice

import plot_som as ps
import random
import sys
import os
import pdb

from minisom import MiniSom
from similar_vec import get_similar_data

#TODO
import pylab as plt

def get_path():
	"""
	Path to .dat file generated by NAO's babbling is given by a user in 
	terminal. If valid, return. 
	"""
	nrarg = len(sys.argv)

	if nrarg<2:
		raise Exception('Missing data path')

	path = str(sys.argv[1])

	if not os.path.exists(path):
		raise Exception('Path doesn\'t exist')

	return path

def read_data(path, nrpts=50):
	"""
	Return babbling coordinates for hands and joints.
		nrpts - take each nrpts-th coordinate for training
	"""
	
	hands = param['hands']
	joints = param['joints']
		
	data = {
		'hands': genfromtxt(path, skiprows=3, usecols=hands)[:nrpts],
		'joints': genfromtxt(path, skiprows=3, usecols=joints)[:nrpts]
		}

	return data 

def train_som(data):
	
	som = MiniSom(
		param['nr_rows'],
		param['nr_cols'], 
		data.shape[1], 
		data, 
		sigma=param['sigma'], 
		learning_rate=param['learning_rate'], 
		norm='minmax')
		
	#som.random_weights_init() # choose initial nodes from data points
	som.train_random(param['nr_epochs']) # random training
	
	return som

def hebbian_learning(som1, som2):
	s1, s2 = som1.weights.shape, som2.weights.shape
	hebb = zeros((param['nr_rows'], param['nr_cols'], \
		param['nr_rows'], param['nr_cols']))
	
	f = lambda x: 1/(1+tanh(x))
	for dp1, dp2 in zip(som1.data, som2.data):
		#pdb.set_trace()
		act1 = som1.activate(dp1)
		act2 = som2.activate(dp2)
				
		idx1 = som1.winner(dp1)
		idx2 = som2.winner(dp2)
		
		hebb[idx1[0], idx1[1], idx2[0], idx2[1]] += param['eta'] * f(act1[idx1]) * f(act2[idx2])
		
	return hebb


# useful plotting, TODO extract to plot som
def plot(som_hands, som_joints):
	wi_0, w_0 = som_hands.get_weights()	
	wi_1, w_1 = som_joints.get_weights()

	ps.plot_3d(final_som=w_0, data=som_hands.data, init_som=wi_0, nr_nodes=param['n_to_plot'], title='SOM Hands')
	ps.plot_3d(final_som=w_1, data=som_joints.data, init_som=wi_1, nr_nodes=param['n_to_plot'], title='SOM Joints')


def plot_inactivated_nodes(som, inact):
	_, w = som.get_weights()	
	act = setdiff1d(arange(w.shape[0]), inact)
	
	fig = plt.figure()
	ax = fig.add_subplot(111, projection = '3d')
	data = som.data
	
	data = data[:,:3]
		
	# plot data points	
	d = ax.plot(data[:, 0], data[:,1], data[:,2], c='b', marker='*', linestyle='None', alpha=0.4, label='data')
	
	# plot activated nodes in green
	act_nod = w[act, :]
	a = ax.plot(act_nod[:, 0], act_nod[:, 1], act_nod[:, 2], c='g', marker='o', alpha = 0.6, label='neurons', markersize=4)
	
	#plot inactivated nodes in red
	in_w = w[inact, :]
	i = ax.plot(in_w[:, 0], in_w[:, 1], in_w[:, 2], c='r', marker='o', alpha = 0.6, label='inact. neurons', markersize=6, linestyle='None')
	plt.legend(numpoints=1)
	
def print_strongest_connections(hebb_weights):
	# print the strongest connections
	for i in xrange(param['nr_rows']):
		for j in xrange(param['nr_cols']):
			maxw = -100
			map2X = 0; 
			map2Y = 0;
			for k in xrange(param['nr_rows']):
				for t in xrange(param['nr_cols']):
					if (hebb_weights[i][j][k][t] > maxw):
						maxw= hebb_weights[i][j][k][t];
						map2X = k; map2Y = t;
			string = "(" + str(map2X) + ", " + str(map2Y) + ")  "
			sys.stdout.write(string)
		print ''

def predict_joints_configuration(hebb, hands):
	"""
	Preditcs coordinates of joints based on the Hebbian weights connecting
	two SOMs
	"""	
	
	return hands

if __name__=="__main__":
	path = get_path()

	# get the coordinates learned during random motor babbling 
	data = read_data(path)

	# train self-organizing maps
	som_hands = train_som(data['hands'])
	som_joints = train_som(data['joints'])

	#plot(som_hands, som_joints)
	inact = som_joints.activation_response(som_joints.data)
	coord_inact = where(inact.flatten()==0)[0]
	plot_inactivated_nodes(som_joints, coord_inact)	
	ps.show()
		
	# hebbian weights connecting maps
	hebb = hebbian_learning(som_hands, som_joints)

	#print_strongest_connections(hebb)	
	nr_pts = 10
	mse = 0
	print 'Hands\t\t\t Joints\t\t\t Predicted joints'
	set_printoptions(precision=3)
	
	for i in xrange(nr_pts):
		idx = random.randint(0, len(som_hands.data)-1) # l(sh.d) == l(sj.d)
		hands_view = som_hands.data[idx, :]
		
		win_1 = som_hands.winner(hands_view)
		win_2 = unravel_index(hebb[win_1[0], win_1[1], :, :].argmax(), \
			som_hands.weights.shape[:2])
		
		joints = som_joints.weights[win_2[0], win_2[1], :]
		
		dist = sum(som_joints.data[idx, :] - joints)
		mse += sum((som_joints.data[idx, :] - joints)**2)
		print hands_view, som_joints.data[idx, :], joints, abs(dist)
		
		
	print 'MSE', mse/nr_pts	
	ps.show()
