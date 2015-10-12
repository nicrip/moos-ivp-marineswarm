#!/usr/bin/env python

import math
import numpy as np

class node(object):
	def __init__(self, ident, x, y, theta):
		self.ident = ident
		self.x = x
		self.y = y
		self.theta = theta*math.pi/180.0
		self.vel = 6.0
		self.angle_steer = 0.0*math.pi/180.0
		self.max_steer = 30.0*math.pi/180.0
		self.rate_steer = 20.0*math.pi/180.0
		self.wheel_base = 4.0
		self.at_wpt_dist = 0.1
		self.wpt_x = 0.0
		self.wpt_y = 0.0
		self.wpt_dist = 0.0
		self.neighbours = {}
		self.behaviour = None	#class that outputs x, y waypoint

	def run(self, dt):
		self.compute_steering(dt)
		self.vehicle_model(dt)

	def compute_steering(self, dt):
		self.wpt_dist = math.sqrt((self.wpt_x - self.x)**2 + (self.wpt_y - self.y)**2)

		deltaG = self.pi_to_pi(math.atan2(self.wpt_y - self.y, self.wpt_x - self.x) - self.theta - self.angle_steer)

		maxDelta = self.rate_steer*dt
		if (abs(deltaG) > maxDelta):
			deltaG = np.sign(deltaG)*maxDelta

		self.angle_steer = self.angle_steer + deltaG
		if (abs(self.angle_steer) > self.max_steer):
			self.angle_steer = np.sign(self.angle_steer)*self.max_steer

	def vehicle_model(self, dt):
		if (self.wpt_dist < self.at_wpt_dist):
			self.angle_steer = 0
			return

		self.x += self.vel*dt*math.cos(self.angle_steer + self.theta)
		self.y += self.vel*dt*math.sin(self.angle_steer + self.theta)
		self.theta = self.pi_to_pi(self.theta + self.vel*dt*np.sin(self.angle_steer)/self.wheel_base)

	def compute_behaviour(self):
		if self.behaviour == None:
			return
		else:
			self.wpt_x, self.wpt_y = self.behaviour.compute(self)

	def pi_to_pi(self, angle_):
		angle_ = angle_ % (2.0*math.pi)

		if (angle_ > math.pi):
			angle_ -= 2.0*math.pi
		elif (angle_ < -math.pi):
			angle_ += 2.0*math.pi

		return angle_