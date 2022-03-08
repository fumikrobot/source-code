//Wall drawing robot - FUMIK
//Meet me at fumik.com
//Date: 2022/Mar

#include <AccelStepper.h>
#include <SD.h>                 // for SD card
#define SD_ChipSelectPin 53     // for SD card
File myFile;                    // for SD card


const int dirPin_c = 7;
const int stepPin_c = 4;
const int dirPin_a = 5;
const int stepPin_a = 2;
const int dirPin_p = 6;   //drive for pen
const int stepPin_p = 3;  //drive for pen
const int enPin = 8;
#define motorInterfaceType 1

AccelStepper stepper_a(motorInterfaceType, stepPin_a, dirPin_a);
AccelStepper stepper_c(motorInterfaceType, stepPin_c, dirPin_c);
AccelStepper stepper_p(motorInterfaceType, stepPin_p, dirPin_p);

long steps_a, steps_c; //curent step of a, c
float a, c, a_0, c_0; //unit mm
float a_dev, c_dev; //deviation value of a, c
float pen_x, pen_y; //coordinate of pen, unit mm
float s_a, s_c, s_a_run, s_c_run; //steps for a, c
float pen_x0, pen_y0, pen_x0_next, pen_y0_next, svg_x0, svg_y0;
float x_ratio, y_ratio, x_angle, y_angle;
//int angle3;

float g1, g2, step1, step2, r1, r2;
float stepper_speed;

boolean pen_pos;  //false = pen at open position; true = pen at write position

void setup() {
  Serial.begin(9600);
  pinMode(enPin, OUTPUT);
  stepper_speed = 1000;
  stepper_a.setMaxSpeed(500); // 550 - 1000
  stepper_a.setAcceleration(200000);  //100,0000 - 120,000
  stepper_a.setSpeed(500);
  
  stepper_c.setMaxSpeed(500); //max 550
  stepper_c.setAcceleration(200000);
  stepper_c.setSpeed(500);

  stepper_p.setMaxSpeed(1400); //max 550
  stepper_p.setAcceleration(100000);
  stepper_p.setSpeed(1400);

  g1 = 63.68; //gear ratio
  g2 = g1;
  step1 = 32; //step/rev in double mode 64/2 = 32
  step2 = step1;
  r1 = 9.45; //radius of wheel mm
  r2 = 9.50;
  a = 0;
  c = 0;

  x_ratio = 1.0;  //1.104651
  y_ratio = 1.0;  //0.876712

  x_angle = -1.43; //effect to axis y rotation
  x_angle = radians(x_angle);

  y_angle = -0.36;  //effect to axis x rotation
  y_angle = radians(y_angle);

  pen_pos = false;  //set "true" if it is already at Open position
  pen_open_pos();

  // check if SD card mounted or not
  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");
    return;   // don't do anything more if not
  }
  else{   
    Serial.println("---------------------------->>>>>>>-------SD ok");
  }
  delay(200);

  myFile = SD.open("draw.txt");
  if (myFile) Serial.println("the file is opened OK");
  else Serial.println("error opening draw.txt");
  
}

void loop() {
    digitalWrite(enPin, LOW);  //enable stepper motor

    // zero position of pen must be same as in real model
    pen_x0 = 400;  //840 1000   //my room: 0840-0390-660   /computer room: 390-400
    pen_y0 = 700;  //990; 1200  //my room: 1200-1350-700   /computer room: 900-700
                    //y0 under real -> C-curve up (u)
                    //y0 over real -> C-curve down (n)
    
  
    svg_x0 = 0;
    svg_y0 = 0;
  
    // declare a_0, c_0 at x0,y0
    find_a_c(pen_x0, pen_y0);
    a_0 = a;  //zero position
    c_0 = c;  //zero position
  
    pen_x0_next = pen_x0;
    pen_y0_next = pen_y0;


    // Reading the file
    String draw_command, string_result;
    char char_temp = '0';
    myFile = SD.open("draw.txt");

    // variables for draw
    float x1, x2, x3, x4, y1, y2, y3, y4;

    while(1){
      if (myFile) draw_command = read_until('(');
      else  Serial.println("error opening draw.txt");
  
      if (draw_command == "move_pen_abs("){
        string_result = read_until(',');  //continue reading file unit character ','
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ','
        x1 = string_result.toFloat();
        string_result = read_until(')');  //continue reading file unit character ')'
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ')'
        y1 = string_result.toFloat();
        move_pen_abs(x1,y1);  //move pen absolute to x1,y1
  
        //Serial.println(draw_command);
        //Serial.println(x1);
        //Serial.println(y1);
      }
      
      if (draw_command == "draw_curve("){
        string_result = read_until(',');  //continue reading file unit character ','
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ','
        x1 = string_result.toFloat();
        string_result = read_until(',');  //continue reading file unit character ')'
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ')'
        y1 = string_result.toFloat();
  
        string_result = read_until(',');  //continue reading file unit character ','
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ','
        x2 = string_result.toFloat();
        string_result = read_until(',');  //continue reading file unit character ')'
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ')'
        y2 = string_result.toFloat();
  
        string_result = read_until(',');  //continue reading file unit character ','
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ','
        x3 = string_result.toFloat();
        string_result = read_until(',');  //continue reading file unit character ')'
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ')'
        y3 = string_result.toFloat();
  
        string_result = read_until(',');  //continue reading file unit character ','
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ','
        x4 = string_result.toFloat();
        string_result = read_until(')');  //continue reading file unit character ')'
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ')'
        y4 = string_result.toFloat();
  
        draw_curve(x1,y1,x2,y2,x3,y3,x4,y4);
        /*
        Serial.println(draw_command);
        Serial.println(x1);
        Serial.println(y1);
        Serial.println(x2);
        Serial.println(y2);
        Serial.println(x3);
        Serial.println(y3);
        Serial.println(x4);
        Serial.println(y4);
        */
      }
  
      if (draw_command == "draw_line("){
        string_result = read_until(',');  //continue reading file unit character ','
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ','
        x1 = string_result.toFloat();
        string_result = read_until(',');  //continue reading file unit character ')'
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ')'
        y1 = string_result.toFloat();
  
        string_result = read_until(',');  //continue reading file unit character ','
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ','
        x2 = string_result.toFloat();
        string_result = read_until(')');  //continue reading file unit character ')'
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ')'
        y2 = string_result.toFloat();
  
        draw_line(x1,y1,x2,y2);
        //Serial.println(draw_command);
        //Serial.println(x1);
        //Serial.println(y1);
        //Serial.println(x2);
        //Serial.println(y2);
      }
  
      if (draw_command == "move_pen_rel("){
        string_result = read_until(',');  //continue reading file unit character ','
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ','
        x1 = string_result.toFloat();
        string_result = read_until(')');  //continue reading file unit character ')'
        string_result = string_result.substring(0,string_result.length()-1);  //will delete last character, it is ')'
        y1 = string_result.toFloat();
  
        move_pen_rel(x1,y1);
        //Serial.println(draw_command);
        //Serial.println(x1);
        //Serial.println(y1);
      }
  
      if (draw_command == "endfile(")  break;
  
      
      for (int i=0;i<3;i++){
        myFile.read();  //go through letter ';' and new line -> next reading is next command
      }
    }



  return_home();
  while(1){
    //stop here
    //digitalWrite(enPin, HIGH);  //un-enable stepper motor
  }
}


//======================================================SUB PROGRAM====================================
void move_a_c(float s1, float s2){
  float sa, sb;
  sa = 0;
  sb = 0;

  if (abs(s1)>=abs(s2)){
    for (long i=0;i<=abs(s1);i++){
      //steps_a++;
      if (s1<0) {
        stepper_a.move(1);  //backward - move up
        while(stepper_a.distanceToGo() != 0){stepper_a.run();}
        steps_a--;
      }
      else {
        stepper_a.move(-1); //forward - move down        
        while(stepper_a.distanceToGo() != 0){stepper_a.run();}
        steps_a++;
      }

      sa = sa + abs(s2/s1); //this will make 2 stepper motor run at same time
      if (sa-sb>=1){
        if (s2<0){
          stepper_c.move(-1); //backward - move up          
          while(stepper_c.distanceToGo() != 0){stepper_c.run();}
          steps_c--;
        }
        else {
          stepper_c.move(1);  //forward - move down 
          while(stepper_c.distanceToGo() != 0){stepper_c.run();}
          steps_c++;
        }
        sb++;
        //steps_c++;
      }
    }
  }
  //-----------------------------------------------------------------------
  if (abs(s2)>abs(s1)){
    for (long i=0;i<abs(s2);i++){
      //steps_c++;
      if (s2<0) {
        stepper_c.move(-1); //backward - move up        
        while(stepper_c.distanceToGo() != 0){stepper_c.run();}
        steps_c--;
      }
      else {
        stepper_c.move(1);  //forward - move down         
        while(stepper_c.distanceToGo() != 0){stepper_c.run();}
        steps_c++;
      }

      sa = sa + abs(s1/s2);
      if (sa-sb>=1){
        if (s1<0){
          stepper_a.move(1);  //backward - move up          
          while(stepper_a.distanceToGo() != 0){stepper_a.run();}
          steps_a--;
        }
        else {
          stepper_a.move(-1); //forward - move down             
          while(stepper_a.distanceToGo() != 0){stepper_a.run();}
          steps_a++;
        }
        sb++;
        //steps_a++;
      }
    }
  }
/*
  Serial.print("steps_a = ");
  Serial.println(steps_a);
  Serial.print("steps_c = ");
  Serial.println(steps_c);*/
}


//------------------------------------------------------------------------------------------------
void find_a_c(float x, float y){
  //This code for Arduino MEGA, SD card, used for computer room
  //Setting calibration for computer-room
  
  float w2y, d, e, p, p1, p2, p3, p4, f1, ag1, ag2, ag3, ag4, ag5, tan1, tan2;
  float a0, b0, x0, fx0, fa0;
  w2y = 40;
  e = 45;
  d = 50;
  p = 3190;
  p1 = 54;
  p2 = 54;
  p3 = 52;
  p4 = 56;


    //a0 = 20;
    //b0 = 120;
    if ((x>0)&(x<=900)){
      a0 = 83;
      b0 = 123;
    }
    if ((x>900)&(x<=1400)){
      a0 = 80;
      b0 = 111;
    }
    if ((x>1400)&(x<=1900)){
      a0 = 76;
      b0 = 105;
    }
    if (x>1900){
      a0 = 68;
      b0 = 100;
    }
    
    
    for (int i=0; i<15;i++){
      x0 = (a0 + b0)/2;
      
      ag3 = radians(x0);
      tan1=(x+tan(ag3)*y+e*cos(ag3)+tan(ag3)*e*sin(ag3))/(y+p3*cos(ag3)-tan(ag3)*x+p3*sin(ag3)*tan(ag3));
      tan2=(y-w2y-cos(ag3)*p4+e*sin(ag3)-(p-x-p4*sin(ag3)-e*cos(ag3))/tan(ag3))/(p-x-p4*sin(ag3)+y/tan(ag3)-w2y/tan(ag3)-cos(ag3)*p4/tan(ag3));
      fx0=(p1+p2)*cos(ag3)-sin(ag3)*(p2-d/tan(ag3))/tan1-sin(ag3)*(p1+d/tan(ag3))/tan2;
  
      //Serial.print("fx0 = ");
      //Serial.println(fx0);
    
      ag3 = radians(a0);
      tan1=(x+tan(ag3)*y+e*cos(ag3)+tan(ag3)*e*sin(ag3))/(y+p3*cos(ag3)-tan(ag3)*x+p3*sin(ag3)*tan(ag3));
      tan2=(y-w2y-cos(ag3)*p4+e*sin(ag3)-(p-x-p4*sin(ag3)-e*cos(ag3))/tan(ag3))/(p-x-p4*sin(ag3)+y/tan(ag3)-w2y/tan(ag3)-cos(ag3)*p4/tan(ag3));
      fa0=(p1+p2)*cos(ag3)-sin(ag3)*(p2-d/tan(ag3))/tan1-sin(ag3)*(p1+d/tan(ag3))/tan2;
  
      //Serial.print("fa0 = ");
      //Serial.println(fa0);
    
      if(abs(fx0)<0.005){
        ag3 = radians(x0);
        break;
      }
      else{
        if (fa0*fx0<0){
          a0 = a0;
          b0 = x0;
        }
        else{
          a0 = x0;
          b0 = b0;
        }
      }
  }
  ag3 = radians(x0);
  
  
  ag1 = atan((x+tan(ag3)*y+e*cos(ag3)+tan(ag3)*e*sin(ag3))/(y+p3*cos(ag3)-tan(ag3)*x+p3*sin(ag3)*tan(ag3)));
  ag2 = atan((y-w2y-cos(ag3)*p4+e*sin(ag3)-(p-x-p4*sin(ag3)-e*cos(ag3))/tan(ag3))/(p-x-p4*sin(ag3)+y/tan(ag3)-w2y/tan(ag3)-cos(ag3)*p4/tan(ag3)));
  ag4 = PI - ag3 + ag2;
  ag5 = PI + ag3 - ag1;
  a = abs((x - p3*sin(ag3) + e*cos(ag3))/cos(ag5));  //find out a
  c = abs((p - x - p4*sin(ag3) - e*cos(ag3))/cos(ag4));  //find out c
  //angle3 = ag3; //save last ag3

  
  //Serial.print("a = ");
  //Serial.println(a);
  //Serial.print("c = ");
  //Serial.println(c);
  Serial.print("ag3 = ");
  Serial.println(degrees(ag3));
  
  
}


float line_len(float x1, float y1, float x2, float y2){
    float len, ag1, test1;
    if(x2-x1!=0){
        ag1 = atan((y1-y2)/(x2-x1));
        if(y1-y2!=0){
            len = abs((y1-y2)/sin(ag1));
        }
        else len = abs(x1-x2);
    }
    else{
        len = abs(y1-y2);
    }
    return len;
}

float curve_len(float p0x, float p0y, float p1x, float p1y, float p2x, float p2y, float p3x, float p3y){
    //calculate Bezier curve len
    float bx0, bx1, by0, by1, len;
    float t;
    int i;
    i = 0;
    t = 0;
    len = 0;
    
    bx0 = pow((1-t),3)*p0x + 3*pow((1-t),2)*t*p1x + 3*(1-t)*t*t*p2x + t*t*t*p3x;
    by0 = pow((1-t),3)*p0y + 3*pow((1-t),2)*t*p1y + 3*(1-t)*t*t*p2y + t*t*t*p3y;
        
    do {
        //timing from 0.00 to 1.00
        t = i;
        t = t/100;
        bx1 = pow((1-t),3)*p0x + 3*pow((1-t),2)*t*p1x + 3*(1-t)*t*t*p2x + t*t*t*p3x;
        by1 = pow((1-t),3)*p0y + 3*pow((1-t),2)*t*p1y + 3*(1-t)*t*t*p2y + t*t*t*p3y;
        
        len += line_len(bx0,by0,bx1,by1);
        bx0 = bx1;  //save point 0
        by0 = by1;  //save point 0
        i ++;
    } while (i<=100); 
    return len;
}

void draw_curve(float p0x, float p0y, float p1x, float p1y, float p2x, float p2y, float p3x, float p3y){
  float bx0, bx1, by0, by1;
  float t, len;
  float p0x_temp, p1x_temp, p2x_temp, p3x_temp, p0y_temp, p1y_temp, p2y_temp, p3y_temp;
  int i, i_total;
  i = 0;
  t = 0;

  p0x = p0x*x_ratio;
  p1x = p1x*x_ratio;
  p2x = p2x*x_ratio;
  p3x = p3x*x_ratio;
  
  p0y = p0y*y_ratio;
  p1y = p1y*y_ratio;
  p2y = p2y*y_ratio;
  p3y = p3y*y_ratio;

  p0x_temp = p0x*cos(x_angle) + p0y*sin(x_angle);
  p1x_temp = p1x*cos(x_angle) + p1y*sin(x_angle);
  p2x_temp = p2x*cos(x_angle) + p2y*sin(x_angle);
  p3x_temp = p3x*cos(x_angle) + p3y*sin(x_angle);

  p0y_temp = -p0x*sin(y_angle) + p0y*cos(y_angle);
  p1y_temp = -p1x*sin(y_angle) + p1y*cos(y_angle);
  p2y_temp = -p2x*sin(y_angle) + p2y*cos(y_angle);
  p3y_temp = -p3x*sin(y_angle) + p3y*cos(y_angle);

  p0x = p0x_temp;
  p1x = p1x_temp;
  p3x = p2x_temp;
  p3x = p3x_temp;

  p0y = p0y_temp;
  p1y = p1y_temp;
  p2y = p2y_temp;
  p3y = p3y_temp;

  p0x = pen_x0_next;
  p0y = pen_y0_next;
  p1x = (p1x)*0.0254 + p0x;
  p1y = (p1y)*0.0254 + p0y;
  p2x = (p2x)*0.0254 + p0x;
  p2y = (p2y)*0.0254 + p0y;
  p3x = (p3x)*0.0254 + p0x;
  p3y = (p3y)*0.0254 + p0y;

  len = curve_len(p0x, p0y, p1x, p1y, p2x, p2y, p3x, p3y);
  i_total = len/0.3 + 1;  //total segment to draw for curve, draw resolution 0.1mm
  
  Serial.println("==============draw curve=====================");
  pen_write_pos();
  
  do {
        //timing from 0.00 to 1.00
        t = i;
        t = t/i_total;
        bx1 = pow((1-t),3)*p0x + 3*pow((1-t),2)*t*p1x + 3*(1-t)*t*t*p2x + t*t*t*p3x;
        by1 = pow((1-t),3)*p0y + 3*pow((1-t),2)*t*p1y + 3*(1-t)*t*t*p2y + t*t*t*p3y;

        find_a_c(bx1, by1);
        /*
        Serial.print(i);
        Serial.println("---------");
        Serial.print("x,y is ");
        Serial.print(bx1);
        Serial.print(", ");
        Serial.print(by1);
        Serial.println("---------");
        */

        a_dev = a - a_0;  //distance to run from a_0 to a (current)
        c_dev = c - c_0;  //distance to run from c_0 to c (current)

        s_a = (g1*step1)*a_dev/(r1*PI*2); //step needed to run from a_0 to a (current)
        s_c = (g2*step2)*c_dev/(r2*PI*2); //step needed to run from c_0 to c (current)
    
        s_a_run = s_a - steps_a;
        s_c_run = s_c - steps_c;
        
        move_a_c(s_a_run, s_c_run);
        i ++;
    } while (i<=i_total);

    pen_x0_next = bx1;
    pen_y0_next = by1;
/*
    Serial.print("total steps a is: ");
    Serial.println(steps_a);
    Serial.print("total steps c is: ");
    Serial.println(steps_c);
    */
    Serial.print("home next x0 is: ");
    Serial.println(pen_x0_next);
    Serial.print("home next y0 is: ");
    Serial.println(pen_y0_next);
    
    
}

void draw_line(float p0x, float p0y, float p1x, float p1y){
  float bx, by;
  float t, len;
  float p0x_temp, p1x_temp, p0y_temp, p1y_temp;
  int i, i_total;
  i = 0;
  t = 0;

  p0x = p0x*x_ratio;
  p0y = p0y*y_ratio;
  p1x = p1x*x_ratio;
  p1y = p1y*y_ratio;

  p0x_temp = p0x*cos(x_angle) + p0y*sin(x_angle);
  p1x_temp = p1x*cos(x_angle) + p1y*sin(x_angle);

  p0y_temp = -p0x*sin(y_angle) + p0y*cos(y_angle);
  p1y_temp = -p1x*sin(y_angle) + p1y*cos(y_angle);

  p0x = p0x_temp;
  p1x = p1x_temp;

  p0y = p0y_temp;
  p1y = p1y_temp;

  
  p0x = pen_x0_next;
  p0y = pen_y0_next;
  p1x = (p1x)*0.0254 + p0x;
  p1y = (p1y)*0.0254 + p0y;
  
  len = line_len(p0x, p0y, p1x, p1y);
  i_total = len/0.3 + 1;  //total segment to draw for curve
  
  Serial.println("==============draw line=====================");
  pen_write_pos();
  
  do {
        //timing from 0.00 to 1.00
        t = i;
        t = t/i_total;
        bx = (1-t)*p0x + t*p1x;
        by = (1-t)*p0y + t*p1y;

        find_a_c(bx, by);

        a_dev = a - a_0;  //distance to run from a_0 to a (current)
        c_dev = c - c_0;  //distance to run from c_0 to c (current)

        s_a = (g1*step1)*a_dev/(r1*PI*2); //step needed to run from a_0 to a (current)
        s_c = (g2*step2)*c_dev/(r2*PI*2); //step needed to run from c_0 to c (current)
    
        s_a_run = s_a - steps_a;
        s_c_run = s_c - steps_c;
        
        move_a_c(s_a_run, s_c_run);
        i ++;
    } while (i<=i_total);

    pen_x0_next = bx;
    pen_y0_next = by;

    Serial.print("home next x0 is: ");
    Serial.println(pen_x0_next);
    Serial.print("home next y0 is: ");
    Serial.println(pen_y0_next);
    
}

void move_pen_abs(float x, float y){
  float x_relative, y_relative, x_move, y_move;
  float x_temp, y_temp;

  x = x*x_ratio;
  y = y*y_ratio;

  x_temp = x*cos(x_angle) + y*sin(x_angle);
  y_temp = -x*sin(y_angle) + y*cos(y_angle);

  x = x_temp;
  y = y_temp;
  
  pen_open_pos();

  x_move = x*0.0254 + pen_x0;
  y_move = y*0.0254 + pen_y0;
  
  Serial.println("==============move_pen_abs=====================");
  Serial.print("x_move = ");
  Serial.println(x_move);
  Serial.print("y_move = ");
  Serial.println(y_move);
  
  find_a_c(x_move, y_move);
  a_dev = a - a_0;  //distance to run from a_0 to a (current)
  c_dev = c - c_0;  //distance to run from c_0 to c (current)

  s_a = (g1*step1)*a_dev/(r1*PI*2); //step needed to run from a_0 to a (current)
  s_c = (g2*step2)*c_dev/(r2*PI*2); //step needed to run from c_0 to c (current)

  s_a_run = s_a - steps_a;
  s_c_run = s_c - steps_c;

  Serial.print("a_0 = "); //for debug
  Serial.println(a_0);
  Serial.print("c_0 = "); //for debug
  Serial.println(c_0);
  Serial.print("s_a = "); //for debug
  Serial.println(s_a);
  Serial.print("s_c = "); //for debug
  Serial.println(s_c);
  
  Serial.print("steps_a = "); //for debug
  Serial.println(steps_a);
  Serial.print("steps_c = "); //for debug
  Serial.println(steps_c);
  Serial.print("s_a_run = "); //for debug
  Serial.println(s_a_run);
  Serial.print("s_c_run = "); //for debug
  Serial.println(s_c_run);
  
  move_a_c(s_a_run, s_c_run);

  pen_x0_next = x_move;
  pen_y0_next = y_move;
}


void move_pen_rel(float x, float y){
  float pen_x0_save, pen_y0_save;

  //x = x*x_ratio;
  //y = y*y_ratio;
  
  //pen_open_pos();
  pen_x0_save = pen_x0;
  pen_y0_save = pen_y0;

  pen_x0 = pen_x0_next;
  pen_y0 = pen_y0_next;
  
  move_pen_abs(x,y);

  pen_x0 = pen_x0_save;
  pen_y0 = pen_y0_save;
  
  

}


void return_home(){
  Serial.println("==============return_home=====================");
  pen_open_pos();
  
  find_a_c(pen_x0, pen_y0);
  a_dev = a - a_0;  //distance to run from a_0 to a (current)
  c_dev = c - c_0;  //distance to run from c_0 to c (current)

  s_a = (g1*step1)*a_dev/(r1*PI*2); //step needed to run from a_0 to a (current)
  s_c = (g2*step2)*c_dev/(r2*PI*2); //step needed to run from c_0 to c (current)

  s_a_run = s_a - steps_a;
  s_c_run = s_c - steps_c;
  
  move_a_c(s_a_run, s_c_run);

  Serial.print("total steps a is: ");
  Serial.println(steps_a);
  Serial.print("total steps c is: ");
  Serial.println(steps_c);
  Serial.print("home next x0 is: ");
  Serial.println(pen_x0_next);
  Serial.print("home next y0 is: ");
  Serial.println(pen_y0_next);
    
}

void pen_write_pos(){
  if (pen_pos==false){
    stepper_p.move(-2380);  //origin: -2900
    while(stepper_p.distanceToGo() != 0){stepper_p.run();}
  }
  pen_pos = true;
}

void pen_open_pos(){
  if (pen_pos==true){
    stepper_p.move(2390); //origin: 2800
    while(stepper_p.distanceToGo() != 0){stepper_p.run();}
  }
  pen_pos = false;
}

String read_until (char char_stop){
  String result_reading;
  char char_read_temp = '0';  //choose '0' because it's not '(' or ')' or ';' or ...
  if (myFile){
    while (char_read_temp!=char_stop){
      char_read_temp = (myFile.read());
      result_reading += char_read_temp;
    }
  }
  else Serial.println("error opening the file on SD card");

  return result_reading;
}
