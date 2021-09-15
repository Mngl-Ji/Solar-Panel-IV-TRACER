// Note :  I-V Tracer code is designed to determine the quality of a panel. A term called Performance Ratio determines whether the generation of solar is as expected compared to theoritical calculation.
// Note :  This device can also determine the Vmp, Imp, Pmp, Voc and Isc of the panel.
// Note :  This project requires NTC   probe and 10k resistor for temperature measurement, solar cell or panel as irradiance meter, button funtion and Datalogger shield (optional) to record data for I-V curve in excel.
// Note :  The data can be saved into SD Card via datalogger shield to plot I-V curve in excel file. By default it comes with datalogger code. If you don't have the datalogger shield, kindly delete relevant codes or it might not function properly.

        /* 0- General */

        int decimalPrecision = 1;                   // decimal places for values (Pmp, Vmp, Voc, Temp and Irrad) in LED Display & Serial Monitor
                                                    // decimal places x 3 for values (Isc and Imp)
                                                    // decimal places x 2 for performance ratio (PR)
        //All GPIOs can be used as outputs except GPIOs 6 to 11 (connected to the integrated SPI flash) and GPIOs 34, 35, 36 and 39 (input only GPIOs);
  
        /* 1- DC Voltage Measurement*/

        int VoltageAnalogInputPin = 34;             // GPIO pin to measure Voltage Value
        float R1 = 18000;                           // Input resistance value for R1 (in ohm) based on Voltage Divider Method. 
        float R2 = 2000;                            // Input resistance value for R2 (in ohm) based on Voltage Divider Method. 
        float voltage  = 0;                         /* to read the value */
        float voltageLastSample   = 0;              /* to count time for each sample. Technically 1 milli second 1 sample is taken */
        int voltageSampleCount = 0;                 /* to count number of sample. */
        float moduleSupplyVoltageV = 5;             /* maximum measuring voltage , default 5V*/
    

        /* 2- DC Current Measurement */
   
        int CurrentAnalogInputPin = 35;             // GPIO pin to measure Current Value 
        int mVperAmpValue = 100;                    // If using ACS712 current module : for 5A module key in 185, for 20A module key in 100, for 30A module key in 66
        float moduleMiddleVoltage = 2500;           /* when there is no reading, the voltage is at middle Vcc. For 5V power supply, the middle voltage is 2500mV;*/
        float moduleSupplyVoltage = 5000;           /* supply voltage to current sensor module, default 5000mV*/
        float current  = 0;                         /* to read the value */
        float currentLastSample  = 0;               /* to count time for each sample. Technically 1 milli second 1 sample is taken */
        int currentSampleCount = 0;                 /* to count number of sample. */


        /* 3- DC Power Wattage calculation */

        float PowerValue =0;                        /* Initial calculation Power Value */
        float powerLastSample = 0;                  /* to count time for each sample. Technically 1 milli second 1 sample is taken */
        int powerSampleCount = 0;                   /* to count number of sample. */


        /* 6 Recording Calculation */

        float PmpSolarPanel = 50;                   // Key in wattage of your solar panel at STC.
        float reducedPmp =0 ;                       /* variable to calculated expected maximum power for a solar panel */
        int InitiateReading = 0;                    /* To switch between functions for start and stop recording */   
        int InitialReset = 0;                       /* to reset the electrical parameters for the next measurement set */
        float recordLastSample=0;                   /* to count time for each sample.*/
        float PmpInitial = 0;                       /* variable for recording Pmp */
        float IscInitial = 0;                       /* variable for recording Isc */
        float VocInitial = 0;                       /* variable for recording Voc */
        float Vmp = 0;                              /* variable for Vmp */
        float Imp = 0;                              /* variable for Imp */

        /* 8 - SD memory card shield */

        #include<SD.h>                              /* Load the SD card Library (by default already built-it with arduino solftware)*/
        #include<SPI.h>                             /* Load the SPI of SPI communication Library (by default already built-it with arduino solftware)*/
        int chipSelect = 10;                        // Select ChipSelect pin based on the SPI communication for SC card shield. This pin cannot be changed.
        File mySensorData;                          // Variable for working with our file object
        #include <WiFi.h>
        #include <WiFiClient.h>
        #include <WebServer.h>  //https://github.com/bbx10/WebServer_tng
        #define I2C_SDA 32
        #define I2C_SCL 33
        const char* wifi_name = "Tenda_31BC98"; // Your Wifi network name here
        const char* wifi_pass = "barcelona";    // Your Wifi network password here
        WiFiServer server(80);    // Server will be at port 80
         


        /*Now we will set the pins for the relay and the IGBTs switch*/
        int IGBTPin = 4; // 1st IGBT as a switch
        int IGBTPin1 = 6; //2nd IGBT as a switch(edit)
        int relay_pin = 2; //to control electronic relay
         void setup()                                        /* The Codes only run 1 time only when it started.*/
         
                {
                Serial.begin (115200);
                pinMode (relay_pin, OUTPUT);
                pinMode (IGBTPin, OUTPUT);
                pinMode (IGBTPin1, OUTPUT);
                pinMode(34, INPUT);
                pinMode(35, INPUT);
                /* 5 - SD memory card shield */
                pinMode(chipSelect,OUTPUT);                 /* reserve pin  as pin to write data in SD card by default, the pin is fixed in Arduino UNO*/
                SD.begin(chipSelect);                       /* Initialise the SD Card with chipSelect connected to pin   by default*/
            
                Serial.print ("Connecting to ");
                Serial.print (wifi_name);
                WiFi.begin (wifi_name, wifi_pass);     // Connecting to the wifi network
                while (WiFi.status() != WL_CONNECTED) // Waiting for the response of wifi network
                {
                  delay (500);
                  Serial.print (".");
                }
                Serial.println("");
                Serial.println("Connection Successful");
                Serial.print("IP address: ");
                Serial.println(WiFi.localIP());           // Getting the IP address
                Serial.println("Type the above IP address into browser search bar"); 
                server.begin();                           // Starting the server
               
                }
        
          
        void loop()      
            // setInterval(function, milliseconds, param1, param2, ...)
        
          {             
                         // IGBTPin1 is control the one IGBT connected directly with solar panel in series to calculate the short circuit currrent of solar panel
                         digitalWrite(IGBTPin1, HIGH);
                         current = analogRead(CurrentAnalogInputPin); 
                         float  I = (((current /1024) *5000) /mVperAmpValue);  
                         float Isc=I;

                         //Now we will charge the capacitor by make the second IGBT ON, through which our current and voltage sensor can take a finite no. of reading to plot a graph
                         if I>0
                         {
                         for(int i=0;i<=50;i++)
                         {
                         digitalWrite(IGBTPin, HIGH);
                         digitalWrite(relay_pin, LOW);
                         voltage = analogRead(VoltageAnalogInputPin);
                         float  V = ((voltage*moduleSupplyVoltageV)/1024.0) / (R2/(R1+R2));                    /* Calculate the expected monitoring votlage */
                         current = analogRead(CurrentAnalogInputPin); 
                         float  I = (((current /1024) *5000) /mVperAmpValue);                                      /* calculate the final RMS current*/ 
                         float Power =  I * V;
                         Serial.println(V);
                         Serial.println(I);
                         Serial.println(PowerValue);
                         }
                         }
                         elseif I=0
                         {
                         digitalWrite(IGBTPin, LOW);
                         float  Voc = ((analogRead(VoltageAnalogInputPin)*moduleSupplyVoltageV)/1024.0) / (R2/(R1+R2)); //This will measure the open source voltage of solar panel while the capacitor behaves as open circuit in fully charged condition
                         digitalWrite(relay_pin, HIGH);
                         Serial.setTimeout(100);
                         }
                         
               
          }      
          
          
          
          /* 6 Recording & Calculation */
        
                if(InitiateReading == 1)                                                                    // If Left button is pressed to start measuring
                  {
                    if(InitialReset == 1)                                                                   /* set all electrical parameter to 0 to start measuring */
                      {
                        PmpInitial = 0;
                        IscInitial = 0;
                        VocInitial = 0;
                        Vmp = 0;
                        Imp = 0;
                        InitialReset = 0;
                        mySensorData=SD.open("IVCur.txt",FILE_WRITE);                                       // Open or create IVCur.txt on the SD card as a file to write to
                        if(mySensorData)                                                                    // only to do these if data file opened sucessfully
                          {
                           mySensorData.println("START");                                                   /* write the word "START" in the file */
                           mySensorData.close();
                          }
                      }
                  
                        /* 8 - SD memory card shield */
                  
                        mySensorData=SD.open("IVCur.txt",FILE_WRITE);                                       // Open or create IVCur.txt on the SD card as a file to write to
                        if(mySensorData)                                                                    // only to do these if data file opened sucessfully
                          {
                            mySensorData.println(V);
                            mySensorData.println(I);
                            mySensorData.println(PowerValue);
                            mySensorData.close();                                                           // close the file
                            Serial.println("written to SD Card !"); 
                          }
                          
                        recordLastSample = millis();
                      }
               
                  }
           }
        
