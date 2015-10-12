#!/usr/bin/env python

'''Perlin/Diamond-Square Terrain Generation with Ray Intersection Test - Nick Rypkema 2013'''

import random
import math
import numpy as np
import matplotlib.delaunay as dly
import matplotlib.tri as tri
import matplotlib.pyplot as plt
from matplotlib import cm
from mpl_toolkits.mplot3d import Axes3D

class Ray:
    def __init__(self, origin, direction):
        self.origin = origin
        self.direction = direction
        self.triID_nodes = None
        self.triID = None
        self.tMax = 20.0
        self.tMin = 0.5
    
    def calcPosOnVec(self, dist):
        return self.origin + (self.direction*dist)
    
    def rotateDirectionVec(self, axis, degrees):
        rad = math.radians(degrees)
        cos = math.cos(rad)
        sin = math.sin(rad)
        if axis == 'x':
            R = np.array([[1,0,0],[0,cos,-sin],[0,sin,cos]])
            d = np.array([list(self.direction)]).T
            result = np.dot(R,d)
            self.direction = result.T[0]
            return True
        elif axis == 'y':
            R = np.array([[cos,0,sin],[0,1,0],[-sin,0,cos]])
            d = np.array([list(self.direction)]).T
            result = np.dot(R,d)
            self.direction = result.T[0]
            return True
        elif axis == 'z':
            R = np.array([[cos,-sin,0],[sin,cos,0],[0,0,1]])
            d = np.array([list(self.direction)]).T
            result = np.dot(R,d)
            self.direction = result.T[0]
            return True
        else:
            return False

class RayTerrain:
    def __init__(self):
        #self.mesh = None
        #self.mesh_planes = None
        self.triangles = None
        self.x_list = None
        self.y_list = None
        self.z_list = None
        self.x_int_grid = None
        self.y_int_grid = None
        self.idx_grid = None
        self.x_world_grid = None
        self.y_world_grid = None
        self.int_world_scale = None
        self.int_world_offset = None
        self.z_grid = None
        self.EPSILON = 1e-8
        self.low_res_factor = 30
        self.x_low_res = None
        self.y_low_res = None
        self.z_low_res = None
    
    def rebin(self, a, newshape):
        assert len(a.shape) == len(newshape)
        
        slices = [ slice(0,old, float(old)/new) for old,new in zip(a.shape,newshape) ]
        coordinates = np.mgrid[slices]
        indices = coordinates.astype('i')   #choose the biggest smaller integer index
        return a[tuple(indices)]
    
    def triangulate(self, x_list, y_list, width, height):
        triIdxs = np.zeros(((width-1)*(height-1)*2,3), dtype=np.int)
        triIdx = 0
        for colIdx in range(width-1):
            for rowIdx in range(height-1):
                tl = (rowIdx, colIdx)
                bl = (rowIdx+1, colIdx)
                br = (rowIdx+1, colIdx+1)
                tr = (rowIdx, colIdx+1)
                triIdxs[triIdx] = [self.idx_grid[tl],self.idx_grid[bl],self.idx_grid[br]]
                triIdx += 1
                triIdxs[triIdx] = [self.idx_grid[tl],self.idx_grid[br],self.idx_grid[tr]]
                triIdx += 1
        return triIdxs
    
    def genPerlinTerrain(self, sidelen, limits, freq, octs, scale, offset):
        def noise(x, y, per):
            def surflet(gridX, gridY):
                distX, distY = abs(x-gridX), abs(y-gridY)
                polyX = 1 - 6*distX**5 + 15*distX**4 - 10*distX**3
                polyY = 1 - 6*distY**5 + 15*distY**4 - 10*distY**3
                hashed = perm[perm[int(gridX)] + int(gridY)]
                #hashed = perm[perm[int(gridX)%per] + int(gridY)%per] #modulo period allows tiling
                grad = (x-gridX)*dirs[hashed][0] + (y-gridY)*dirs[hashed][1]
                return polyX * polyY * grad
            intX, intY = int(x), int(y)
            return (surflet(intX+0, intY+0) + surflet(intX+1, intY+0) +
                    surflet(intX+0, intY+1) + surflet(intX+1, intY+1))
        
        def fBm(x, y, per, octs):
            val = 0
            for o in range(octs):
                val += 0.5**o * noise(x*2**o, y*2**o, per*2**o)
            return val
        
        print 'replacing mesh with a Perlin terrain of size', sidelen, 'by', sidelen
        
        perm = range(256)
        random.shuffle(perm)
        perm += perm
        dirs = [(math.cos(a * 2.0 * math.pi / 256),
                 math.sin(a * 2.0 * math.pi / 256))
                 for a in range(256)]
        
        z = np.zeros(shape=(sidelen,sidelen))
        for y in range(sidelen):
            for x in range(sidelen):
                z[y,x] = fBm(x*freq, y*freq, int(sidelen*freq), octs)*scale + offset
        grid = np.indices((sidelen,sidelen))
        x = grid[1]
        y = grid[0]
        self.x_int_grid = x
        self.y_int_grid = y
        self.z_grid = z
        
        #build triangle node index grid
        self.idx_grid = np.array(range(sidelen*sidelen),dtype=np.int)
        self.idx_grid = np.reshape(self.idx_grid, (sidelen,sidelen), 1)
        
        self.x_world_grid, self.y_world_grid = np.meshgrid(np.linspace(limits[0],limits[1],sidelen), np.linspace(limits[2],limits[3],sidelen))
        
        #calc linear transformation from world to integer grid
        world_half_x_pixel = (self.x_world_grid[0,1] - self.x_world_grid[0,0])/2
        world_half_y_pixel = (self.y_world_grid[1,0] - self.y_world_grid[0,0])/2
        self.int_world_offset = (self.x_world_grid[0,0]+world_half_x_pixel, self.y_world_grid[0,0]+world_half_y_pixel)
        world_dif_x = self.x_world_grid[0,-1] - self.x_world_grid[0,0]
        int_dif_x = self.x_int_grid[0,-1] - self.x_int_grid[0,0]
        world_dif_y = self.y_world_grid[-1,0] - self.y_world_grid[0,0]
        int_dif_y = self.y_int_grid[-1,0] - self.y_int_grid[0,0]
        self.int_world_scale = (world_dif_x/int_dif_x,world_dif_y/int_dif_y)
        
        self.x_list = self.x_world_grid.flatten()
        self.y_list = self.y_world_grid.flatten()
        self.z_list = z.flatten()
        self.triangles = self.triangulate(self.x_list, self.y_list, sidelen, sidelen)
        #self.mesh = dly.Triangulation(self.x_list, self.y_list)
        #self.mesh_planes = self.mesh.linear_interpolator(self.z_list)
        
        #create low-res grid for visualisation (downsample)
        self.x_low_res = self.rebin(self.x_world_grid, (self.low_res_factor,self.low_res_factor))
        self.y_low_res = self.rebin(self.y_world_grid, (self.low_res_factor,self.low_res_factor))
        self.z_low_res = self.rebin(self.z_grid, (self.low_res_factor,self.low_res_factor))
    
    def genDiamSquareTerrain(self, sidelen, limits, seed, zScale, roughness, offset):
        class Point:
            def __init__(self, X, Y, Z=0.0):
                self.x = X
                self.y = Y
                self.z = Z
        
        def avgDiamPoints(i, j, offset, sidelen, grid):
            '''In this diagram, our input stride is 1, the i,j location is
            indicated by "X", and the four value we want to average are
               "*"s:
                   .   *   .
        
                   *   X   *
        
                   .   *   .
               */
        
            /* In order to support tiled surfaces which meet seamless at the
               edges (that is, they "wrap"), We need to be careful how we
               calculate averages when the i,j diamond center lies on an edge
               of the array. The first four 'if' clauses handle these
               cases. The final 'else' clause handles the general case (in
               which i,j is not on an edge).
            '''
            if (i==0):
                return (grid[i,j-offset]+grid[i,j+offset]+grid[i+offset,j])/3.0
            elif (i==sidelen):
                return (grid[i,j-offset]+grid[i,j+offset]+grid[i-offset,j])/3.0
            elif (j==0):
                return (grid[i,j+offset]+grid[i+offset,j]+grid[i-offset,j])/3.0
            elif (j==sidelen):
                return (grid[i,j-offset]+grid[i+offset,j]+grid[i-offset,j])/3.0
            else:
                return (grid[i,j-offset]+grid[i,j+offset]+grid[i+offset,j]+grid[i-offset,j])/4.0
        
        def avgSqPoints(i, j, offset, sidelen, grid):
            '''In this diagram, our input stride is 1, the i,j location is
               indicated by "*", and the four value we want to average are
               "X"s:
                   X   .   X
        
                   .   *   .
        
                   X   .   X
            '''
            return (grid[i-offset,j-offset]+grid[i-offset,j+offset]+grid[i+offset,j+offset]+grid[i+offset,j-offset])/4.0
        
        def isPowOf2(num):
            return num != 0 and ((num & (num - 1)) == 0)
            
        def buildFractalTerrain(sidelen, limits, seed, zScale, roughness):
            #check if sidelen is an int
            if not isinstance(sidelen,(int)):
                print 'length of side is not an integer!'
                return False
            #check sidelen to be power of two
            if not isPowOf2(sidelen):
                print 'length of side is not a power of 2!'
                return False
            else:
                print 'replacing mesh with a fractal terrain of size', sidelen+1, 'by', sidelen+1
            #create matrix of length (sidelen+1)x(sidelen+1)
            count = sidelen
            fractalGrid = np.zeros(shape=(sidelen+1,sidelen+1))
            #initialise roughness constants, etc.
            ratio = math.pow(2.0,-roughness)
            scale = zScale*ratio
            '''Seed the first four values. For example, in a 4x4 array, we
               would initialize the data points indicated by '*':
        
                   *   .   .   .   *
        
                   .   .   .   .   .
        
                   .   .   .   .   .
        
                   .   .   .   .   .
        
                   *   .   .   .   *
        
               In terms of the "diamond-square" algorithm, this gives us
               "squares".
        
               If the four corners are set to the same value, this will
               cause the grid to tile seamleassly at the edges.'''
            count = count/2
            '''fractalGrid[0,0] = scale*(random.random()-0.5)
            fractalGrid[0,sidelen] = scale*(random.random()-0.5)
            fractalGrid[sidelen,sidelen] = scale*(random.random()-0.5)
            fractalGrid[sidelen,0] = scale*(random.random()-0.5)'''
            fractalGrid[0,0] = scale*(random.random()-0.5) + offset
            fractalGrid[0,sidelen] = fractalGrid[0,0]
            fractalGrid[sidelen,sidelen] = fractalGrid[0,0]
            fractalGrid[sidelen,0] = fractalGrid[0,0]
            #loop through and fill the points using diamond-square algorithm!
            #loop terminated by continous division by 2 of count (since the grid
            #is a power of 2)
            while(count):
                '''Take the existing "square" data and produce "diamond"
                data. On the first pass through with a 4x4 matrix, the
                existing data is shown as "X"s, and we need to generate the
                "*" now:
        
                       X   .   .   .   X
        
                       .   .   .   .   .
        
                       .   .   *   .   .
        
                       .   .   .   .   .
        
                       X   .   .   .   X
        
                It doesn't look like diamonds. What it actually is, for the
                first pass, is the corners of four diamonds meeting at the
                center of the array.'''
                i = count
                while (i<sidelen):
                    j = count
                    while(j<sidelen):
                        fractalGrid[i,j] = scale*(random.random()-0.5) + avgSqPoints(i,j,count,sidelen,fractalGrid)
                        #print i,j,count
                        #print fractalGrid,avgSqPoints(i,j,count,sidelen,fractalGrid)
                        j = j+count+count
                    i = i+count+count
                
                '''for i in range(count,sidelen,count):
                    for j in range(count,sidelen,count):
                        if ((i+j)%2):
                            continue
                        #print i,j,count
                        fractalGrid[i,j] = (scale*(random.random()-0.5)) + avgSqPoints(i,j,count,sidelen,fractalGrid)
                        #print i,j,fractalGrid[i,j]
                        j = j+count
                    i = i+count'''
                
                '''Take the existing "diamond" data and make it into
                "squares". Back to our 4X4 example: The first time we
                encounter this code, the existing values are represented by
                "X"s, and the values we want to generate here are "*"s:
        
                       X   .   *   .   X
        
                       .   .   .   .   .
        
                       *   .   X   .   *
        
                       .   .   .   .   .
        
                       X   .   *   .   X
        
                i and j represent our (x,y) position in the array.'''
                oddline = 0
                k = 0
                while (k<sidelen+count):
                    oddline = (oddline == 0)
                    l = 0
                    while (l<sidelen+count):
                        if ((oddline) and (not l)):
                            l = l+count;
                        fractalGrid[k,l] = scale*(random.random()-0.5) + avgDiamPoints(k,l,count,sidelen,fractalGrid)
                        #print k,l,count
                        #print fractalGrid,avgDiamPoints(k,l,count,sidelen,fractalGrid)
                        l = l+count+count
                    k = k+count
                
                scale = scale*ratio
                count = count/2
            return fractalGrid
            
        z = buildFractalTerrain(sidelen, limits, seed, zScale, roughness)
        grid = np.indices((sidelen+1,sidelen+1))
        x = grid[1]
        y = grid[0]
        self.x_int_grid = x
        self.y_int_grid = y
        self.z_grid = z
        
        #build triangle node index grid
        self.idx_grid = np.array(range(sidelen*sidelen),dtype=np.int)
        self.idx_grid = np.reshape(self.idx_grid, (sidelen,sidelen), 1)
        
        self.x_world_grid, self.y_world_grid = np.meshgrid(np.linspace(limits[0],limits[1],sidelen+1), np.linspace(limits[2],limits[3],sidelen+1))
        
        #calc linear transformation from world to integer grid
        world_half_x_pixel = (self.x_world_grid[0,1] - self.x_world_grid[0,0])/2
        world_half_y_pixel = (self.y_world_grid[1,0] - self.y_world_grid[0,0])/2
        self.int_world_offset = (self.x_world_grid[0,0]+world_half_x_pixel, self.y_world_grid[0,0]+world_half_y_pixel)
        world_dif_x = self.x_world_grid[0,-1] - self.x_world_grid[0,0]
        int_dif_x = self.x_int_grid[0,-1] - self.x_int_grid[0,0]
        world_dif_y = self.y_world_grid[-1,0] - self.y_world_grid[0,0]
        int_dif_y = self.y_int_grid[-1,0] - self.y_int_grid[0,0]
        self.int_world_scale = (world_dif_x/int_dif_x,world_dif_y/int_dif_y)
        
        self.x_list = self.x_world_grid.flatten()
        self.y_list = self.y_world_grid.flatten()
        self.z_list = z.flatten()
        self.triangles = self.triangulate(self.x_list, self.y_list, sidelen, sidelen)
        #self.mesh = dly.Triangulation(self.x_list, self.y_list)
        #self.mesh_planes = self.mesh.linear_interpolator(self.z_list)
    
    def rayTerrainIntersect(self, ray, triIdxsList=None):
        if triIdxsList == None:
            triIdxsList = self.likelyTerrainIntersectIdxs(ray)
        elif(triIdxsList == "ALL"):
            #triIdxsList = range(len(self.mesh.triangle_nodes))
            triIdxsList = range(len(self.triangles))
        isect_t = None
        isect_u = None
        isect_v = None
        isect_idx = None
        tNear = ray.tMax
        for triIdx in triIdxsList:
            #don't check negative index or index greater than max
            if triIdx >= 0 and triIdx < len(self.triangles):
                #triIdxs = self.mesh.triangle_nodes[triIdx]
                triIdxs = self.triangles[triIdx]
                idx0 = triIdxs[0]
                idx1 = triIdxs[1]
                idx2 = triIdxs[2]
                #check that the ray z value can reach the current triangle
                rayZ = ray.calcPosOnVec(ray.tMax)[2]
                if (rayZ < self.z_list[idx0]) or (rayZ < self.z_list[idx1]) or (rayZ < self.z_list[idx2]):
                    vrtx0 = np.array([self.x_list[idx0], self.y_list[idx0], self.z_list[idx0]])
                    vrtx1 = np.array([self.x_list[idx1], self.y_list[idx1], self.z_list[idx1]])
                    vrtx2 = np.array([self.x_list[idx2], self.y_list[idx2], self.z_list[idx2]])
                    u,v,t = self.rayTriangleIntersect(ray, vrtx0, vrtx1, vrtx2)
                    if (t != False and t < tNear):
                        tNear = t
                        isect_t = t
                        isect_u = u
                        isect_v = v
                        ray.triID_nodes = triIdxs
                        ray.triID = triIdx
                        isect_idx = triIdx
        if (tNear < ray.tMax and tNear > ray.tMin):
            return isect_u, isect_v, isect_t
        else:
            return False, False, False
    
    def rayTriangleIntersect(self, ray, v0, v1, v2):
        edge1 = v1-v0
        edge2 = v2-v0
        pvec = np.cross(ray.direction, edge2)
        det = np.dot(edge1, pvec)
        if (det > -self.EPSILON and det < self.EPSILON):
            return False, False, False
        invDet = 1.0/det
        tvec = ray.origin - v0
        u = np.dot(tvec, pvec)*invDet
        if (u < 0 or u > 1):
            return False, False, False
        qvec = np.cross(tvec, edge1)
        v = np.dot(ray.direction, qvec)*invDet
        if (v < 0 or (u+v) > 1):
            return False, False, False
        t = np.dot(edge2, qvec)*invDet
        return u,v,t
    
    def likelyTerrainIntersectIdxs(self, ray):
        x0_world,y0_world,z0_world = ray.origin
        x1_world,y1_world,z1_world = ray.calcPosOnVec(ray.tMax)
        x0_int,y0_int = self.world_2_int_grid(x0_world,y0_world)
        x1_int,y1_int = self.world_2_int_grid(x1_world,y1_world)
        
        #using bresenham's line algo and the ray in x,y plane, determine which
        #triangles are likely to be intersected by the ray, and return this
        #list of triangle indices
        
        #-------------  list index      grid index     triangle index     to convert from grid to triangle index
        #|\  |\  |\  |
        #| \ | \ | \ |   0  1  2      0,0  0,1  0,2     1\0  3\2  5\4 
        #|  \|  \|  \|
        #-------------                                                       triIdx1 = 2*[(row*len(row))+col]
        #|\  |\  |\  |                                                       triIdx2 = triIdx1+1
        #| \ | \ | \ |   3  4  5      1,0  1,1  1,2     7\6  9\8  11\10
        #|  \|  \|  \|
        #-------------
        
        xcoords, ycoords = self.bresenham(round(x0_int),round(y0_int),round(x1_int),round(y1_int))
        xycoords = zip(xcoords, ycoords)
        rowLen = len(self.x_int_grid[0]) - 1 
        triIdxs = []
        for gridIdx in xycoords:
            triIdx1 = 2*((gridIdx[1]*rowLen)+gridIdx[0])
            triIdx2 = triIdx1+1
            triIdxs.append(triIdx1)
            triIdxs.append(triIdx2)
            #add the 8 surrounding 'pixels' to the list of triangles as well (ray may intersect close to edge of 'pixel')
            #'pixel' above
            triIdx_a1 = triIdx1-(2*rowLen)
            triIdx_a2 = triIdx_a1+1
            #'pixel' below
            triIdx_b1 = triIdx1+(2*rowLen)
            triIdx_b2 = triIdx_b1+1
            #'pixel' left
            triIdx_l1 = triIdx1-2
            triIdx_l2 = triIdx1-1
            #'pixel' right
            triIdx_r1 = triIdx1+2
            triIdx_r2 = triIdx1+3
            #'pixel' above left
            triIdx_al1 = triIdx_a1-2
            triIdx_al2 = triIdx_a1-1
            #'pixel' above right
            triIdx_ar1 = triIdx_a1+2
            triIdx_ar2 = triIdx_a1+3
            #'pixel' below left
            triIdx_bl1 = triIdx_b1-2
            triIdx_bl2 = triIdx_b1-1
            #'pixel' below right
            triIdx_br1 = triIdx_b1+2
            triIdx_br2 = triIdx_b1+3
            triIdxs.extend([triIdx_a1,triIdx_a2,triIdx_b1,triIdx_b2,triIdx_l1,triIdx_l2,triIdx_r1,triIdx_r2,triIdx_al1,triIdx_al2,
                            triIdx_ar1,triIdx_ar2,triIdx_bl1,triIdx_bl2,triIdx_br1,triIdx_br2])
        #remove duplicates
        triIdxs = list(set(triIdxs))
        return triIdxs
    
    def bresenham(self, x0, y0, x1, y1):
        x0 = int(x0)
        y0 = int(y0)
        x1 = int(x1)
        y1 = int(y1)
        xcoords = []
        ycoords = []
        steep = abs(y1-y0) > abs(x1-x0)
        if steep:
            x0,y0 = y0,x0
            x1,y1 = y1,x1
        backwards = x0 > x1
        if backwards:
            x0,x1 = x1,x0
            y0,y1 = y1,y0
        dx = x1 - x0
        dy = abs(y1 - y0)
        error = dx / 2.0
        y = y0
        ystep = 1 if (y0 < y1) else -1
        for x in range(x0,x1+1):
            if steep:
                xcoords.append(y)
                ycoords.append(x)
            else:
                xcoords.append(x)
                ycoords.append(y)
            error -= dy
            if error < 0:
                y += ystep
                error += dx
        if backwards:
            xcoords.reverse()
            ycoords.reverse()
        return xcoords, ycoords

    def world_2_int_grid(self, x_world, y_world):
        x_int = (x_world-self.int_world_offset[0])/self.int_world_scale[0]
        y_int = (y_world-self.int_world_offset[1])/self.int_world_scale[1]
        return x_int, y_int
    
    def int_2_world_grid(self, x_int, y_int):
        x_world = (x_int*self.int_world_scale[0]) + self.int_world_offset[0]
        y_world = (y_int*self.int_world_scale[1]) + self.int_world_offset[1]
        return x_world, y_world

#draw a vector
from matplotlib.patches import FancyArrowPatch
from mpl_toolkits.mplot3d import proj3d
class Arrow3D(FancyArrowPatch):
    def __init__(self, xs, ys, zs, *args, **kwargs):
        FancyArrowPatch.__init__(self, (0,0), (0,0), *args, **kwargs)
        self._verts3d = xs, ys, zs
    def draw(self, renderer):
        xs3d, ys3d, zs3d = self._verts3d
        xs, ys, zs = proj3d.proj_transform(xs3d, ys3d, zs3d, renderer.M)
        self.set_positions((xs[0],ys[0]),(xs[1],ys[1]))
        FancyArrowPatch.draw(self, renderer)

'''--------------------------------------------------------------------------'''

test = RayTerrain()
test.genPerlinTerrain(300, [-500,500,-500,500], 1/64.0, 5, 10, -10)
#test.genDiamSquareTerrain(64, [-500,550,-100,200], 1, 100, 1.25, -5)
fig = plt.figure()
ax = fig.add_subplot(111, projection='3d')
#ax.plot_trisurf(test.x_list,test.y_list,test.z_list,alpha=.5)
#ax.plot_trisurf(test.x_low_res.flatten(),test.y_low_res.flatten(),test.z_low_res.flatten(),alpha=.5,cmap=cm.coolwarm)
ax.plot_surface(test.x_world_grid, test.y_world_grid, test.z_grid, linewidth=0, antialiased=False, rstride=2, cstride=2)
#plt.pcolormesh(test.x_grid,test.y_grid,test.z_grid)
print 'mesh done'

print 'rays start'
for i in range(-30,30):
   rayOrig = np.array([0,0,0])
   rayDir = np.array([0,0,-1])
   rayDirUnit = rayDir/np.linalg.norm(rayDir)
   ray = Ray(rayOrig, rayDirUnit)
   ray.rotateDirectionVec('y', (i+0.5)*2)
   ray.rotateDirectionVec('x', 45)
   ray.rotateDirectionVec('z', 30)
   u,v,t = test.rayTerrainIntersect(ray)
   #print 'ray done! -', i
   if t:
       arrow = Arrow3D([ray.origin[0],ray.calcPosOnVec(t)[0]],[ray.origin[1],ray.calcPosOnVec(t)[1]],[ray.origin[2],ray.calcPosOnVec(t)[2]], mutation_scale=20, lw=1, arrowstyle="-")
       #ax.scatter(ray.calcPosOnVec(t)[0],ray.calcPosOnVec(t)[1],ray.calcPosOnVec(t)[2])
   else:
       arrow = Arrow3D([ray.origin[0],ray.calcPosOnVec(ray.tMax)[0]],[ray.origin[1],ray.calcPosOnVec(ray.tMax)[1]],[ray.origin[2],ray.calcPosOnVec(ray.tMax)[2]], mutation_scale=20, lw=1, arrowstyle="-|>")
   if i%4 == 0:
       ax.add_artist(arrow)
print 'rays done'

ax.set_xlim(-500,500)
ax.set_ylim(-500,500)
ax.set_zlim(-20,5)
plt.show()