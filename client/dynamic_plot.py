import numpy as np
import matplotlib.pyplot as plt
import matplotlib.animation as animation

fig = plt.figure();
ax1 = fig.add_subplot(221)
#axref1 = ax1.twinx()
ax2 = fig.add_subplot(222)
#axref2 = ax2.twinx()
ax3 = fig.add_subplot(223)
#axref3 = ax3.twinx()
ax4 = fig.add_subplot(224)
#axref4 = ax4.twinx()

def animate(i) :
	linesX = [line.strip() for line in open('xcoord.txt')]
	linesTemp = [line.strip() for line in open('temps.txt')]
	linesY = [line.strip() for line in open('ycoord.txt')]
	linesZ = [line.strip() for line in open('zcoord.txt')]
	linesPsy = [line.strip() for line in open('psy.txt')]
	linesRef = [line.strip() for line in open('ref.txt')]
	xcoord = []
	ycoord = []
	zcoord =[]
	psy = []
	ref = []
	time =[]
	for i, x in enumerate(linesX):
		xcoord.append(float(x))
 
	for i, x in enumerate(linesTemp):
		time.append(float(x))
 
	for i, x in enumerate(linesY):
		ycoord.append(float(x))

	for i, x in enumerate(linesZ):
		zcoord.append(float(x))

	for i, x in enumerate(linesPsy):
		psy.append(float(x))

	for i, x in enumerate(linesRef):
		ref.append(float(x))

	min_length = min(min(min(min(len(time),len(xcoord)),len(ycoord)),len(zcoord)),len(psy))
	ax1.clear()
	ax1.plot(time[:min_length],xcoord[:min_length],'r-',time[:min_length],ref[:min_length],'m-')
	ax2.clear()
	ax2.plot(time[:min_length],ycoord[:min_length],'b-',time[:min_length],ref[:min_length],'m-')
	ax3.clear()
	ax3.plot(time[:min_length],zcoord[:min_length],'g-',time[:min_length],ref[:min_length],'m-')
	ax4.clear()
	ax4.plot(time[:min_length],psy[:min_length],'o-',time[:min_length],ref[:min_length],'m-')
	#axref1.clear()
	#axref2.clear()
	#axref3.clear()
	#axref4.clear()
	#axref1.plot(time,ref,'m-')
	#axref2.plot(time,ref,'m-')
	#axref3.plot(time,ref,'m-')
	#axref4.plot(time,ref,'m-')

ani = animation.FuncAnimation(fig,animate,interval = 1000)
 
plt.show()