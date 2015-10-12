#!/usr/bin/env python

from pylab import *
from node import node
from comms import comms
from bhv_isosceles import bhv_isosceles
from bhv_physicomimetics import bhv_physicomimetics
import random

num_veh = 15
veh_list = []
veh_list_xy = np.zeros((2, num_veh))
veh_trail_list = []
trail_len = 50
veh_trail_list_x = np.full((num_veh, trail_len), np.nan)
veh_trail_list_y = np.full((num_veh, trail_len), np.nan)

#initialize nodes and node behaviour
for i in range(0,num_veh):
	n = node(i, random.random()*10, random.random()*10, random.random()*2*math.pi)
	b = bhv_isosceles(173.205)
	#b = bhv_physicomimetics(150, 3600, 2, 1, 1)
	n.behaviour = b
	veh_list.append(n)

fig = plt.figure()
ax = plt.subplot()

colors = np.random.rand(num_veh)
area = np.full((num_veh, 1), 100, dtype=np.int)
nodes = ax.scatter(veh_list_xy[0,:], veh_list_xy[1,:], s=area, c=colors, alpha=0.5)

for i in range(0,num_veh):
	trail, = ax.plot([0.0], [0.0], color='m', linewidth=2, linestyle='-', marker=None)
	veh_trail_list.append(trail)

ax.axis('equal')
ax.set_xlim(-500, 500)
ax.set_ylim(-500, 500)

update_interval = 10
#dt = update_interval/100.0
dt = 0.5

node_drag = False
node_drag_idx = 0

veh_comms = comms(veh_list, 300)
comms_interval = 1000

def update_timer_callback():
	global ratio, veh_trail_list_x, veh_trail_list_y

	veh_trail_list_x = np.roll(veh_trail_list_x, 1, axis=1)
	veh_trail_list_y = np.roll(veh_trail_list_y, 1, axis=1)
	for idx, node in enumerate(veh_list):
		node.run(dt)
		node.compute_behaviour()
		veh_list_xy[0,idx] = node.x
		veh_list_xy[1,idx] = node.y
		veh_trail_list_x[idx,0] = node.x
		veh_trail_list_y[idx,0] = node.y
		veh_trail_list[idx].set_xdata(veh_trail_list_x[idx])
		veh_trail_list[idx].set_ydata(veh_trail_list_y[idx])
		#if (idx==0):
		#	print node.neighbours

	nodes.set_offsets(np.transpose(veh_list_xy))
	
	xmin, xmax = ax.get_xlim()
	ratio = abs(xmax-xmin)/fig.get_size_inches()[0]

	fig.canvas.draw()

def comms_timer_callback():
	veh_comms.update()

def on_button_press(event):
	global node_drag, node_drag_idx

	if event.button == 1:
		for idx, node in enumerate(veh_list):
			dist = math.sqrt((event.xdata - node.x)**2 + (event.ydata - node.y)**2)
			if dist < 0.1*ratio:
				node_drag = True
				node_drag_idx = idx

def on_button_release(event):
	global node_drag, node_drag_idx

	if event.button == 1:
		node_drag = False
	elif event.button == 3:
		if event.xdata == None or event.ydata == None:
			return
		veh_list[node_drag_idx].wpt_x = event.xdata
		veh_list[node_drag_idx].wpt_y = event.ydata

def on_mouse_motion(event):
	if node_drag:
		if event.xdata == None or event.ydata == None:
			return
		veh_list[node_drag_idx].x = event.xdata
		veh_list[node_drag_idx].y = event.ydata
		veh_list_xy[0,node_drag_idx] = event.xdata
		veh_list_xy[1,node_drag_idx] = event.ydata
		nodes.set_offsets(np.transpose(veh_list_xy))

def on_key_press(event):
	global veh_trail_list_x, veh_trail_list_y

	if event.key == 'c':
		veh_trail_list_x = np.full((num_veh, trail_len), np.nan)
		veh_trail_list_y = np.full((num_veh, trail_len), np.nan)

update_timer = fig.canvas.new_timer(interval=update_interval)
update_timer.add_callback(update_timer_callback)
update_timer.start()

comms_timer = fig.canvas.new_timer(interval=comms_interval)
comms_timer.add_callback(comms_timer_callback)
comms_timer.start()

fig.canvas.mpl_connect('button_press_event', on_button_press)
fig.canvas.mpl_connect('button_release_event', on_button_release)
fig.canvas.mpl_connect('motion_notify_event', on_mouse_motion)
fig.canvas.mpl_connect('key_press_event', on_key_press)

plt.show()
