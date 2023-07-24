#include <Adafruit_LiquidCrystal.h>

// LCD interface initialization
Adafruit_LiquidCrystal lcd(0);

// Arduino's pins numbers for RGB
const int RED_RGB_PIN = 13;
const int BLUE_RGB_PIN = 12;
const int GREEN_RGB_PIN = 11;

// Arduino's pin number for buzzer
const int BUZZER = 6;

// Arduino's pin for bulb light
const int LAMP_PIN = 16;

// Arduino's pins for ultrasonic sensor, one pin for both trigger and echo
const int ULTRASONIC_PIN = 5;

// Arduino's pin for temperature sensor
const int TEMPERATURE_PIN = A0;

// Arduino's pin for light sensor
const int LIGHT_SENSOR_PIN = A3;

// Arduino's pin for soil moisture senseor
const int SOIL_MOISTURE_PIN = A1;

// Arduino's pins for control buttons
const int BTN_TOGGLE_PIN = 7;
const int UP_BTN_PIN = 10;
const int DOWN_BTN_PIN = 8;
const int MENU_BTN_PIN = 1;

// Arduino's pins for DC motor
const int DC_INPUT1_PIN = 2;
const int DC_INPUT2_PIN = 4;
const int DC_PWM = 3; // to write analag signal
const int DC_PUMP_PIN = 9; // To open/close water

// Store current sensors values
int current_distance = 0;
int current_temperature = 0;
int current_light_intensity = 0;
int current_soil_moisture = 0;

// keep track of milliseconds passed since the Arduino board began running the current program
unsigned long prev_milliseconds = 0;
const int TIME_2_SECONDS = 2000;
const int TIME_500_MILLIS = 500;
const int TIME_1_SECOND = 1000;
const int TIME_3_SECOND = 3000;

// Menu
int menu_option = 0;
int prev_option = 0; // store prev option
bool background_process = false; // Display all operation in background such watering
const int HOME_OPTION = 0;
const int TEMPERATURE_OPTION = 1;
const int LIGHT_OPTION = 2;
const int DISTANCE_OPTION = 3;
const int SOIL_OPTION = 4;
const int BACKGROUND_OPTION = 5;
int btn_toggle = 1;  // move between internal options
bool no_buzzer = false;
// Store buttons state
bool menu_btn_state = false;
bool toggle_btn_state = false;
bool up_btn_state = false;
bool down_btn_state = false;
const int BTN_DELAY_TIME = 3; // delay after button is pressed
const int BTN_STAY_TIME = 10; // stay in loop if up/down is pressed
// set default values and store entered values if any
int max_temperature = 35; //celcuis
int min_temperature = 20;
int distance_gap = 50; // default 60 cm
int max_light_intensity = 50; // default 50
int min_soil_moinstrure = 50; // default 50
bool is_error = false; // used to turn off/on rgb
unsigned long WATERING_INTERVAL_3S = 3000; // used to water plants every ns interval
unsigned long prev_watering_time = 0; // used to water plants after certain time 

/*========== Functions =============*/

/*
Print a string to lcd screen
@param String msg_row_1 display in row 1
@param String msg_row_2 display in row 2
@param int delay_val delay time before clearing screen
@param bool no_clear if true clear screen before and after msg
*/
void print_message(String msg_row_1, String msg_row_2, int delay_val = 100, bool no_clear = false){
    if(!no_clear){
        lcd.clear();
    }
    lcd.setCursor(0,0);
    lcd.print(msg_row_1);
    lcd.setCursor(0,1);
    lcd.print(msg_row_2);
    delay(delay_val);
    if(!no_clear){
        lcd.clear();
    }
}

/*
Print a int to lcd screen
@param int msg the value to print
@param int col
@param int row
@param int num_of_cells to clear at specified position before printing the message.
@param String msg_after print messgae after printig first message, default empty
*/
void print_message(int msg, int col, int row, int num_of_cells = 0, String msg_after = ""){
    for (int i = 0; i < num_of_cells; ++i)
    {
        lcd.setCursor(col + i,row);
        lcd.write(' ');
    }
    lcd.setCursor(col,row);
    lcd.print(msg);
    lcd.print(msg_after);
}


/*
Read ultra sonic distance's sensor value
@return int distance in cm
*/
int read_distance(int triggerPin, int echoPin)
{
    pinMode(triggerPin, OUTPUT);  // Clear the trigger
    digitalWrite(triggerPin, LOW);
    delayMicroseconds(2);
    // Sets the trigger pin to HIGH state for 10 microseconds
    digitalWrite(triggerPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(triggerPin, LOW);
    pinMode(echoPin, INPUT);
    // sound travel time is 0.0344 cm/microsocnd, for forward and backward divide with 2
    // Reads the echo pin, and returns the sound wave travel time in microseconds
    return 0.01723 * pulseIn(echoPin, HIGH);
}

/*
Read temperature's sensor value
@return int temperature in celcuis
*/
int read_temperature(int temperature_sensor_pin){
    // Get the voltage reading from the TMP36
    float reading = analogRead(temperature_sensor_pin);
    // Convert that reading into voltage
    float voltage = reading * (5.0 / 1024.0);
    // Convert the voltage into the temperature in Celsius and return it
    return round((voltage - 0.5) * 100.0);
}

/*
Read light intensity sensor
@return int light intensity in percent
*/
int read_light_intensity(){
    // Get the voltage reading from the light sensor
    float reading = analogRead(LIGHT_SENSOR_PIN);
    //The highest reading the sensor gave is 900, divdie the reading with 900 and multiply with 100 to get value present
    return (reading/900)*100.0;
}

/*
Read soil moisture sensor
@return int soil moisture in percent
*/
int read_soil_moisture(){
    // Get the voltage reading from the soil sensor
    float reading = analogRead(SOIL_MOISTURE_PIN);
    //The highest reading the sensor gave is 876, divdie the reading with 876 and multiply with 100 to get value present
    return (reading/876)*100.0;
}

/*Print current sensor's values, prints the following:
DI: Distance in cm
TE: Temperature in celcuis
LI: light intensity
SM: soil moisture
*/
void print_sensors_values(){
    lcd.setCursor(0,0);
    lcd.print("DI:");
    lcd.print(current_distance);
    lcd.print("CM,");

    lcd.print("TE:");
    lcd.print(current_temperature);
    lcd.print("  ");

    lcd.setCursor(0,1);
    lcd.print("LT:");
    lcd.print(current_light_intensity);
    lcd.print(",");
    lcd.print("SM:");
    lcd.print(current_soil_moisture);
    lcd.print("  ");
}

/* Set RGB colors
@param int red
@param int green
@param int blue
 */
void set_rgb_color(int red_value,int green_value,int blue_value)
{
    analogWrite(RED_RGB_PIN, red_value);
    analogWrite(GREEN_RGB_PIN, green_value);
    analogWrite(BLUE_RGB_PIN, blue_value);
}

/* Set min and max temperature.
@param interval to increase/decrease with interval value
 */
void set_temperature_interval(int interval){

    unsigned long time_begin = millis();;
    unsigned long time_end = 0;
    do{

    // Switch between min and max value based on btn state
    if(digitalRead(BTN_TOGGLE_PIN) == HIGH || toggle_btn_state){
        ++btn_toggle;
        btn_toggle = ((btn_toggle % 2) == 1)? 1:2;
    }

    String row_1_msg = "Min temp:";
    String row_2_msg = "Max temp:";
    int column = row_2_msg.length();

    // print stored values
    print_message(row_1_msg + String(min_temperature),row_2_msg + String(max_temperature),0,true);

    // Set min temparature
    if(btn_toggle == 1){
        lcd.setCursor(12,1);
        lcd.write(' ');
        lcd.setCursor(12,0);
        lcd.write('<');
        lcd.setCursor(column,0);
        // increase value
        while(digitalRead(UP_BTN_PIN) == HIGH){
            min_temperature += interval; 
            if(min_temperature > 140){ // value limit
                min_temperature = 140;
            }
            print_message(min_temperature, column,0,3);
            delay(BTN_DELAY_TIME);
            time_end = millis();
            time_begin += BTN_STAY_TIME;
        }
        // decrease value
        while(digitalRead(DOWN_BTN_PIN) == HIGH){
            min_temperature -= interval; 
            if(min_temperature < 0){ // value limit
                min_temperature = 0;
            }
            print_message(min_temperature,column ,0,3);
            delay(BTN_DELAY_TIME);
            time_end = millis();
            time_begin += BTN_STAY_TIME;
        }
    }

    // Set max temparature
    if(btn_toggle == 2){
        lcd.setCursor(12,0);
        lcd.write(' ');
        lcd.setCursor(12,1);
        lcd.write('<');
        lcd.setCursor(column,1);
        // increase value
        while(digitalRead(UP_BTN_PIN) == HIGH){
            max_temperature += interval; 
            if(max_temperature > 140){ // value limit
                max_temperature = 140;
            }
            print_message(max_temperature, column,1,3);
            delay(BTN_DELAY_TIME);
            time_end = millis();
            time_begin += BTN_STAY_TIME;
        }
        // decrease value
        while(digitalRead(DOWN_BTN_PIN) == HIGH){
            max_temperature -= interval; 
            if(max_temperature < 0){ // value limit
                max_temperature = 0;
            }
            print_message(max_temperature,column ,1,3);
            delay(BTN_DELAY_TIME);
            time_end = millis();
            time_begin += BTN_STAY_TIME;
        }
    }
    }while(time_end - time_begin < BTN_STAY_TIME);
}

/* Set light intensity
@param interval to increase/decrease with interval value
 */
void set_light_intensity(int interval){
    String row_1_msg = "LIGHT INTENSITY";
    String row_2_msg = "Percent:";
    int column = row_2_msg.length();
    print_message(row_1_msg ,row_2_msg + String(max_light_intensity),0,true); // Print stored value
    // increase value
    lcd.cursor();
    unsigned long time_begin = millis();;
    unsigned long time_end = 0;
    do{
        while(digitalRead(UP_BTN_PIN) == HIGH){
            max_light_intensity += interval; 
            if(max_light_intensity > 100){ // value limit
                max_light_intensity = 100;
            }
            print_message(max_light_intensity, column,1,3);
            delay(BTN_DELAY_TIME);
            time_end = millis();
            time_begin += BTN_STAY_TIME;
        }
        // decrease value
        while(digitalRead(DOWN_BTN_PIN) == HIGH){
            max_light_intensity -= interval;
            if(max_light_intensity < 0){ // value limit
                max_light_intensity = 0;
            }
            print_message(max_light_intensity,column ,1,3);
            delay(BTN_DELAY_TIME);
            time_end = millis();
            time_begin += BTN_STAY_TIME;
        }
    }while(time_end - time_begin < BTN_STAY_TIME);

    lcd.noCursor();
}

/* Set the gap between the surface/object and the lamp's arm.
@param interval to increase/decrease with interval value
 */
void set_distance_gap(int interval){
    String row_1_msg = "GAP FROM OBJECT";
    String row_2_msg = "Gap IN CM:";
    int column = row_2_msg.length();
    print_message(row_1_msg ,row_2_msg + String(distance_gap),0,true); // print stored values
    lcd.cursor();
    unsigned long time_begin = millis();;
    unsigned long time_end = 0;
    do{
        // increase value when btn is high
        while(digitalRead(UP_BTN_PIN) == HIGH){
            distance_gap += interval;
            if(distance_gap > 100){ // value limit
                distance_gap = 100;
            } 
            print_message(distance_gap, column,1,3);
            delay(BTN_DELAY_TIME);
            time_end = millis();
            time_begin += BTN_STAY_TIME;
        }
        // decrease value when btn is high
        while(digitalRead(DOWN_BTN_PIN) == HIGH){
            distance_gap -= interval; 
            if(distance_gap < 0){ // value limit
                distance_gap = 0;
            }
            print_message(distance_gap,column ,1,3);
            delay(BTN_DELAY_TIME);
            time_end = millis();
            time_begin += BTN_STAY_TIME;
        }
    }while(time_end - time_begin < BTN_STAY_TIME);

    lcd.noCursor();
}

/* set soil moisture level
@param interval to increase/decrease with interval value
 */
void set_soil_moisture_level(int interval){
    String row_1_msg = "SOIL MOISTURE %";
    String row_2_msg = "Percent:";
    int column = row_2_msg.length();
    print_message(row_1_msg ,row_2_msg + String(min_soil_moinstrure),0,true); // print stored values
    lcd.cursor();
    unsigned long time_begin = millis();;
    unsigned long time_end = 0;
    do{
        // increase value when btn is high
        while(digitalRead(UP_BTN_PIN) == HIGH){
            min_soil_moinstrure += interval; 
            if(min_soil_moinstrure > 100){ // value limit
                min_soil_moinstrure = 100;
            }
            print_message(min_soil_moinstrure, column,1,3);
            delay(BTN_DELAY_TIME);
            time_end = millis();
            time_begin += BTN_STAY_TIME;
        }
        // decrease value when btn is high
        while(digitalRead(DOWN_BTN_PIN) == HIGH){
            min_soil_moinstrure -= interval; 
            if(min_soil_moinstrure < 0){ // value limit
                min_soil_moinstrure = 0;
            }
            print_message(min_soil_moinstrure,column ,1,3);
            delay(BTN_DELAY_TIME);
            time_end = millis();
            time_begin += BTN_STAY_TIME;
        }

    }while(time_end - time_begin < BTN_STAY_TIME);
    lcd.noCursor();
}

/*
Start DC motor to water plants every n sconds if current soil moisture less than stored value.
*/
void water_plants(){
    if(current_soil_moisture < min_soil_moinstrure && current_soil_moisture != 0){
        digitalWrite(DC_PUMP_PIN,HIGH);
        set_rgb_color(0,0,255); // blue color
        if(background_process){
            print_message("DRY SOIL! DC ON","WATERING....",200); 
            print_message("WATERING DONE!","DC OFF...",200); 
        }
        digitalWrite(DC_PUMP_PIN,LOW);
        delay(100);
    }
}

/*
if the distance between the object and lamp arm is smaller/bigger than defined gap 
start DC motor to increase/decrease the distance between the object and lamp arm
@param int error_tolerance how much in cm +- more or less than current distance 
*/
void keep_gap(int error_tolerance){
   /* if(current_distance  < distance_gap - error_tolerance){
        set_rgb_color(255,0,0); // red color for extreme warning
        print_message("WARNIGN!!","PLANTS NEAR ARM");
    }*/
    if(current_distance  < distance_gap - error_tolerance){
        digitalWrite(DC_INPUT1_PIN,HIGH);
        digitalWrite(DC_INPUT2_PIN,LOW);
        analogWrite(DC_PWM,255);
        if(background_process)
            print_message("LIFTING UP DIST","DC ON....",300); 
    }else if(current_distance  > distance_gap + error_tolerance){
        digitalWrite(DC_INPUT1_PIN,LOW);
        digitalWrite(DC_INPUT2_PIN,HIGH);
        analogWrite(DC_PWM,255);
        if(background_process)
            print_message("SINKING DOWN DIST","DC ON....",300); 
    }else{ // turn off dc motor
        digitalWrite(DC_INPUT1_PIN,LOW);
        digitalWrite(DC_INPUT2_PIN,LOW);
        analogWrite(DC_PWM,0); 
    }
    delay(100);
}

/*
Check light intensity if smaller than max light intensity turn on lamp, otherwise turn off lamp
*/
void check_light(){
    // Check light intensity if smaller than defined turn on lamp 
    if(max_light_intensity < current_light_intensity){
        digitalWrite(LAMP_PIN, HIGH);
    }else{ // Turn off lamp
        digitalWrite(LAMP_PIN,LOW);
    }
}

/*
Send 1KHz sound signal to buzzer sensor.
If global variable no_bazzer is true then no signal will be sent
*/
void run_buzzer(){
    if(!no_buzzer){
        tone(BUZZER, 1000); // Send 1KHz sound signal
    }else{
        noTone(BUZZER); // Stop sound
    }
}


/* turn on rgb and alert with/without buzzer when something is wrong */
void alert(){
    if(current_temperature > max_temperature){
        set_rgb_color(255,0,0); // red color for extreme warning
        run_buzzer();
        print_message("WARNIGN!!","High TEMPERATURE");
    }else if(current_soil_moisture == 0){
        set_rgb_color(255,0,0); // red color for warning
        print_message("WARNIGN!!","DRY SOIL..");
    }else{
        noTone(BUZZER); // Stop sound
        // set color to green to indicate no problem
        set_rgb_color(0,255,0);
    }
}

/*
Turn off/on buzzer on long press on toggle btn
if press time between 1s turn off buzzer
if press time more than 1s turn on buzzer
@param unsigned long current_time
*/
void reset_buzzer(unsigned long current_time){
    if(menu_option == TEMPERATURE_OPTION){
        return;
    }
    while(digitalRead(BTN_TOGGLE_PIN) == HIGH){
        unsigned long btn_press_time = millis() - current_time; // store press time
        // if press between 1s and 3s turn off buzzer
        if(btn_press_time > 1000 && !no_buzzer){
            no_buzzer = true;
            print_message("BUZZER OFF...","");
            noTone(BUZZER);
            return;
        }
        // if press more than 3s turn on buzzer
        if(btn_press_time > 1000){
            no_buzzer = false;
            // Send 1KHz sound signal to inform that buzzer is on
            tone(BUZZER, 1000);
            print_message("BUZZER ON...","");
            noTone(BUZZER);
            return;
        }
    }
}

/*Print menu options*/
void menu(){
    // on menu click move to next menu option
    if(menu_btn_state){
        ++menu_option;
    }
    // clear lcd when moving to another menu option
    if(menu_option != prev_option){
        lcd.clear();
        prev_option = menu_option;
    }
    int interval = 5;
    background_process = false;
    switch (menu_option) {
        case HOME_OPTION:
            print_sensors_values();
            break;
        case TEMPERATURE_OPTION:
            set_temperature_interval(interval);
            break;
        case LIGHT_OPTION:
            set_light_intensity(interval);
            break;
        case DISTANCE_OPTION:
            set_distance_gap(interval);
            break;
        case SOIL_OPTION:
            set_soil_moisture_level(interval);
            break;
        case BACKGROUND_OPTION:
            background_process = true;
            print_message("BACKGROUANDS","OPERATIONS ");
            break;
        default:
            menu_option = -1; // rest menu options
            break;
    }
}

void setup() {
    // set up the LCD's number of columns and rows:
    lcd.begin(16, 2);

    // Set RGB pins mode
    pinMode(RED_RGB_PIN, OUTPUT);
    pinMode(BLUE_RGB_PIN, OUTPUT);
    pinMode(GREEN_RGB_PIN, OUTPUT);

    // Set buzzer pin mode
    pinMode(BUZZER, OUTPUT);

    // Set light sensor pin mode
    pinMode(LIGHT_SENSOR_PIN,INPUT);
    pinMode(LAMP_PIN,OUTPUT);

    // Set DC motor pins mode
    pinMode(DC_INPUT1_PIN,OUTPUT);
    pinMode(DC_INPUT2_PIN,OUTPUT);
    pinMode(DC_PWM,OUTPUT);
    pinMode(DC_PUMP_PIN,OUTPUT);
    
    // Set buttons pin mode
    pinMode(MENU_BTN_PIN,INPUT);
    pinMode(BTN_TOGGLE_PIN,INPUT);
    pinMode(UP_BTN_PIN,INPUT);
    pinMode(DOWN_BTN_PIN,INPUT);
}

void loop() {

    // Read and assign current sensor's values
    current_distance = read_distance(ULTRASONIC_PIN,ULTRASONIC_PIN);
    current_temperature = read_temperature(TEMPERATURE_PIN);
    current_light_intensity = read_light_intensity();
    current_soil_moisture = read_soil_moisture();
    do{
        // read button's state
        menu_btn_state = digitalRead(MENU_BTN_PIN);
        toggle_btn_state = digitalRead(BTN_TOGGLE_PIN);
        delay(BTN_DELAY_TIME);
        // Print menu options
        menu();
    }while(digitalRead(MENU_BTN_PIN));

    unsigned long current_milliseconds = millis(); // track time

    if(!menu_btn_state){
        // Start watering every Ns
        if(current_milliseconds - prev_milliseconds > TIME_2_SECONDS){
            water_plants();
            prev_milliseconds = current_milliseconds;
        }
        // set alert if there are any errors
        alert();
        // keep fixed gap with 10 cm tolerance error 
        keep_gap(10); 
    }
    // turn on/off lamp
    check_light(); 
    //turn on/off buzzer
    reset_buzzer(current_milliseconds);
    // Reset button's state
    menu_btn_state = false;
    toggle_btn_state = false;
}
