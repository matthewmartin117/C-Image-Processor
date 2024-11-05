#include <iostream>
#include <vector>
#include <fstream>
#include <cmath>
#include <algorithm>
using namespace std;

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION BELOW                                    //
//***************************************************************************************************//

// Pixel structure
struct Pixel
{
    // Red, green, blue color values
    int red;
    int green;
    int blue;
};

/**
 * Gets an integer from a binary stream.
 * Helper function for read_image()
 * @param stream the stream
 * @param offset the offset at which to read the integer
 * @param bytes  the number of bytes to read
 * @return the integer starting at the given offset
 */ 
int get_int(fstream& stream, int offset, int bytes)
{
    stream.seekg(offset);
    int result = 0;
    int base = 1;
    for (int i = 0; i < bytes; i++)
    {   
        result = result + stream.get() * base;
        base = base * 256;
    }
    return result;
}

/**
 * Reads the BMP image specified and returns the resulting image as a vector
 * @param filename BMP image filename
 * @return the image as a vector of vector of Pixels
 */
vector<vector<Pixel>> read_image(string filename)
{
    // Open the binary file
    fstream stream;
    stream.open(filename, ios::in | ios::binary);

    // Get the image properties
    int file_size = get_int(stream, 2, 4);
    int start = get_int(stream, 10, 4);
    int width = get_int(stream, 18, 4);
    int height = get_int(stream, 22, 4);
    int bits_per_pixel = get_int(stream, 28, 2);

    // Scan lines must occupy multiples of four bytes
    int scanline_size = width * (bits_per_pixel / 8);
    int padding = 0;
    if (scanline_size % 4 != 0)
    {
        padding = 4 - scanline_size % 4;
    }

    // Return empty vector if this is not a valid image
    if (file_size != start + (scanline_size + padding) * height)
    {
        return {};
    }

    // Create a vector the size of the input image
    vector<vector<Pixel>> image(height, vector<Pixel> (width));

    int pos = start;
    // For each row, starting from the last row to the first
    // Note: BMP files store pixels from bottom to top
    for (int i = height - 1; i >= 0; i--)
    {
        // For each column
        for (int j = 0; j < width; j++)
        {
            // Go to the pixel position
            stream.seekg(pos);

            // Save the pixel values to the image vector
            // Note: BMP files store pixels in blue, green, red order
            image[i][j].blue = stream.get();
            image[i][j].green = stream.get();
            image[i][j].red = stream.get();

            // We are ignoring the alpha channel if there is one

            // Advance the position to the next pixel
            pos = pos + (bits_per_pixel / 8);
        }

        // Skip the padding at the end of each row
        stream.seekg(padding, ios::cur);
        pos = pos + padding;
    }

    // Close the stream and return the image vector
    stream.close();
    return image;
}

/**
 * Sets a value to the char array starting at the offset using the size
 * specified by the bytes.
 * This is a helper function for write_image()
 * @param arr    Array to set values for
 * @param offset Starting index offset
 * @param bytes  Number of bytes to set
 * @param value  Value to set
 * @return nothing
 */
void set_bytes(unsigned char arr[], int offset, int bytes, int value)
{
    for (int i = 0; i < bytes; i++)
    {
        arr[offset+i] = (unsigned char)(value>>(i*8));
    }
}

/**
 * Write the input image to a BMP file name specified
 * @param filename The BMP file name to save the image to
 * @param image    The input image to save
 * @return True if successful and false otherwise
 */
bool write_image(string filename, const vector<vector<Pixel>>& image)
{
    // Get the image width and height in pixels
    int width_pixels = image[0].size();
    int height_pixels = image.size();

    // Calculate the width in bytes incorporating padding (4 byte alignment)
    int width_bytes = width_pixels * 3;
    int padding_bytes = 0;
    padding_bytes = (4 - width_bytes % 4) % 4;
    width_bytes = width_bytes + padding_bytes;

    // Pixel array size in bytes, including padding
    int array_bytes = width_bytes * height_pixels;

    // Open a file stream for writing to a binary file
    fstream stream;
    stream.open(filename, ios::out | ios::binary);

    // If there was a problem opening the file, return false
    if (!stream.is_open())
    {
        return false;
    }

    // Create the BMP and DIB Headers
    const int BMP_HEADER_SIZE = 14;
    const int DIB_HEADER_SIZE = 40;
    unsigned char bmp_header[BMP_HEADER_SIZE] = {0};
    unsigned char dib_header[DIB_HEADER_SIZE] = {0};

    // BMP Header
    set_bytes(bmp_header,  0, 1, 'B');              // ID field
    set_bytes(bmp_header,  1, 1, 'M');              // ID field
    set_bytes(bmp_header,  2, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE+array_bytes); // Size of BMP file
    set_bytes(bmp_header,  6, 2, 0);                // Reserved
    set_bytes(bmp_header,  8, 2, 0);                // Reserved
    set_bytes(bmp_header, 10, 4, BMP_HEADER_SIZE+DIB_HEADER_SIZE); // Pixel array offset

    // DIB Header
    set_bytes(dib_header,  0, 4, DIB_HEADER_SIZE);  // DIB header size
    set_bytes(dib_header,  4, 4, width_pixels);     // Width of bitmap in pixels
    set_bytes(dib_header,  8, 4, height_pixels);    // Height of bitmap in pixels
    set_bytes(dib_header, 12, 2, 1);                // Number of color planes
    set_bytes(dib_header, 14, 2, 24);               // Number of bits per pixel
    set_bytes(dib_header, 16, 4, 0);                // Compression method (0=BI_RGB)
    set_bytes(dib_header, 20, 4, array_bytes);      // Size of raw bitmap data (including padding)                     
    set_bytes(dib_header, 24, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 28, 4, 2835);             // Print resolution of image (2835 pixels/meter)
    set_bytes(dib_header, 32, 4, 0);                // Number of colors in palette
    set_bytes(dib_header, 36, 4, 0);                // Number of important colors

    // Write the BMP and DIB Headers to the file
    stream.write((char*)bmp_header, sizeof(bmp_header));
    stream.write((char*)dib_header, sizeof(dib_header));

    // Initialize pixel and padding
    unsigned char pixel[3] = {0};
    unsigned char padding[3] = {0};

    // Pixel Array (Left to right, bottom to top, with padding)
    for (int h = height_pixels - 1; h >= 0; h--)
    {
        for (int w = 0; w < width_pixels; w++)
        {
            // Write the pixel (Blue, Green, Red)
            pixel[0] = image[h][w].blue;
            pixel[1] = image[h][w].green;
            pixel[2] = image[h][w].red;
            stream.write((char*)pixel, 3);
        }
        // Write the padding bytes
        stream.write((char *)padding, padding_bytes);
    }

    // Close the stream and return true
    stream.close();
    return true;
}

//***************************************************************************************************//
//                                DO NOT MODIFY THE SECTION ABOVE                                    //
//***************************************************************************************************//


//
// YOUR FUNCTION DEFINITIONS HERE



 // process 1 - update : working 12/12/23
    vector<vector<Pixel>> process_1(const vector<vector<Pixel>>& image){
    // to get the height and width of the pixels
    int num_rows = image.size(); // get the height of the 2D vector called image
    int num_columns = image[0].size(); // Gets the number of columns (i.e. width) in a 2D vector named image
    int height = num_columns; // alt var name for convenience
    int width = num_columns; // alt var name
    
    
    // create a new image and prepopulate it
   vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));     
    // write a nested for loop that loops thru every pixel value 
    for (int row = 0;row < num_rows;row++){
        for(int col = 0;col < num_columns;col++){
             // perform process 1 on each frame RGB
           
            // //piazza distance and scaling code
            // int distance = sqrt(pow(row - num_rows/2, 2) + pow(col - num_columns/2, 2));
            // int scaling_factor = (num_rows - distance)/ num_rows;
            
            int distance = sqrt(pow(row - num_rows/2, 2) + pow(col - num_columns/2, 2));

            // Calculate the scaling factor based on the distance
            double scaling_factor = double (num_rows - distance) / num_rows;
            
        
            new_image[row][col].red = image[row][col].red * scaling_factor;
            new_image[row][col].green = image[row][col].green * scaling_factor;
            new_image[row][col].blue = image[row][col].blue * scaling_factor;
        } 
    }
        return new_image;
    } 
 // end process 1

// process 2 - works correctly 12/12/23
 vector<vector<Pixel>> process_2(const vector<vector<Pixel>>& image , double scaling_factor){
    // to get the height and width of the pixels
    int num_rows = image.size(); // get the height of the 2D vector called image
    int num_columns = image[0].size(); // Gets the number of columns (i.e. width) in a 2D vector named image
    int height = num_columns; // alt var name for convenience
    int width = num_columns; // alt var name
    
    
    // create a new image and prepopulate it
   vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));     
    // write a nested for loop that loops thru every pixel value 
    for (int row = 0;row < num_rows;row++){
        for(int col = 0;col < num_columns;col++){
            
           
            // get the R G B vals
            int red_val = image[row][col].red;
            int green_val = image[row][col].green;
            int blue_val = image[row][col].blue;

            // avg the values
            double avg = (red_val + green_val + blue_val) / 3;
            
            // if the cell is light make it lighter
            if (avg >= 170) {
                new_image[row][col].red = (255 - (255 - red_val)*scaling_factor);
                new_image[row][col].green = (255 - (255 - green_val)*scaling_factor);
                new_image[row][col].blue = (255 - (255 - blue_val)*scaling_factor);
            }
            else if(avg< 90) {
                new_image[row][col].red = red_val *scaling_factor;
                new_image[row][col].green = green_val*scaling_factor;
                new_image[row][col].blue =  blue_val*scaling_factor;
            }
            
        else {
            new_image[row][col].red = image[row][col].red;
            new_image[row][col].green = image[row][col].green;
            new_image[row][col].blue = image[row][col].blue;
           }
        } 
    }
        return new_image;
    } 

// end process 2

// process 3 grayscale working 12/12/23
vector<vector<Pixel>> process_3(const vector<vector<Pixel>>& image){
    // to get the height and width of the pixels
    int num_rows = image.size(); // get the height of the 2D vector called image
    int num_columns = image[0].size(); // Gets the number of columns (i.e. width) in a 2D vector named image
    int height = num_columns; // alt var name for convenience
    int width = num_columns; // alt var name
    
    
    // create a new image and prepopulate it
   vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));     
    // write a nested for loop that loops thru every pixel value 
    for (int row = 0;row < num_rows;row++){
        for(int col = 0;col < num_columns;col++){
            
           
            // get the R G B vals
            int red_val = image[row][col].red;
            int green_val = image[row][col].green;
            int blue_val = image[row][col].blue;

            // avg the values
             int gray_val = (red_val + green_val + blue_val) / 3;
            
            // set all vals to gray val
            new_image[row][col].red = gray_val;
            new_image[row][col].green = gray_val;
            new_image[row][col].blue = gray_val;
            
         
      } 
    }
        return new_image;
    } 



// end process 3

// process 4 rotate -working:12/12/23
 vector<vector<Pixel>> process_4(const vector<vector<Pixel>>& image){
    // to get the height and width of the pixels
    int num_rows = image.size(); // get the height of the 2D vector called image
    int num_columns = image[0].size(); // Gets the number of columns (i.e. width) in a 2D vector named image
    int height = num_columns; // alt var name for convenience
    int width = num_columns; // alt var name
    
    
    // create a new image and prepopulate it
   vector<vector<Pixel>> new_image(num_columns, vector<Pixel> (num_rows));     
    // write a nested for loop that loops thru every pixel value 
    for (int row = 0;row < num_rows;row++){
        for(int col = 0;col < num_columns;col++){
            // rotate pix
            int new_row = col; // Rotate the column to become the new row
            int new_column = num_rows - 1 - row;

            // Assign the pixel value from the original image to the rotated position
            new_image[new_row][new_column] = image[row][col];
        } 
     }
        return new_image;
    } 


// end process 4 


// process 5 rotate by int UPDATE: works, but says returns void, due to 
 vector<vector<Pixel>> process_5(const vector<vector<Pixel>>& image, int number){
    // to get the height and width of the pixels
    int num_rows = image.size(); // get the height of the 2D vector called image
    int num_columns = image[0].size(); // Gets the number of columns (i.e. width) in a 2D vector named image
    int height = num_columns; // alt var name for convenience
    int width = num_columns; // alt var name
    
     // calculate angle
     int angle = number*90;
     if (angle % 90 != 0){
        cout<< "angle must be a multiple of 90 degrees." << endl;
     return image;}
    else if (angle%360 == 0){
        return image;
        }
    else if (angle%360 == 90){
        return process_4(image);
        }
    else if (angle%360 == 180){
        return process_4(process_4(image));
        }
    else {
        return process_4(process_4(process_4(image)));
        }   
    } 


// end process 5 



// process 6 enlarge image - tested and works 12/12/23
 vector<vector<Pixel>> process_6(const vector<vector<Pixel>>& image, int x_scale, int y_scale){
    // to get the height and width of the pixels
    int original_height = image.size();
    int original_width = image[0].size();
     
    // create new width and height
     int newheight =  original_height * y_scale;
     int newwidth  =  original_width  * x_scale;
    
    
    // create a new image and prepopulate it with the new width and height
   vector<vector<Pixel>> new_image(newheight, vector<Pixel> (newwidth));     
    // write a nested for loop that loops thru every pixel value 
    for (int row = 0;row < newheight;row++){
        for(int col = 0;col < newwidth ;col++){
          // add pixels to each value
            int originalRow = int (row / y_scale);
            int originalCol = int (col / x_scale);

            // Assign the pixel value from the original image to the rotated position
            new_image[row][col] = image[originalRow][originalCol];
        } 
     }
        return new_image;
    } 




// end process 6



// process 7 B & W - working 12/12/23
vector<vector<Pixel>> process_7(const vector<vector<Pixel>>& image){
    // to get the height and width of the pixels
    int num_rows = image.size(); // get the height of the 2D vector called image
    int num_columns = image[0].size(); // Gets the number of columns (i.e. width) in a 2D vector named image
    int height = num_columns; // alt var name
    int width = num_columns; // alt var name
    
    
    // create a new image and prepopulate it
   vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));     
    // write a nested for loop that loops thru every pixel value 
    for (int row = 0;row < num_rows;row++){
        for(int col = 0;col < num_columns;col++){
            
           
            // get the R G B vals
            int red_val = image[row][col].red;
            int green_val = image[row][col].green;
            int blue_val = image[row][col].blue;

            // avg the values
             int gray_val = (red_val + green_val + blue_val) / 3;
            
            // if gray val is higher than 255/2 
            if ( gray_val>= 255/2) {
                new_image[row][col].red = 255;
                new_image[row][col].green = 255;
                new_image[row][col].blue = 255;
                }   
            else {
                new_image[row][col].red = 0;
                new_image[row][col].green = 0;
                new_image[row][col].blue = 0;
               }
            
         
       } 
    }
        return new_image;
    } 



// end process 7 

// process 8 lighten by a scaling factor - tested: working 12/12/23
vector<vector<Pixel>> process_8(const vector<vector<Pixel>>& image, double scaling_factor){
    // to get the height and width of the pixels
    int num_rows = image.size(); // get the height of the 2D vector called image
    int num_columns = image[0].size(); // Gets the number of columns (i.e. width) in a 2D vector named image
    int height = num_columns; // alt var name
    int width = num_columns; // alt var name
    
    
    // create a new image and prepopulate it
   vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));     
    // write a nested for loop that loops thru every pixel value 
    for (int row = 0;row < num_rows;row++){
        for(int col = 0;col < num_columns;col++){
            
           
            // get the R G B vals
            int red_val = image[row][col].red;
            int green_val = image[row][col].green;
            int blue_val = image[row][col].blue;
            
            // set new vals 
            new_image[row][col].red = (255 - (255 - red_val)*scaling_factor);
            new_image[row][col].green = (255 - (255 - green_val)*scaling_factor);
            new_image[row][col].blue = (255 - (255 - blue_val)*scaling_factor);
         } 
      }
        return new_image;
    } 



// end process 8 

// start process 9 darken by a scaling factor tested:working - 12/12/23
vector<vector<Pixel>> process_9(const vector<vector<Pixel>>& image, double scaling_factor){
    // to get the height and width of the pixels
    int num_rows = image.size(); // get the height of the 2D vector called image
    int num_columns = image[0].size(); // Gets the number of columns (i.e. width) in a 2D vector named image
    int height = num_columns; // alt var name
    int width = num_columns; // alt var name
    
    
    // create a new image and prepopulate it
   vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));     
    // write a nested for loop that loops thru every pixel value 
    for (int row = 0;row < num_rows;row++){
        for(int col = 0;col < num_columns;col++){
            
           
            // get the R G B vals
            int red_val = image[row][col].red;
            int green_val = image[row][col].green;
            int blue_val = image[row][col].blue;
            
            //  set new vals
            new_image[row][col].red =red_val * scaling_factor;
            new_image[row][col].green = green_val *scaling_factor ;
            new_image[row][col].blue = blue_val * scaling_factor ;
         } 
      }
        return new_image;
    } 

// end process 9 


// start process 10  W B R G B - working 12/12/23
 vector<vector<Pixel>> process_10(const vector<vector<Pixel>>& image){
    // to get the height and width of the pixels
    int num_rows = image.size(); // get the height of the 2D vector called image
    int num_columns = image[0].size(); // Gets the number of columns (i.e. width) in a 2D vector named image
    int height = num_columns; // alt var name for convenience
    int width = num_columns; // alt var name
    
    
    // create a new image and prepopulate it
   vector<vector<Pixel>> new_image(num_rows, vector<Pixel> (num_columns));     
    // write a nested for loop that loops thru every pixel value 
    for (int row = 0;row < num_rows;row++){
        for(int col = 0;col < num_columns;col++){
            
           
            // get the R G B vals
            int red_val = image[row][col].red;
            int green_val = image[row][col].green;
            int blue_val = image[row][col].blue;

            // max color
            int max_color = max({red_val, green_val, blue_val});
            
            // adjust colors
            if (red_val + green_val + blue_val >= 550) {
                new_image[row][col].red = 255;
                new_image[row][col].green = 255;
                new_image[row][col].blue = 255;
            }
            else if(red_val + green_val + blue_val <= 150) {
                new_image[row][col].red = 0;
                new_image[row][col].green = 0;
                new_image[row][col].blue =  0;
            }
            else if(max_color == red_val) {
                new_image[row][col].red = 255;
                new_image[row][col].green = 0;
                new_image[row][col].blue =  0;
            }
            else if(max_color == green_val) {
                new_image[row][col].red = 0;
                new_image[row][col].green = 255;
                new_image[row][col].blue =  0;
            }
        else {
            new_image[row][col].red = 0;
            new_image[row][col].green = 0;
            new_image[row][col].blue =  255;
           }
        } 
    }
        return new_image;
    } 

// end process 10


// perform image processing function 
vector<vector<Pixel>> perform_image_processing(const vector<vector<Pixel>>& image, int selection) {
    vector<vector<Pixel>> new_image;
    
        // proccess 1
         if (selection == 1) {
        cout << "Vignette selected"<< endl; 
        new_image = process_1(image); 
            // process 2
    } else if (selection == 2) {
        cout << "Enter scaling factor"<< endl;
        double scale;
        cin >> scale;
        new_image = process_2(image, scale);
    } else if (selection == 3) {
        new_image = process_3(image);
    } else if (selection == 4) {
        new_image = process_4(image);
    } else if (selection == 5) {
        cout << "Enter a mutiple of 90 degrees"<< endl;
        int multiple;
        cin >> multiple;
        new_image = process_5(image,multiple);
    } else if (selection == 6) {
        cout<< "Enter an x value to expand the width " << endl;
        int x_factor;
        int y_factor;
        cin >> x_factor;
        cout << "Enter an y value to expand the height" << endl;
        cin >> y_factor;  
        new_image = process_6(image,x_factor,y_factor);
    } else if (selection == 7) {
        new_image = process_7(image);
    } else if (selection == 8) {
        cout << "Enter a factor to lighten the image by"<< endl;
        double lightening;
        cin >> lightening;
        new_image = process_8(image, lightening);
    } else if (selection == 9) {
        cout << "Enter a factor to darken the image by"<< endl;
        double darkening;
        cin >> darkening;
        new_image = process_9(image, darkening);
    } else if (selection == 10) {
        new_image = process_10(image);
    } else {
        cout<<"invalid input"<<endl;
    }

    

    return new_image;
}



int main()
{   //UI
    cout << "CSPB 1300 Image Processing Application" << endl;
    cout << "Hello" << endl;
    // prompt the user for filenaem, automatically add the tag later, prevents filename errors
    cout << "Enter input filename (just the name, no need for .bmp tag):" << endl;
    string filename;
    cin >> filename;
    // takes in images
    vector<vector<Pixel>> image = read_image(filename + ".bmp");
    // if image cannot be found
   if (image.empty()) {
    cout << "File could not be found. Please restart the program." << endl;
    return 0;
}
   
    // allow for modified image 
    vector<vector<Pixel>> modified_image = image;
    
    // print the menu 
    cout << "IMAGE PROCESSING MENU" << endl;
    cout << " 0) Change image (current: " + filename + ")" << endl;
    cout << " 1) Vignette" << endl;
    cout << " 2) Clarendon" << endl;
    cout << " 3) Grayscale" << endl;
    cout << " 4) Rotate 90 degrees" << endl;
    cout << " 5) Rotate multiple 90 degrees" << endl;
    cout << " 6) Enlarge" << endl;
    cout << " 7) High contrast" << endl;
    cout << " 8) Lighten" << endl;
    cout << " 9) Darken" << endl;
    cout << " 10) Black, white, red, green, blue" << endl;
    
    // program is done flag
    bool done = false;
    // while program is ongoing
    while (!done) {
        cout << "Enter menu selection (Q to quit):" << endl;
        int selection;
        cin >> selection;

        if (cin.fail()) {
            done = true;
        } else {
            // Loop until a valid selection is entered
            while (selection < 0 || selection > 10) {
                cout << "Invalid Input. Enter a number between 0 and 10:" << endl;
                cin.clear(); // Clear error flags
                cin >> selection;

                // If cin fails, set done to true to exit the loop
                if (cin.fail()) {
                    done = true;
                    break;
                }
            }
            // enter this loop once a valid input is entered
            if (!done) {
                // accounts for 0 functionality, switches images
                if (selection == 0) {
                    cout << "Please enter the filename you want to switch to:" << endl;
                    cin >> filename;
                    image = read_image(filename + ".bmp");
                    cout << "What process do you want to run?" << endl;
                    cin >> selection;
                    // chooses a process and applies it 
                    if (selection >= 1 && selection <= 10) {
                        modified_image = perform_image_processing(image, selection);
                    }
                } else if (selection >= 1 && selection <= 10) {
                    modified_image = perform_image_processing(modified_image, selection);
                } else {
                    cout << "Invalid Input" << endl;
                }
                // image save sucess flag
                bool success = false;
                string new_filename;
                // prompt user for filename to save as
                cout << "Enter the new file name for the processed image (no need for the .bmp tag):" << endl;
                cin >> new_filename;
                // prevent overwriting files
                if (new_filename == filename) {
                    cout << "ERROR: Do not use the name of the original image, it will overwrite it if you do" << endl;
                    cout << "Enter a different filename:" << endl;
                } else {
                    success = write_image(new_filename + ".bmp", modified_image);
                }
                // if image save was sucessful
                if (success) {
                    cout << "Image processing and writing to file successful!" << endl;
                    cout << "The File was saved as " + new_filename + ".bmp" << endl;
                } else {
                    cout << "Error: Failed to write the processed image to a file." << endl;
                }
            }
        }
    }

    cout << "Thank you for using" << endl;
    return 0;
}



// CHECKLIST 
    // implement 0 functionality to the menu  (ability to change the image) - done 12/13/2023
    // handle undesired inputs
    
    
    
    
