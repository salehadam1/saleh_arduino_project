#include <LiquidCrystal.h> // include the LCD library code:
#include <Servo.h> // Include the Servo library 
#include <Adafruit_MLX90614.h>// Include the MLX Temp library 
#include "WiFiEsp.h" // include wifi library code

// Emulate Serial1 on pins 6/7 if not present
#ifndef HAVE_HWSERIAL1
#include "SoftwareSerial.h"
SoftwareSerial Serial1(6, 7); // RX, TX
#endif

char ssid[] = "Adam";            // your network SSID (name)
char pass[] = "12356623";        // your network password
int status = WL_IDLE_STATUS;     // the Wifi radio's status
int reqCount = 0;                // number of requests received
WiFiEspServer server(80);


//Declare lcd pint to the arduion pin rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 2;
LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
Servo myservo;  // create servo object to control a servo
Adafruit_MLX90614 mlx = Adafruit_MLX90614();


const int LED_RED = 47;// Declare red led to pin 9 on the arduion
const int LED_YELLOW = 49;// Declare yellow led to pin 9 on the arduion
const int SERVO_PIN = 37; // Declare the Servo pin 
const int FRINT_IRSensor = 21; // Declare ir sensor pin to arduino pin 
const int BLACK_IRSensor = 20; // Declare ir sensor pin to arduino pin 
const int MAX_Peopel = 5; //Declare max number of people alowed to enter
const int MAX_TEM = 37; //Declare max temp

// Variables will change:
int IRState;             // the current reading from the input pin
int lastIrState = LOW;   // the previous reading from the input pin
// the following variables are unsigned longs because the time, measured in
// milliseconds, will quickly become a bigger number than can be stored in an int
unsigned long lastDebounceTime = 0;  // the last time the output pin was toggled
unsigned long debounceDelay = 50;  // the debounce time; increase if the output flickers
int coun;// for the interrupt deponds
double temp_obj; // for te person temp
int numOfpeopel; //Declare count number of people inside
int total_pepelenter;//Declare the total enter per day
int total_pepelReject;//Declare the total enter per day
double total_temp ;//Declare the total enter per day
double avr_temp;// Declare the averge temp
int reject_peopel[8];//Declare reject temp
int enter_peopel[8];//Declare reject temp


//time reader and print the resulet per hour
unsigned long mile_Time;//Declare to resive from millis()
unsigned long mytimen;//save he hour
unsigned long checkTime = 36000000; // check every 1 hour 1 hour = 3600000 milis
volatile int hour_reader = 0;


void setup()
{
  pinMode(LED_RED, OUTPUT); // Declare the red LED as an output
  pinMode(LED_YELLOW, OUTPUT); // Declare the yellow LED as an output
  pinMode(FRINT_IRSensor, INPUT); // Front sensor pin INPUT 
  pinMode(BLACK_IRSensor, INPUT); // Back sensor pin INPUT 

  lcd.begin(16, 2);// set up the LCD's number of columns and rows:
  myservo.attach(SERVO_PIN); // We need to attach the servo to the used pin number
  attachInterrupt(digitalPinToInterrupt(BLACK_IRSensor), count,RISING);// hte Interrupt
  mlx.begin();//Inicializa o MLX90614

  Serial.begin(115200);// initialize serial for debugging
  Serial1.begin(9600);// initialize serial for ESP module
  WiFi.init(&Serial1); // initialize ESP module
  if (WiFi.status() == WL_NO_SHIELD) // check for the presence of the shield
  { 
    Serial.println("WiFi shield not present");
    while (true);// don't continue  
  }
  while ( status != WL_CONNECTED) // attempt to connect to WiFi network
  {
    Serial.print("Attempting to connect to WPA SSID: ");
    Serial.println(ssid);
    status = WiFi.begin(ssid, pass);// Connect to WPA/WPA2 network
  }
  Serial.println("You're connected to the network");
  printWifiStatus();
  server.begin(); // start the web server on port 80
}//end of setup

void loop()
{
  mile_Time = millis();
  if((mile_Time - mytimen ) >= checkTime)
  {
    mytimen = mile_Time;
    hour_reader++;
  }
  digitalWrite(LED_RED, HIGH); // Turn the red LED on
  if(numOfpeopel < MAX_Peopel)//check if there is place to enter
  {
    //print out the number of people ins
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("people inside: ");
    lcd.setCursor(0, 1);
    lcd.print(numOfpeopel);
    delay(1000);
    if(digitalRead(FRINT_IRSensor)!= 1) //Check the front IR senser if it LOW then take the temp
    {
      digitalWrite(LED_RED, LOW);// Turn the RED LED OFF
      delay(1000);//waits 5s so the MLX take the temp
      temp_obj = mlx.readObjectTempC();//take the temp
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("temp: ");
      lcd.setCursor(0, 1);
      lcd.print(temp_obj);
      delay(1000);
      if(temp_obj < MAX_TEM)//if the temp low then 37 
      {
        numOfpeopel++; // calculator the number of peopel inside
        total_pepelenter++; // calculator the total peopel has enter
        total_temp = total_temp + temp_obj; // cal the total of the temp
        avr_temp = total_temp / total_pepelenter; // cal the averg of the temp
        digitalWrite(LED_YELLOW, HIGH); // Turn the yellow LED on
        lcd.clear();
        lcd.setCursor(0, 0);// print in the lcd the door open and you can enter
        lcd.print("you can Enter: ");
        myservo.write(0);// the door open and wait for 5 sec and then close
        delay(3000);
        myservo.write(90);//the door close
        digitalWrite(LED_YELLOW, LOW); // Turn the yellow LED on
      }
      else  //if the temp hight then 37  
      {
        total_pepelReject++;//calculator the total peopel been reject
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("you can't Enter: ");// print in the lcd you can not enter
        delay(1000);
      }
    }
  }//end of if
  while(numOfpeopel >= MAX_Peopel)// thers no place inside so you hvae to wait
  {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("please wait!");
    digitalWrite(LED_RED, LOW); // Turn the red LED on make the red led flashing
    delay(500);
    digitalWrite(LED_RED, HIGH); // Turn the red LED on
    delay(500);
  }//end of while loop

  // save the deatil for each hour
  for(int i = 0 ; i < 8 ; i++)
  {
    if(hour_reader == i)
    {
      //calculator the peopel been reject per hour
      reject_peopel[i]+= total_pepelReject - reject_peopel[0]+reject_peopel[1]+reject_peopel[2]+reject_peopel[3]+reject_peopel[4]+reject_peopel[5]+reject_peopel[6]+reject_peopel[7];
      // calculator the number of people been enter per hour
      enter_peopel[i]+= total_pepelenter - enter_peopel[0]+enter_peopel[1]+enter_peopel[2]+enter_peopel[3]+enter_peopel[4]+enter_peopel[5]+enter_peopel[6]+enter_peopel[7];
    }//end of if
  }//end of FOR


// listen for incoming clients
  WiFiEspClient client = server.available();
  if (client) {
    Serial.println("New client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          Serial.println("Sending response");
          
          // send a standard http response header
          // use \r\n instead of many println statements to speedup data send
          client.print(
            "HTTP/1.1 200 OK\r\n"
            "Content-Type: text/html\r\n"
            "Connection: close\r\n"  // the connection will be closed after completion of the response
            "Refresh: 20\r\n"        // refresh the page automatically every 20 sec
            "\r\n");
          client.print("<!DOCTYPE HTML>");
          client.print("<html>");
          client.print("<head>");
          client.print("<style>");
          client.print(".colu1 {height: auto; width: auto;}");
          client.print(".colu2 { width: 48%; float: left;padding: 2px;}");
          client.print(".colu3 {width: 48%; float: left;padding: 2px;}  ");
          client.print(".h1 {text-align: center;}");
          client.print("table {font-family: arial, sans-serif; width: 100%;}");
          client.print("td, th {border: 1px solid #dddddd; text-align: left; padding: 8px;}");
          client.print("tr:nth-child(even) {background-color: #dddddd;}");
          client.print("</style>");
          client.print("</head>");
          client.print("<body>");
          client.print("<div class=\"colu1\">");
          client.print("<div class=\"h1\">");
          client.print("<h3>INTERNET OF THINGS PROJECT</h3>");
          client.print("<h3>Covid-19 Doorway Security</h3>");
          client.print("</div>");
          client.print("<table>");
          client.print("<tr>");
          client.print("<th>Open Time</th>");
          client.print("<th>Close Time</th>");
          client.print("<th>MAX Number</th>");
          client.print("<th>Number of people inside now</th>");
          client.print("<th>Total Enter</th>");
          client.print("<th>Total Reject</th>");
          client.print("<th>Average Temp</th>");
          client.print("</tr>");
          client.print("<tr>");
          client.print("<td>09:00</td>");
          client.print(" <td>17:00</td>");
          client.print("<td>");
          client.print(MAX_Peopel);
          client.print("</td>");
          client.print("<td>");
          client.print(numOfpeopel);
          client.print("</td>");
          client.print("<td>");
          client.print(total_pepelenter);
          client.print("</td>");
          client.print("<td>");
          client.print(total_pepelReject);
          client.print("</td>");
          client.print("<td>");
          client.print(avr_temp);
          client.print("</td>");
          client.print("</tr>");
          client.print("</table>");
          client.print("</div>");
          client.print("<div class=\"colu2\">");
          client.print("<button type=\"button\" onclick=\"alert('It is device that take the temperature measuring with out touching the body and counting the number of people with and LCD screen to show up the result, and save the result in to database and web server.So the device measuring the temperature of the people before they entering any facility if the temperature is come up 38 degrees Celsius or more then that red light will show up and the person he/she not allowed to enter with message tell them in the LCD screen.otherwise checking the number of people inside the facility for the number limited inside the facility if there is no space inside the facility orange light and message will show up in the LCD screen description the time of waiting and the number of the people inside the facility.otherwise green light and message will show up in the LCD screen allow the person to enter the facility.')\">What is Covid-19 Doorway Security</button>");
          client.print("<br>");
          client.print("<br>");
          client.print("<button type=\"button\" onclick=\"alert('During the time of covid-19 (pandemic) we see business and school shut down and life become hard and uncomfortable by another word the life had stop, that why I came up with TM&CD.The purpose of the device is to live with covid-19 and for our safety and health life without effect on the economic, education, mental health, and community.')\">The Purpose</button>");
          client.print("</div>");
          client.print("<div class=\"colu3\">");
          client.print("<table>");
          client.print("<tr>");
          client.print("<th>");
          client.print("<script type=\"text/javascript\" src=\"https://www.gstatic.com/charts/loader.js\"></script>");
          client.print("<script type=\"text/javascript\">");
          client.print("google.charts.load('current', {'packages':['corechart']});");
          client.print("google.charts.setOnLoadCallback(drawChart);");
          client.print("function drawChart() {");
          client.print("var data = google.visualization.arrayToDataTable([");
          client.print("['Hours', 'enter', 'reject'],");
          client.print("['1th',");
          client.print(enter_peopel[0]);
          client.print(",");
          client.print(reject_peopel[0]);
          client.print("],");
          client.print("['2th',");
          client.print(enter_peopel[1]);
          client.print(",");
          client.print(reject_peopel[1]);
          client.print("],");
          client.print("['3th',");
          client.print(enter_peopel[2]);
          client.print(",");
          client.print(reject_peopel[2]);
          client.print("],");
          client.print("['4th',");
          client.print(enter_peopel[3]);
          client.print(",");
          client.print(reject_peopel[3]);
          client.print("],");
          client.print("['5th',");
          client.print(enter_peopel[4]);
          client.print(",");
          client.print(reject_peopel[4]);
          client.print("],");
          client.print("['6th',");
          client.print(enter_peopel[5]);
          client.print(",");
          client.print(reject_peopel[5]);
          client.print("],");
          client.print("['7th',");
          client.print(enter_peopel[6]);
          client.print(",");
          client.print(reject_peopel[6]);
          client.print("],");
          client.print("['8th',");
          client.print(enter_peopel[7]);
          client.print(",");
          client.print(reject_peopel[7]);
          client.print("],");
          client.print("['9th',  0,      0]");
          client.print("]);");
          client.print("var options = {");
          client.print("title: 'Hourly Count',");
          client.print("curveType: 'function',");
          client.print("legend: { position: 'bottom' }");
          client.print("};");
          client.print("var chart = new google.visualization.LineChart(document.getElementById('curve_chart'));");
          client.print("chart.draw(data, options);");
          client.print("}");
          client.print("</script>");
          client.print("</head>");
          client.print("<body>");
          client.print("<div id=\"curve_chart\" style=\"width: 660px; height: 460px;\"></div>");
          client.print("</th>");
          client.print("</tr>");
          client.print("</table>");
          client.print("</div>");
          client.print("</body>");
          client.print("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(10);

    // close the connection:
    client.stop();
    Serial.println("Client disconnected");
  }//end of if cline
}//end of loop

void printWifiStatus()
{
  // print the SSID of the network you're attached to
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);
  
  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);
  Serial.println();
}//end of void print







void count()//interrupt the back IR senser
{
  // read the state of the IR into a local variable:
  int reading = digitalRead(BLACK_IRSensor);

  // If the IR changed, due to noise or pressing:
  if (reading != lastIrState) {
    // reset the debouncing timer
    lastDebounceTime = millis();
  }
  if ((millis() - lastDebounceTime) > debounceDelay) {
    // whatever the reading is at, it's been there for longer than the debounce
    // delay, so take it as the actual current state:

    // if the IR state has changed:
    if (reading != IRState) {
      IRState = reading;

      // only count one if the new IR state is HIGH
      if (IRState == HIGH) {
        if(numOfpeopel != 0 )
        {
          numOfpeopel--;
        }
      }
    }
  }
  // save the reading. Next time through the loop, it'll be the lastIrState:
  lastIrState = reading; 
}
