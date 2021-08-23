Install Ubuntu 20.0.4
Install Oracle Virtual Box
Install home assistant VMDK
Configure Virtual Box to get Hassio running
Access the UI via the IP provided by Hassio's CLI
Install ESPHOME via Home assistant

Windows:
Connect to computer and download ESPHOME flasher
Download setup binary and upload using COM port
THen OTA (Over the Air) uploads from then on...

Ubuntu:
Connect to power source
Upload OTA because previous setup done by Windows
Code for LED (switch) and Button (binary_sensor)
  

1.//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
2.//////////////////////////// Above is old attempt see below for 'new' architecture ///////////////////////////////
3.//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
Date: 27/06/2021
Setup: Ai Thinker ESP32 CAM, Blynk v1, ESP Wrover device setup (Huge App Partition) 
1. Program ESP32 CAM to connect to Wi-Fi to stream video on http://192.168.1.xxx:81/stream (replace xxx with localIP())
2. Program ESP32 CAM to connect to Blynk using ssid, password and project authorisation code - here create a button to turn on led to see if it works
3. Create webhook view to stream the IP above
NOTE: IT works like that buuuuuttt the stream has 3 floating dots which is really annoying so can't get rid of them. if you're happy with them then leave it but im not so here is ther next few steps
read up on chunked http response
4. Try to use video streaming (NOTE: it doesnt work) - the reason why it works on the same IP on laptop browser and not on phone browser is because of "chunked http response"
5. To get around this, we can use MJPEG streaming and program the chip to use MJPEG in which there is no chunked HTTP response
6. IT WORKSSSSSSSSSSSSS, the stream link is: http://192.168.1.xxx/mjpeg/1
7. Ok so there is a bit of a delay like 2 seconds (kind of a lot) but we can fix that hopefully
8. Now the plan is to integrate this new code with the Blynk code and test the LED
9. Then add a push button to send a notification to Blynk and push to live stream
10. Then we can do a box test and wifi provisioning later


UPDATE 09/07/21
So in the loop part of the code there are 2 pieces of code
1. Blynk.run();
2. server.handleClient();
Now each one is essential for connection or mjpeg stream. They interfere with each other. Now testing one without the other to see if it is affected.

Lots of brownout errors were occuring when we switched the board to a breadboard. Instead we decided to solder the parts together on a circuit board. Stronger connections and no errors now.

Blynk notifications work. Now time to incorporate them with the stream.

UPDATE 23/08/21
The doorbell works now
When turned on there is a 30 second period where there is no light and during that time the doorbell attempts to connect to the WiFi
If it the first time it has been turned on then the doorbell after 30 seconds will go into access point mode then once details are entered then a 3 minute wait period hap0pens where the LED blinks. Then after that the LED goes green indicating the doorbell is ready to connect

But if its not the first time then the white blinking LED will happen in a shorter time space. Then the rest conitnue after that (this only happens when there is a working WiFi network)

But if the WiFi network is down then the LED will turn red and after every 30 seconds the doorbell will retry connection

If the WiFI network is down but there is another one which the user would like the doorbell to connect to, then once the doorbell's been turned on, after the 30 second connection time, the user needs to hold the reset button for 3 seconds during the white LED blinking time in order to reopen access point mode and write down wifi credentials

That's all 
