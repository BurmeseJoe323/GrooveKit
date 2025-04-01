## Building Project with CMake
1. clone tracktion engine
2. in the cmake directory, create JUCEPath.cmake and TracktionPath.cmake files. 
3. In JUCEPath.cmake add this line:
    - set(JUCE_DIR "/path/to/JUCE" CACHE PATH "Path to JUCE" FORCE)
      - replace with your actual path to JUCE 
    - set(TRACKTION_ENGINE_PATH "/path/to/tracktion_engine" CACHE PATH "Path to Tracktion Engine")
      - replace with your actual path to Tracktion
4. In your terminal project root directory, remove old cmake-build-debug file if it exists.
    - rm -rf cmake-build-debug
5. Build project:
    - cmake -B cmake-build-debug
    - cmake --build cmake-build-debug

### Note: When building JUCEPath.cmake was still cached and not ignored, to ignore it do the following
1. Before doing this, It may have actually solved itself when I removed it so check this once pulled
   a) git status
   b) do the above once you modified your JUCEPath.cmake to your needs
2. in your root directory call this
   a) git rm --cached path/to/JUCEPath.cmake
2. After that you should be good

## Building Project With Projucer
1. **Open Project** - Within Projucer open up the solution to the Project
2. **Link Tracktion Engine to Your Project** - To do this do these steps
   - Click the Dropdown menu for Modules on the left sidebar
   - Click on the plus sign within the drop down menu
   - Click add module from a specified folder
   - Find Tracktion in your file explorer and within the Tracktion root directory, do this
         - Find modules and click it
         - And then select and add tracktion_engine
   - Do the same steps to add tracktion_graph
   - It should be added now
3. **If tracktion_graph and tracktion_engine exists** - Click on tracktion_graph or tracktion_engine in projucer, then modify it's paths from there
4. **Open preferred IDE**
5. **Try Building it**
   - Note: You might get build errors, so here are some fixes for them
   - **BUILD WITH C++20 OR ELSE TRACKTION WON'T BUILD PROPERLY**
     - For Visual Studio,
          - go to project properties,
          - then go to c/c++ dropdown,
          - From there modify c++ standard language to c++20
   - **Extra Compilation Flags** - /bigobj
     - For Visual Studio
          - Go to project Properties
          - then go to c/c++ dropdown
          - Click on the command line from that dropdown
          - In the textbox named **Additional Options** add the following
            - /bigobj
