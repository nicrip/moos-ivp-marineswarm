#!/usr/bin/env python

import math
import itertools

class bhv_isosceles(object):
	def __init__(self, distance):
		self.neighbour_1 = None
		self.neighbour_2 = None
		self.neighbour_1_pos = None
		self.neighbour_2_pos = None
		self.neighbour_dists = {}
		self.neighbour_dists_sorted = []
		self.distance = distance

	def compute(self, node):
		self.neighbour_dists = {}
		self.neighbour_dists_sorted = []
		for neighbour in node.neighbours:
			dist = math.sqrt((node.x - node.neighbours[neighbour][0])**2 + (node.y - node.neighbours[neighbour][1])**2)
			self.neighbour_dists[neighbour] = dist
		self.neighbour_dists_sorted = sorted(self.neighbour_dists.items(), key=lambda x: (x[1],x[0]))	#sort by ascending distances
		self.neighbour_1 = self.neighbour_dists_sorted[0][0]
		self.neighbour_1_pos = node.neighbours[self.neighbour_1]
		
		#dist_1 = self.neighbour_dists_sorted[0][1]
		total_dist = float("inf")
		final_neighbour_2 = None
		for n2 in itertools.islice(self.neighbour_dists_sorted, 1, None):
			self.neighbour_2 = n2[0]
			self.neighbour_2_pos = node.neighbours[self.neighbour_2]
			dist_2 = n2[1]
			dist_3 = math.sqrt((self.neighbour_1_pos[0] - self.neighbour_2_pos[0])**2 + (self.neighbour_1_pos[1] - self.neighbour_2_pos[1])**2)
			if (dist_2 + dist_3) < total_dist:
				final_neighbour_2 = self.neighbour_2
		self.neighbour_2 = final_neighbour_2
		
		# self.neighbour_2 = self.neighbour_dists_sorted[1][0]
		
		self.neighbour_2_pos = node.neighbours[self.neighbour_2]
		midpoint = [(self.neighbour_1_pos[0] + self.neighbour_2_pos[0])/2.0, (self.neighbour_1_pos[1] + self.neighbour_2_pos[1])/2.0]
		angle = math.atan2(self.neighbour_1_pos[1] - self.neighbour_2_pos[1], self.neighbour_1_pos[0] - self.neighbour_2_pos[0])
		candidate_point_1 = [midpoint[0] + self.distance*math.cos(angle+90*math.pi/180), midpoint[1] + self.distance*math.sin(angle+90*math.pi/180)]
		candidate_point_2 = [midpoint[0] + self.distance*math.cos(angle-90*math.pi/180), midpoint[1] + self.distance*math.sin(angle-90*math.pi/180)]
		dist_1 = ((node.x - candidate_point_1[0])**2 + (node.y - candidate_point_1[1])**2)
		dist_2 = ((node.x - candidate_point_2[0])**2 + (node.y - candidate_point_2[1])**2)
		print "NODE ", node.ident
		print self.neighbour_dists_sorted
		print self.neighbour_1, self.neighbour_2
		print ""
		if dist_1 < dist_2:
			return candidate_point_1[0], candidate_point_1[1]
		else:
			return candidate_point_2[0], candidate_point_2[1]