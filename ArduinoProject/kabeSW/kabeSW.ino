#include "AdvancedOTA.h"
#include "CiniParser.h"
#include "rootPage.h"
#include <Servo.h>
#include "switch.h"

#define INIFNM "/config.ini"

#define servo1Pow 13
#define servo2Pow 12
//#define servoSig 1  //TX
#define servoSig 14  //REMOTE
#define leftSW 4
#define rightSW 5
#define bootSW 0

Servo myservo;
Switch topSwitch(myservo, servo2Pow, 255, 0, 100);
Switch bottomSwitch(myservo, servo1Pow, 0, 255, 100);

ESP8266WiFiMulti WiFiMulti;
ESP8266WebServer server(80);


void setup() {
//  Serial.begin(74880);
  Serial.end();

  pinMode(leftSW, INPUT_PULLUP);
  pinMode(rightSW, INPUT_PULLUP);
  pinMode(bootSW, INPUT);
  myservo.attach(servoSig);

  if(initialize()){
    //されていなかったら
    setServoParam(false);
  }
  
  wifiSetup(&WiFiMulti, &server);
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/turn", handleTurn);

//--- ini File Sample ---
  server.on("/ini", handleIniSample);
  server.on("/writeini", handleWriteIni);
  server.on("/setini", handleSetIni);

  //for debug
  server.on("/procini", handleProcIni);
  server.on("/delini", handleDelIni);
}


void loop() {
  listener();

  if (digitalRead(leftSW) == LOW) {
    topSwitch.turn();
  }

  if (digitalRead(rightSW) == LOW) {
    bottomSwitch.turn();
  }

  if (digitalRead(bootSW) == LOW) {
    delay(100);
    while (digitalRead(bootSW) == LOW) {}

    setServoParam(true);
    while (digitalRead(bootSW) == LOW) {}
  }

  delay(100);
}

//- handle for HTTP ------------------------------------------------------------------------

// http://esp8266.local/
// ↑ここにアクセス

void handleRoot() {
  Serial.println("root");
  server.send(200, "text/html", ROOT_HTML );
}

void handleOn() {
  topSwitch.on();
  bottomSwitch.on();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleOff() {
  topSwitch.off();
  bottomSwitch.off();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleTurn(){
  topSwitch.turn();
  bottomSwitch.turn();
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

//- Control Servo ------------------------------------------------------------------------

// 取得完了 : 0
// 取得失敗 : 1
int initialize(){
  ////// ini Update
  CiniParser ini;
  String strMinimum, strMaximum;
  int minimum, maximum;
  int res1=0,res2=0;
  
  if(ini.setIniFileName( INIFNM )){
    //Serial.println("File not exist");
  }

  res1 = ini.rwIni("KabeCon", "minimum1", &strMinimum, READ);
  res2 = ini.rwIni("KabeCon", "maximum1", &strMaximum, READ);
  
  if(res1 != 3 || res2 != 3){
    return 1;
  }
  minimum = strMinimum.toInt();
  maximum = strMaximum.toInt();
  bottomSwitch.setPositions(minimum, maximum, (maximum + minimum) / 2);

  res1 = ini.rwIni("KabeCon", "minimum0", &strMinimum, READ);
  res2 = ini.rwIni("KabeCon", "maximum0", &strMaximum, READ);

  if(res1 != 3 || res2 != 3){
    return 1;
  }
  minimum = strMinimum.toInt();
  maximum = strMaximum.toInt();
  topSwitch.setPositions(maximum, minimum, (maximum + minimum) / 2);
  
  return 0;
}

void setServoParam(bool wifiSetup) {
  const unsigned long interval = 1000;
  unsigned long previousMills = 0;
  unsigned long currentMills = millis();

  const int sensitive = 100;

  int minimum = 256;
  int maximum = 0;
  int pos = 100;
  myservo.write(pos);

  // topSwitch
  while(1){
    if(wifiSetup)listener();
    digitalWrite(servo2Pow, HIGH);

    currentMills = millis();
    if(currentMills - previousMills >= interval){
      previousMills = currentMills;
      digitalWrite(leftSW, LOW);
      pinMode(leftSW, OUTPUT);
      delay(100);
      digitalWrite(leftSW,HIGH);
      pinMode(leftSW, INPUT_PULLUP);
    }

    if (digitalRead(rightSW) == LOW) {
      if(0 < pos) pos -= 1;
      if(pos < minimum) minimum = pos;
      delay(sensitive);
    }

    if (digitalRead(leftSW) == LOW) {
      if(pos < 256) pos += 1;
    if(maximum < pos) maximum = pos;
      delay(sensitive);
    }

    myservo.write(pos);

    if (digitalRead(bootSW) == LOW) {
      digitalWrite(servo2Pow, LOW);
      break;
    }
  }

  ////// ini Update
  if(maximum){
    CiniParser ini;
    String strMinimum;
    String strMaximum;
    strMinimum = minimum;
    strMaximum = maximum;
    if(ini.setIniFileName( INIFNM )){
      //Serial.println("File not exist");
    }
    ini.rwIni("KabeCon", "minimum0", &strMinimum, WRITE);
    ini.rwIni("KabeCon", "maximum0", &strMaximum, WRITE);
    topSwitch.setPositions(maximum, minimum, (minimum + maximum) / 2);
  }

  minimum = 256;
  maximum = 0;
  pos = 100;
  myservo.write(pos);
  // bottomSwitch
  while(1){
    if(wifiSetup)listener();
    digitalWrite(servo1Pow, HIGH);

    currentMills = millis();
    if(currentMills - previousMills >= interval){
      previousMills = currentMills;
      digitalWrite(rightSW, LOW);
      pinMode(rightSW, OUTPUT);
      delay(100);
      digitalWrite(rightSW,HIGH);
      pinMode(rightSW, INPUT_PULLUP);
    }

    if (digitalRead(leftSW) == LOW) {
      if(0 < pos) pos -= 1;
      if(pos < minimum) minimum = pos;
      delay(sensitive);
    }

    if (digitalRead(rightSW) == LOW) {
      if(pos < 256) pos += 1;
    if(maximum < pos) maximum = pos;
      delay(sensitive);
    }

    myservo.write(pos);

    if (digitalRead(bootSW) == LOW) {
      digitalWrite(servo1Pow, LOW);
      break;
    }
  }

  ////// ini Update
  if(maximum){
    CiniParser ini;
    String strMinimum;
    String strMaximum;
    strMinimum = minimum;
    strMaximum = maximum;
    if(ini.setIniFileName( INIFNM )){
      //Serial.println("File not exist");
    }
    ini.rwIni("KabeCon", "minimum1", &strMinimum, WRITE);
    ini.rwIni("KabeCon", "maximum1", &strMaximum, WRITE);
    bottomSwitch.setPositions(minimum, maximum, (minimum + maximum) / 2);
  }
  
}



//- ini File Access Sample ------------------------------------------------------------------------

void handleIniSample(){
  String message = "<HTML><BODY>";

  CiniParser testini;
  if(testini.setIniFileName( INIFNM )){
    message += "File not exist";
    message += testini.createIniFile();
  }

  message += "<CENTER><FONT SIZE=5>";
  message += "ini Test sample page</FONT></CENTER><BR>";


//SPIFFS 情報表示
  message += "<B>SPIFFS information </B><BR>";
  SPIFFS.begin();
  FSInfo fsinfo;
  SPIFFS.info(fsinfo);
  message += "<TABLE border=1>";
  message += "<TR><TD>totalBytes</TD><TD>";
  message += fsinfo.totalBytes;
  message += "</TD></TR>";
  message += "<TR><TD>usedBytes</TD><TD>";
  message += fsinfo.usedBytes;
  message += "</TD></TR>";
  message += "<TR><TD>blockSize</TD><TD>";
  message += fsinfo.blockSize;
  message += "</TD></TR>";
  message += "<TR><TD>pageSize</TD><TD>";
  message += fsinfo.pageSize;
  message += "</TD></TR>";
  message += "<TR><TD>maxOpenFiles</TD><TD>";
  message += fsinfo.maxOpenFiles;
  message += "</TD></TR>";
  message += "<TR><TD>maxPathLength</TD><TD>";
  message += fsinfo.maxPathLength;
  message += "</TD></TR>";
  message += "<TR><TD>use rate</TD><TD>";
  message += (fsinfo.usedBytes / fsinfo.totalBytes * 100.00);
  message += "%";
  message += "</TD></TR>";
  message += "</TABLE><BR><BR>";


//ファイル一覧を表示
  message += "<B>Directry information </B><BR>";
  message += "<TABLE border=1>";
  Dir dir = SPIFFS.openDir("/");
  while ( dir.next() ){
    message += "<TR><TD>";
    message += dir.fileName();
    message += "</TD><TD>";
    File f = dir.openFile("r");
    message += f.size();
    message += "</TD></TR>";
    f.close(); 
  }
  message += "</TABLE><BR><BR>";


//iniファイルの情報表示
  message += "<B>ini information </B><BR>";
  message += "fileName: ";
  message += INIFNM;
  message += "<BR>filesize: ";
  message += testini.getIniFileSize();
  message += " byte<BR><BR>";

  String sect = server.arg("Section");
  String name = server.arg("Name");
  String valu;

  if(sect.length()*name.length()){
    testini.rwIni(sect, name, &valu, READ);
  }

//ini ファイル書き込み
  message += "<B>write ini test </B><BR>";
  message += "<FORM method=POST action=/writeini>";
  message += "Section: <INPUT type=text name='Section' value='";
  message += sect;
  message += "'>Name: <INPUT type=text name='Name' value='";
  message += name;
  message += "'>Value: <INPUT type=text name='Value' value='";
  message += valu;
  message += "'><INPUT type=submit value='Write'></FORM>";


//ini ファイル取得
  message += "<B>read ini test </B><BR>";
  message += "<FORM method=POST action=/ini>";
  message += "Section: <INPUT type=text name='Section' value='";
  message += sect;
  message += "'>Name: <INPUT type=text name='Name' value='";
  message += name;
  message += "'>Value: <INPUT type=text name='Value' value='";
  message += valu;
  message += "'disabled='disabled'><INPUT type=submit value='Read'></FORM>";

  
//ini ファイルの内容を表示
  message += "<B>ini file viewer </B><BR>";
  message += "<FORM method=POST action=/setini><TEXTAREA NAME=ini cols=100 rows=40>";
  testini.readIniFile(&message);
  message += "</TEXTAREA>";
  message += "<INPUT type=submit value='submit'></FORM><BR>";


//WiFi ini 設定
  message += "<B>WiFi ini Setting</B><BR>";
  message += "<FORM method=POST action=/writeini>";
  message += "Section: <INPUT type=text name='Section' value='WiFi' disabled='disabled'>";
  message += "Name: <INPUT type=text name='Name' value='SSID' disabled='disabled'>";
  message += "<INPUT type=hidden name='Section' value='WiFi'>";
  message += "<INPUT type=hidden name='Name' value='SSID' >";
  message += "Value: <INPUT type=text name='Value'>";
  message += "<INPUT type=submit value='Update SSID'></FORM>";
  message += "<FORM method=POST action=/writeini>";
  message += "Section: <INPUT type=text name='Section' value='WiFi' disabled='disabled'>";
  message += "Name: <INPUT type=text name='Name' value='PASS' disabled='disabled'>";
  message += "<INPUT type=hidden name='Section' value='WiFi'>";
  message += "<INPUT type=hidden name='Name' value='PASS' >";
  message += "Value: <INPUT type=text name='Value'>";
  message += "<INPUT type=submit value='Update PASS'></FORM>";
  
  message += "</BODY></HTML>";
  server.send(200, "text/html", message);
}


void handleWriteIni(){
  CiniParser testini;
  if(testini.setIniFileName( INIFNM )){
    Serial.println( "File not exist");
    handleIniSample();
    return;
  }

  String sect = server.arg("Section");
  String name = server.arg("Name");
  String valu = server.arg("Value");

  if(sect.length()*name.length()){
    testini.rwIni(sect, name, &valu, WRITE);
  }
  
  server.sendHeader("Location", "/ini", true);
  server.send(301, "text/plain", "");
}

void handleSetIni(){
  String ini = server.arg("ini");
  
  CiniParser testini;
  if(testini.setIniFileName( INIFNM )){
    Serial.println("File not exist");
  }else{
    testini.writeIniFile(&ini);
  }

  server.sendHeader("Location", "/ini", true);
  server.send(301, "text/plain", "");
}


void handleProcIni(){
  String message = "<HTML><BODY>";

  SPIFFS.begin();
  
  FSInfo fsinfo;
  SPIFFS.info(fsinfo);

  message += "<TABLE border=1>";
  message += "<TR><TD>totalBytes</TD><TD>";
  message += fsinfo.totalBytes;
  message += "</TD></TR>";
  message += "<TR><TD>usedBytes</TD><TD>";
  message += fsinfo.usedBytes;
  message += "</TD></TR>";
  message += "<TR><TD>blockSize</TD><TD>";
  message += fsinfo.blockSize;
  message += "</TD></TR>";
  message += "<TR><TD>pageSize</TD><TD>";
  message += fsinfo.pageSize;
  message += "</TD></TR>";
  message += "<TR><TD>maxOpenFiles</TD><TD>";
  message += fsinfo.maxOpenFiles;
  message += "</TD></TR>";
  message += "<TR><TD>maxPathLength</TD><TD>";
  message += fsinfo.maxPathLength;
  message += "</TD></TR>";
  message += "<TR><TD>use rate</TD><TD>";
  message += (fsinfo.usedBytes / fsinfo.totalBytes * 100.00);
  message += "%";
  message += "</TD></TR>";
  message += "</TABLE><BR><BR>";

  message += "<TABLE border=1>";
  Dir dir = SPIFFS.openDir("/");
  while ( dir.next() ){
    message += "<TR><TD>";
    message += dir.fileName();
    message += "</TD><TD>";
    File f = dir.openFile("r");
    message += f.size();
    message += "</TD></TR>";
    message += "<TR><TD>";
            CiniParser testini;
            if(testini.setIniFileName( (char*)f.name() )){
              message += "didn't open";
            }
            testini.readIniFile(&message);
    message += "</TD></TR>";
    f.close();
 
  }
  message += "</TABLE><BR><BR>";
  

  message += "<FORM method=POST action=/setini><TEXTAREA NAME=ini cols=100 rows=40>";

            CiniParser testini;
            if(testini.setIniFileName( INIFNM )){
              message += "didn't open";
            }
            testini.readIniFile(&message);

  message += "</TEXTAREA>";
  message += "<INPUT type=submit value='submit'></FORM>";

  message += "</BODY></HTML><FORM>";

  SPIFFS.end();
  server.send(200, "text/html", message);
}

void handleDelIni(){
  String message = "<HTML><BODY>";

  CiniParser testini;
  if(testini.setIniFileName( INIFNM )){
    message += "File not exist";
  }
  message += testini.deleteIniFile();

  message += "<FORM method=POST action=/setini><TEXTAREA NAME=ini cols=100 rows=40>";
  
  testini.readIniFile(&message);

  message += "</TEXTAREA>";
  message += "<INPUT type=submit value='submit'></FORM>";
  message += "</BODY></HTML><FORM>";
  server.send(200, "text/html", message);
}

