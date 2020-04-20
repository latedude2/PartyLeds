import ddf.minim.*;
import ddf.minim.signals.*; 
import ddf.minim.analysis.*; 
import ddf.minim.effects.*;
import ddf.minim.ugens.*;
import processing.serial.*;
 
Serial serialPort;
 
Minim minim;
AudioInput in;
LowPassFS lowPass;

// This app uses 15 % of my CPU (i5-9300H) lol 
// Make sure to use a sound system capable of playing sounds below 60 Hz
void setup() {
  size(512, 260);
  
  minim = new Minim(this);
  in = minim.getLineIn(Minim.MONO, 2048);
  
  lowPass = new LowPassFS(200, in.sampleRate());      //Set cutoff frequency here
  
  in.addEffect(lowPass);
  
  printArray(Serial.list());
  
  // **** Note: Choose the element from the list that corresponds with your arduino ****
  serialPort = new Serial(this, Serial.list()[0], 9600);
}

void draw() {
  //background(0);
  //stroke(255);
  //strokeWeight(2);
  
  float p = 0;
  float high = 0;
  for(int i = 0; i < in.bufferSize() - 1; i++) {
    p += in.left.get(i);
    if (in.left.get(i) > high) {
      high = in.left.get(i);
    }
    
    /*
    stroke(255 - i, 255 / 2 + i, 255 - i / 2);
    line(i, 50 - in.left.get(i)*50, i+1, 50 - in.left.get(i+1)*50);
    line(i, 150 - in.right.get(i)*50, i+1, 150 - in.right.get(i+1)*50);
    */
  }
  
  //noStroke();
  
  float mapped = map(min(p, 30), 0, 30, 0, 1);
  /*
  fill(255 * mapped, 255 * mapped, 255 * mapped);
  ellipse(width/2,height - 50,50,50);
  */
  
  int a = round(mapped * 100);
  int h = round(high * 100);
  serialPort.write(h);
  //println("p: " + p + " high: " + high + " a: " + a);
}
 
 
void stop()
{
  // always close Minim audio classes when you are done with them
  minim.stop();
  super.stop();
}
