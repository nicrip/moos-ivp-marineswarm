#!/usr/bin/env python

import math
import numpy as np
import itertools

class comms(object):
	def __init__(self, node_list, comms_dist):
		self.node_list = node_list
		self.comms_dist = comms_dist
		self.update()

	def update(self):
		dist = None
		for idx, node in enumerate(self.node_list):	#loop through list of nodes
			ident = node.ident
			if (idx < len(self.node_list)-1):
				for node_neighbour in itertools.islice(self.node_list, idx+1, None):	#loop through remaining list of nodes after current node
					ident_neighbour = node_neighbour.ident
					dist = math.sqrt((node.x - node_neighbour.x)**2 + (node.y - node_neighbour.y)**2)
					if dist <= self.comms_dist:
						node.neighbours[ident_neighbour] = [node_neighbour.x, node_neighbour.y]
						node_neighbour.neighbours[ident] = [node.x, node.y]
					else:
						node.neighbours.pop(ident_neighbour, None)
						node_neighbour.neighbours.pop(ident, None)

