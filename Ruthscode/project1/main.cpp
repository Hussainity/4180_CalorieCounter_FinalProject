#include "mbed.h"
#include "uLCD_4DGL.h" 
#include "ultrasonic.h"
#include "rtos.h"
#include "PinDetect.h"
#include "SDFileSystem.h"
#include "wave_player.h"
#include "PowerControl/PowerControl.h"
#include "PowerControl/EthernetPowerControl.h"
#include <time.h>
#include <string>
#include <sstream>

uLCD_4DGL uLCD(p28, p27, p29); // serial tx, serial rx, reset pin;

SDFileSystem sd(p5, p6, p7, p8, "sd"); //SD card
AnalogOut DACout(p18);
wave_player waver(&DACout);

DigitalOut led1(LED1);
Serial pc(USBTX, USBRX);
PinDetect pb(p25);
Mutex howMany;
//Ticker checkTime;
//HistoryCache timeConverter;

int year, month, day, hr, mn, sec; 

int howManyTodayold = 0;
int howManyToday = 0;
int dailyLimit;
int increment = 20; // DEFAULT: USE DATABASE TO UPDATE THE AMOUNT
std::string foodName = "";  // so save the food name
std::string macro = ""; // to save user's desired macro
std::string daily = ""; // to save daily amount
std:: string stringTime  = ""; // to save current time and date
std:: string todayIs = "";


void updateTime(){
    time_t curTime = time(NULL);
    FILE *fp = fopen("/sd/Logging.csv", "a");
    std:: string test = asctime(localtime(&curTime));
    if (todayIs.compare(test.substr(0,10)) != 0){
        howManyToday = 0;
        todayIs = test.substr(0,10);
        fprintf(fp, "\n%s\n\n", todayIs.c_str());
    }
    fprintf(fp, "%s, %d g, %s\n", foodName.c_str(), increment, macro.c_str());  
    fclose(fp);
}

void play_song() {
    std::string song = "/sd/NoNoNo3.wav";
    FILE *wave_file;
    wave_file=fopen(song.c_str(),"r");
    waver.play(wave_file);
    fclose(wave_file);
}

time_t asUnixTime(int year, int mon, int mday, int hour, int min, int sec) {
    struct tm   t;
    t.tm_year = year - 1900;
    t.tm_mon =  mon - 1;        // convert to 0 based month
    t.tm_mday = mday;
    t.tm_hour = hour;
    t.tm_min = min;
    t.tm_sec = sec;
    t.tm_isdst = -1;            // Is Daylight saving time on? 1 = yes, 0 = no, -1 = unknown
 
    return mktime(&t);          // returns seconds elapsed since January 1, 1970 (begin of the Epoch)
}

void dist(int distance)
{
    //put code here to execute when the distance has changed
    led1  = !led1;
    howMany.lock();
    if (howManyToday != howManyTodayold) {
        updateTime();
        if (howManyToday >= dailyLimit) {
            uLCD.cls();
            uLCD.text_width(2);
            uLCD.text_height(2);
            uLCD.filled_rectangle(0,0,127,127,RED);
            uLCD.filled_rectangle(15,10,110,90,0x50C878);
            uLCD.textbackground_color(0x50C878);
            uLCD.color(BLACK);
            uLCD.locate(2,1);
            uLCD.printf("Hello!");
            uLCD.locate(3,3);
            uLCD.text_width(1);
            uLCD.text_height(2);
            uLCD.printf("%d / %d\n", howManyToday, dailyLimit);
            uLCD.color(WHITE);
            uLCD.text_width(1);
            uLCD.text_height(2);
            uLCD.textbackground_color(RED);
            uLCD.locate(6,14);
            uLCD.printf("%s", macro);

            // Play warning song
            play_song();
            
            } else {
                uLCD.cls();
                uLCD.text_width(2);
                uLCD.text_height(2);
                uLCD.filled_rectangle(0,0,127,127,0x86c5da);
                uLCD.filled_rectangle(15,10,110,90,0x50C878);
                uLCD.textbackground_color(0x50C878);
                uLCD.color(BLACK);
                uLCD.locate(2,1);
                uLCD.printf("Hello!");
                uLCD.locate(3,3);
                uLCD.text_width(1);
                uLCD.text_height(2);
                uLCD.printf("%d / %d\n", howManyToday, dailyLimit);
                uLCD.color(WHITE);
                uLCD.text_width(1);
                uLCD.text_height(1);
                uLCD.locate(6,14);
                uLCD.textbackground_color(0x86c5da);
                uLCD.printf("%s", macro);
      
            }
                howManyTodayold = howManyToday;
            }
            howMany.unlock();
    //pc.printf("Distance %d mm\r\n", distance);
}


ultrasonic mu(p12, p13, .1, 1, &dist);    //Set the trigger pin to D12 and the echo pin to D13
                                        //have updates every .1 seconds and a timeout after 1
                                        //second, and call dist when the distance changes


void updateHowManyToday(void) {
    if (mu.getCurrentDistance() < 50) {
        howMany.lock();
        howManyTodayold = howManyToday;
        howManyToday = howManyToday + increment;
        howMany.unlock();
    }
    

}

int main()
{
    
    bool startUp = 1, name = 1, limit = 1, settingTime = 1;
    uLCD.cls();
    uLCD.filled_rectangle(0,0,127,127,0x86c5da);
    uLCD.line(0, 9, 127, 9, GREEN);
    uLCD.filled_rectangle(0,0,127,9,GREEN);
    uLCD.line(0, 118, 127, 118, GREEN);
    uLCD.filled_rectangle(0,118,127,127,GREEN);
    uLCD.textbackground_color(0x86c5da);
    uLCD.color(DGREY);
    uLCD.text_height(1.80);
    
    uLCD.locate(4,2);
    uLCD.printf("WELCOME!");
    uLCD.color(BLACK);
    uLCD.locate(1,4);
    uLCD.printf("Please enter the  name");
    uLCD.printf(" of the food.");
    uLCD.color(WHITE);
    uLCD.locate(1,7);
    
    while (name) {
        if (pc.readable()) {
            char v = pc.getc();
            if (v == '!'){
                name = 0;
            } else if (int(v) == 8) {
                 foodName.erase(foodName.end()-1);
                } else {
                foodName.push_back(v);
                uLCD.printf("%c" , v);
                }
            }
        }
        
        
        uLCD.cls();
        uLCD.filled_rectangle(0,0,127,127,0x86c5da);
        uLCD.line(0, 9, 127, 9, GREEN);
        uLCD.filled_rectangle(0,0,127,9,GREEN);
        uLCD.line(0, 118, 127, 118, GREEN);
        uLCD.filled_rectangle(0,118,127,127,GREEN);
        uLCD.textbackground_color(0x86c5da);
        uLCD.color(BLACK);
        uLCD.text_height(1);
            
        uLCD.locate(1,1);
        uLCD.printf("What macro are you monitoring? : \n\n1. Carbs\n2. Protein\n3. Fat \n4. Calories \n");
        uLCD.color(WHITE);
        while(startUp) {
            if (pc.readable() ){
                char v = pc.getc();
                if (v == '1') {
                    startUp = 0;
                    macro = "Carbs";
                } else if (v == '2') {
                    startUp = 0;
                    macro = "Protein";
                } else if (v == '3') {
                    startUp = 0;
                    macro = "Fat";
                } else if (v == '4') {
                    startUp = 0;
                    macro = "Calories";
                    
                    }
            }
        }
        
        uLCD.cls();
        uLCD.filled_rectangle(0,0,127,127,0x86c5da);
        uLCD.line(0, 9, 127, 9, GREEN);
        uLCD.filled_rectangle(0,0,127,9,GREEN);
        uLCD.line(0, 118, 127, 118, GREEN);
        uLCD.filled_rectangle(0,118,127,127,GREEN);
        uLCD.textbackground_color(0x86c5da);
        uLCD.color(BLACK);
        uLCD.locate(1,2);
        uLCD.text_height(1);
            
        uLCD.printf("What will be your daily limit?\n ");
        uLCD.color(WHITE);
        
        while (limit) {
            if (pc.readable()){
                char v = pc.getc();
                if (v == '!') {
                    limit = 0;
                } else {
                    daily.push_back(v);
                    uLCD.printf("%c" , v);
                }
            }
            
        }
        
        std::istringstream iss (daily);
        iss >> dailyLimit;
        
        uLCD.cls();
        uLCD.filled_rectangle(0,0,127,127,0x86c5da);
        uLCD.line(0, 9, 127, 9, GREEN);
        uLCD.filled_rectangle(0,0,127,9,GREEN);
        uLCD.line(0, 118, 127, 118, GREEN);
        uLCD.filled_rectangle(0,118,127,127,GREEN);
        uLCD.textbackground_color(0x86c5da);
        uLCD.color(BLACK);
        uLCD.text_height(1);
        uLCD.locate(1,1);
        uLCD.printf("Please enter date and military time\nMM/DD/YYYY HH:MM:SS)\n");
        uLCD.color(WHITE);
        
                
        while (settingTime) {
            if (pc.readable()){
                char v = pc.getc();
                if (v == '!') {
                    settingTime = 0;
                } else {
                    stringTime.push_back(v);
                    uLCD.printf("%c" , v);
                }
            }
            
        }
        
        
            // CALL DATABASE ENTRY HERE using "foodName" and "macro" TO UPDATE "increment (int)"
            

            
// TIME
    // converts epoch time to day, month, year
    std::istringstream mon (stringTime.substr(0,2));
    mon >> month; 
    std::istringstream d (stringTime.substr(3,2));
    d >> day; 
    std::istringstream y (stringTime.substr(6,4));
    y >> year; 
    std::istringstream h (stringTime.substr(11,2));
    h >> hr; 
    std::istringstream m (stringTime.substr(14,2));
    m >> mn; 
    std::istringstream sc (stringTime.substr(17,2));
    sc >> sec; 
               
    time_t result = asUnixTime(year, month, day, hr, mn, sec);
    
    pc.printf("%s\n",asctime(localtime(&result)));
    
    set_time(result);
    
    todayIs = asctime(localtime(&result));
    todayIs = todayIs.substr(0,10);
    
    uLCD.cls();
    uLCD.filled_rectangle(0,0,127,127,0x86c5da);
    uLCD.filled_rectangle(15,10,110,90,0x50C878);
    uLCD.textbackground_color(0x50C878);
    uLCD.color(BLACK);
    uLCD.text_width(2);
    uLCD.text_height(2);
    uLCD.locate(2,1);
    uLCD.printf("Hello!");
    uLCD.locate(3,3);
    uLCD.text_width(1);
    uLCD.text_height(2);
    uLCD.printf("%d / %d", howManyToday, dailyLimit);
    uLCD.color(WHITE);
    uLCD.textbackground_color(0x86c5da);
    uLCD.text_width(1);
    uLCD.text_height(1);
    uLCD.locate(6,14);
    uLCD.printf("%s", macro);
    
    pc.baud(9600);
    mu.startUpdates();//start measuring the distance
    pb.attach_deasserted(&updateHowManyToday);
    pb.mode(PullUp);
    pb.setSampleFrequency();
    
    //checkTime.attach(&updateTime, 10);
    /*
    // PWR MNGT
    PHY_PowerDown();
    //Peripheral_PowerDown(0xFEF6FFCF);
    Peripheral_PowerDown(LPC1768_PCONP_PCTIM0);  // Timer/Counter 0
    Peripheral_PowerDown(LPC1768_PCONP_PCTIM1);  // Timer/Counter 1
    Peripheral_PowerDown(LPC1768_PCONP_PCUART0);  //UART 0
    Peripheral_PowerDown(LPC1768_PCONP_PCUART1); //UART 1
    Peripheral_PowerDown(LPC1768_PCONP_PCPWM1); // PWM
    Peripheral_PowerDown(LPC1768_PCONP_PCI2C0); //bit 7: I2c interface
    Peripheral_PowerDown(LPC1768_PCONP_PCSPI);  //SPI
    Peripheral_PowerDown(LPC1768_PCONP_PCRTC);  //RTC
    Peripheral_PowerDown(LPC1768_PCONP_PCSSP1); //SSP
    
    // bit 11: Reserved

    Peripheral_PowerDown(LPC1768_PCONP_PCADC); // bit12: A/D converter power/clock enable
    Peripheral_PowerDown(LPC1768_PCONP_PCCAN1); // bit 13: CAN controller 1 power/clock enable
    Peripheral_PowerDown(LPC1768_PCONP_PCCAN2);   // bit 14: CAN controller 2 power/clock enable
// bit 15: PCGPIO: GPIOs power/clock enable
    //Peripheral_PowerDown(LPC1768_PCONP_PCGPIO); 

    Peripheral_PowerDown(LPC1768_PCONP_PCRIT); //bit 16: Repetitive interrupt timer power/clock enable
    Peripheral_PowerDown(LPC1768_PCONP_PCMCPWM);  // bit 17: Motor control PWM power/clock enable
    Peripheral_PowerDown(LPC1768_PCONP_PCQEI); // bit 18: Quadrature encoder interface power/clock enable
    Peripheral_PowerDown(LPC1768_PCONP_PCI2C1); // bit 19: I2C interface 1 power/clock enable
    
// bit 20: Reserved
    Peripheral_PowerDown(LPC1768_PCONP_PCSSP0); // bit 21: PCSSP0: SSP interface 0 power/clock enable 
    Peripheral_PowerDown(LPC1768_PCONP_PCTIM2); // bit 22: PCTIM2: Timer 2 power/clock enable
// bit 23: PCTIM3: Timer 3 power/clock enable
    //Peripheral_PowerDown(LPC1768_PCONP_PCQTIM3); 
// bit 24: PCUART2: UART 2 power/clock enable
    //Peripheral_PowerDown(LPC1768_PCONP_PCUART2); 
    
    Peripheral_PowerDown(LPC1768_PCONP_PCUART3); // bit 25: UART 3 power/clock enable
    Peripheral_PowerDown(LPC1768_PCONP_PCI2C2); // bit 26: I2C interface 2 power/clock enable
    Peripheral_PowerDown(LPC1768_PCONP_PCI2S);  // bit 27: PCI2S: I2S interface power/clock enable      
    
// bit 28: Reserved
    Peripheral_PowerDown(LPC1768_PCONP_PCGPDMA);  // bit 29:GP DMA function power/clock enable
    Peripheral_PowerDown(LPC1768_PCONP_PCENET); // bit 30:Ethernet block power/clock enable
    Peripheral_PowerDown(LPC1768_PCONP_PCUSB); // bit 31: PCUSB: USB interface power/clock enable     
  
     // only BIT 23, 24, 15 need to be on, but htis ths isnt working
     //Peripheral_PowerDown(0xFFFEFE7F);
    //Peripheral_PowerDown(0x7BEEF677);
    */
    while(1)
    {
        //pc.printf("Distance %d mm\r\n", mu.getCurrentDistance());
        Sleep();
        mu.checkDistance();     //call checkDistance() as much as possible, as this is where
                                //the class checks if dist needs to be called.
    wait(0.002);                           
    }
}