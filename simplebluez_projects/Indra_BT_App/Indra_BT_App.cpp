
#include <simplebluez/Bluez.h>
#include <simplebluez/Exceptions.h>

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <thread>
#include <cstdbool>

SimpleBluez::Bluez bluez;

std::atomic_bool async_thread_active = true;
void async_thread_function() {
    while (async_thread_active) {
        bluez.run_async();
        std::this_thread::sleep_for(std::chrono::microseconds(100));
    }
}

void millisecond_delay(int ms) {
    for (int i = 0; i < ms; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

void print_byte_array(SimpleBluez::ByteArray& bytes) {
    for (auto byte : bytes) {
        std::cout << std::hex << std::setfill('0') << (uint32_t)((uint8_t)byte) << " ";
        break;
    }
    std::cout << std::endl;
}

std::vector<std::shared_ptr<SimpleBluez::Device>> peripherals;

int main(int argc, char* argv[]) {

    // Define a constant string for the App Name which is the peripheral identifier we are looking for
    const std::string INSTALLER_APP_NAME = "IndraApp";

    // Define a constant string for the Service UUID
    const std::string SERVICE_UUID = "0a4b7471-21dd-4acd-a4c6-191fec072157";

    // Define a constant string for the Read Characteristic UUID
    const std::string CHARACTERISTIC_UUID = "9c099460-a0fb-46b5-b717-5a4873f913e5";

    // Define a Constant Value to Scan for
    const std::uint8_t DELAY_TIME = 30;

    bluez.init();
    std::thread* async_thread = new std::thread(async_thread_function);

    // I do not know if we need this but will leave it in for now
    auto agent = bluez.get_agent();
    // This is to set the agent so no codes are required, at least from this end
    // still possible that pass/cancel option appears on the phone
    agent->set_capabilities(SimpleBluez::Agent::Capabilities::NoInputNoOutput);

    // Declare a vector and put the adapters in it
    auto adapters = bluez.get_adapters();
    std::cout << "Checking adapters:" << std::endl;

    // If there is more than one adapter then we have a problem so should stop
    if ( adapters.size() > 1 )
    {
        std::cout << "Error; more than one adapter found" << std::endl;
        return EXIT_FAILURE;
    }
    for (int i = 0; i < adapters.size(); i++) {
        std::cout << "[" << i << "] " << adapters[i]->identifier() << " [" << adapters[i]->address() << "]"
                  << std::endl;
    }

    auto adapter = adapters[0];

    std::cout << "Scanning " << adapter->identifier() << " [" << adapter->address() << "]" << std::endl;

    SimpleBluez::Adapter::DiscoveryFilter filter;
    filter.Transport = SimpleBluez::Adapter::DiscoveryFilter::TransportType::LE;
    adapter->discovery_filter(filter);


    // Start discovering the available peripherals
    adapter->discovery_start();
    std::cout << "Scanning for Peripheral Devices" << std::endl;

    adapter->set_on_device_updated([](std::shared_ptr<SimpleBluez::Device> device) {
        if (std::find(peripherals.begin(), peripherals.end(), device) == peripherals.end()) {
            std::cout << "Found device: " << device->name() << " [" << device->address() << "]" << std::endl;
            peripherals.push_back(device);
        }
    });

    // Pause to allow device discovery
    millisecond_delay(2000);
    adapter->discovery_stop();

    uint8_t target_device = 0;

    std::cout << "The following devices were found:" << std::endl;
    for (int i = 0; i < peripherals.size(); i++) {
        std::cout << "[" << i << "] " << peripherals[i]->name() << " [" << peripherals[i]->address() << "]"
                  << std::endl;
        if (peripherals[i]->name() == INSTALLER_APP_NAME)
        {
            std::cout << "Target Acquired" << std::endl;
            target_device =i;
        }
    }

    if (target_device >= 0 && target_device < peripherals.size()) {
        auto peripheral = peripherals[target_device];
        std::cout << "Connecting to " << peripheral->name() << " [" << peripheral->address() << "]" << std::endl;

        int counter = 0;
        bool target_connect = false;

        while ((counter < 30) && (target_connect == false))
        {
            try 
            {
                peripheral->connect();
                millisecond_delay(500);
            } 
            catch (SimpleDBus::Exception::SendFailed& e) 
            {
                millisecond_delay(500);
            }
            if (peripheral->connected())
            {
                std::cout << "target connected" << std::endl;
                target_connect = true;
                millisecond_delay(500);
            }
            else
            {
                counter++;
            }
        }

        // Reset the counter
        counter = 0;

        while ((!peripheral->services_resolved()) && (counter < DELAY_TIME))
        {
            millisecond_delay(50);
            counter++;
        }

        // Add a short delay
        millisecond_delay(5000);

        // Store all services and characteristics in a vector.
        std::vector<std::pair<std::shared_ptr<SimpleBluez::Service>, std::shared_ptr<SimpleBluez::Characteristic>>>
                char_list;
        for (auto service : peripheral->services()) {
            for (auto characteristic : service->characteristics()) {
                char_list.push_back(std::make_pair(service, characteristic));
            }
        }

        // Add a short delay
        millisecond_delay(500);

        // Reset the counter
        counter = 0;

        std::cout << "The following services and characteristics were found:" << std::endl;
        for (int i = 0; i < char_list.size(); i++) {
            std::cout << "[" << i << "] " << char_list[i].first->uuid() << " " << char_list[i].second->uuid()
                      << std::endl;
            // If the service amd characteristic match this is our servce
            if ((char_list[i].first->uuid() == SERVICE_UUID) && (char_list[i].second->uuid() == CHARACTERISTIC_UUID))
            {
                while (counter < 5)
                {
                    try {
                        millisecond_delay(1000);
                        SimpleBluez::ByteArray value = char_list[i].second->read();
                        std::cout << "Characteristic contents were: ";
                        print_byte_array(value);
                    }
                    catch (SimpleDBus::Exception::SendFailed& e) {
                        millisecond_delay(1000);
                    }
                    counter++;
                    std::cout << counter << std::endl;
                }
            }
        }

        // Reset the counter
        counter = 0;

        peripheral->disconnect();

        // Sleep for an additional second before returning.
        // If there are any unexpected events, this example will help debug them.
        millisecond_delay(1000);
    }

    async_thread_active = false;
    while (!async_thread->joinable()) {
        millisecond_delay(10);
    }
    async_thread->join();
    delete async_thread;

    return 0;
}
