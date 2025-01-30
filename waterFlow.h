#ifndef WATERFLOW_H_INCLUDED
#define WATERFLOW_H_INCLUDED

void calculateWater(volatile int &flowFrequency, float &volume, unsigned long &currentTime, unsigned long &cloopTime){
    if(currentTime >= (cloopTime + 1000)){
        cloopTime = currentTime;
        volume = (flowFrequency / (7.5 * 3600));
        flowFrequency = 0;
    }
}
void resetVolume(float &volume){
  if(volume >= 20.0){
    volume = 0;
  }
}

#endif // WATERFLOW_H_INCLUDED
