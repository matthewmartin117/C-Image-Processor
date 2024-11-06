## Image Processing Application #
### Overview ###
This C++ application is a command-line tool for basic image processing. 
It allows users to load a .bmp image, apply various filters and transformations, and save the modified image as a new file. 
The program offers a simple user interface to guide users through selecting different image processing options, including grayscale and edge detection, rotation, and other adjustments.

### Features ###
The application includes the following image processing features:

- Vignette - Applies a vignette effect to the image, darkening the edges.
  
- Clarendon - Adjusts the image with a filter effect for enhanced colors.
  
- Grayscale and Edge Detection - Converts the image to grayscale and applies edge detection using the Sobel operator.
  
- Rotate 90 Degrees - Rotates the image by 90 degrees clockwise.
  
- Rotate Multiple 90 Degrees - Allows users to rotate the image by multiple 90-degree increments.
  
- Enlarge - Increases the image size while maintaining proportions.
  
- High Contrast - Adjusts the image to high contrast.
  
- Lighten - Increases the brightness of the image.
  
- Darken - Decreases the brightness of the image.
  
- Black, White, Red, Green, Blue - Filters the image to isolate or highlight a specific color.

### How to Use ###
#### Requirements ####
- C++ compiler that supports C++11 or later.
  
- .bmp image files for processing.

1. Compile the program with a C++ compilier
   
   g++ -o image_processing_app main.cpp

2. Run the executable
   
   ./image_processing_app

### Program Flow Guide ###

1. When the program is started the user will be prompted to enter the filename of the image they wish to process. The program will automatically append .bmp to the filename.
The filename: sample is included for user convenience. 

2. After loading the image, the program displays a menu of processing options. Enter the corresponding number to apply a specific effect.

3. After processing, the program will prompt for a new filename to save the processed image. The new filename should be different from the original to avoid overwriting.

4. At any point, the user can load a different image by selecting option 0 in the menu.

5. To quit, enter Q at any prompt.


