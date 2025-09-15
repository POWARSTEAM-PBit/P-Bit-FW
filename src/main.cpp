#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <Arduino.h>
#include <esp_system.h>
#include <string.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define REQUEST_MSG 0x01
#define REPLY_MSG 0x02
#define MAC_ADDR_LEN 6
#define ERR_UNK_MAC 1
#define ERR_INVALID_LEN 2
#define NUM_SENSORS 5  // Total number of sensors

// Sensor types
#define SENSOR_TYPE_TEMPERATURE 0x01
#define SENSOR_TYPE_HUMIDITY    0x02
#define SENSOR_TYPE_PRESSURE    0x03
#define SENSOR_TYPE_LIGHT       0x04
#define SENSOR_TYPE_BATTERY     0x05

// Global pointer to characteristic for sending responses
BLECharacteristic *pCharacteristic;
uint8_t device_mac[MAC_ADDR_LEN];

bool compare_mac_addr(uint8_t * received_mac) {
    return memcmp(received_mac, device_mac, MAC_ADDR_LEN) == 0;
}

// Generate random sensor value based on sensor type
uint16_t generateSensorValue(uint8_t sensor_type) {
    switch(sensor_type) {
        case SENSOR_TYPE_TEMPERATURE:
            return random(150, 350);  // 15.0 to 35.0 degrees (scaled by 10)
        case SENSOR_TYPE_HUMIDITY:
            return random(300, 800);  // 30% to 80% humidity (scaled by 10)
        case SENSOR_TYPE_PRESSURE:
            return random(9800, 10200); // 98.0 to 102.0 kPa (scaled by 100)
        case SENSOR_TYPE_LIGHT:
            return random(0, 1000);   // 0 to 1000 lux
        case SENSOR_TYPE_BATTERY:
            return random(0, 100);    // 0% to 100% battery
        default:
            return random(0, 65535);  // Random 16-bit value
    }
}

/***
 * Reply packet format:
 * Byte 0: REPLY_MSG (0x02)
 * Byte 1: error_code (0=success, 1=unknown MAC, 2=invalid length)
 * If error_code == 0 (success), for each sensor:
 *   Byte N: sensor_type
 *   Bytes N+1, N+2: sensor_value (16-bit, little endian)
 * Total success packet: 2 + (3 * NUM_SENSORS) = 17 bytes
 ***/
void send_reply(int error_code)
{
    const int packet_size = 2 + (3 * NUM_SENSORS);
    uint8_t reply_msg[packet_size];  // Message type + error code + (type + 2-byte value) * NUM_SENSORS
    int reply_length = 2;  // Default length for error replies
    
    reply_msg[0] = REPLY_MSG;
    reply_msg[1] = error_code;
    
    // If no error, add all sensor data
    if (error_code == 0) {
        Serial.println("Generating all sensor data...");
        
        const char* sensor_names[] = {"", "Temperature", "Humidity", "Pressure", "Light", "Battery"};
        uint8_t sensor_types[] = {SENSOR_TYPE_TEMPERATURE, SENSOR_TYPE_HUMIDITY, 
                                 SENSOR_TYPE_PRESSURE, SENSOR_TYPE_LIGHT, SENSOR_TYPE_BATTERY};
        
        int byte_index = 2;  // Start after message type and error code
        
        // Add all sensors to the packet
        for (int i = 0; i < NUM_SENSORS; i++) {
            uint8_t sensor_type = sensor_types[i];
            uint16_t sensor_value = generateSensorValue(sensor_type);
            
            // Add sensor type
            reply_msg[byte_index] = sensor_type;
            byte_index++;
            
            // Add sensor value (little endian)
            reply_msg[byte_index] = sensor_value & 0xFF;        // Low byte
            reply_msg[byte_index + 1] = (sensor_value >> 8) & 0xFF; // High byte
            byte_index += 2;
            
            // Log each sensor
            Serial.printf("Sensor %d: Type=%d (%s), Value=%d (0x%04X)\n", 
                         i+1, sensor_type, sensor_names[sensor_type], sensor_value, sensor_value);
        }
        
        reply_length = byte_index;
        Serial.printf("Total sensors in packet: %d\n", NUM_SENSORS);
    } else {
        Serial.printf("Sending error reply: %d\n", error_code);
    }
    
    std::string reply_string((char*)reply_msg, reply_length);
    pCharacteristic->setValue(reply_string);
    pCharacteristic->notify();
    
    Serial.printf("Reply sent: %d bytes\n", reply_length);
    
    // Print raw packet bytes for debugging
    Serial.print("Raw packet: ");
    for (int i = 0; i < reply_length; i++) {
        Serial.printf("0x%02X ", reply_msg[i]);
    }
    Serial.println();
}

class MyCallbacks : public BLECharacteristicCallbacks
{
    private:
        int packet_count;
        unsigned long last_request_time;

    public:
        MyCallbacks() {
            packet_count = 0;
            last_request_time = 0;
        }
        
        void onWrite(BLECharacteristic *pCharacteristic)
        {
            std::string value = pCharacteristic->getValue();
            size_t msg_len = value.length();
            uint8_t mac_addr[MAC_ADDR_LEN];
            char msg_type;
            size_t i;
            int error_code = 0;
            
            // Update statistics
            packet_count++;
            last_request_time = millis();

            printf("%d\n", msg_len);
            
            if (msg_len > 0)
            {
                msg_type = value[0];
                if (msg_type == REQUEST_MSG)
                {
                    logRequest(msg_len);
                    
                    if (msg_len != MAC_ADDR_LEN + 1)
                    {
                        printf("Invalid packet\n");
                        error_code = ERR_INVALID_LEN;
                    }
                    else
                    {
                        for (i = 1; i < msg_len; i++)
                        {
                            mac_addr[i - 1] = value[i];
                        }
                        
                        // MAC checking is commented out - always accept valid length packets
                        // if (!compare_mac_addr(mac_addr))
                        // {
                        //     printf("Invalid mac addr in the pkt\n");
                        //     error_code = ERR_UNK_MAC;
                        //     logMacMismatch(mac_addr);
                        // }
                        // else
                        // {
                        //     logMacMatch();
                        // }
                    }

                    send_reply(error_code);
                }
            }
        }
        
        // Custom method to log request details
        void logRequest(size_t length) 
        {
            Serial.printf("[%lu] Request #%d - Length: %d bytes\n", 
                        millis(), packet_count, length);
        }
        
        // Custom method to log MAC mismatches
        void logMacMismatch(uint8_t* received_mac) {
            Serial.print("MAC Mismatch - Received: ");
            printMac(received_mac);
            Serial.print(", Expected: ");
            printMac(device_mac);
        }
        
        // Custom method to log successful MAC matches
        void logMacMatch() {
            Serial.printf("[%lu] ✓ MAC Address verified successfully!\n", millis());
        }
        
        // Helper method to print MAC addresses
        void printMac(uint8_t* mac) {
            for (int i = 0; i < MAC_ADDR_LEN; i++) {
                Serial.printf("%02X", mac[i]);
                if (i < MAC_ADDR_LEN - 1) Serial.print(":");
            }
            Serial.println();
        }
        
        // Method to get statistics
        void printStats() {
            Serial.printf("=== Statistics ===\n");
            Serial.printf("Total packets: %d\n", packet_count);
            Serial.printf("Last request: %lu ms ago\n", 
                        millis() - last_request_time);
            Serial.printf("Uptime: %lu ms\n", millis());
        }
        
        // Method to reset statistics
        void resetStats() {
            packet_count = 0;
            last_request_time = 0;
            Serial.println("Statistics reset");
        }
        
        // Method to check if device is active
        bool isRecentActivity(unsigned long timeout_ms = 30000) {
            return (millis() - last_request_time) < timeout_ms;
        }
        
        // Method to validate packet format (could be extended for other message types)
        bool validatePacket(const std::string& value) {
            if (value.length() == 0) return false;
            
            char msg_type = value[0];
            switch (msg_type) {
                case REQUEST_MSG:
                    return value.length() == MAC_ADDR_LEN + 1;
                case REPLY_MSG:
                    return value.length() == 2;
                default:
                    return false;
            }
        }
};

// Global instance
MyCallbacks* myCallbacks;

void setup() {
    Serial.begin(115200);
    Serial.println("Starting BLE work!");
    
    // Initialize random seed
    randomSeed(analogRead(0) + millis());
    
    esp_read_mac(device_mac, ESP_MAC_WIFI_STA);
    
    BLEDevice::init("Long name works now");
    BLEServer *pServer = BLEDevice::createServer();
    BLEService *pService = pServer->createService(SERVICE_UUID);
    
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ |
        BLECharacteristic::PROPERTY_WRITE |
        BLECharacteristic::PROPERTY_NOTIFY
    );
    
    pCharacteristic->setValue("Hello World says Neil");
    
    // Create instance and set callbacks
    myCallbacks = new MyCallbacks();
    pCharacteristic->setCallbacks(myCallbacks);
    
    pService->start();
    
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06);
    pAdvertising->setMinPreferred(0x12);
    
    BLEDevice::startAdvertising();
    Serial.println("Characteristic defined! Now you can read/write it in your phone!");
}

void loop() {
    static unsigned long last_stats = 0;
    
    // Print stats every 30 seconds
    if (millis() - last_stats > 30000) {
        myCallbacks->printStats();
        last_stats = millis();
    }
    
    delay(2000);
}