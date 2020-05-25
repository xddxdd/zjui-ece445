
import numpy as np
import matplotlib.pyplot as plt
import matplotlib.image as img
import csv
import sys
from windrose import WindroseAxes

class Map:

    '''
     input: width, in unit of cm, width of map
            height, in unit of cm, height of map
            grid, horizontal and vertical interval between the points
            ploting_scale, scale of real distance and the distance in map（比例尺）
    '''
    def __init__(self, width, height, grid, ploting_scale):
        self.map = np.zeros((int(height/grid), int(width/grid)))
        self.map_width = width
        self.map_height = height
        self.grid = grid
        self.ploting_scale = ploting_scale

    '''
    input: samples, a 3D list or array of sample information with formal of (real_height,real_width,concentration)
    '''
    def addSample(self,samples):
        self.samples = [(i[0]/self.grid,i[1]/self.grid, i[2]) for i in samples]
        for sample in samples:
            sample_height = int(sample[0]/self.grid)
            sample_width = int(sample[1]/self.grid)
            self.map[sample_height,sample_width] = sample[2]

    '''
    distance between point1 and point2
    '''
    def findDistance(self,point1, point2):
        return ((point1[0]-point2[0])**2+(point1[1]-point2[1])**2)**0.5


    '''
    find three nearest_neighbors from sample points for target point
    @important: three neighbors can not be in the same line, and I have not solved this problem
    '''
    def findNearestNeighbors(self, point):
        ret = [(-1,-1),(-1,-1),(-1,-1)]   #x,y
        minimum_distances = [float("inf"),float("inf"),float("inf")]
        for i in self.samples:
            # print(i[:2])
            d = self.findDistance(point,i[:2])
            # print(d)
            if d < np.max(minimum_distances):
                max_index = np.argmax(minimum_distances)
                minimum_distances[max_index] = d
                ret[max_index] = i[:2]
                # print(ret)
        return ret


    '''
    input: target, (y,x)
           neighbors, [sample_point1, sample_point2, sample_point3] with each point in format (y,x)
    output: weights u,v
    algorithm:
            target point can be represented by the sample points as the following equation:
            P = (1-u-v)*P1+u*P2+v*P3 <=> P-P1 = (P2-P1)u + (P3-P1)v
            this eqution should be satisfied in both x and y dimension
    source: https://www.cnblogs.com/wangchengfeng/p/3453194.html
    '''
    def findWeight(self, target, neighbors):
        p1,p2,p3 = neighbors
        # print(p1[0])
        A = np.array([[p2[0]-p1[0], p3[0]-p1[0]],
                      [p2[1]-p1[1], p3[1]-p1[0]]])
        b = np.array([[target[0]-p1[0], target[1]-p1[1]]]).T
        r = np.linalg.solve(A,b)
        # print(r)
        return r


    '''
    input: target, (y,x)
    output: a list of of weights of all sample points to target
    algorithm:
        for sample points, it is [0,0,...1,0,0] where the index of 1 is the index of the sample
        for other points, the weights is 1/d/(1/d0+1/d1+1/d2+....)
    '''
    def findWeights(self,target):
        distances = []
        for index, sample in enumerate(self.samples):
            d = self.findDistance(target,(sample[0],sample[1]))
            if d == 0:
                ret = [0]*len(self.samples)
                ret[index] = 1
                return ret
            else:
                distances.append(1./d)
        sum = np.sum(distances)
        ret = []
        for i in distances:
            ret.append(i/sum)
        return ret




    '''
    map generation algorithm
    P = (1-u-v)*P1+u*P2+v*P3
    @important: P should be inside the triangle whose peaks are P1,P2,P3
    '''
    def fillMap(self):
        grid_height = int(self.map_height/self.grid)
        grid_width = int(self.map_width/self.grid)
        # print(grid_height,grid_width)
        for y in range(grid_height):
            for x in range(grid_width):
                # print(y,x)
                if self.map[y,x] == 0:
                    target_point = (y,x)
                    # p1,p2,p3 = self.findNearestNeighbors(target_point)
                    # weight = self.findWeight(target_point, (p1,p2,p3)).T
                    # # print(weight)
                    # # print(nearest_neighbors)
                    #
                    # self.map[y,x] = (1-weight[0,0]-weight[0,1])*self.map[int(p1[0]),int(p1[1])]\
                    #                 + weight[0,0]*self.map[int(p2[0]),int(p2[1])]\
                    #                 + weight[0,1]*self.map[int(p3[0]),int(p3[1])]

                    weights = self.findWeights(target_point)
                    # print(weights)
                    self.map[y,x]=0
                    for index in range(len(self.samples)):
                        self.map[y,x] += weights[index]*self.samples[index][2]
        # print(self.map[,,:])

    '''
    input: location would be (height,width) and contains the real-world coordinate
    get concentration of partcular point
    '''

    def getConcentration(self,location):
        return self.map[int(location[0]/self.grid), int(location[1]/self.grid)]


    def generateMap(self, path=None,max_value=None, min_value=None):
        x = np.linspace(0,int(self.map_width),int(self.map_width/self.grid))
        y = np.linspace(0,int(self.map_height),int(self.map_height/self.grid))
        X, Y = np.meshgrid(x, y)
        if max_value == None:
            max_value = np.max(self.map)
        if min_value == None:
            min_value = np.min(self.map)
        
        interval = (max_value-min_value)/10
        # print(min_value, max_value, interval)
        # print(np.array((self.map - min_value) / interval, dtype = np.int))

        color=np.arange(min_value,max_value+interval,interval)
        plt.clf()

        fig = plt.figure(frameon=False)
        fig.set_size_inches(len(x) / len(y), 1, forward=False)
        ax = plt.Axes(fig, [0., 0., 1., 1.])
        ax.set_axis_off()
        fig.add_axes(ax)

        # plt.xticks(())
        # plt.yticks(())
        ax.contourf(X, Y, self.map,color,cmap="jet")
        # plt.colorbar()
        # plt.legend()
        if path != None:
            plt.savefig(path, bbox_inches='tight', pad_inches=0, dpi = 1000)
        else:
            plt.show()
        # print(self.map.shape)
        plt.close()

        # print(x.shape)

    def writeToCsv(self,path):
        with open(path,'w')as f:
            f_csv = csv.writer(f)
            # f_csv.writerow(headers)
            # f_csv.writerows(rows)
            grid_height = int(self.map_height/self.grid)
            grid_width = int(self.map_width/self.grid)
            for y in range(grid_height):
                for x in range(grid_width):
                    row = [x,y,self.map[y][x]]
                    f_csv.writerow(row)

    '''
    This function would apply diffusion to the whole map
    input: t, diffusion time
           nu, diffusion coefficient

    source code from: https://blog.csdn.net/qq_42000453/article/details/83097516
    '''

    def applyDiffusion(self, t, nu):
        for n in range(t):
	           old_map = self.map.copy()
	           self.map[1:-1,1:-1] = (old_map[1:-1,1:-1] +
		             nu * (old_map[2: ,1:-1] - 2 * old_map[1:-1,1:-1] + old_map[0:-2,1:-1]) +
		             nu * (old_map[1:-1,2:] - 2 * old_map[1:-1,1:-1] + old_map[1:-1, 0:-2]) )
	           # self.map[0, :] = 1
	           # self.map[-1, :] = 1
	           # self.map[:, 0] = 1
	           # self.map[:, -1] = 1
        # print(self.map[70,80])


    '''
    This function applies wind speed and direction to the whole map
    The concentration of the gas would translate in the direction of the wind direction with the distance of vt
    input: v: wind speed in unit of m/s
           d: direction. in unit of angle, [0,360), 0 means it is in the northern direction
           t: time in unit of s
    '''
    def applyWind(self,v,d,t):
        distance = v*t/self.ploting_scale/self.grid
        # print(distance)
        dy = int(round(distance*np.cos(d*np.pi/180)))
        dx = int(round(distance*np.sin(d*np.pi/180)))
        # print("dy,dx:",dy,dx)
        old_map = self.map.copy()
        grid_height = int(self.map_height/self.grid)
        grid_width = int(self.map_width/self.grid)

        if abs(dy) >= grid_height or abs(dx) >= grid_width:
            print('shifting is too large and we cannot predict it, you may decrease shifting time')
        if dy == 0 and dx == 0:
            print('shifting is too small and we  can not make prediction')


        if dy>=0 and dx>=0:     # northeastern
            dy = abs(dy)
            dx = abs(dx)
            # print('dy,dx2:',dy,dx)
            for y in range(grid_height):
                for x in range(grid_width):
                    if y>=dy and x>=dx:
                        self.map[y,x] = old_map[y-dy,x-dx]
                    elif y>=dy and x<dx:
                        self.map[y,x] = old_map[y-dy,0]
                    elif y<dy and x>=dx:
                        self.map[y,x] = old_map[0,x-dx]
                    else:
                        self.map[y,x] = old_map[0,0]

        if dy<0 and dx>= 0:     #southeastern
            dy = abs(dy)
            dx = abs(dx)
            # print('dy,dx2:',dy,dx)
            for y in range(grid_height):
                for x in range(grid_width):
                    if y<grid_height-dy and x>=dx:
                        self.map[y,x] = old_map[y+dy,x-dx]
                    elif y>=grid_height-dy and x>=dx:
                        self.map[y,x] = old_map[grid_height-1,x-dx]
                    elif y<grid_height-dy and x<dx:
                        self.map[y,x] = old_map[y+dy,0]
                    else:
                        self.map[y,x] = old_map[grid_height-1,0]

        if dy<0 and dx< 0:     #southwestern
            dy = abs(dy)
            dx = abs(dx)
            # print('dy,dx2:',dy,dx)
            for y in range(grid_height):
                for x in range(grid_width):
                    if y<grid_height-dy and x<grid_width-dx:
                        self.map[y,x] = old_map[y+dy,x+dx]
                    elif y>=grid_height-dy and x<grid_width-dx:
                        self.map[y,x] = old_map[grid_height-1,x+dx]
                    elif y<grid_height-dy and x>=grid_width-dx:
                        self.map[y,x] = old_map[y+dy,grid_width-1]
                    else:
                        self.map[y,x] = old_map[grid_height-1,grid_width-1]

        if dy>=0 and dx< 0:     #northwestern
            dy = abs(dy)
            dx = abs(dx)
            # print('dy,dx2:',dy,dx)
            for y in range(grid_height):
                for x in range(grid_width):
                    if y>=dy and x<grid_width-dx:
                        self.map[y,x] = old_map[y-dy,x+dx]
                    elif y>=dy and x>=grid_width-dx:
                        self.map[y,x] = old_map[y-dy,grid_width-1]
                    elif y<dy and x<grid_width-dx:
                        self.map[y,x] = old_map[0,x+dx]
                    else:
                        self.map[y,x] = old_map[0,grid_width-1]

    '''
    in gps mode, gps_mode = True, topleftPosition and bottomrightPosition
                should be input to get the range of map
                the file user input should be in format:longitude， latitude and value for each row
    not in gps mode, gps_mode = false
                the file user input should be in format: x dimension ,y dimension, value; 
                (0,0) for bottomleft point
    '''

    def loadSampleFile(self, path, gps_mode=False, topleftPosition=None, bottomrightPosition=None):  
        x,y,value = [],[],[]
        with open(path,"r",encoding='UTF-8-sig') as csvfile:
            reader = csv.reader(csvfile)
            for line in reader:
                x.append(float(line[0]))
                y.append(float(line[1]))
                value.append(float(line[2]))
        
        return self.preprocessSample(x, y, value, gps_mode, topleftPosition, bottomrightPosition)

    def preprocessSample(self, x, y, value, gps_mode=False, topleftPosition=None, bottomrightPosition=None):  
        if not gps_mode:
            ret = [(x[i],y[i],value[i]) for i in range(len(x))]
        else:
            if topleftPosition==None or bottomrightPosition==None:
                print("no topleft position or bottomright position")
                sys.exit(0)

            ret = [((x[i] - topleftPosition[0])/(bottomrightPosition[0]-topleftPosition[0])*self.map_width,\
                    (y[i] - bottomrightPosition[1])/(topleftPosition[1]-bottomrightPosition[1])*self.map_height,\
                    value[i]) for i in range(len(x))
                    if x[i] >= topleftPosition[0] and x[i] < bottomrightPosition[0]
                    and y[i] >= bottomrightPosition[1] and y[i] < topleftPosition[1]]
        return ret

    def generateWindroseMap(self, ws, wd, path=None):
        ax = WindroseAxes.from_ax()
        ax.bar(wd, ws, normed=True, opening=0.8, edgecolor='none')
        ax.set_legend()
 
        # plt.show()
        if path != None:
            plt.savefig(path, bbox_inches='tight', pad_inches=0)
