#include "math.h"


float r0, r1, r2, u0, u1, u2, v;
int i=0;
int my_time;
int Ptotal , PrmsInt= 0;
float Arms, AMS, Prms, Ptot = 0;
float CAL = 1.05;

char resultstr[64];


void setup() {
    Particle.variable("Prms",PrmsInt);
    Particle.variable("Ptot",Ptotal);
    Particle.variable("result", resultstr); 
    my_time = Time.now();
}

void loop() {
  // send the value of analog input 0:
  r2 = r1;
  r1 = r0;
  r0 = analogRead(A0);
  u2 = u1;
  u1 = u0;
  // 0.5Hz to 200Hz Band Pass Filter
  u0 = 0.2929*(r0-r2) + 1.411*u1 -0.4142*u2;
  v = u0;

  // Calculate Mean-Square Current (Amps)
  AMS = 0.99*AMS +0.01*v*v;
  // Calculate Root-Mean-Square (Amps)
  Arms = sqrt(AMS);
  // Convert to RMS Power:
  // Multipy by 230V (rms)
  // 30*3.3/4096 accounts for the gain of the Current Transformer and ADC
  Prms = 230*30*Arms*3.3/4096*CAL;
  delay(1);
 
 // Gather data for 5 seconds 
  if (Time.now() - my_time > 5) {
    if (i == 719) {  // Report at 60 min intervals
      // This in in kWh
      Ptotal = (int) (Ptot/720);
      sprintf(resultstr, "{\"data1\":%d}", Ptotal); 
      i = 0;
      Ptot = 0;
    }
    else {
      i++;
      Ptot+=Prms;
    }
    my_time = Time.now();
    PrmsInt = (int)Prms;
  }
}
