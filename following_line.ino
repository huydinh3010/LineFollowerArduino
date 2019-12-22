#define IR_1 14
#define IR_2 12
#define IR_3 13
#define IR_4 5
#define IR_5 4
#define ML1 15
#define ML2 16
#define MR1 2
#define MR2 0


// cac thong so se hieu chinh
const int Kp = 200; 
const int Ki = 10;
const int Kd = 80;
int lbase_speed = 800;
int rbase_speed = 850;
//

int error = 0;
int lastError = 0;
int out = 0;
int P;
int I = 0;
int D;

int computePID(){                                                    
		// P
		P = error;
    out = P*Kp;
    I = I + error;
    D = error - lastError;
    out = Kp*P + Kd*D + Ki*I;                      
    if(error == 0) I = 0;                                  
    return out;                                        
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);     // Initialize the LED_BUILTIN pin as an output
  pinMode(IR_1, INPUT);
  pinMode(IR_2, INPUT);
  pinMode(IR_3, INPUT);
  pinMode(IR_4, INPUT);
  pinMode(IR_5, INPUT);
  pinMode(ML1, OUTPUT);
  pinMode(ML2, OUTPUT);
  pinMode(MR1, OUTPUT);
  pinMode(MR2, OUTPUT);
  analogWrite(ML2, 0);
  analogWrite(MR2, 0);
  analogWrite(ML1, lbase_speed);
  analogWrite(MR1, rbase_speed);    
}

void readSensor(){
  int sensor[5];
  sensor[0] = digitalRead(IR_1);
  sensor[1] = digitalRead(IR_2);
  sensor[2] = digitalRead(IR_3);
  sensor[3] = digitalRead(IR_4);
  sensor[4] = digitalRead(IR_5);
  if((sensor[0]==1)&&(sensor[1]==1)&&(sensor[2]==1)&&(sensor[3]==1)&&(sensor[4]==0))
  error=4;
  else if((sensor[0]==1)&&(sensor[1]==1)&&(sensor[2]==1)&&(sensor[3]==0)&&(sensor[4]==0))
  error=3;
  else if((sensor[0]==1)&&(sensor[1]==1)&&(sensor[2]==1)&&(sensor[3]==0)&&(sensor[4]==1))
  error=1;
  else if((sensor[0]==1)&&(sensor[1]==1)&&(sensor[2]==0)&&(sensor[3]==0)&&(sensor[4]==1))
  error=1;
  else if((sensor[0]==1)&&(sensor[1]==1)&&(sensor[2]==0)&&(sensor[3]==1)&&(sensor[4]==1))
  error=0;
  else if((sensor[0]==1)&&(sensor[1]==0)&&(sensor[2]==0)&&(sensor[3]==1)&&(sensor[4]==1))
  error=-1;
  else if((sensor[0]==1)&&(sensor[1]==0)&&(sensor[2]==1)&&(sensor[3]==1)&&(sensor[4]==1))
  error=-1;
  else if((sensor[0]==0)&&(sensor[1]==0)&&(sensor[2]==1)&&(sensor[3]==1)&&(sensor[4]==1))
  error=-3;
  else if((sensor[0]==0)&&(sensor[1]==1)&&(sensor[2]==1)&&(sensor[3]==1)&&(sensor[4]==1))
  error=-4;
}

void controlMotor(int inp){
  int right_speed = rbase_speed - inp;
  int left_speed = lbase_speed + inp;
  right_speed = constrain(right_speed, 0, 1023);
  left_speed = constrain(left_speed, 0, 1023);
  analogWrite(ML1, left_speed);
  analogWrite(MR1, right_speed);
}

void loop() {
    readSensor();
    if(lastError != error) controlMotor(computePID());
    lastError = error;         
}
