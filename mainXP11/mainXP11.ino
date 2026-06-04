#define ENG_MODE 2 // # of engines 2 or 4

// LCD pin definitions
#define TFT_CLK 15
#define TFT_MOSI 16
#define TFT_MISO 14
#define TFT_CS 18
#define TFT_DC 19
#define TFT_RST   20

#define L_BTN_PIN         4
#define R_BTN_PIN         16
#define L_SWITCH_ON_PIN   6
#define L_SWITCH_OFF_PIN  5
#define R_SWITCH_ON_PIN   7
#define R_SWITCH_OFF_PIN  8
#define L_POT_PIN         A9
#define R_POT_PIN         A10

#define AT_SPEED 1000
#define AT_ACC 5
#define ASSIST_ACC 50
#define TORQUE_MOVE_THRESHOLD 8
#define INITIAL_MOVE_OFFSET 300
#define ABS_TORQUE_COEF 0.2
#define EXP_SCALE 30000
#define TORQUE_RESIDUAL 0.3
#define TORQUE_DISSIPATION 0.08
#define TORQUE_DSG 120

#include <Lever.h>
#include <Servo.h>
#include <Button.h>
#include <Switch.h>
#include <Potentiometer.h>
#include <XPLDirect.h>
#include <Joystick.h>
//#include <SPI.h>
//#include <Adafruit_GFX.h>
//#include <ILI9341_Fast.h>
//#include <SPI.h>
//#include <Ucglib.h>

//ILI9341 TFTscreen = ILI9341(TFT_DC, TFT_RST, TFT_CS);
//Ucglib_ILI9341_18x240x320_SWSPI TFTscreen(/*sclk=*/ TFT_CLK, /*data=*/ TFT_MOSI, /*cd=*/ TFT_DC , /*cs=*/ TFT_CS, /*reset=*/ TFT_RST);

XPLDirect Xinterface(&Serial);

Joystick_ Joystick(JOYSTICK_DEFAULT_REPORT_ID,JOYSTICK_TYPE_GAMEPAD,
  3, 0,                  // Button Count, Hat Switch Count
  false, false, false,    // X, Y, Z
  false, false, false,   // Rx, Ry, Rz
  false, false,          // rudder, throttle
  false, false, false);  // accelerator, brake, steering

Servo L_Servo(1);
Servo R_Servo(2);
Button L_Button(L_BTN_PIN);
Button R_Button(R_BTN_PIN);
Lever L1(L_Servo, L_Button);
Lever L2(R_Servo, R_Button);
Switch AT_Switch(L_SWITCH_ON_PIN, L_SWITCH_OFF_PIN);
Switch SYNC_Switch(R_SWITCH_ON_PIN, R_SWITCH_OFF_PIN);
Potentiometer BRT_Pot(L_POT_PIN, 1023);
Potentiometer SMT_Pot(R_POT_PIN, 1023);

//uint16_t tft_ID;
long int AT;
bool SYNC = false;
float t_vals[ENG_MODE];
long int p_modes[ENG_MODE];
float inst_brt[4];

float res_torque_1 = 0;
int16_t adjusted_speed_1 = 0;

float res_torque_2 = 0;
int16_t adjusted_speed_2 = 0;

void setup() {
  //TFTscreen.begin(UCG_FONT_MODE_TRANSPARENT);
  //TFTscreen.setRotate270(); // 0,1,2,3->0,90,180,270
  //display(0);
  delay(500);
  Serial1.begin(1000000);
  delay(500);

  L_Button.init();
  R_Button.init();
  AT_Switch.init();
  SYNC_Switch.init();
  BRT_Pot.init();
  SMT_Pot.init();
  stsInit();
  L_Servo.init();
  R_Servo.init();

  //Joystick.setXAxisRange(0, 1023);

  delay(500);

  Serial.begin(XPLDIRECT_BAUDRATE);
  runCalibration();

  Xinterface.begin("AT_Device");

  while (!Serial)
  {
    delay(300);
  }

  Xinterface.registerDataRef("sim/cockpit2/autopilot/autothrottle_enabled", XPL_READ, 0, 0, &AT);
  Xinterface.registerDataRef("sim/flightmodel/engine/ENGN_thro", XPL_READWRITE, 0, 0, t_vals);
  Xinterface.registerDataRef("sim/flightmodel/engine/ENGN_propmode", XPL_WRITE, 0, 0, p_modes);
  Xinterface.registerDataRef("sim/cockpit2/switches/panel_brightness_ratio", XPL_WRITE, 0, 0, inst_brt);

  /*while(!Xinterface.allDataRefsRegistered()) {
    delay(10);
  }*/

  Joystick.begin(false);
}

void loop() {
  Xinterface.xloop();
  
  //Xinterface.datarefRead("sim/cockpit2/autopilot/autothrottle_enabled", &AT);
  //Xinterface.datarefRead("sim/cockpit2/autopilot/autothrottle_on", &AT_on);
  //Xinterface.datarefRead("sim/flightmodel/engine/ENGN_thro", t_vals);

  float pred_torque_1;
  uint16_t target_pos_1; // 0-100

  float pred_torque_2;
  uint16_t target_pos_2;

  bool btn_triggered = false;

  L_Button.update();
  //R_Button.update();
  AT_Switch.update();
  SYNC_Switch.update();
  BRT_Pot.update();
  SMT_Pot.update();
  L_Servo.update();
  //R_Servo.update();
  L1.update();
  //L2.update();

  pred_torque_1 = speed2Torque(L_Servo.speed);
  //pred_torque_2 = speed2Torque(R_Servo.speed);

  res_torque_1 = residualTorque(res_torque_1, L_Servo.torque);
  //res_torque_2 = residualTorque(res_torque_2, R_Servo.torque);

  if(AT_Switch.holding) {
    if(!AT_Switch.state) {
      Joystick.setButton(1, 0);
      Joystick.setButton(1, 1);
    } else if(AT_Switch.state){
      Joystick.setButton(1, 0);
      Joystick.setButton(0, 1);
    }
  } else {
    Joystick.setButton(0, 0);
    Joystick.setButton(1, 0);
  }

  // A/T
  if(L_Button.state || R_Button.state || L1.value < 0 || abs(pred_torque_1 - L_Servo.torque) > TORQUE_DSG){//|| L2.value < 0){ || AT && (abs(pred_torque_1 - res_torque_1) > 5 || abs(pred_torque_2 - res_torque_2) > 5)) {
    Joystick.setButton(2, 1);
  } else {
    Joystick.setButton(2, 0);
  }
  Joystick.sendState();
  //Xinterface.datarefWrite("sim/cockpit2/autopilot/autothrottle_enabled", AT);

  // SYNC
  /*if(SYNC_Switch.state) {
    SYNC = true;
  } else if (SYNC_Switch.holding) {
    SYNC = false;
  }*/

  if(SYNC) {
    // Add torque offset and calculate new speed
  } else {
    //adjusted_speed_1 = adjustedSpeed(adjusted_speed_1, L_Servo.torque, pred_torque_1, res_torque_1, 1);//SMT_Pot.scaled_val);
    //adjusted_speed_2 = adjustedSpeed(adjusted_speed_2, R_Servo.torque, pred_torque_2, res_torque_2, SMT_Pot.scaled_val);
  }

  if(AT > 0) {
    L_Servo.enableTorque(1);
    //get target position
    //connector.sendGetValueById(3001);
    target_pos_1 = t_vals[0] * 100;
    //target_pos_2 = t_vals[1] * 100;
    L1.setPosition(target_pos_1, AT_SPEED, AT_ACC);
    //L2.setPosition(target_pos_2, AT_SPEED, AT_ACC);
  } else {
    L_Servo.enableTorque(0);
    //L1.writeSpeed(adjusted_speed_1, ASSIST_ACC);
    //L2.writeSpeed(adjusted_speed_2, ASSIST_ACC);
     if(ENG_MODE == 2) {
      if (L1.value < 0) {
        p_modes[0] = 3;
      } else {
        p_modes[0] = 1;
      }
      /*if (L2.value < 0) {
        p_modes[1] = 3;
      } else {
        p_modes[1] = 1;
      }*/
      t_vals[0] = abs(L1.value/100);
      //t_vals[1] = L2.value/100;
      //Xinterface.datarefWrite("sim/flightmodel/engine/ENGN_thro", t_vals);
      //Xinterface.datarefWrite("sim/flightmodel/engine/ENGN_propmode", p_modes);
    }
  }

  inst_brt[0] = BRT_Pot.scaled_val;

  /*if(btn_triggered) {
    Joystick.setButton(0, 0);
    Joystick.setButton(1, 0);
    Joystick.sendState();
  }*/
  //Xinterface.datarefWrite("sim/cockpit/electrical/instrument_brightness", BRT_Pot.scaled_val);
  
  //display(2);

  // IMPORTANT: LET CALIBRATION BE LAST STEP TO AVOID POSITION BEING ADJUSTED TO ARBITRARY VALUE AFTER CALIBRATION
  // Calibration
  /*if(!AT_Switch.state && AT_Switch.holdTime() > 3000 && !SYNC_Switch.state && SYNC_Switch.holdTime() > 3000) {
    runCalibration();
  }

  Serial.print("P1: ");Serial.print(p_modes[0]);
  Serial.print(" T1: ");Serial.print(t_vals[0]);
  Serial.print(" L1: ");Serial.print(L1.value);
  Serial.print(" S1: ");Serial.println(L_Servo.pos);
  delay(500);*/
}

///WIP
void runCalibration() {
  res_torque_1 = 0;
  //res_torque_2 = 0;
  int zero_pos_1;
  int max_pos_1;
  int rev_pos_1;
  //int zero_pos_2;
  //int max_pos_2;
  //int rev_pos_2;
  L_Servo.enableTorque(0);
  //R_Servo.enableTorque(0);
  // display screen 1
  // detect any input
  //TFTscreen.setPrintPos(10, 10);
  //TFTscreen.print("REV");
  L_Button.update();
  R_Button.update();
  while(!L_Button.state && !R_Button.state) {
    L_Button.update();
    R_Button.update();
    delay(10);
  }
  L_Servo.enableTorque(1);
  //R_Servo.enableTorque(1);
  delay(10);
  L_Servo.calibrationOfs();
  //R_Servo.calibrationOfs();
  delay(10);
  L_Servo.update();
  //R_Servo.update();
  rev_pos_1 = L_Servo.pos;
  //rev_pos_2 = R_Servo.pos;
  Serial.print("Rev: ");
  Serial.println(rev_pos_1);

  L_Servo.enableTorque(0);
  //R_Servo.enableTorque(0);

  //TFTscreen.print("\nZERO");

  while(L_Button.state || R_Button.state) {
    L_Button.update();
    R_Button.update();
    delay(10);
  }
  // display screen 2
  // detect any input
  while(!L_Button.state && !R_Button.state) {
    L_Button.update();
    R_Button.update();
    delay(10);
  }
  L_Servo.update();
  //R_Servo.update();
  zero_pos_1 = L_Servo.pos;
  //zero_pos_2 = R_Servo.pos;
  Serial.print("Zero: ");
  Serial.println(zero_pos_1);

  //TFTscreen.print("\nMAX");

  while(L_Button.state || R_Button.state) {
    L_Button.update();
    delay(10);
  }
  // display screen 3
  // detect any input
  while(!L_Button.state && !R_Button.state) {
    L_Button.update();
    delay(10);
  }
  L_Servo.update();
  //R_Servo.update();
  max_pos_1 = L_Servo.pos;
  //max_pos_2 = R_Servo.pos;
  Serial.print("Max: ");
  Serial.println(max_pos_1);

  // calibration complete
  L1.calibrate(zero_pos_1, max_pos_1, rev_pos_1);
  //L2.calibrate(zero_pos_2, max_pos_2, rev_pos_2);
  //display(1);
}

int16_t adjustedSpeed(int16_t res_speed, int16_t sensor_torque, float pred_torque, float res_torque, float ast_coef) {
  int16_t adjusted_speed;
  if(abs(res_torque) < TORQUE_MOVE_THRESHOLD) {
    adjusted_speed = 0;
  } else {
    adjusted_speed = ast_coef * (res_speed + sensor_torque - pred_torque) * exp(-(max(abs(res_speed) - INITIAL_MOVE_OFFSET,0) + ABS_TORQUE_COEF * abs(sensor_torque))/EXP_SCALE);
  }
  return adjusted_speed;
}

float residualTorque(float res_torque, int16_t sensor_torque) {
  float new_res_torque = (res_torque * TORQUE_RESIDUAL + sensor_torque * (1-TORQUE_RESIDUAL)) * (1-TORQUE_DISSIPATION);
  return new_res_torque;
}

float speed2Torque(int16_t speed) {
  float pred_torque = speed * -0.25;
  return pred_torque;
}

//char text_buffer[32];

/*void display(int graphic_id) {
  switch(graphic_id) {
    case 0:
      TFTscreen.clearScreen();
      TFTscreen.setFont(ucg_font_logisoso30_tn);
      TFTscreen.setFontPosCenter();
      TFTscreen.setColor(255, 255, 255);
      break;
    case 1:
      TFTscreen.clearScreen();
      TFTscreen.setPrintPos(10, 10);
      TFTscreen.print("L1:\nL2:\nA/T:\nBRT:");
      break;
    case 2:
      TFTscreen.setPrintPos(100, 10);
      TFTscreen.setColor(0, 0, 0);
      TFTscreen.print(text_buffer);
      sprintf(text_buffer, "%i%%\n%i%%\n%d\n%i%%", L1.value, L2.value, AT, BRT_Pot.scaled_val);
      TFTscreen.setPrintPos(100, 10);
      TFTscreen.setColor(255, 255, 255);
      TFTscreen.print(text_buffer);
      break;
  }
}*/