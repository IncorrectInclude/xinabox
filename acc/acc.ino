#include <xCore.h>
#include <xSI01.h> 
#include <xSL01.h>
#include <xSW01.h>
#include <xSN01.h>
#include <xSG33.h>

void dumpSI01(void);
// light sensor 
void dumpSL01(void);
// weather sensor (pressure, humidity, rough altitude)
void dumpSW01(void);
// gps sensor (location)
void dumpSN01(void);
void dumpSG33(void);

/*
 * {
 *  "name": "SI01",
 *  "classname: "xSI01",
 *  "github_repository": "https://github.com/xinabox/arduino-SI01",
 *  "header_src": "https://github.com/xinabox/arduino-SI01/blob/master/src/xSI01.h",
 *  "description": "Triple Axis Accelerometer, Magnetometer and Gyroscope",
 *  "product_page": "https://xinabox.cc/products/SI01/",
 * }
 */
xSI01 SI01;

/*
 * {
 *  "name": "SL01",
 *  "classname: "xSL01",
 *  "github_repository": "https://github.com/xinabox/arduino-SL01",
 *  "header_src": "https://github.com/xinabox/arduino-SL01/blob/master/src/xSL01.h",
 *  "description": "Measures luminosity(visual brightness) and UV radiation",
 *  "product_page": "https://xinabox.cc/products/SL01/",
 * }
 */
xSL01 SL01;

/*
 * {
 *  "name": "SL01",
 *  "classname: "xSL01",
 *  "github_repository": "https://github.com/xinabox/arduino-SL01",
 *  "header_src": "https://github.com/xinabox/arduino-SL01/blob/master/src/xSL01.h",
 *  "description": "Measures luminosity(visual brightness) and UV radiation",
 *  "product_page": "https://xinabox.cc/products/SL01/",
 * }
 */
xSW01 SW01;

/*
 * {
 *  "name": "SN01",
 *  "classname: "xSN01",
 *  "github_repository": "https://github.com/xinabox/arduino-SN01",
 *  "header_src": "https://github.com/xinabox/arduino-SN01/blob/master/src/xSN01.h",
 *  "description": "GPS Based location, time, date, and location helpers.",
 *  "product_page": "https://xinabox.cc/products/SN01/",
 * }
 */
xSN01 SN01;
/*
 * {
 *  "name": "SG33",
 *  "classname: "xSG33",
 *  "github_repository": "https://github.com/xinabox/arduino-SG33",
 *  "header_src": "https://github.com/xinabox/arduino-SN01/blob/master/src/xSG33.h",
 *  "description": "Digital gas sensor (measures CO2)",
 *  "product_page": "https://xinabox.cc/products/SG33/",
 * }
 */
xSG33 SG33;

namespace time_duration { 
  int seconds(int seconds) { 
    return seconds * 1000;
  }

  int minutes(int minutes) { 
    return seconds(minutes * 60);
  }

  int hours(int hours) { 
    return minutes(hours * 60);
  }

  int milliseconds(int milliseconds) { 
    return milliseconds;
  }
}
class IntervalFunction {
  private:
    void (*const func_pointer)(void);
    const uint16_t interval; 
    uint16_t lastCalled = 0;

    
    /**
     * Sets the last time, in milliseconds since
     * this interval function was called (if the time was 
     * was passed to the call, as is the case with callIfRequired).
     * 
     * @param called the time, in milliseconds that this function was last called at
     */
    void setLastCalled(uint16_t called) { 
      lastCalled = called;
    }

  public: 
    /**
     * Constructs an interval function
     * 
     * @param func_pointer a pointer to a void function
     * @param interval how often (in milliseconds) to call that function. 
     */
    IntervalFunction(void(*const func_pointer)(void), uint16_t interval) : interval(interval), func_pointer(func_pointer) {
    }

    /**
     * Gets the time (in milliseconds) since this
     * function was last called
     * 
     * @returns the time in milliseconds at which this function was last called
     */
    uint16_t getLastCalled() {
       return lastCalled;
    }

    /**
     * Call this function if it is the appropriate time to call
     * it (if enough time has elapsed).
     * 
     * @param currentMilliseconds the current time in milliseconds
     * 
     */
    void callIfRequired(int currentMilliseconds) {
      if (currentMilliseconds - getLastCalled() >= getInterval()) {
        call(currentMilliseconds);
      }
    }

    /**
     * Get how often (in milliseconds) that this 
     * function is supposed to be called
     */
    uint16_t getInterval() {
      return interval;
    }

    /**
     * Call this interval function, NOW
     * 
     * Useful for one-off calls.
     */
    void call(){ 
      (*this->func_pointer)();
    }

    /**
     * Call this interval function, NOW,
     * and update the last time it was called, 
     * to match this.
     * 
     * Useful if it needs to be called, out 
     * of schedule, otherwise the scheduler
     * (eventually) calls this.
     * 
     * @param currentMilliseconds the current time(in milliseconds)
     */
    void call(int currentMilliseconds){
      call();
      setLastCalled(currentMilliseconds);  
    }
};

// define how many interval functions we are going to use
#define INTERVAL_FUNCTION_COUNT 5

// create an array of the interval functions
IntervalFunction intervalFunctions[INTERVAL_FUNCTION_COUNT] = {
  // create an interval function object out of the function dumpSI01 and call it every n milliseconds
  IntervalFunction(/* call this function: */ dumpSI01, /* every */ time_duration::seconds(20)),
  IntervalFunction(/* call this function: */ dumpSL01, /* every */ time_duration::seconds(20)),
  IntervalFunction(/* call this function: */ dumpSW01, /* every */ time_duration::seconds(20)),
  IntervalFunction(/* call this function: */ dumpSN01, /* every */ time_duration::seconds(20)),
  IntervalFunction(/* call this function: */ dumpSG33, /* every */ time_duration::seconds(20)),
};


/**
 * This class allows for the repeated calling of 
 * interval functions on demand.
 * Just call Schedular::execute() each loop through
 * the loop function, and it may, or may not, depending
 * on whether it's time, call the interval function in 
 * the interval functions array;
 */
class Scheduler { 
   public:
   /**
    * Executes the interval functions if it is 
    * time for that function to be executed.
    */
    static void execute() {
      for (int i = 0; i < INTERVAL_FUNCTION_COUNT; i++) { 
        intervalFunctions[i].callIfRequired(millis());
      }
    }
};

// Time values that are used for multitasking
#define PRINT_SPEED 1000
static unsigned long lastPrint = 0;
// End of section

class Vec3 { 
  private:
    float x;
    float y; 
    float z;
  public: 
    Vec3(float x, float y, float z){ 
      this->x = x;
      this->y = y;
      this->z = z;
    }

    void dumpSerial(const char* cstring) { 
      Serial.print("[");
      Serial.print(cstring);
      Serial.print("]: ");
      Serial.print("{ x: ");
      Serial.print(x, 2);
      Serial.print(", y: ");
      Serial.print(y, 2);
      Serial.print(", z: ");
      Serial.print(z, 2);
      Serial.println(" }");
    }
    void dumpSerial(){ 
      dumpSerial("[anonymous vec]: ");
    }
};

void setup() {
  // Start the Serial Monitor at 115200 BAUD
  Serial.begin(115200);
 
  // Set the I2C Pins for CW01
  Wire.pins(2, 14);
  Wire.setClockStretchLimit(15000);
  Wire.begin();

 
  if (!SL01.begin()) { 
    Serial.println("Failed to communicate with SL01.");
    Serial.println("Check the Connector");
  }
  if (!SI01.begin()) {    // Check if the sensor is working
    Serial.println("Failed to communicate with SI01.");
    Serial.println("Check the Connector");
  }

  delay(4000);
  if (!SW01.begin()) {    // Check if the sensor is working
    Serial.println("Failed to communicate with SW01.");
    Serial.println("Check the Connector");
  }

  if (!SN01.begin()) {    // Check if the sensor is working
    Serial.println("Failed to communicate with SN01.");
    Serial.println("Check the Connector");
  }

  if (!SG33.begin()) {    // Check if the sensor is working
    Serial.println("Failed to communicate with SG33.");
    Serial.println("Check the Connector");
  }
}


void loop() {
  Scheduler::execute();
}

void dumpSI01() { 
    // Read and calculate data from SL01 sensor
    SI01.poll();
    
    Serial.println("-------   <SI01>  ---------");
    printGyro();
    printAccel();
    printMag();
    printAttitude();
  Serial.println("-------  </SI01>  ---------");
}


void dumpSG33() { 
    // Create a variable to store the data read from SG33
  int eCO2, TVOC;

  // Read and calculate data from SG33 sensor
    Serial.println("-------   <SG33>  ---------");
  if (SG33.dataAvailable())
  {
    SG33.getAlgorithmResults();
    SG33.setEnvironmentData(SW01.getHumidity(), SW01.getTempC());
    
    // Read the data from the SG33
    eCO2 = SG33.getCO2();
    TVOC = SG33.getTVOC();

    // Display the recoreded data over the Serial Monitor
    Serial.print("eCO2: ");
    Serial.println(eCO2);
    Serial.print("TVOC: ");
    Serial.println(TVOC);
  }

  Serial.println("-------  </SG33>  ---------");
}

void dumpSW01() { 
  // dumps the weather sensor

  Serial.println("-------   <SW01>  ---------");
  SW01.poll();
  Serial.print("Altitude (QNE): ");
  Serial.println(SW01.getQNE());
  Serial.print("Temperature (Celsius): ");
  Serial.println(SW01.getTempC());
  Serial.print("Humidity (rel %): ");
  Serial.println(SW01.getHumidity());
  Serial.print("Pressure (pascals): ");
  Serial.println(SW01.getPressure());
  Serial.print("Dew Point (celsius): ");
  Serial.println(SW01.getDewPoint());
  Serial.println("-------  </SW01>  ---------");
}

void dumpSN01() {
  SN01.poll();
   
  String date = SN01.getDate();
  String gpsTime = SN01.getTime();
  long latitude = SN01.getLatitude();
  long longitude = SN01.getLongitude();

  Serial.println("-------   <SN01>  ---------");
  Serial.print("Date: ");
  Serial.println(date);
  Serial.print("GPS Time: ");
  Serial.println(gpsTime);
  Serial.print("Location: { ");
  Serial.print(latitude);
  Serial.print(", ");
  Serial.print(longitude);
  Serial.println(" }");
  Serial.println("-------  </SN01>  ---------");
}
void dumpSL01() {
    float lux;
    lux = 0;

    // Poll Sensor for collect data
    SL01.poll();

    // Request SL01 to return calculated UVB intensity
    lux = SL01.getLUX();


  Serial.println("-------  <SL01>  ---------");
    // Display Data on the Serial monitor
    Serial.print("Ambient Light Level: ");
    Serial.print(lux);
    Serial.println(" LUX");
    
  Serial.println("-------  </SL01>  ---------");

}
void printGyro() {
  // create a vector of our data
  Vec3 magnometer(SI01.getGX(), SI01.getGY(), SI01.getGZ());
  // dump data to serial
  magnometer.dumpSerial("gyro");
}

void printAccel() {
  // create a vector of our data
  Vec3 magnometer(SI01.getAX(), SI01.getAY(), SI01.getAZ());
  // dump data to serial
  magnometer.dumpSerial("acc");
}

void printMag() {
  // create a vector of our data
  Vec3 magnometer(SI01.getMX(), SI01.getMY(), SI01.getMZ());
  // dump data to serial
  magnometer.dumpSerial("mag");
}

void printAttitude() {
  Serial.print("Roll: ");
  Serial.println(SI01.getRoll(), 2);
  Serial.print("Pitch :");
  Serial.println(SI01.getPitch(), 2);
  Serial.print("GForce :");
  Serial.println(SI01.getGForce(), 2);
}
