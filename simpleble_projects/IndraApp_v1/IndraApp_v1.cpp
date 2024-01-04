#include <chrono>
#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <bits/stdc++.h>

#include "../common/utils.hpp"

#include "simpleble/SimpleBLE.h"

using namespace std::chrono_literals;

int main() {

    // Define a constant string for the App Name which is the peripheral identifier we are looking for
    const std::string INSTALLER_APP_NAME = "IndraApp";

    // Define a constant string for the Service UUID
    const std::string SERVICE_UUID = "0a4b7471-21dd-4acd-a4c6-191fec072157";

    // Define a constant string for the Read Characteristic UUID
    const std::string CHARACTERISTIC_UUID = "9c099460-a0fb-46b5-b717-5a4873f913e5";

    // Define a constant string for the Response to the App that the process has succeeded
    const std::string RESPONSE_MESSAGE = "Data Transfer Success";

    // Define a Check String to find in the data from the App
    const std::string Check_String = "SSID";

    // Define a Constant Value to Scan for
    const std::uint8_t DELAY_TIME = 30;

    auto adapter_optional = Utils::getAdapter();

    if (!adapter_optional.has_value()) {
        return EXIT_FAILURE;
    }

    auto adapter = adapter_optional.value();

    std::vector<SimpleBLE::Peripheral> peripherals;

    adapter.set_callback_on_scan_start([]() { std::cout << "Scan started." << std::endl; });
    adapter.set_callback_on_scan_stop([]() { std::cout << "Scan stopped." << std::endl; });

    // Start the scan
    adapter.scan_start();

    // Set a boolean to indicate that the scan if happening
    bool Scan_Active = true;

    // Initialise a counter to measure the delay time
    uint8_t Counter = 0;

    // Initialise an Index to keep track of devices found
    uint8_t Device_Index = 0;

    // Initialise a string to hold the peripheral identifier
    std::string New_Identifier;

    // Declare a variable to store the index of the target device
    uint8_t Target_Index = 0;

    // Start a while loop for Delay Time and the Scan is Active
    while ((Counter < DELAY_TIME) && (Scan_Active))
    {
        adapter.set_callback_on_scan_found([&](SimpleBLE::Peripheral peripheral)
        {
            peripherals.push_back(peripheral);
            // Does the peripheral identifier match the App name
            if (peripheral.identifier() == INSTALLER_APP_NAME)
            {
                std::cout << "Found device Target" << std::endl;
                // Exit the while loop
                Scan_Active = false;
                // Stop the scan
                adapter.scan_stop();
                // This is the device we want so to keep this index value
                Target_Index = Device_Index;
            }
            else
            {
                // Not our device so increment the Device Index
                Device_Index++;
            }
        });

        // Add a delay
        std::this_thread::sleep_for(1s);

        // Increment the counter
        Counter++;
    }

    // If Scan Active is still false at this point then we have timed out so no
    // App was detected and so we should exit with an error
    if (Scan_Active)
    {
        std::cout << "Error: No Target Device Found" << std::endl;
        return EXIT_FAILURE;
    }

    // Scan was a success so select the device to connect to from vector list
    auto peripheral = peripherals[Target_Index];

    // Declare a boolean to check is we are connected to the App
    bool App_Connected = false;

    // Zero the counter
    Counter = 0;

    std::cout << "Trying to pair" << std::endl;

    // Start a while loop for Delay Time and check connection
    while ((Counter < DELAY_TIME) && (!App_Connected)) {
        // Try and connect but catch errors
        try
        {
            peripheral.connect();
        }
        catch(...)
        {
            // Add a short delay
            std::this_thread::sleep_for(1s);
        }
        // Check for a connection
        if (peripheral.is_connected())
        {
            // We are connected so exit the loop
            App_Connected = true;
            std::cout << "Target Connected" << std::endl;
        }
        else
        {
            // Add a short delay
            // std::this_thread::sleep_for(0.5s);
            // Increment the Counter
            Counter++;
        }
    }

    // If the App is not connect at the point we exit with an error
    if (!peripheral.is_connected())
    {
        std::cout << "Error: Target Device could not be connected" << std::endl;
        return EXIT_FAILURE;
    }

    // If we are connected then move on to scan for services
    // Initialise an Index to keep track of services found
    uint8_t Service_Index = 0;

    // Declare a variable to store the index if the UUIDs we want
    uint8_t Target_Uuid = 0;
    // Store all service and characteristic uuids in a vector.
    std::vector<std::pair<SimpleBLE::BluetoothUUID, SimpleBLE::BluetoothUUID>> uuids;

    for (auto service : peripheral.services())
    {
        for (auto characteristic : service.characteristics())
        {
            uuids.push_back(std::make_pair(service.uuid(), characteristic.uuid()));
            // Check to see if these characteristics match the ones we are looking for
            if ((service.uuid() == SERVICE_UUID) && (characteristic.uuid() == CHARACTERISTIC_UUID))
            {
                // Store the service number
                Target_Uuid = Service_Index;
                std::cout << "Read service detected" << std::endl;
            }
            else
            {
                // Increment Service Index
                Service_Index++;
            }
        }
    }

    // If no services were found then we should disconnect and return an error
    if (Service_Index == 0)
    {
        peripheral.disconnect();
        std::cout << "Error: Service not detected" << std::endl;
        return EXIT_FAILURE;
    }
    else
    {
        // Services Received to send feed back
        std::cout << "Services Detected" << std::endl;
    }

    // Set a boolean to indicate that the correct data has been received
    bool Data_Received = false;

    // Declare and String for the output
    std::string Output_String;

    // Zero the Counter
    Counter = 0;

    // Attempt to read the characteristic for the Delay Time.
    while ((Counter < DELAY_TIME) && (!Data_Received))
    {
        SimpleBLE::ByteArray rx_data;
        // Read the Data
        try
        {
            rx_data = peripheral.read(uuids[Target_Uuid].first, uuids[Target_Uuid].second);
        }
        catch (...)
        {
            // Add a delay
            std::this_thread::sleep_for(0.5s);
        }
        // Check if data has been received
        if (rx_data.length() > 0)
        {
            for (size_t i = 0; i < rx_data.length(); i++) {
                char ch = rx_data[i];
                Output_String += ch;
            }
            std::cout << Output_String << std::endl;
            // If the Output String contains the Check String we can stop
            if (Output_String.find(Check_String) != std::string::npos) {
                // Found the correct data, stop reading
                Data_Received = true;
            }
        }
        // Add a delay
        std::this_thread::sleep_for(0.5s);
        // Increment the counter
        Counter++;
    }

    // To Do
    // 1) Parse the received string to get the SSID and Password strings
    // 2) Write them to the wpa_supplicant.conf file
    // 3) Restart the WIFI(?)
    // 4) Test the WIFI connection
    // 5) Tell the App that the set-up is finished successfully (or not)

    // If we do not receive the correct data then disconnect and return an error
    if (!Data_Received)
    {
        peripheral.disconnect();
        std::cout << "Error: WIFI data incorrect" << std::endl;
        return EXIT_FAILURE;
    }

    // Parse the OutputString
    // Declare Strings for SSID and Password
    std::string SSID_String;
    std::string Pass_String;

    // Find the position of the & character
    size_t Position_And = Output_String.find('&');

    SSID_String = Output_String.substr(5, (Position_And - 5));
    // Add 8 to the number of characters to get the Password position
    Pass_String = Output_String.substr((Position_And + 10), Output_String.length());
    
    std::cout << SSID_String << std::endl;
    std::cout << Pass_String << std::endl;

    // Send the Response Back to the App
    // `write_request` is for unacknowledged writes.
    // `write_command` is for acknowledged writes.
    try
    {
        peripheral.write_request(uuids[Target_Uuid].first, uuids[Target_Uuid].second, RESPONSE_MESSAGE);
    }
    catch (...)
    {
        std::cout << "Error: Response Not Sent" << std::endl;
    }

    std::cout << "Response Sent" << std::endl;

    peripheral.disconnect();

    return EXIT_SUCCESS;
}
