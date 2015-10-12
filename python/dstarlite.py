#!/usr/bin/env python

import numpy as np
import math
import copy
import matplotlib.pyplot as plt
import matplotlib
from pylab import *

INF = float("inf")
MAX_STEPS = 100000
HASH_CONSTANT = 1000000 #if map width is larger than this, have to increase hash 'constant' to preserve uniqueness

class Cell:
    def __init__(self, x, y, cost, nbrs):
        self.x = x
        self.y = y
        self.cost = cost
        self.nbrs = nbrs

def cellhash(cell):
    return HASH_CONSTANT*cell.y+cell.x

class Planner:
    _cell_hash = dict() #dictionairy containing all cells that have been hashed
    _km = 0             #cumulative heuristic value
    _map = None         #numpy 'map'
    _path = []          #list of cells that make up the path
    _open_list = dict() #dictionary containing the open cells... order the keys when iterating through dstar
    _open_list_keysort = []
    _open_hash = dict() #dictionary containting pointers to open cell by hashing
    _start = None       #start cell
    _goal = None        #goal cell
    _last = None        #last cell
    
    def __init__(self, map_, start_, goal_):
        self._cell_hash = dict()
        self._open_list = dict()
        self._open_hash = dict()
        self._open_list_keysort = []
        self._path = []
        self._km = 0
        self._map = map_
        self._start = start_
        self._goal = goal_
        self._last = start_
        
        self._rhs(self._goal, 0.0)
        
        self._list_insert(self._goal, (self._h(self._start, self._goal), 0))
    
    def path(self):
        return self._path
    
    def goal(self, cell=None):
        if cell==None:
            return self._goal
        
        self._goal = cell
        return self._goal
    
    def replan(self):
        self._path = []
        print 1.25
        result = self._compute()
        print 1.5
        if not result:
            return False
        
        current = self._start
        self._path.append(current)
        
        while current != self._goal:
            if (current == None) or (self._g(current) == INF):
                return False
            current = self._min_succ(current)[0]
            
            self._path.append(current)
        print 1.6
        return True
    
    def start(self, cell=None):
        if cell==None:
            return self._start
        
        self._start = cell
        return self._start
    
    def update(self, cell, cost):
        if cell == self._goal:
            return
        
        self._km += self._h(self._last, self._start)
        self._last = self._start
        
        self._cell(cell)
        
        cost_old = cell.cost
        cost_new = cost
        cell.cost = cost
        #print 'updating'
        for i in range(0,len(cell.nbrs)):
            if cell.nbrs[i] != None:
                cell.cost = cost_old
                tmp_cost_old = self._cost(cell, cell.nbrs[i])
                cell.cost = cost_new
                tmp_cost_new = self._cost(cell, cell.nbrs[i])
                
                tmp_rhs = self._rhs(cell)
                tmp_g = self._g(cell.nbrs[i])
                
                if tmp_cost_old > tmp_cost_new:
                    if cell != self._goal:
                        self._rhs(cell, min(tmp_rhs, (tmp_cost_new + tmp_g)))
                elif tmp_rhs == (tmp_cost_old+tmp_g):
                    if cell != self._goal:
                        self._rhs(cell, self._min_succ(cell)[1])
        
        self._update(cell)
        
        for i in range(0, len(cell.nbrs)):
            if cell.nbrs[i] != None:
                cell.cost = cost_old
                tmp_cost_old = self._cost(cell, cell.nbrs[i])
                cell.cost = cost_new
                tmp_cost_new = self._cost(cell, cell.nbrs[i])
                
                tmp_rhs = self._rhs(cell.nbrs[i])
                tmp_g = self._g(cell)
                #print tmp_g, tmp_rhs, tmp_cost_old, tmp_cost_new, (tmp_cost_old+tmp_g)
                
                if tmp_cost_old > tmp_cost_new:
                    if cell.nbrs[i] != self._goal:
                        self._rhs(cell.nbrs[i], min(tmp_rhs, (tmp_cost_new + tmp_g)))
                elif tmp_rhs == (tmp_cost_old+tmp_g):
                    if cell.nbrs[i] != self._goal:
                        self._rhs(cell.nbrs[i], self._min_succ(cell.nbrs[i])[1])
                
                self._update(cell.nbrs[i])
    
    def _cell(self, cell):
        try:
            hash_list = self._cell_hash[cellhash(cell)]
            return
        except:
            self._cell_hash[cellhash(cell)] = (INF, INF)
    
    def _compute(self):
        if len(self._open_list) == 0:
            return False
        
        attempts = 0
        
        while ((len(self._open_list) > 0) and (self._open_list[self._open_list_keysort[0]][0][0] < self._k(self._start))) or (self._rhs(self._start) != self._g(self._start)):
            
            attempts += 1
            if attempts > MAX_STEPS:
                return False
            
            cell = self._open_list[self._open_list_keysort[0]][0][1]
            k_old = self._open_list[self._open_list_keysort[0]][0][0]
            k_new = self._k(cell)
            
            tmp_rhs = self._rhs(cell)
            tmp_g = self._g(cell)
            
            if k_old < k_new:
                self._list_update(cell, k_new)
            elif tmp_g > tmp_rhs:
                self._g(cell, tmp_rhs)
                tmp_g = tmp_rhs
                
                self._list_remove(cell)
                
                nbrs = cell.nbrs
                
                for i in range(0, len(nbrs)):
                    if nbrs[i] != None:
                        if nbrs[i] != self._goal:
                            self._rhs(nbrs[i], min(self._rhs(nbrs[i]), self._cost(nbrs[i], cell)+tmp_g))
                        #if nbrs[i] != self._goal:
                        #    self._rhs(nbrs[i], self._min_succ(nbrs[i])[1])
                        self._update(nbrs[i])
            else:
                g_old = tmp_g
                self._g(cell, INF)
                
                if cell != self._goal:
                    self._rhs(cell, self._min_succ(cell)[1])
                
                self._update(cell)
                
                nbrs = cell.nbrs
                
                for i in range(0, len(nbrs)):
                    if nbrs[i] != None:
                        if self._rhs(nbrs[i]) == (self._cost(nbrs[i], cell) + g_old):
                            if nbrs[i] != self._goal:
                                self._rhs(nbrs[i], self._min_succ(nbrs[i])[1])
                        #if nbrs[i] != self._goal:
                        #    self._rhs(nbrs[i], self._min_succ(nbrs[i])[1])
                        self._update(nbrs[i])
        print 'done'
        return True
    
    def _cost(self, cell_a, cell_b):
        if cell_a.cost == INF or cell_b.cost == INF:
            return INF
        
        dx = abs(cell_a.x - cell_b.x)
        dy = abs(cell_a.y - cell_b.y)
        scale = 1.0
        
        if ((dx + dy) > 1):
            scale = math.sqrt(2.0)
        
        return scale*((cell_a.cost + cell_b.cost)/2.0)
    
    def _g(self, cell, value=None):
        self._cell(cell)
        g_rhs = self._cell_hash[cellhash(cell)]
        
        if value != None:
            g_rhs = (value, g_rhs[1])
            self._cell_hash[cellhash(cell)] = (value, self._cell_hash[cellhash(cell)][1])
        
        return g_rhs[0]
    
    def _h(self, cell_a, cell_b):
        minval = abs(cell_a.x - cell_b.x)
        maxval = abs(cell_a.y - cell_b.y)
        
        if minval > maxval:
            tmp = minval
            minval = maxval
            maxval = tmp
        
        #return ((math.sqrt(2)-1.0)*minval+maxval)
        return maxval
    
    def _k(self, cell):
        g = self._g(cell)
        rhs = self._rhs(cell)
        if g < rhs:
            minval = g
        else:
            minval = rhs
        return (minval+self._h(self._start, cell)+self._km, minval)
    
    def _list_insert(self, cell, key):
        pair = (key, cell)
        try:
            self._open_list[key].append(pair)
        except:
            self._open_list[key] = [pair]
        self._open_hash[cellhash(cell)] = pair
        self._open_list_keysort = self._open_list.keys()
        self._open_list_keysort.sort()
    
    def _list_remove(self, cell):
        #print 'REMOVE BEFORE'
        #print cell, cellhash(cell), self._open_hash[cellhash(cell)]
        pair = self._open_hash[cellhash(cell)]
        k = pair[0]
        c = pair[1]
        a = self._open_list.keys()
        a.sort()
        self._open_list[k].remove(pair)
        if len(self._open_list[k]) == 0:
            self._open_list_keysort.remove(k)
            del self._open_list[k]
            self._open_list_keysort = self._open_list.keys()
            self._open_list_keysort.sort()
        del self._open_hash[cellhash(cell)]
        #print 'REMOVE AFTER'
    
    def _list_update(self, cell, key):
        self._list_remove(cell)
        self._list_insert(cell, key)
    
    def _min_succ(self, cell):
        nbrs = cell.nbrs
        #print "NEIGHBOURS START"
        #print cell.y, cell.x
        min_cell = None
        min_cost = INF
        
        for i in range(0, len(cell.nbrs)):
            if cell.nbrs[i] != None:
                tmp_cost = self._cost(cell, cell.nbrs[i])
                
                tmp_g = self._g(cell.nbrs[i])
                #print tmp_cost, tmp_g
                if tmp_cost == INF or tmp_g == INF:
                    continue
                
                tmp_cost += tmp_g
                
                if tmp_cost < min_cost:
                    min_cell = cell.nbrs[i]
                    min_cost = tmp_cost
        #print "NEIGHBOURS END"
        return (min_cell, min_cost)
    
    def _rhs(self, cell, value=None):
        if cell == self._goal:
            return 0
        
        self._cell(cell)
        g_rhs = self._cell_hash[cellhash(cell)]
        
        if value != None:
            g_rhs = (g_rhs[0], value)
            self._cell_hash[cellhash(cell)] = (self._cell_hash[cellhash(cell)][0], value)
        
        return g_rhs[1]
    
    def _update(self, cell):
        diff = self._g(cell) != self._rhs(cell)
        
        try:
            self._open_hash[cellhash(cell)]
            exists = True
        except:
            exists = False
        if diff and exists:
            self._list_update(cell, self._k(cell))
        elif diff and not exists:
            self._list_insert(cell, self._k(cell))
        elif not diff and exists:
            self._list_remove(cell)

testMap = np.empty((5, 3), dtype=object)
#init cells
for rownum in range(0, len(testMap[:,0])):
    for colnum in range(0, len(testMap[0,:])):
        testMap[rownum, colnum] = Cell(colnum, rownum, 1, [None, None, None, None, None, None, None, None])
#init neighbour cells
for rownum in range(0, len(testMap[:,0])):
    for colnum in range(0, len(testMap[0,:])):
        cell = testMap[rownum, colnum]
        if rownum != 0:
            if colnum != 0:
                cell.nbrs[0] = testMap[rownum-1, colnum-1]
            cell.nbrs[1] = testMap[rownum-1, colnum]
            if colnum < len(testMap[0,:])-1:
                cell.nbrs[2] = testMap[rownum-1, colnum+1]
        if colnum < len(testMap[0,:])-1:
            cell.nbrs[3] = testMap[rownum, colnum+1]
        if rownum < len(testMap[:,0])-1:
            if colnum < len(testMap[0,:])-1:
                cell.nbrs[4] = testMap[rownum+1, colnum+1]
            cell.nbrs[5] = testMap[rownum+1, colnum]
            if colnum != 0:
                cell.nbrs[6] = testMap[rownum+1, colnum-1]
        if colnum != 0:
            cell.nbrs[7] = testMap[rownum, colnum-1]
#place high cost cells
testMap[1,1].cost = INF
testMap[2,1].cost = INF

#testPlanner = Planner(testMap, testMap[1,0], testMap[4,2])
#testPlanner.replan()
#for cell in testPlanner._path:
#    print cell.y, cell.x
#for row in testPlanner._map:
#    for cell in row:
#        print cell.y, cell.x, 'g', testPlanner._g(cell), 'rhs', testPlanner._rhs(cell), 'k', testPlanner._k(cell)
#print testPlanner._open_list_keysort
#del testPlanner._path[0]
#testPlanner.start(testPlanner._path[0])
#testPlanner.update(testMap[3,1], INF)
#testPlanner.replan()
#for cell in testPlanner._path:
#    print cell.y, cell.x
#for row in testPlanner._map:
#    for cell in row:
#        print cell.y, cell.x, 'g', testPlanner._g(cell), 'rhs', testPlanner._rhs(cell), 'k', testPlanner._k(cell)
#print testPlanner._open_list_keysort

#create robot 'sensed' map
robotMap = np.empty((100, 100), dtype=object)
#init cells
for rownum in range(0, len(robotMap[:,0])):
    for colnum in range(0, len(robotMap[0,:])):
        robotMap[rownum, colnum] = Cell(colnum, rownum, 1, [None, None, None, None, None, None, None, None])
#init neighbour cells
for rownum in range(0, len(robotMap[:,0])):
    for colnum in range(0, len(robotMap[0,:])):
        cell = robotMap[rownum, colnum]
        if rownum != 0:
            if colnum != 0:
                cell.nbrs[0] = robotMap[rownum-1, colnum-1]
            cell.nbrs[1] = robotMap[rownum-1, colnum]
            if colnum < len(robotMap[0,:])-1:
                cell.nbrs[2] = robotMap[rownum-1, colnum+1]
        if colnum < len(robotMap[0,:])-1:
            cell.nbrs[3] = robotMap[rownum, colnum+1]
        if rownum < len(robotMap[:,0])-1:
            if colnum < len(robotMap[0,:])-1:
                cell.nbrs[4] = robotMap[rownum+1, colnum+1]
            cell.nbrs[5] = robotMap[rownum+1, colnum]
            if colnum != 0:
                cell.nbrs[6] = robotMap[rownum+1, colnum-1]
        if colnum != 0:
            cell.nbrs[7] = robotMap[rownum, colnum-1]
#robotMap[25, 25].cost = INF

#create actual map
realMap = np.empty((100, 100), dtype=object)
#init cells
for rownum in range(0, len(realMap[:,0])):
    for colnum in range(0, len(realMap[0,:])):
        realMap[rownum, colnum] = Cell(colnum, rownum, 1, [None, None, None, None, None, None, None, None])
#place untraversable cells
for colnum in range(25, 75):
    realMap[25, colnum].cost = INF
for rownum in range(25, 75):
    realMap[rownum, 25].cost = INF
for colnum in range(25, 60):
    realMap[75, colnum].cost = INF
for rownum in range(40, 75):
    realMap[rownum, 59].cost = INF
for colnum in range(40, 60):
    realMap[40, colnum].cost = INF

def update_map(x, y, robotMap, realMap, planner):
    error = False
    for i in range(-15,16):
        for j in range(-15,16):
            if (y+i >= 0 and y+i < len(robotMap[:,0])) and (x+j >= 0 and x+j < len(robotMap[0,:])):
                if realMap[y+i, x+j].cost != robotMap[y+i, x+j].cost:
                    error = True
                    cost = realMap[y+i, x+j].cost
                    planner.update(robotMap[y+i, x+j], cost)
    return error

def execute(planner, pos, ax, z):
    global takenPath
    count = 0
    while planner._start != planner._goal:
        print 1
        if count%1 == 0:
            if update_map(pos.x, pos.y, robotMap, realMap, planner):
                planner.replan()
                print 1.75
        count += 1
        print 2
        if len(planner._path) > 0:
            del planner._path[0]   
        print 3
        planner.start(planner._path[0])
        pos = planner._path[0]
        print pos.y, pos.x
        takenPath.append(pos)
        #for cell in planner._path:
        #    z[cell.y, cell.x] = count
        #z[pos.y, pos.x] = 100
        #ax.pcolor(x,y,z)
        #draw()
    
    pos = planner._goal

#plt.ion()
#fig = plt.figure()
#ax = fig.add_subplot(111)
#grid = np.indices((100,100))
#x = grid[1]
#y = grid[0]
#z = np.empty((100, 100), dtype=float)
#for rownum in range(0, len(realMap[:,0])):
#    for colnum in range(0, len(realMap[0,:])):
#        if realMap[rownum,colnum].cost == INF:
#            cost = 100
#        else:
#            cost = realMap[rownum,colnum].cost
#        z[rownum,colnum] = cost
#
#takenPath = []
#robotPlanner = Planner(robotMap, robotMap[1,1], robotMap[50,50])
#robotPlanner.replan()
#execute(robotPlanner, robotMap[1,1], ax, z)
#c = 0
#for cell in takenPath:
#    c += 1
#    z[cell.y, cell.x] = c
#ax.pcolor(x,y,z)
#plt.show()

import ray_terrain
terrain = ray_terrain.RayTerrain()
terrain.genPerlinTerrain(200, [-500,500,-500,500], 1/64.0, 5, 10, -10)
print 'terrain generation complete.'

robotMap = np.empty((200, 200), dtype=object)
#init cells
for rownum in range(0, len(robotMap[:,0])):
    for colnum in range(0, len(robotMap[0,:])):
        robotMap[rownum, colnum] = Cell(colnum, rownum, 1, [None, None, None, None, None, None, None, None])
#init neighbour cells
for rownum in range(0, len(robotMap[:,0])):
    for colnum in range(0, len(robotMap[0,:])):
        cell = robotMap[rownum, colnum]
        if rownum != 0:
            if colnum != 0:
                cell.nbrs[0] = robotMap[rownum-1, colnum-1]
            cell.nbrs[1] = robotMap[rownum-1, colnum]
            if colnum < len(robotMap[0,:])-1:
                cell.nbrs[2] = robotMap[rownum-1, colnum+1]
        if colnum < len(robotMap[0,:])-1:
            cell.nbrs[3] = robotMap[rownum, colnum+1]
        if rownum < len(robotMap[:,0])-1:
            if colnum < len(robotMap[0,:])-1:
                cell.nbrs[4] = robotMap[rownum+1, colnum+1]
            cell.nbrs[5] = robotMap[rownum+1, colnum]
            if colnum != 0:
                cell.nbrs[6] = robotMap[rownum+1, colnum-1]
        if colnum != 0:
            cell.nbrs[7] = robotMap[rownum, colnum-1]

#create actual map
realMap = np.empty((200, 200), dtype=object)
#init cells
for rownum in range(0, len(realMap[:,0])):
    for colnum in range(0, len(realMap[0,:])):
        realMap[rownum, colnum] = Cell(colnum, rownum, (-1*terrain.z_grid[rownum, colnum])**3.0, [None, None, None, None, None, None, None, None])

fig = plt.figure(figsize=(15, 6))
ax = fig.add_subplot(121)
ax2 = fig.add_subplot(122)
grid = np.indices((200,200))
x = grid[1]
y = grid[0]
z = np.empty((200, 200), dtype=float)
maxn = 0
for rownum in range(0, len(realMap[:,0])):
    for colnum in range(0, len(realMap[0,:])):
        #z[rownum,colnum] = terrain.z_grid[rownum, colnum]
        z[rownum,colnum] = realMap[rownum, colnum].cost
        if z[rownum,colnum] > maxn:
            maxn = z[rownum,colnum]
ax.pcolor(x,y,z)
plt.show()
z2 = np.empty((200, 200), dtype=float)
for rown in range(0, len(robotMap[:,0])):
    for coln in range(0, len(robotMap[0,:])):
        z2[rown,coln] = robotMap[rown, coln].cost

takenPath = []
robotPlanner = Planner(robotMap, robotMap[1,1], robotMap[100,100])
robotPlanner.replan()
execute(robotPlanner, robotMap[1,1], ax, z)
c = 0
d = 1
for cell in takenPath:
    c += d
    if c == 10:
        d = -1
    if c == 0:
        d = 1
    z[cell.y, cell.x] = c
ax.pcolor(x,y,z)

z2 = np.empty((200, 200), dtype=float)
for rown in range(0, len(robotMap[:,0])):
    for coln in range(0, len(robotMap[0,:])):
        z2[rown,coln] = robotMap[rown, coln].cost

ax2.pcolor(x,y,z2)
plt.savefig('test.png')
plt.show()