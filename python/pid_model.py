from trackedData import *
import matplotlib.pyplot as plt
import pid

MIN_I = -0.03
MAX_I = 0.03

SET_POINT = 1
REF_LEVEL = 8192

fillData = []
drainData = []
levelData = []
outData = []

TIME=0
LEVEL=1

p = pid.PID(P=0.2, I=0.00, D=0.0)

def reset():
	fillData = []
	drainData = []
	levelData = []
	outData = []

def getStartAndEndTimeMicros():
	return (TRACK_DATA[0][TIME]*10, TRACK_DATA[-1][TIME]*10)

def genFillAndDrainData():
	prevTime = TRACK_DATA[0][TIME]
	prevLevel = TRACK_DATA[0][LEVEL]
	
	for time, level in TRACK_DATA[1:]:
		if level > prevLevel:
			fillData.append((time*10, level-prevLevel))
		else:
			drainData.append((time*10, prevLevel-level))

		prevLevel = level

def simulate():
	startTimeMicros, endTimeMicros = getStartAndEndTimeMicros()
	level = SET_POINT
	fillDataIdx = 0
	drainDataIdx = 0
	timeStretch = 1.0
	correction = 0
	
	nextDrainTime = (drainData[drainDataIdx][TIME]-startTimeMicros)*timeStretch + startTimeMicros

	cumsum, moving_aves = [0], []
	i=1
	N=10000
	moving_ave = SET_POINT

	for t in range(startTimeMicros, endTimeMicros):
		if len(fillData) > fillDataIdx:
			if t >= fillData[fillDataIdx][TIME]:
				level += fillData[fillDataIdx][LEVEL]
				fillDataIdx += 1

		if len(drainData) > drainDataIdx+1:
			if t >= nextDrainTime:
				level -= drainData[drainDataIdx][LEVEL]
				nextDrainTime = (drainData[drainDataIdx+1][TIME]-drainData[drainDataIdx][TIME])*timeStretch + t
				drainDataIdx += 1

		levelData.append(level)

		cumsum.append(cumsum[i-1] + level)
    	
		if i>=N:
			moving_ave = (cumsum[i] - cumsum[i-N])/N
			#can do stuff with moving_ave here
			moving_aves.append(moving_ave)

		if t%1000 == 0:
			correction = p.update(1.0*(moving_ave-REF_LEVEL)/REF_LEVEL)
			if correction > 0.05:
				correction = 0.05
			if correction < -0.05:
				correction = -0.05
			timeStretch = 1.0+correction

		outData.append(correction)
		t+=1
		i+=1
		print "%d"%((100.0*(t-startTimeMicros))/(endTimeMicros-startTimeMicros))

def plot():
	plt.figure(1)
	plt.plot(outData)
	plt.show(block=False)
	plt.figure(2)
	plt.plot(levelData)
	plt.show()
	
def go():
	reset()
	genFillAndDrainData()
	simulate()
	plot()

if __name__ == "__main__":
    go()