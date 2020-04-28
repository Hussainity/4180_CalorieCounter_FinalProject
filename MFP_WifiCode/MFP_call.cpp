
#include "mbed.h"
#incldue <string>

int  count,ended,timeout;
char buf[2024];
char snd[1024];

char ssid[32] = "SwagInABag";     // enter WiFi router ssid inside the quotes
char pwd [32] = "yoloswag"; // enter WiFi router password inside the quotes


double MFP_LookupTest(string food, string macro)
{
  return food.at(0) - 'a';
}


double MFP_Lookup(string food, string macro)
{
    reset=0; //hardware reset for 8266
    pc.baud(9600);  // set what you want here depending on your terminal program speed
    pc.printf("\f\n\r-------------ESP8266 Hardware Reset-------------\n\r");
    wait(0.5);
    reset=1;
    timeout=2;
    getreply();
    esp.baud(9600);   // change this to the new ESP8266 baudrate if it is changed at any time.
    ESPconfig();        //******************  include Config to set the ESP8266 configuration  ***********************
    return queryMFP(food, macro);

}

double queryMFP(string food, string macro){
  wait(1);
  strcpy(snd,"srv = net.createConnection(net.TCP, 0)\r\n");
  SendCMD();
  wait(1);
  strcpy(snd,"srv:on(\"receive\", function(sck, c) print(c) end)\r\n");
  SendCMD();
  wait(1);
  strcpy(snd,"srv:connect(80,\"spectrosam.org\")\r\n");
  SendCMD();
  wait(1);
  sprintf(snd, "sck:send(\"GET /trial.php?query=%s HTTP/1.1\r\nHost: http://spectrosam.org/\r\nAccept: */*\r\n\r\n\")", food);
  SendCMD();
  wait(1);
  timeout=15;
  getreply();
  pc.printf(buf);

  string resp(buf);
  size_t found = resp.find(macro);
  int start = found + macro.size() + 2; //after finding the macro string, go to the end of it, and add two for the quotes and colon
  int end = start + 3; // D.D of macro value
  return stod(resp.substr(start,end));
}


//  +++++++++++++++++++++++++++++++++ This is for ESP8266 config only, run this once to set up the ESP8266 +++++++++++++++
void ESPconfig()
{

    wait(5);
    pc.printf("\f---------- Starting ESP Config ----------\r\n\n");
    strcpy(snd,".\r\n.\r\n");
    SendCMD();
    wait(1);
    pc.printf("---------- Reset & get Firmware ----------\r\n");
    strcpy(snd,"node.restart()\r\n");
    SendCMD();
    timeout=5;
    getreply();
    pc.printf(buf);
    wait(2);

    // set CWMODE to 1=Station,2=AP,3=BOTH, default mode 1 (Station)
    pc.printf("\n---------- Setting Mode ----------\r\n");
    strcpy(snd, "wifi.setmode(wifi.STATION)\r\n");
    SendCMD();
    timeout=4;
    getreply();
    pc.printf(buf);
    wait(2);

    pc.printf("\n---------- Connecting to AP ----------\r\n");
    pc.printf("ssid = %s   pwd = %s\r\n",ssid,pwd);
    strcpy(snd, "wifi.sta.config(\"");
    strcat(snd, ssid);
    strcat(snd, "\",\"");
    strcat(snd, pwd);
    strcat(snd, "\")\r\n");
    SendCMD();
    timeout=10;
    getreply();
    pc.printf(buf);

    wait(5);

  }
}

void SendCMD()
{
    esp.printf("%s", snd);
}

void getreply()
{
    memset(buf, '\0', sizeof(buf));
    t.start();
    ended=0;
    count=0;
    while(!ended) {
        if(esp.readable()) {
            buf[count] = esp.getc();
            count++;
        }
        if(t.read() > timeout) {
            ended = 1;
            t.stop();
            t.reset();
        }
    }
}
