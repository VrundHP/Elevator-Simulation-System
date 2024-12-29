#include "ECElevatorSim1.h"
#include "ECGraphicViewImp.h"
#include "ElevatorObserver.h"
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>
#include <memory>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>

// Function to read requests from input file
std::vector<ECElevatorSimRequest> ReadRequestsFromFile(const std::string &filename) {
    std::vector<ECElevatorSimRequest> requests;
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);
    }

    std::string line;
    int time, floorSrc, floorDest;

    // Read the input file line by line
    while (std::getline(inFile, line)) {
        if (line.empty() || line[0] == '#') {
            continue; // Skip comments and empty lines
        }

        std::istringstream iss(line);
        if (iss >> time >> floorSrc >> floorDest) {
            // Store each request for the simulation
            requests.emplace_back(time, floorSrc, floorDest);
        }
    }

    inFile.close();
    return requests;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <input_file> <output_file>" << std::endl;
        return 1;
    }

    std::string inputFilename = argv[1];
    std::string outputFilename = argv[2];

    // Initialize Allegro
    if (!al_init()) {
        std::cerr << "Failed to initialize Allegro." << std::endl;
        return -1;
    }
    al_init_primitives_addon();
    al_init_font_addon();
    al_init_ttf_addon();
    al_install_keyboard();

    // Create Allegro display
    ALLEGRO_DISPLAY *display = al_create_display(1000, 1000);
    if (!display) {
        std::cerr << "Failed to create display." << std::endl;
        return -1;
    }

    // Create Allegro timer for frame updates (10 FPS)
    ALLEGRO_TIMER *frame_timer = al_create_timer(1.0 / 10.0);
    ALLEGRO_EVENT_QUEUE *event_queue = al_create_event_queue();
    al_register_event_source(event_queue, al_get_timer_event_source(frame_timer));
    al_register_event_source(event_queue, al_get_display_event_source(display));
    al_register_event_source(event_queue, al_get_keyboard_event_source());

    // Run the backend to generate the simulation output file
    std::vector<ECElevatorSimRequest> requests = ReadRequestsFromFile(inputFilename);
    ECElevatorSim elevatorSim(5, requests);
    elevatorSim.Simulate(50, outputFilename);  // Corrected to use `outputFilename`

    // Create the graphical view
    ECGraphicViewImp graphicView(1000, 1000);

    // Create the ElevatorHandler with the output file to visualize the states
    std::cout << "Initializing ElevatorHandler..." << std::endl;
    ElevatorHandler elevatorHandler(graphicView, std::make_shared<ECElevatorSim>(elevatorSim), outputFilename); // Corrected to use `outputFilename`
    std::cout << "ElevatorHandler initialized." << std::endl;

    // Attach the ElevatorHandler as an observer to the graphic view
    graphicView.Attach(&elevatorHandler);

    // Start the timer and event loop
    al_start_timer(frame_timer);

    bool redraw = true;
    bool running = true;

    while (running && !elevatorHandler.IsSimulationComplete()) {
        ALLEGRO_EVENT ev;
        al_wait_for_event(event_queue, &ev);

        if (ev.type == ALLEGRO_EVENT_TIMER) {
            if (ev.timer.source == frame_timer) {
                redraw = true;

                // ElevatorHandler handles its updates internally
                elevatorHandler.Update();

                // Check if the simulation is complete
                if (elevatorHandler.IsSimulationComplete()) {
                    running = false;
                }
            }
        } else if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
            running = false;
        } else if (ev.type == ALLEGRO_EVENT_KEY_DOWN) {
            if (ev.keyboard.keycode == ALLEGRO_KEY_SPACE) {
                elevatorHandler.TogglePause();
            } else if (ev.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
                running = false;
            }
        }

        if (redraw && al_is_event_queue_empty(event_queue)) {
            redraw = false;

            // Draw scene
            elevatorHandler.DrawScene();
            al_flip_display();
        }
    }

    // Print the final simulation message if not already done
    if (!elevatorHandler.HasPrintedCompletionMessage()) {
        std::cout << "Simulation over." << std::endl;
    }

    // Clean up
    al_destroy_timer(frame_timer);
    al_destroy_event_queue(event_queue);
    al_destroy_display(display);

    return 0;
}