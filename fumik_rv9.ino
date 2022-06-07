//Wall drawing robot - FUMIK
//Meet me at fumik.com - mail baank83@gmail.com
//Author: Nguyen Huu Phuc - 2022/May. All rights reserved.
//This code for Arduino MEGA, SD card


#include <AccelStepper.h>
#include <SD.h>                 // for SD card
#define SD_ChipSelectPin 53     // for SD card
File myFile;                    // for SD card
#include <Servo.h>              // for servo pen              
Servo pen_servo;               // for servo pen

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
float draw_res; //draw resolution
float w2y, p;

float g1, g2, step1, step2, r1, r2;
float stepper_speed, basic_speed;

boolean pen_pos;  //false = pen at open position; true = pen at write position

void setup() {
  Serial.begin(9600);
  pen_servo.attach (11); //servo motor is connected to pin 11 Arduino Mega ~ (Z+ End Stop) of Arduino cnc Shield
  pinMode(enPin, OUTPUT);
  stepper_speed = 250; //unit steps/sec
  basic_speed = 250;   //unit steps/sec; note: high speed + low resolution (draw_res) will make vibration
  stepper_a.setMaxSpeed(2000);
  stepper_a.setAcceleration(200000);  //set to max
  stepper_a.setSpeed(stepper_speed);
  
  stepper_c.setMaxSpeed(2000);
  stepper_c.setAcceleration(200000);  //set to max
  stepper_c.setSpeed(stepper_speed);

  stepper_p.setMaxSpeed(1400); //max 550
  stepper_p.setAcceleration(100000);
  stepper_p.setSpeed(1400);

  g1 = 63.68; //gear ratio
  g2 = g1;
  step1 = 32; //step/rev (full-step mode: 32; half-step mode: 64)
  step2 = step1;
  r1 = 9.5; //radius of wheel mm
  r2 = 9.5;
  a = 0;
  c = 0;

  p = 3190;   //mm, distance pin_a to pin_c (horizontal direction) 1600
  w2y = 40;  //mm, distance pin_a to pin_c (vertical direction)  4
 
  // zero position of pen must be same as in real model
  pen_x0 = 400;  //mm, computer room: 390-400-310
  pen_y0 = 700;  //mm, computer room: 900-700-410
                 //y0 under real -> C-curve up (u)
                 //y0 over real -> C-curve down (n)

  x_ratio = 1.0;  //adjust to change ratio of picture in x_axis
  y_ratio = 1.0;  //adjust to change ratio of picture in y_axis

  draw_res = 0.7; //mm, draw resolution is 0.7mm (high number -> high draw speed + low resolution)

  x_angle = -1.43; //degree, effect to rotation of picture in x_axis
  x_angle = radians(x_angle);

  y_angle = -0.36;  //degree, effect to rotation of picture in y_axis
  y_angle = radians(y_angle);

  pen_servo.write(50);  //set pen at open position, to make sure pen at open position
  pen_pos = false;      //now pen at open position, init pen_pos = "false"

  // check if SD card mounted or not
  if (!SD.begin(SD_ChipSelectPin)) {  // see if the card is present and can be initialized:
    Serial.println("SD fail");
    return;   // don't do anything more if not
  }
  else{   
    Serial.println("---------------------------->>>>>>>-------SD card ok");
  }
  delay(200);

  myFile = SD.open("draw.txt");
  if (myFile) Serial.println("the file is opened OK");
  else Serial.println("error opening draw.txt");
  
}

void loop() {
    digitalWrite(enPin, LOW);  //enable stepper motor
    
    pen_write_pos();
    delay(1000);
    pen_open_pos();
    delay(1000);
          
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
    delay(5000);
    //digitalWrite(enPin, HIGH);  //un-enable stepper motor
  }
}


//======================================================SUB PROGRAM====================================
void move_a_c(float s1, float s2){
  int stepper_a_speed, stepper_c_speed; //unit steps/sec

  // Set the current position to 0:
  stepper_a.setCurrentPosition(0);
  stepper_c.setCurrentPosition(0);
  
  if(abs(s1)>=abs(s2)){
    if(s1<0) {stepper_a_speed = basic_speed;} //speed pos: motor a makes robot move up
    else stepper_a_speed = -basic_speed;      //speed neg: motor a makes robot move down

    if(s2<0) {stepper_c_speed = -abs(s2/s1)*basic_speed;}  //speed pos: motor c makes robot move down
    else stepper_c_speed = abs(s2/s1)*basic_speed;         //speed neg: motor c makes robot move up
  }
  else{
    if(s2<0) {stepper_c_speed = -basic_speed;}  //speed pos: motor c makes robot move down
    else stepper_c_speed = basic_speed;         //speed neg: motor c makes robot move up

    if(s1<0) {stepper_a_speed = abs(s1/s2)*basic_speed;}    //speed pos: motor a makes robot move up
    else stepper_a_speed = -abs(s1/s2)*basic_speed;         //speed neg: motor a makes robot move down
  }


  if (abs(stepper_a_speed) < 5){
    if (stepper_a_speed < 0) stepper_a_speed = -5;
    else stepper_a_speed = 5;
  }
  if (abs(stepper_c_speed) < 5){
    if (stepper_c_speed < 0) stepper_c_speed = -5;
    else stepper_c_speed = 5;
  }
  /*

  Serial.print("stepper_a_speed = ");
  Serial.println(stepper_a_speed);
  Serial.print("stepper_c_speed = ");
  Serial.println(stepper_c_speed);
  Serial.print("s1 = ");
  Serial.println(s1);
  Serial.print("s2 = ");
  Serial.println(s2);
  */
  

  // Run the motor
  while((abs(stepper_a.currentPosition()) < abs((long)s1))||(abs(stepper_c.currentPosition()) < abs((long)s2)))
  {
    stepper_a.setSpeed(stepper_a_speed); //speed pos: motor a makes robot move up; 
    if((abs(stepper_a.currentPosition()) < abs((long)s1))) {stepper_a.runSpeed();}
    
    stepper_c.setSpeed(stepper_c_speed); //speed pos: motor c makes robot move down
    if((abs(stepper_c.currentPosition()) < abs((long)s2))) {stepper_c.runSpeed();}
  }
  steps_a += -stepper_a.currentPosition();
  steps_c += stepper_c.currentPosition();

  
  Serial.print("steps_a = ");
  Serial.println(steps_a);
  Serial.print("steps_c = ");
  Serial.println(steps_c);
  
  
  
}
//------------------------------------------------------------------------------------------------
void find_a_c(float x, float y){
  float d, e, p1, p2, p3, p4, f1, ag1, ag2, ag3, ag4, ag5, tan1, tan2;
  float a0, b0, x0, fx0, fa0;
  //w2y = 450;  //distance motor_a to motor_c (vertical) -> not effect to C-curve problem; origin 40
  e = 45;   //pen distance mm -> not effect to C-curve problem
  d = 50;   //gravity distance mm -> not effect to C-curve problem
  //p = 2170; //distance motor_a to motor_c (horizontal) 2690 2730 my room/ 3190 computer room
            //below real distance -> make more C-curve ( down?)
            //over real distance -> C-curve is up
  p1 = 54;  //distance motor_a to gravity base
  p2 = 54;  //distance motor_c to gravity base
  p3 = 52;  //pen distance motor_a to pen base
  p4 = 56;  //pen distance motor_c to pen base


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
  ag4 = PI/2 - ag3 + ag2;
  ag5 = PI/2 + ag3 - ag1;
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
  i_total = len/draw_res + 1;  //total segment to draw for curve, draw resolution
  
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
  i_total = len/draw_res + 1;  //total segment to draw for line, draw resolution
  
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
  float basic_speed_for_save;

  basic_speed_for_save = basic_speed;
  basic_speed = 500; //set high speed when moving without drawing

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

  basic_speed = basic_speed_for_save;; //return to basic speed after high speed
}


void move_pen_rel(float x, float y){
  float pen_x0_save, pen_y0_save;

  //x = x*x_ratio;
  //y = y*y_ratio;
  
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
  int servo_position; //this variable for pen driver by servo motor SG90
  if (pen_pos==false){
    // this program for pen driver by servo motor SG90
    Serial.println("-------------------------------------------------- set pen at write position");
    for (servo_position = 50; servo_position >0; servo_position--){
      pen_servo.write(servo_position);
      delay(20);
    }
  }
  pen_pos = true; //now, pen at write position
}

void pen_open_pos(){
  int servo_position; //this variable for pen driver by servo motor SG90
  if (pen_pos==true){
    // this program for pen driver by servo motor SG90
    Serial.println("-------------------------------------------------- set pen at open position");
    for (servo_position = 0; servo_position <50; servo_position++){
      pen_servo.write(servo_position);
      delay(20);
    }
  }
  pen_pos = false;  //now, pen at open position
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
