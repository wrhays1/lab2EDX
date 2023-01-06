#include "mbed.h"
#include "ble/BLE.h"
#include "stm32l475e_iot01_accelero.h"
#include "stm32l475e_iot01_gyro.h"
#include "stm32l475e_iot01_hsensor.h"
#include "stm32l475e_iot01_magneto.h"
#include "stm32l475e_iot01_psensor.h"
#include "stm32l475e_iot01_qspi.h"
#include "stm32l475e_iot01_tsensor.h"
#include "stm32l475e_iot01.h"
#include "BufferedSerial.h"
#include "DigitalOut.h"
#include "mbed_events.h"
#include "gatt_server_process.h"
#include "gatt_client_process.h"
#include <cstdint>


void sensorsystemInit();
static void readSensors();
static BufferedSerial pc(USBTX, USBRX, 9600);  


//class definitions

class Lab3ServerProcess : public BLEProcess{
    public:
    Lab3ServerProcess(events::EventQueue &event_queue, BLE &ble_interface)  : 
    BLEProcess(event_queue,  ble_interface)  {}
    const char *get_device_name() override {

    static const char   name[] = "\nLab 3 Server....\n";
        
        return name;
    }

};


class SensorReadingCharacteristic : public GattCharacteristic     {

    public:
    SensorReadingCharacteristic(const UUID &uuid) : 
     GattCharacteristic(uuid, (uint8_t *)&stringRepr_, sizeof(stringRepr_), 
        sizeof(stringRepr_),GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
         GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY),
         internalValue_(0) {
             updateStringRepr();

         }
         void updateValue(BLE &ble, float newValue){
             internalValue_ = newValue;
             updateStringRepr();
              ble.gattServer().write(getValueHandle(), (uint8_t *)&stringRepr_,
              sizeof(stringRepr_));

         }
         
       
    private:

    float internalValue_;
    uint8_t stringRepr_[16];

    void updateStringRepr(){
        sprintf((char*)stringRepr_, "%f", internalValue_ );

    }
    
};



class Lab3Server : ble::GattServer::EventHandler {

    public:
    Lab3Server() :
    temperature_(GattCharacteristic::UUID_TEMPERATURE_CHAR),
    humidity_(GattCharacteristic::UUID_HUMIDITY_CHAR),
    pressure_(GattCharacteristic::UUID_PNP_ID_CHAR) {}

   ~Lab3Server(){}

    void start (BLE &ble, events::EventQueue &event_queue){
        const UUID uuid = GattService::UUID_ENVIRONMENTAL_SERVICE;

        GattCharacteristic *charTable [] = {&temperature_, &humidity_, &pressure_};

        GattService sensorService(uuid, charTable, sizeof(charTable)/sizeof(charTable[0]));

        ble.gattServer().addService(sensorService);
        ble.gattServer().setEventHandler(this); 
       
        printf("\nService Started......\n");
        event_queue.call_every(1000ms, [this, &ble] {updateSensors(ble);});

    }
    void updateSensors(BLE &ble){

        printf("Updating Sensors.....\n");

       temperature_.updateValue(ble, BSP_TSENSOR_ReadTemp());
       humidity_.updateValue(ble, BSP_HSENSOR_ReadHumidity());
       pressure_.updateValue(ble, BSP_PSENSOR_ReadPressure());


    }
    private:
      SensorReadingCharacteristic temperature_;
      SensorReadingCharacteristic humidity_;
      SensorReadingCharacteristic pressure_;

};


static EventQueue event_queue (10*EVENTS_EVENT_SIZE);


// main() runs in its own thread in the OS

int main()
{ 
    sensorsystemInit();
    BLE  &ble = BLE::Instance();
    printf ("Lab3 - BLE\n");

    Lab3ServerProcess ble_process (event_queue, ble);
    Lab3Server server;
    ble_process.on_init(callback(&server, &Lab3Server::start));
    ble_process.start();
   
     
  
    return 0;
}
      
void sensorsystemInit(){

     BSP_TSENSOR_Init();
     BSP_HSENSOR_Init();
     BSP_PSENSOR_Init();
  //   BSP_MAGNETO_Init();
   //  BSP_ACCELERO_Init();
  //   BSP_GYRO_Init();
    
}





static void readSensors(){

    float temp = BSP_TSENSOR_ReadTemp();
  
    float hum = BSP_HSENSOR_ReadHumidity();
   
    float pres = BSP_PSENSOR_ReadPressure();
  

}