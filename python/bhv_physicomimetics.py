#!/usr/bin/env python

import math
import random

class bhv_physicomimetics(object):
	def __init__(self, R, G, p, Fmax, deltaT):
		self.R = R
		self.G = G
		self.p = p
		self.Fmax = Fmax
		self.deltaT = deltaT

	def compute(self, node):
		theta = 0
		index = 0
		sum_fx = 0.0
		sum_fy = 0.0
		vx = 0.0
		vy = 0.0
		F = 0.0
		dist = None
		angle = None
		for neighbour in node.neighbours:
			dist = math.sqrt((node.x - node.neighbours[neighbour][0])**2 + (node.y - node.neighbours[neighbour][1])**2)
			angle = math.atan2(node.y - node.neighbours[neighbour][1], node.x - node.neighbours[neighbour][0])
			if (dist > 1.5*self.R):
				F = 0.0
			elif (dist <= 1):
				F = self.Fmax + 1
				angle = random.random()*2*math.pi
			else:
				F = self.G / (dist**self.p)
				if (F > self.Fmax):
					F = self.Fmax
				#if (F < 0.1):
				#	F = 0.0
				if (dist < self.R):
					F = -F

			fx = F*math.cos(angle)
			fy = F*math.sin(angle)
			sum_fx = sum_fx + fx
			sum_fy = sum_fy + fy
		delta_vx = self.deltaT*sum_fx
		delta_vy = self.deltaT*sum_fy
		vx = vx + delta_vx
		vy = vy + delta_vy
		delta_x = -self.deltaT*vx
		delta_y = -self.deltaT*vy
		print "NODE:", node.ident
		print "force: ", F, "deltas: ",  delta_x, delta_y
		print "dist: ", dist, "angle: ", angle
		return node.x + delta_x, node.y + delta_y